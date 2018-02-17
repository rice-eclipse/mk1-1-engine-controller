//
// Created by eclipse on 2/17/18.
//

#include "../util/circular_buffer.hpp"
#include "../server/worker.hpp"
#include "../adc/lib/adc_block.hpp"
#include "../util/timestamps.hpp"
#include "main_network_worker.hpp"

class timed_item {

public:
    timestamp_t scheduled;
    timestamp_t time_delay;
    circular_buffer *buffer; // The output buffer used by this item.
    adc_info_t adc_info; // The adc info used to call the sampler:
    work_queue_item_action action;
    bool enabled;
    timestamp_t last_send; // A dumb value used to track when it was last sent.
    size_t nbytes_last_send; // The number of bytes that had been written into the circular buffer before the last send.

    timed_item(timestamp_t sched, int del, circular_buffer *buff, adc_info_t adc,
               work_queue_item_action wq_action, bool enable, timestamp_t last_send)
        : scheduled(sched)
        , time_delay((timestamp_t) del)
        , buffer(buff)
        , adc_info(adc)
        , action(wq_action)
        , enabled(enable)
        , last_send(last_send)
        , nbytes_last_send(0)
    {};

    void enable(timestamp_t now) {
        scheduled = now;
        enabled = true;
        // std::cout << "Enabling timed item." << ti->action << std::endl;
    }

    void disable() {
        enabled = false;
    }

};