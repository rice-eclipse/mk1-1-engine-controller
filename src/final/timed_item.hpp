//
// Created by Tommy on 2/17/18.
//

#ifndef SOFTWARE_TIMED_ITEM_HPP
#define SOFTWARE_TIMED_ITEM_HPP

#define LC_MAIN_T 500

#define LC1_T 1000
#define LC2_T LC1_T
#define LC3_T LC1_T

#define PT_FEED_T 1000
#define PT_INJE_T PT_FEED_T
#define PT_COMB_T PT_FEED_T

#define TC1_T 20000
#define TC2_T TC1_T
#define TC3_T TC1_T

#define IGN2_T 750000 //750ms
#define IGN3_T 7000000 // 7000 ms

#define MAX_TIMED_LIST_LEN 20

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

    timed_item::timed_item() = default;

    timed_item::timed_item(timestamp_t sched, int del, circular_buffer *buff, adc_info_t adc,
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

    ~timed_item() = default;

    void enable(timestamp_t now);

    void disable();
};

#endif //SOFTWARE_PTIMED_ITEM_HPP
