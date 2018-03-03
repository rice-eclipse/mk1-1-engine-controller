//
// Created by rjcunningham on 9/30/17.
//

#ifndef SOFTWARE_QUEUE_ITEMS_HPP
#define SOFTWARE_QUEUE_ITEMS_HPP

#include <cstdint>
#include "../util/circular_buffer.hpp"
class timed_item;

enum work_queue_item_action {
    wq_none = 0,
    wq_process,
    wq_start,
    wq_stop,
    wq_timed, // Do some timed item with a given datap.
    // Items used for timed items:
            lc_main = 9,
    lc1 = 10,
    lc2 = 11,
    lc3 = 12,
    pt_feed = 13,
    pt_inje = 14,
    pt_comb = 15,
    tc1 = 16,
    tc2 = 17,
    tc3 = 18,
    ign1,
    ign2,
    ign3,
};

enum network_queue_item_action {
    nq_none,
    nq_send,
    nq_recv,
    nq_send_ack,
};

struct network_queue_item {
    network_queue_item_action action;
    circular_buffer *buff;
    size_t nbytes; //The number of bytes to send.
    size_t total_bytes; //The total number of bytes written into the relevant buffer at this point.
    char data[8]; // An extra data field used for simple transactions. Should deprecate this.
};

struct work_queue_item {
    //FIXME this is a really bad structure if we're actually doing anything with these.
    work_queue_item_action action;
    size_t nbytes; //The size of memory at datap.
    timed_item *extra_datap; //A pointer to the relevant data.
    char data[8]; // An extra data field for simple transactions.
};

enum send_code {
    ack = 1,
    payload = 2, // TODO this payload needs to be something like load cell, etc. etc.
    text = 3,
    unset_valve = 4,
    set_valve = 5,
    unset_ignition = 6,
    set_ignition = 7,
    ign_normal = 8,

    // Send codes used for dealing with this stuff.
    lc_main_send = 9,
    lc1_send = 10,
    lc2_send = 11,
    lc3_send = 12,
    pt_feed_send = 13,
    pt_inje_send = 14,
    pt_comb_send = 15,
    tc1_send = 16,
    tc2_send = 17,
    tc3_send = 18
};

struct send_header_t {
    send_code code;
    size_t nbytes;
}; typedef struct send_header_t send_header_t;
#endif //SOFTWARE_QUEUE_ITEMS_HPP
