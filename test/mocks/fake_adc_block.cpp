//
// Created by rjcunningham on 1/21/18.
//

#include <unistd.h>
#include "../../src/adc/lib/adc_block.hpp"
// A simple implementation of an adc_block that abstracts away the raspberry pi.

// A internal counter that increments when the ADC is accessed. Decent way to generate fake data.
static uint16_t count = 0;

adc_block::adc_block(uint8_t num_adcs) {

};
adc_block::~adc_block() {};

uint16_t adc_block::read_item(adc_info idx) {
    usleep(50);
    return count++;
}

uint16_t adc_block::read_item(uint8_t adc_num, bool single_channel, uint8_t channel) {
    this->read_item({});
}

void adc_block::register_pin(uint8_t adc_num, RPiGPIOPin pin_num) {}

