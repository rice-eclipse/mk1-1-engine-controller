//
// Created by rjcunningham on 1/27/18.
//

#ifndef SOFTWARE_MAIN_BUFF_LOGGER_HPP
#define SOFTWARE_MAIN_BUFF_LOGGER_HPP

#include "../server/queue_items.hpp"

/**
 * A simple class for dumping binary logs from the main worker as they would be written from the network worker.
 */

// TODO properly merge logic with the network worker binary sender.

ssize_t write_from_nqi(network_queue_item nqi);

ssize_t write_buff(circular_buffer *buff, send_code h, size_t nbytes, size_t total_bytes) ;

#endif //SOFTWARE_MAIN_BUFF_LOGGER_HPP
