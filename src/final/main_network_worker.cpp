//
// Created by rjcunningham on 11/29/17.
//


#include <iostream>
#include <cstring>
#include <unistd.h>
#include "main_network_worker.hpp"
#include "../util/logger.hpp"
#include <arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/types.h>


bool main_network_worker::process_nqi(network_queue_item &nqi) {
    // char* combined_buff = new char[1 << 12];

    Logger logger("logs/nw.log", "network thread", LOG_INFO);

    switch (nqi.action) {
        case (nq_recv): {
            std::cerr << "Processing a recv\n";
            return true;
        }
        case (nq_send_ack): {
            // Don't actually send an ack. Just don't timeout for now.
            break;
        }
        case (nq_send): {
            circular_buffer *buff = nqi.buff;
            send_header_t header;

            header.code = (send_code) nqi.data[0];
            header.nbytes = nqi.nbytes;

            // TODO this header should correspond to something from the nqi data.

            // TODO error check
//            std::cerr << "Preparing header" << std::endl;
//
//            send_header_t sh = (send_header_t) {h, nqi.nbytes};
//
//            //send_header_t* header = network_worker::prepare_header(h, nqi.nbytes);
//
//            std::cerr << "Preparing to copy header" << std::endl;
//
//            std::memcpy(combined_buff, &sh, sizeof(send_header_t));
//
//            std::cerr << "Copying header complete" << std::endl;
//
//            if (!buff->copy_data(combined_buff + sizeof(send_header_t), nqi.nbytes, nqi.total_bytes)) {
//                std::cerr << "Copy bytes from buffer failed!" << std::endl;
//            }

//            ssize_t bytes_written = 0;
//            if ((bytes_written = write(connfd, combined_buff, nqi.nbytes)) != nqi.nbytes) {
//                std:: cerr << "Incorrect number of bytes written: " << "Expected " << nqi.nbytes << ", Actual " << bytes_written << std::endl;
//            }

//            sendto(connfd, "abc", 4, 0, &sa_udp, sizeof(struct sockaddr_in));
//            std::cout << "Sent on UDP" << std::endl;

            if (buff->write_data_datagram(connfd_udp, nqi.nbytes, nqi.total_bytes, &header, sizeof(send_header_t), &sa_udp) != 0) {
                std::cerr << "Connection Closed" << std::endl;
                //exit(0);
            }

            return true;
        }

        default: {
            // TODO should this ever happen?
            return network_worker::process_nqi(nqi);
        }
    }
}
