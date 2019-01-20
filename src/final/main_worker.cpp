//
// Created by rjcunningham on 12/1/17.
//

//#include <assert.h>
//#include <climits>
//#include <iostream>
//#include "unistd.h"
#include "main_worker.hpp"
//#include "pins.hpp"
//#include "../util/logger.hpp"
//#include "main_buff_logger.hpp"
//#include "timed_item.hpp"
//#include "timed_item_list.hpp"

void main_worker::worker_method() {
    work_queue_item wq_item = { };
    // assert(ti_list[0].adc_info.pin == LC_ADC);
    while (true) {
        // assert(ti_list[0].buffer != NULL);
        //std::cout << "Backworker entering loop:\n";
        wq_item = qw.poll();
        // TODO: work in a logger below (currently all in the visitor)
        // logger.debugv("Data worker got work item: " + std::to_string(wq_item.action) + ", visiting...");

        wqv->visit(wq_item);

    }
}
