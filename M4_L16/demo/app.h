/*
 * app.h - lesson 16 demo application layer.
 *
 * main.c is owned by CubeMX. Our code lives in app.c and is called from two
 * USER CODE sections in main.c, so "Generate Code" never touches it.
 */
#ifndef APP_H
#define APP_H

void app_init(void);  /* call once, after all MX_..._Init() calls */
void app_loop(void);  /* call from the while (1) loop, never blocks for long */

#endif /* APP_H */
