/*
 * Isikukood - gender and age
 *
 * Reads an Estonian personal code (isikukood) from the command line, works
 * out the person's gender and age, then draws them as ASCII art.
 *
 * Isikukood layout, 11 digits: G YY MM DD SSS C
 *   G    - century and gender: 1/2 = male/female, born 1800-1899
 *                               3/4 = male/female, born 1900-1999
 *                               5/6 = male/female, born 2000-2099
 *   YY   - year within that century
 *   MM   - birth month
 *   DD   - birth day
 *   SSS  - serial number
 *   C    - check digit
 *
 * Compile and run:
 *   g++ -std=c++17 isikukood.cpp -o isikukood
 *   ./isikukood
 */

#include <cstdint>
#include <cstdio>

static const uint8_t ISIKUKOOD_LENGTH = 11U;

static const uint8_t GENDER_BIT = 0x80U;  // 1 = female, 0 = male
static const uint8_t AGE_MASK = 0x7FU;    // age fits in the low 7 bits (0-127)

static const uint8_t INVALID_RESULT = 0xFFU;

static const uint8_t CHILD_MAX_AGE = 12U;
static const uint8_t SENIOR_MIN_AGE = 40U;

// Return gender and age packed into one byte (8 bits)
static uint8_t discriminate(const char *isikukood) {
    bool gender = true;
    
    // Subtracting  '0' from first character it to get the actual digit
    const uint8_t gender_digit = isikukood[0] - '0';
    if (gender_digit == 1 || gender_digit == 3 || gender_digit == 5) {
        gender = false;
    } else if (gender_digit == 2 || gender_digit == 4 || gender_digit == 6) {
        gender = true;
    } else {
        return INVALID_RESULT;
    }

    const uint8_t year_digits = (isikukood[1] - '0') * 10 + (isikukood[2] - '0');

    uint8_t age = 0U;
    if (gender_digit == 1 || gender_digit == 2) {
        age = 26 + 200 - year_digits;
    } else if (gender_digit == 3 || gender_digit == 4) {
        age = 26 + 100 - year_digits;
    } else if ((gender_digit == 5 || gender_digit == 6) && (year_digits <= 26)) {
        age = 26 - year_digits;
    } else {
        return INVALID_RESULT;
    }

    const uint8_t value = (gender ? GENDER_BIT : 0U) | (age & AGE_MASK);

    printf("\nGender and age packed into 8 bits: %d ", (value >> 7) & 1U);
    for (int8_t i = 6; i >= 0; i--) {
        printf("%d", (value >> i) & 1U);
    }
    printf("\n\n");

    return value;
}

/*
 * Draw a small ASCII picture representing this gender and age.
 */
static void visualize(bool gender, uint8_t age) {
    const bool is_child = (age <= CHILD_MAX_AGE);
    const bool is_senior = (age >= SENIOR_MIN_AGE);

    const char *hair = gender ? "  ,(   ),   " : "   .---.  ";
    const char *face = is_child ? "   (^_^)    " : is_senior ? "   (-_-)    " : "   (o_o)    ";
    const char *body = "   /|||\\  ";
    const char *waist = gender ? "  /~~~~~\\ " : "   |||||   ";
    const char *feet = "  o     o ";

    printf("%s\n", hair);
    printf("%s\n", face);
    printf("%s\n", body);
    printf("%s\n", waist);
    printf("%s", feet);

    printf("\n");
}

int main() {
    char isikukood[ISIKUKOOD_LENGTH + 1U];

    printf("Isikukood: ");
    scanf("%11s", isikukood);

    const uint8_t result = discriminate(isikukood);
    if (result == INVALID_RESULT) {
        printf("Not a valid isikukood.\n");
        return 1;
    }

    const bool gender = (result & GENDER_BIT) != 0U;
    const uint8_t age = result & AGE_MASK;

    printf("%u y.o. %s\n", (unsigned)age, gender ? "female\n" : "male\n");

    visualize(gender, age);

    return 0;
}
