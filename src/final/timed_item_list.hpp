//
// Created by Tommy on 4/7/18.
//

#ifndef SOFTWARE_TI_LIST_HPP
#define SOFTWARE_TI_LIST_HPP

#define TI_COUNT    13

#include <iostream>
#include "timed_item.hpp"
#include "pins.hpp"
#include <map>

class timed_item_list {
    public:
        uint8_t length;
        size_t buff_size;
        std::map <work_queue_item_action, int> actionMap;
        timed_item tis[13];
        timed_item lc_main_ti;
        timed_item lc1_ti;
        timed_item lc2_ti;
        timed_item lc3_ti;
        timed_item pt_feed_ti;
        timed_item pt_inje_ti;
        timed_item pt_comb_ti;
        timed_item tc1_ti;
        timed_item tc2_ti;
        timed_item tc3_ti;
        timed_item ign2_ti;
        timed_item ign3_ti;
        timed_item gitvc_ti;

        timed_item_list(uint8_t length, size_t buff_size)
                : length(length)
                , buff_size(buff_size)
        {

	    // todo Hardware bug on ADC channels
            lc_main_ti =
                    timed_item(0, LC_MAIN_T, new circular_buffer(buff_size), adc_info_t(LC_ADC, true, 0), lc_main, true, 0);
            lc1_ti =
                    timed_item(0, LC1_T, new circular_buffer(buff_size), adc_info_t(LC_ADC, true, 1), lc1, true, 0);
            lc2_ti =
                    timed_item(0, LC2_T, new circular_buffer(buff_size), adc_info_t(LC_ADC, true, 3), lc2, true, 0);
            lc3_ti =
                    timed_item(0, LC3_T, new circular_buffer(buff_size), adc_info_t(LC_ADC, true, 4), lc3, true, 0);

            pt_inje_ti =
                    timed_item(0, PT_INJE_T, new circular_buffer(buff_size), adc_info_t(PT_ADC, true, 1), pt_inje, true, 0);
            pt_comb_ti =
                    timed_item(0, PT_COMB_T, new circular_buffer(buff_size), adc_info_t(PT_ADC, true, 2), pt_comb, true, 0);
            pt_feed_ti =
                    timed_item(0, PT_FEED_T, new circular_buffer(buff_size), adc_info_t(PT_ADC, true, 0), pt_feed, true, 0);

            tc1_ti =
                    timed_item(0, TC1_T, new circular_buffer(buff_size), adc_info_t(TC_ADC, true, 4), tc1, true, 0);
            tc2_ti =
                    timed_item(0, TC2_T, new circular_buffer(buff_size), adc_info_t(TC_ADC, true, 5), tc2, true, 0);
            tc3_ti =
                    timed_item(0, TC3_T, new circular_buffer(buff_size), adc_info_t(TC_ADC, true, 6), tc3, true, 0);

            ign2_ti = timed_item(0, 0, nullptr, adc_info_t(), ign2, false, 0);

            ign3_ti = timed_item(0, 0 * 1000, nullptr, adc_info_t(), ign3, false, 0);

            gitvc_ti = timed_item(0, 0, nullptr, adc_info_t(), gitvc, false, 0);

            timed_item temp_ti_list[] = {lc_main_ti, lc1_ti, lc2_ti, lc3_ti, pt_inje_ti, pt_comb_ti, pt_feed_ti, tc1_ti, tc2_ti,
                                         tc3_ti, ign2_ti, ign3_ti, gitvc_ti};
            std::copy(temp_ti_list, temp_ti_list + 13, tis);

            for (int i = 0; i < length; i++) {
                actionMap.insert(std::pair<work_queue_item_action, int>(tis[i].action, i));
            }
        };

        ~timed_item_list() = default;

        void enable (work_queue_item_action ti, timestamp_t now);

        void disable(work_queue_item_action ti);

        void set_delay(work_queue_item_action ti, timestamp_t new_delay);

        bool get_status(work_queue_item_action ti);

};

#endif //SOFTWARE_TI_LIST_HPP
