//
// Created by rjcunningham on 11/18/17.
//
#include <iostream>
#include <poll.h>
#include <stddef.h>
#include "network_worker.hpp"
#include "../util/listener.hpp"
#include "../util/timestamps.hpp"
#include "../final/ini_config.hpp"
#include <arpa/inet.h>

void network_worker::worker_method() {
    ssize_t read_result;
    char c;
    network_queue_item network_queue_item = {};
    work_queue_item work_queue_item = {};

    fd_set rfds, wfds;

    // Initialize read select
    FD_ZERO(&rfds);
    FD_SET(connfd_tcp, &rfds);

    // Initialize write select
    FD_ZERO(&wfds);
    FD_SET(connfd_udp, &wfds);

    if (!connected) {
        std::cout << "Attempting to connect" << std::endl;
        open_connection();
    }

    while (true) {
        FD_SET(connfd_tcp, &rfds);
        FD_SET(connfd_udp, &wfds);

        select(connfd_tcp + 1, &rfds, &wfds, nullptr, nullptr);

        if (FD_ISSET(connfd_tcp, &rfds)) {
            read_result = do_recv(connfd_tcp, &c, 1);

            if (read_result == 0) {
                printf("Socket is dead. I no longer have a reason to live.\n");
                exit(1);
            }

            work_queue_item.action = wq_process;
            work_queue_item.data[0] = c;
            qw.enqueue(work_queue_item);
        }

        if (FD_ISSET(connfd_udp, &wfds)) {
            // printf("UDP send\n");
            network_queue_item = qn.poll();
            if (!process_nqi(network_queue_item)) {
                std::cerr << "Could not process request on network thread: " << network_queue_item.action << std::endl;
            }
        }

//        //std::cout << "Networker entering loop:\n";
//        network_queue_item = qn.poll();
//        //std::cout << "Networker got item:\n";

    }
}

bool network_worker::process_nqi(network_queue_item &network_queue_item) {
    switch (network_queue_item.action) {
        case (nq_none): {
            timestamp_t t = get_time();
            // First check if we are close to timing out:
            if (timeout > 0 && t - last_recv > timeout) {
                std::cerr << "Connection timed out." << std::endl;
                fail_connection();
                return true;
            } else if (timeout > 0 && t - last_recv > timeout / 2) {
                //TODO maybe add this as debug option.
                std::cerr << "Connection inactive. Sending ack." << std::endl;
                network_queue_item.action = nq_send_ack;
                qn.enqueue(network_queue_item);
                return true;
            }

            //is this messing with an object we don't own? Doesn't seem to be.
//            network_queue_item.action = nq_recv;
//            qn.enqueue(network_queue_item); //Just always be reading because otherwise we're screwed.
            // FIXME need to do some checking to make sure this happens frequently.
            return true;
        }
        default: {
            return false;
        }
    }
}

ssize_t network_worker::do_recv(int fd, char *b, size_t nbytes) {
    ssize_t read_result;

    //Poll before we read:
//    if (poll(&pf, 1, 0) >= 0) {
//        if (!(pf.revents & POLLIN)) {
//            // Nothing to read.
//            //std::cerr << "Socket blocked on read" << std::endl;
//            //Go back to looping.
//            return -2;
//        }
//    } else {
//        std::cerr << "Poll failed" << std::endl;
//        exit(1);
//    }
    read_result = read(fd, b, nbytes);
    if (read_result > 0) {
        /*
         * Update our timing on when we last received.
         */
        timestamp_t trecv = get_time();
        this->last_recv = trecv;
    } else if (read_result == 0) {
        /*
         * Check if the file descriptor has closed:
         */
        std::cerr << "Read nothing from socket. Assuming it is dead." << std::endl;
        fail_connection();
    }

    //std::cout << "Read " << read_result << " bytes from socket." << std::endl;
    return read_result;
}

void network_worker::fail_connection() {
    close(connfd_tcp);
    if (connfd_udp != -1) {
        close(connfd_udp);
    }
    connfd_tcp = -1;
    connfd_udp = -1;

    connected = false;
}

void network_worker::open_connection() {
    connfd_tcp = wait_for_connection(port, &sa_tcp);
    if (connfd_tcp < 0)
        std::cerr << "Could not open TCP connection fd." << std::endl;

    if (config_map["Server.Protocol"].as<std::string>() == "UDP") {
        connfd_udp = create_send_fd(port, (sockaddr_in *) &sa_udp);
//        if (connect(connfd_udp,(struct sockaddr *) &sa_udp, sizeof(sa_udp)) == -1) {
//            connfd_udp = -1;
//            std::cerr << "Could not open UDP connection fd." << std::endl;
//        }
        std::cerr << "Successfully connected on udp." << std::endl;
    }

    /*
     * Update the last received and mark that we are connected:
     */
    connected = true;
    last_recv = get_time();
}

ssize_t network_worker::send_header(send_code h, size_t nbytes) {
    send_header_t sh;
    sh.code = h;
    sh.nbytes = nbytes;

    // If the UDP Socket is setup, use it.
    if (connfd_udp != -1) {
        ssize_t result = write(connfd_udp, &sh, sizeof(sh));
    } else {
        ssize_t result = write(connfd_tcp, &sh, sizeof(sh));
    }
}