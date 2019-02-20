//
// Created by rjcunningham on 12/1/17.
//

#ifndef SOFTWARE_PINS_HPP
#define SOFTWARE_PINS_HPP

#include <bcm2835.h>
#define ADC_0_CS RPI_V2_GPIO_P1_26
#define ADC_1_CS RPI_V2_GPIO_P1_24
#define ADC_2_CS RPI_V2_GPIO_P1_22

#define MAIN_VALVE RPI_V2_GPIO_P1_11
#define WATER_VALVE RPI_V2_GPIO_P1_13
#define HEATING_TAPE RPI_V2_GPIO_P1_18
#define GITVC_VALVE RPI_V2_GPIO_P1_16
#define IGN_START RPI_V2_GPIO_P1_15

#define LC_ADC ADC_2_CS
#define PT_ADC ADC_1_CS
#define TC_ADC ADC_1_CS

// defines for Titan
#define TANK GITVC_VALVE // same as GITVC
#define VENT WATER_VALVE // same as Water

#endif //SOFTWARE_PINS_HPP
