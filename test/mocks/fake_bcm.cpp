//
// Created by rjcunningham on 1/21/18.
//
/**
 * Fake function definitions for final_mocked.
 */
#include "bcm2835.h"
void bcm2835_gpio_write(uint8_t pin, uint8_t on) { }

int bcm2835_init() { return 1; }

void bcm2835_pwm_set_mode(uint8_t channel, uint8_t markspace, uint8_t enabled) { }
