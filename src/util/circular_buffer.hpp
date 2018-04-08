//
// Created by rjcunningham on 10/4/17.
//

#ifndef SOFTWARE_CIRCULAR_BUFFER_HPP
#define SOFTWARE_CIRCULAR_BUFFER_HPP


#include <cstdio>
#include <atomic>

/**
 * A simple circular buffer class for handling growing streams of sensor data that should be sent.
 * Has an atomic thing to track current position so that it doesn't overflow.
 * Class is not safe from multiple access to data.
 */
class circular_buffer {
    public:
        /**
         * The size of the buffer in bytes.
         */
        size_t nbytes;
        /**
         * The number of bytes that have been written into the buffer (in total).
         */
        std::atomic_size_t bytes_written;
        char *data;

        circular_buffer(size_t);

        ~circular_buffer();

        /**
         * Copies in n bytes from p into the buffer.
         * @param p A pointer for data to copy.
         * @param n The number of bytes to copy.
         */
        void add_data(void *p, size_t n);

        /**
         * Copies data into *dest, which will be written.
         * @param dest destination buffer.
         * @param n number of bytes to copy.
         * @param offset first byte in data we want to copy to dest.
         * @return
         */
        bool copy_data(char *dest, size_t n, size_t offset);

        /**
         * Writes n bytes to fd.
         * @param fd an open file descriptor.
         * @param n number of bytes to write.
         * @return
         */
        ssize_t write_data(int fd, size_t n, size_t offset);

        void zero();
};


#endif //SOFTWARE_CIRCULAR_BUFFER_HPP
