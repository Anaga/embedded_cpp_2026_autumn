/*
 * app.c - lesson 16 demo: STM32F411 Black Pill, HAL in polling mode.
 *
 * - LED (PC13, active low) blinks fast while USB is not ready, then slowly
 * - Log output goes over USB CDC (virtual COM port, any baud rate)
 * - I2C1 on PB6 (SCL) / PB7 (SDA): MPU-6050 on a GY-521 and a BH1750
 * - KEY (PA0) prints the banner again: I2C scan, sensor IDs
 *
 * Requires the CubeMX settings listed in demo/README.md, in particular the
 * pin labels LED and KEY (they create LED_Pin, LED_GPIO_Port, KEY_Pin,
 * KEY_GPIO_Port in main.h).
 */
#include "app.h"

#include "main.h"         /* HAL and the pin labels from CubeMX */
#include "usb_device.h"
#include "usbd_cdc_if.h"  /* CDC_Transmit_FS() */

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* Handles are defined by CubeMX in main.c and usb_device.c */
extern I2C_HandleTypeDef hi2c1;
extern USBD_HandleTypeDef hUsbDeviceFS;

/* ---------------------------------------------------------------- timing */

static const uint32_t SAMPLE_PERIOD_MS   = 500U;
static const uint32_t WAIT_BLINK_MS      = 100U;
static const uint32_t TERMINAL_GRACE_MS  = 2000U; /* time to open a terminal */
static const uint32_t DEBOUNCE_MS        = 30U;
static const uint32_t I2C_TIMEOUT_MS     = 10U;
static const uint32_t CDC_TIMEOUT_MS     = 20U;

/* ------------------------------------------------------------ I2C devices */

/* HAL takes the 7-bit address shifted left by one (the R/W bit position). */
static const uint16_t MPU_ADDR = (uint16_t)(0x68U << 1); /* GY-521, AD0 low */
static const uint16_t BH_ADDR  = (uint16_t)(0x23U << 1); /* BH1750, ADDR low */

/* MPU-6050: register based */
static const uint16_t MPU_REG_ACCEL_XOUT_H = 0x3BU;
static const uint16_t MPU_REG_PWR_MGMT_1   = 0x6BU;
static const uint16_t MPU_REG_WHO_AM_I     = 0x75U;
static const uint8_t  MPU_WAKE             = 0x00U; /* clear SLEEP bit */
static const int32_t  MPU_LSB_PER_G        = 16384;  /* +-2 g, the default */
enum { MPU_BURST_LEN = 14 };  /* accel XYZ, temp, gyro XYZ: 7 x int16 */

/* BH1750: command based, no registers */
static const uint8_t BH_CMD_POWER_ON  = 0x01U;
static const uint8_t BH_CMD_CONT_HRES = 0x10U; /* 1 lx resolution, ~120 ms */

static const uint8_t I2C_FIRST_ADDR = 0x08U;
static const uint8_t I2C_LAST_ADDR  = 0x77U;

enum { LOG_BUF_SIZE = 160 };

/* ------------------------------------------------------------------ state */

typedef enum {
    APP_WAIT_USB,  /* host has not configured the device yet */
    APP_GRACE,     /* configured, give the user time to open a terminal */
    APP_RUN
} AppState;

static AppState state = APP_WAIT_USB;
static uint32_t state_since_ms = 0U;
static uint32_t last_blink_ms = 0U;
static uint32_t last_sample_ms = 0U;

static bool    mpu_ok = false;
static uint8_t mpu_who = 0U;
static bool    bh_ok = false;

/* ----------------------------------------------------------------- USB log */

static bool usb_configured(void)
{
    return hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED;
}

/*
 * Wait until the previous USB transfer is finished. CDC_Transmit_FS() only
 * starts a transfer; the USB peripheral reads our buffer later. Writing into
 * the buffer while a transfer runs corrupts the output.
 */
static bool cdc_wait_idle(uint32_t timeout_ms)
{
    if (!usb_configured()) {
        return false;
    }
    const USBD_CDC_HandleTypeDef *hcdc =
        (const USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;
    if (hcdc == NULL) {
        return false;
    }
    const uint32_t start = HAL_GetTick();
    while (hcdc->TxState != 0U) {
        if ((HAL_GetTick() - start) >= timeout_ms) {
            return false;  /* nobody reads the port: drop this line */
        }
    }
    return true;
}

/* printf-like output over USB. No %f: newlib-nano printf has no floats. */
static void log_printf(const char *fmt, ...)
{
    static char buf[LOG_BUF_SIZE];  /* static: USB reads it after we return */

    if (!cdc_wait_idle(CDC_TIMEOUT_MS)) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof buf, fmt, args);
    va_end(args);

    if (len <= 0) {
        return;
    }
    if ((size_t)len >= sizeof buf) {
        len = (int)(sizeof buf - 1U);  /* truncated */
    }
    (void)CDC_Transmit_FS((uint8_t *)buf, (uint16_t)len);
}

/* v is a value in tenths: 237 -> "23.7", -5 -> "-0.5" */
static const char *tenths(int32_t v, char *out, size_t n)
{
    const char *sign = (v < 0) ? "-" : "";
    const uint32_t a = (v < 0) ? (uint32_t)(-v) : (uint32_t)v;
    (void)snprintf(out, n, "%s%lu.%lu", sign,
                   (unsigned long)(a / 10U), (unsigned long)(a % 10U));
    return out;
}

/* --------------------------------------------------------------- MPU-6050 */

static int16_t be16(const uint8_t *p)
{
    return (int16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static bool mpu_init(void)
{
    if (HAL_I2C_Mem_Read(&hi2c1, MPU_ADDR, MPU_REG_WHO_AM_I,
                         I2C_MEMADD_SIZE_8BIT, &mpu_who, 1U,
                         I2C_TIMEOUT_MS) != HAL_OK) {
        return false;
    }
    uint8_t wake = MPU_WAKE;  /* after power-up the chip sleeps */
    return HAL_I2C_Mem_Write(&hi2c1, MPU_ADDR, MPU_REG_PWR_MGMT_1,
                             I2C_MEMADD_SIZE_8BIT, &wake, 1U,
                             I2C_TIMEOUT_MS) == HAL_OK;
}

typedef struct {
    int32_t ax_mg;
    int32_t ay_mg;
    int32_t az_mg;
    int32_t temp_c10;  /* tenths of a degree */
} MpuSample;

static bool mpu_read(MpuSample *s)
{
    uint8_t b[MPU_BURST_LEN];
    /* One transaction: write register 0x3B, repeated START, read 14 bytes.
       The chip auto-increments the register address. */
    if (HAL_I2C_Mem_Read(&hi2c1, MPU_ADDR, MPU_REG_ACCEL_XOUT_H,
                         I2C_MEMADD_SIZE_8BIT, b, MPU_BURST_LEN,
                         I2C_TIMEOUT_MS) != HAL_OK) {
        return false;
    }
    s->ax_mg = (int32_t)be16(&b[0]) * 1000 / MPU_LSB_PER_G;
    s->ay_mg = (int32_t)be16(&b[2]) * 1000 / MPU_LSB_PER_G;
    s->az_mg = (int32_t)be16(&b[4]) * 1000 / MPU_LSB_PER_G;
    /* Datasheet: T = raw / 340 + 36.53 C  ->  tenths: raw * 10 / 340 + 365 */
    s->temp_c10 = (int32_t)be16(&b[6]) * 10 / 340 + 365;
    return true;
}

/* ----------------------------------------------------------------- BH1750 */

static bool bh_send(uint8_t cmd)
{
    return HAL_I2C_Master_Transmit(&hi2c1, BH_ADDR, &cmd, 1U,
                                   I2C_TIMEOUT_MS) == HAL_OK;
}

static bool bh_init(void)
{
    return bh_send(BH_CMD_POWER_ON) && bh_send(BH_CMD_CONT_HRES);
}

static bool bh_read_lux10(uint32_t *lux10)
{
    uint8_t b[2];
    if (HAL_I2C_Master_Receive(&hi2c1, BH_ADDR, b, sizeof b,
                               I2C_TIMEOUT_MS) != HAL_OK) {
        return false;
    }
    const uint32_t raw = ((uint32_t)b[0] << 8) | b[1];
    /* Datasheet: lux = raw / 1.2  ->  tenths: raw * 10 / 1.2 = raw * 25 / 3 */
    *lux10 = raw * 25U / 3U;
    return true;
}

/* ------------------------------------------------------------------- KEY */

/* Returns true once per press. KEY is active low (button to GND). */
static bool key_pressed(uint32_t now)
{
    static GPIO_PinState last_raw = GPIO_PIN_SET;
    static GPIO_PinState stable = GPIO_PIN_SET;
    static uint32_t changed_at = 0U;

    const GPIO_PinState raw = HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin);
    if (raw != last_raw) {
        last_raw = raw;
        changed_at = now;
    }
    if (((now - changed_at) >= DEBOUNCE_MS) && (raw != stable)) {
        stable = raw;
        return stable == GPIO_PIN_RESET;
    }
    return false;
}

/* ---------------------------------------------------------------- banner */

static void i2c_scan(void)
{
    uint32_t found = 0U;
    log_printf("I2C1 scan (PB6 SCL, PB7 SDA):\r\n");
    for (uint8_t a = I2C_FIRST_ADDR; a <= I2C_LAST_ADDR; a++) {
        if (HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(a << 1), 1U,
                                  I2C_TIMEOUT_MS) == HAL_OK) {
            log_printf("  0x%02X\r\n", (unsigned)a);
            found++;
        }
    }
    log_printf("  %lu device(s)\r\n", (unsigned long)found);
}

static void sensors_init(void)
{
    mpu_ok = mpu_init();
    bh_ok = bh_init();
}

static void print_banner(void)
{
    log_printf("\r\nLesson 16 - STM32F411 Black Pill, HAL, USB CDC\r\n");
    log_printf("SYSCLK %lu Hz\r\n", (unsigned long)HAL_RCC_GetSysClockFreq());
    i2c_scan();
    sensors_init();
    if (mpu_ok) {
        log_printf("MPU-6050 WHO_AM_I = 0x%02X (0x68 on a genuine part)\r\n",
                   (unsigned)mpu_who);
    } else {
        log_printf("MPU-6050 not found at 0x68\r\n");
    }
    log_printf("BH1750 %s\r\n", bh_ok ? "ok" : "not found at 0x23");
    log_printf("Press KEY to show this again.\r\n\r\n");
}

/* ------------------------------------------------------------------ sample */

static void sample_and_print(void)
{
    char t1[16];
    char t2[16];
    MpuSample m;
    uint32_t lux10 = 0U;

    const bool m_ok = mpu_read(&m);
    const bool l_ok = bh_read_lux10(&lux10);

    log_printf("%8lu ms  ", (unsigned long)HAL_GetTick());
    if (l_ok) {
        log_printf("lux %s  ", tenths((int32_t)lux10, t1, sizeof t1));
    } else {
        log_printf("lux n/a  ");
    }
    if (m_ok) {
        log_printf("acc %ld %ld %ld mg  temp %s C\r\n",
                   (long)m.ax_mg, (long)m.ay_mg, (long)m.az_mg,
                   tenths(m.temp_c10, t2, sizeof t2));
    } else {
        log_printf("acc n/a\r\n");
    }
}

/* -------------------------------------------------------------- interface */

void app_init(void)
{
    /* LED is active low: SET = off */
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    sensors_init();  /* silent: USB is not up yet, nothing could be printed */
    state = APP_WAIT_USB;
    state_since_ms = HAL_GetTick();
}

void app_loop(void)
{
    const uint32_t now = HAL_GetTick();

    switch (state) {
    case APP_WAIT_USB:
        if ((now - last_blink_ms) >= WAIT_BLINK_MS) {
            last_blink_ms = now;
            HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        }
        if (usb_configured()) {
            state = APP_GRACE;
            state_since_ms = now;
        }
        break;

    case APP_GRACE:
        if ((now - state_since_ms) >= TERMINAL_GRACE_MS) {
            print_banner();
            /* The banner can take a second (I2C timeouts with no sensor).
               Restart the sample timer so we do not catch up afterwards. */
            last_sample_ms = HAL_GetTick();
            state = APP_RUN;
        }
        break;

    case APP_RUN:
        if (key_pressed(now)) {
            print_banner();
            last_sample_ms = HAL_GetTick();
        }
        if ((now - last_sample_ms) >= SAMPLE_PERIOD_MS) {
            last_sample_ms += SAMPLE_PERIOD_MS;
            HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            sample_and_print();
        }
        if (!usb_configured()) {
            state = APP_WAIT_USB;  /* cable or host went away */
        }
        break;

    default:
        state = APP_WAIT_USB;
        break;
    }
}
