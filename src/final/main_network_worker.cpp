//
// Created by rjcunningham on 11/29/17.
//


#include <iostream>
#include "main_network_worker.hpp"
#include "../util/logger.hpp"

bool main_network_worker::process_nqi(network_queue_item &nqi) {
    char c;
    work_queue_item wqi;
    ssize_t read_result;

    Logger logger("logs/nw.log", "network thread", LOG_INFO);

    switch (nqi.action) {
        case (nq_recv): {
            //Poll before we read:
            read_result = do_recv(connfd_tcp, &c, 1);
            if (read_result <= 0) {
                //FIXME, do something better?
                return true;
            }
            //TODO just make this a process and let the logic in the worker handle it.
            /*
             * If we get a '0' then start processing stuff.
             * If we get a '1' then stop processing stuff.
             * Otherwise ignore the message.
             */
            logger.info("Received command " + std::to_string((uint8_t) c));
            wqi.action = wq_process;
            wqi.data[0] = c;
            qw.enqueue(wqi);
            return true;
        }
        case (nq_send_ack): {
            // Don't actually send an ack. Just don't timeout for now.
            //network_worker::send_header(ack, 0);
            break;
        }
        case (nq_send): {
            //Poll before we read:
            if (poll(&pf, 1, 0) == 0) {
                if (!pf.revents & POLLOUT) {
                    //Cannot write. Will block.
                    logger.error("Socket blocked on write.");
                    return true;
                }
            }
            circular_buffer *buff = nqi.buff;

            send_code h = (send_code) nqi.data[0];

            // TODO this header should correspond to something from the nqi data.

            network_worker::send_header(h, nqi.nbytes);
            logger.debug("Writing data: Nbytes: " + std::to_string(nqi.nbytes) + " Type: " + std::to_string(h));
            int connfd = (connfd_udp != -1) ? connfd_udp : connfd_tcp; //Use UDP if the socket is configured
            if (buff->write_data(connfd, nqi.nbytes, nqi.total_bytes) != 0) {
                std::cerr << "Connection Closed" << std::endl;
                //exit(0);
            }

            return true;
        }

        default: {
            return network_worker::process_nqi(nqi);
        }
    }
}