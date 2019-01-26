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
    char c;
    work_queue_item wqi;
    ssize_t read_result;
    // char* combined_buff = new char[1 << 12];

    Logger logger("logs/nw.log", "network thread", LOG_INFO);

    switch (nqi.action) {
        case (nq_recv): {
//            //Poll before we read:
//            read_result = do_recv(connfd_tcp, &c, 1);
//            if (read_result <= 0) {
//                //FIXME, do something better?
//                return true;
//            }
//            //TODO just make this a process and let the logic in the worker handle it.
//            /*
//             * If we get a '0' then start processing stuff.
//             * If we get a '1' then stop processing stuff.
//             * Otherwise ignore the message.
//             */
//            logger.info("Received command " + std::to_string((uint8_t) c));
//            wqi.action = wq_process;
//            wqi.data[0] = c;
//            qw.enqueue(wqi);
            std::cerr << "Processing a recv\n";
            return true;
        }
        case (nq_send_ack): {
            // Don't actually send an ack. Just don't timeout for now.
            //network_worker::send_header(ack, 0);
            break;
        }
        case (nq_send): {
            //Poll before we read:
//            if (poll(&pf, 1, 0) == 0) {
//                if (!pf.revents & POLLOUT) {
//                    //Cannot write. Will block.
//                    logger.error("Socket blocked on write.");
//                    return true;
//                }
//            }
            circular_buffer *buff = nqi.buff;

            send_code h = (send_code) nqi.data[0];

            // TODO this header should correspond to something from the nqi data.

            network_worker::send_header(h, nqi.nbytes);

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

            logger.debug("Writing data: Nbytes: " + std::to_string(nqi.nbytes) + " Type: " + std::to_string(h));
            int connfd = (connfd_udp != -1) ? connfd_udp : connfd_tcp; //Use UDP if the socket is configured

//            ssize_t bytes_written = 0;
//            if ((bytes_written = write(connfd, combined_buff, nqi.nbytes)) != nqi.nbytes) {
//                std:: cerr << "Incorrect number of bytes written: " << "Expected " << nqi.nbytes << ", Actual " << bytes_written << std::endl;
//            }

            sendto(connfd, "abc", 4, 0, &sa_udp, sizeof(struct sockaddr_in));
            std::cout << "Sent on UDP" << std::endl;

//            if (buff->write_data(connfd, nqi.nbytes, nqi.total_bytes) != 0) {
//                std::cerr << "Connection Closed" << std::endl;
//                //exit(0);
//            }

            return true;
        }

        default: {
            // TODO should this ever happen?
            return network_worker::process_nqi(nqi);
        }
    }
}
