//
// Created by Tommy on 4/7/18.
//

#include "../server/queue_items.hpp"
#include "../util/timestamps.hpp"
#include "timed_item.hpp"
#include "timed_item_list.hpp"

void timed_item_list::enable(work_queue_item_action ti, timestamp_t now) {
    tis[actionMap.at(ti)].enable(now);
}

void timed_item_list::disable(work_queue_item_action ti) {
    tis[actionMap.at(ti)].disable();
}

void timed_item_list::set_delay(work_queue_item_action ti, timestamp_t new_delay) {
    tis[actionMap.at(ti)].time_delay = new_delay;
}

bool timed_item_list::get_status(work_queue_item_action ti) {
    return tis[actionMap.at(ti)].enabled;
}

