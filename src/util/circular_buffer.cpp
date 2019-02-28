//
// Created by rjcunningham on 10/4/17.
//

#include "unistd.h"
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include "circular_buffer.hpp"
#include <arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/types.h>

//#define DEBUG_CIRC_SEND

circular_buffer::circular_buffer(size_t size)
    : bytes_written(0)
    {
    this->nbytes = size;
    this->data = new char[size];
    }

circular_buffer::~circular_buffer() {
    delete [] this->data;
}

void circular_buffer::add_data(void *p, size_t n) {
    size_t bw = bytes_written.load();
    size_t to_add = nbytes - bw % this->nbytes; // The distance between start and the end of the buffer.
    to_add = to_add > n ? n : to_add; //Take the minimum value of n and to_add.
    memcpy(this->data + bw % this->nbytes, p, to_add);
    n -= to_add;
    bytes_written += to_add;
    if (n != 0)
        this->add_data((char *)p + to_add, n);
}

bool circular_buffer::copy_data(char *dest, size_t n, size_t offset) {
    if (n == 0) {
        return false;
    }
    if (n > this->nbytes) {
        std::cerr << "n (" << n << ") is larger than buffer size (" << this->nbytes << ")" << std::endl;
        return false;
    }

    size_t bw = bytes_written.load();

    if ((long) offset <  (long) (bw - this->nbytes)) {
        std::cerr << "Bytes already overwritten before sending: Num_to_write:" << n
                  << " Offset:" << offset
                  << " Total bytes written into buffer" << bw
                  << " Total buffer size: " << nbytes << std::endl;
        //return -1; //TODO don't collide with write error?
    }

    size_t right_bytes = nbytes - offset; // Number of bytes to the right of offset
    size_t right_copy = right_bytes >= n ? n : right_bytes; // Take the smaller value in case we wrap around

//    size_t to_send = nbytes - offset % this->nbytes; // The distance between start and the end of the buffer.
//    to_send = to_send > n ? n : to_send; //Take the minimum value of n and to_send.

    std::memcpy(dest, this->data + offset, right_copy);
    if (right_copy == right_bytes) { // If the data we want to copy wraps around
        // copy remaining num of bytes from the beginning
        std::memcpy(dest + right_copy, this->data, n - right_copy);
    }
    return true;
}

ssize_t circular_buffer::write_data(int fd, size_t n, size_t offset) {
    if (n == 0) {
        return 0;
    }
    size_t bw = bytes_written.load();
    /*
    offset is smallest number we're trying to write.
    bw - nbytes is the smallest number that hasn't been overwritten.
    We want to ensure that we aren't trying to write data that has been overwritten.
     */
    if ((long) offset <  (long) (bw - this->nbytes)) {
        std::cerr << "Bytes already overwritten before sending: Num_to_write:" << n
                  << " Offset:" << offset
                  << " Total bytes written into buffer" << bw
                  << " Total buffer size: " << nbytes << std::endl;
        //return -1; //TODO don't collide with write error?
    }
    size_t to_send = nbytes - offset % this->nbytes; // The distance between start and the end of the buffer.
    to_send = to_send > n ? n : to_send; //Take the minimum value of n and to_send.
#ifdef DEBUG_CIRC_SEND
    if (to_send > 4 * 16) {
        size_t temp_offset = offset % this->nbytes;
        
        fprintf(stdout, "Offset %#u \n", (uint16_t) temp_offset);
        fprintf(stdout, "Sending Bytes: %X %lld %X %lld %X %lld %X %lld \n",
            *((uint16_t *)(data + temp_offset)), *((uint64_t *)(data + temp_offset + 8)),
            *((uint16_t *)(data + temp_offset + 16)), *((uint64_t *)(data + temp_offset + 24)),
            *((uint16_t *)(data + temp_offset + 32)), *((uint64_t *)(data + temp_offset + 40)),
            *((uint16_t *)(data + temp_offset + 48)), *((uint64_t *)(data + temp_offset + 56)));
    }
#endif
    ssize_t result;
    result = write(fd, data + offset % this->nbytes, to_send);
    // result = sendto(fd, data + offset % this->nbytes, to_send, 0, destination, sizeof(struct sockaddr_in));

    if (result <= 0) {
        perror("help?");
        std::cerr << "Error while writing." << std::endl;
        return result;
    } else {
        //Need to send slightly less data now:
        //TODO check if write_data failed.
        return write_data(fd, n - result, offset + result);
    }
}

ssize_t circular_buffer::write_data_datagram(int fd, size_t n, size_t offset, void *header, size_t header_size, sockaddr *destination) {
    size_t bw = bytes_written.load();
    char *send_buff = new char[header_size + n];
    char *pos = send_buff;

    /*
    offset is smallest number we're trying to write.
    bw - nbytes is the smallest number that hasn't been overwritten.
    We want to ensure that we aren't trying to write data that has been overwritten.
     */
    if ((long) offset <  (long) (bw - this->nbytes)) {
        std::cerr << "Bytes already overwritten before sending: Num_to_write:" << n
                  << " Offset:" << offset
                  << " Total bytes written into buffer" << bw
                  << " Total buffer size: " << nbytes << std::endl;
        //return -1; //TODO don't collide with write error?
    }

    // Copy in the header at the beginning
    memcpy(pos, header, header_size);
    pos += header_size;

    size_t to_send = nbytes - offset % this->nbytes; // The distance between start and the end of the buffer.
    to_send = to_send > n ? n : to_send; //Take the minimum value of n and to_send.

    // Copy in the data
    memcpy(pos, data + offset % this->nbytes, to_send);
    pos += offset % this->nbytes;

    if (n > to_send) {
        memcpy(pos, data + (offset + to_send) % this->nbytes, n - to_send);
    }

#ifdef DEBUG_CIRC_SEND
    if (to_send > 4 * 16) {
        size_t temp_offset = offset % this->nbytes;

        fprintf(stdout, "Offset %#u \n", (uint16_t) temp_offset);
        fprintf(stdout, "Sending Bytes: %X %lld %X %lld %X %lld %X %lld \n",
            *((uint16_t *)(data + temp_offset)), *((uint64_t *)(data + temp_offset + 8)),
            *((uint16_t *)(data + temp_offset + 16)), *((uint64_t *)(data + temp_offset + 24)),
            *((uint16_t *)(data + temp_offset + 32)), *((uint64_t *)(data + temp_offset + 40)),
            *((uint16_t *)(data + temp_offset + 48)), *((uint64_t *)(data + temp_offset + 56)));
    }
#endif
    ssize_t result;
    // result = write(fd, data + offset % this->nbytes, to_send);
    result = sendto(fd, send_buff, to_send, 0, destination, sizeof(struct sockaddr_in));

    if (result <= 0) {
        perror("help?");
        std::cerr << "Error while writing." << std::endl;
        return result;
    } else if (result != n) {
        std::cerr << "Error while writing. Wrote " << result << " bytes, expecting " << n << " bytes total\n";
        return -1;
    }

    return 0;
}

void circular_buffer::zero() {
    bzero(data, nbytes);
    bytes_written = 0;
}
