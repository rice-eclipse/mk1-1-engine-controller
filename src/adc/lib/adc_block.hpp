//
// Created by rjcunningham on 9/20/17.
//

#ifndef SOFTWARE_ADC_BLOCK_HPP
#define SOFTWARE_ADC_BLOCK_HPP

#include <stdint.h>
#include <bcm2835.h>

extern RPiGPIOPin SPI_CS_0;
extern RPiGPIOPin SPI_CS_1;

/**
 * A simple structure that holds information on which ADC to read from.
 * Used to pair a sensor with an ADC and a channel
 */
struct adc_info_t {
    RPiGPIOPin pin;         //ADC to use
    int pad             :4; //Pad an extra four bits.
    bool single_channel :1; //The :1 means use one bit for this.
    uint8_t channel     :3; //The three bits to pick channel.

    adc_info_t() = default;

    adc_info_t(RPiGPIOPin pin, bool single_channel, uint8_t chan)
    : pin(pin)
    , pad(0)
    , single_channel(single_channel)
    , channel(chan)
    {};
};

/**
 * A class that abstracts away all the ADCs on the system.
 */
class adc_block {

    private:
        /**
         * An array of the physical pin that corresponds to the chip select pin.
         */
        RPiGPIOPin *adc_cs_pin;
        /**
         * The number of ADCs that this block controls.
         */
        uint8_t num_adcs;

    public :
        /**
         * Creates this with a set number of adcs to use.
         * @param num_adcs The number of adcs to use.
         */
        explicit adc_block(uint8_t num_adcs);

        /**
         * The destructor for the adc_block. Frees memory used to store pin assignments.
         */
        ~adc_block();

        /**
         * Read a single item from the specified registered item given by idx.
         * @param idx The channel to read from.
         * @return The result of reading that channel on that adc.
         */
        uint16_t read_item(adc_info_t idx);

        /**
         * Reads a single item from the specified adc with parameters given here.
         * Does no error checking, however 0 is likely an error. It is not possible for the hardware to
         * discern 0 from no connection, but 0 usually corresponds to no connection as the hardware struggles to read
         * voltages very near ground anyway.
         *
         * @param adc_num Which ADC to select and read from.
         * @param single_channel
         * @param channel
         * @return The 12 bit value read from the SPI communication with the ADC chip.
         */
        uint16_t read_item(uint8_t adc_num, bool single_channel, uint8_t channel);

        /**
         * Registers a pin with this adc_block.
         * @param adc_num The adc number to associate with the pin.
         * @param pin_num The pin to use as chip select for this adc.
         */
        void register_pin(uint8_t adc_num, RPiGPIOPin pin_num);
};

#endif //SOFTWARE_ADC_BLOCK_HPP
