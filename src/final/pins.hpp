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
#define WATER_VALVE RPI_V2_GPIO_P1_13 // Water
#define VALVE_3 RPI_V2_GPIO_P1_15
#define GITVC_VALVE RPI_V2_GPIO_P1_18 // GITVC
#define IGN_START RPI_V2_GPIO_P1_16

#define LC_ADC ADC_2_CS
#define PT_ADC ADC_1_CS
#define TC_ADC ADC_1_CS

//extern RPiGPIOPin ADC_0_CS;
//extern RPiGPIOPin ADC_1_CS;
//extern RPiGPIOPin ADC_2_CS;
//
//extern RPiGPIOPin MAIN_VALVE;
//extern RPiGPIOPin WATER_VALVE;
//extern RPiGPIOPin VALVE_3;
//extern RPiGPIOPin GITVC_VALVE;
//extern RPiGPIOPin IGN_START;
//
//RPiGPIOPin LC_ADC = ADC_2_CS;
//extern RPiGPIOPin PT_ADC;
//extern RPiGPIOPin TC_ADC;
#endif //SOFTWARE_PINS_HPP
