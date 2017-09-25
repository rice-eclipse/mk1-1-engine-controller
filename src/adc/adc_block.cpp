//
// Created by rjcunningham on 9/20/17.
//

#include "adc_block.hpp"
#include <bcm2835.h>
#include <stdio.h>
#include <byteswap.h>

//Creates an internal array of adc_infos
adc_block::adc_block(uint8_t num_adcs) {
    //these adc_infos need to be initialized
    //Add a default initialiation header file?
    struct adc_info adcs[num_adcs];
    for (int i = 0; i < num_adcs; i++) {
        adcs[i] = {0x00, 0, 1, 0x00};
    }
}

adc_block::~adc_block() {
    //this may or may not be what this method is supposed to do

    printf("deleting adc_block with\n");
    for (int i = 0; i < num_adcs; i++) {
        printf("pin %d is now free\n", adcs[i].pin);
    }
    delete [] adcs;
}

//Read a SINGLE item from the specified registered item given by idx.
//Not sure if this is the best way - there seems to be a lot of initialization
//if we're going to call this for each item we read.
uint16_t adc_block::read_item(adc_info idx) {
    //adapted from spi.cpp example from bcm2835 library
    char channel = (char)idx.single_channel << 3 | (char)idx.channel;
    char writeb[3] = {channel, 0, 0}; //Write to pick CH in idx
    volatile char readb[3] = {0,0,0};
    uint16_t out_value = 0;

    if (!bcm2835_init()) {
        printf("bcm2835_init failed. Are you running as root??\n");
        return 1;
    }
    if (!bcm2835_spi_begin()) {
        printf("bcm2835_spi_begin failed. Are you running as root??\n");
        return 1;
    }
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // TODO
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE3);                   // The MCP... uses this.
    //bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256); // Just under 1MHz
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_1024); // Seems like 1Mhz is too fast to charge internal cap.
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default
    //while (1) {
    // Send 3 bytes to the slave and simultaneously read bytes back from the slave
    // If you tie MISO to MOSI, you should read back what was sent
    bcm2835_spi_transfernb(writeb, readb, 3);
    //Copy the middle two bytes into out_value:
    readb[2] = ((readb[2] >> 2) | readb[1] << 6) & 0xFF; //Since we left shifted writeb earlier
    readb[1] = (readb[1] >> 2) & 0x0F;

    out_value = *((uint16_t *)(readb + 1)); //read the last two bytes in readb
    // Next swap endianness because it's backwards for us.
    out_value = __bswap_16(out_value);
    printf("Sent to SPI: 0x%01X. Read back from SPI: 0x%03X.\n", 0x0c, out_value);
    bcm2835_spi_end();
    bcm2835_close();
    return out_value;
    //}
}

//same as the other read_item except the data isn't contained in an adc_info struct
//just use the other one
uint16_t adc_block::read_item(uint8_t adc_num, bool single_channel, uint8_t channel) {
    //TODO
}

//Updates the chip select pin of the adc corresponding to adc_num
void adc_block::register_pin(uint8_t adc_num, uint8_t pin_num) {
    //there is probably a better way to do this
    //maybe adcs[adc_num] = adcs[adc_num].setPin(pin_num)? But structs don't have methods.
    adcs[adc_num] = {pin_num, adcs[adc_num].pad, adcs[adc_num].single_channel, adcs[adc_num].channel};
    //also, is the adcs array defined in the adc_block initializer accessible from this method?
}