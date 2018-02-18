//
// Created by Tommy on 2/17/18.
//

#include "../util/circular_buffer.hpp"
#include "../util/timestamps.hpp"
#include "timed_item.hpp"

void timed_item::enable(timestamp_t now) {
    scheduled = now;
    enabled = true;
    // std::cout << "Enabling timed item." << ti->action << std::endl;
}

void timed_item::disable() {
    enabled = false;
}