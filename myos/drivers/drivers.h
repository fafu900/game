/* drivers.h */
#ifndef DRIVERS_H
#define DRIVERS_H
#include <stdint.h>
#include <stdbool.h>

void keyboard_init(void);

void timer_init(void);
uint32_t timer_get_ticks(void);
void sleep_ms(uint32_t ms);
char kb_get_last(void);           /* забрать последний символ (для shell) */
bool kb_ctrl_pressed(void);

#endif
