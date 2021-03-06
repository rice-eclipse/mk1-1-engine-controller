//
// Created by rjcunningham on 1/27/18.
//

#ifndef SOFTWARE_MAIN_BUFF_LOGGER_HPP

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "main_buff_logger.hpp"

ssize_t write_from_nqi(network_queue_item nqi) {
    circular_buffer *buff = nqi.buff;

    auto h = (send_code) nqi.data[0];

    return write_buff(buff, h, nqi.nbytes, nqi.total_bytes);

}

ssize_t write_buff(circular_buffer *buff, send_code h, size_t nbytes, size_t total_bytes) {
    const char *fname;

    int fd;

    switch (h) {
        case (lc_main_send):
            fname = "logs/lc_main.log";
            break;
        case (lc1_send):
            fname = "logs/lc1.log";
            break;
        case (lc2_send):
            fname = "logs/lc2.log";
            break;
        case (lc3_send):
            fname = "logs/lc3.log";
            break;
        case (pt_comb_send):
            fname = "logs/pt_comb.log";
            break;
        case (pt_feed_send):
            fname = "logs/pt_feed.log";
            break;
        case (pt_inje_send):
            fname = "logs/pt_inje.log";
            break;
        case (tc1_send):
            fname = "logs/tc1.log";
            break;
        case (tc2_send):
            fname = "logs/tc2.log";
            break;
        case (tc3_send):
            fname = "logs/tc3.log";
            break;
        default :
            // This shouldn't happen.
            return -1;
    }

    // Open the file to append and possibly create it.
    fd = open(fname, O_RDWR | O_APPEND | O_CREAT, 0644);

    if (fd < 0) {
        return -1;
    }

    ssize_t result = buff->write_data(fd, nbytes, total_bytes);

    close(fd);

    return result;
}

#endif //SOFTWARE_MAIN_BUFF_LOGGER_HPP