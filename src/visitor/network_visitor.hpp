/**
 * Defines interfaces for networking visitors.
 */

#ifndef SOFTWARE_QUEUE_VISITOR_HPP
#define SOFTWARE_QUEUE_VISITOR_HPP

#include "queue_items.hpp"
#include "safe_queue.hpp"
#include "../final/main_network_worker.hpp"

/*
 * An interface to "visit" an item in a network queue and perform the appropriate
 * task based on the item's action. (Like work_queue_visitor, just using different
 * actions.)
 *
 * NOTE: the visit methods in this interface have return values (bool), unlike those
 * in work_queue_visitor.
 *
 * Based on a design by Arne Mertz.
 * See https://arne-mertz.de/2016/04/visitor-pattern-part-2-enum-based-visitor/
 */
class network_queue_visitor {
    private:
        /**
         * Operation corresponding to nq_recv.
         */
        virtual bool visitRecv(network_queue_item&) = 0;

        /**
         * Operation corresponding to nq_send_ack.
         */
        virtual bool visitAck(network_queue_item&) = 0;

        /**
         * Operation corresponding to nq_send.
         */
        virtual bool visitSend(network_queue_item&) = 0;

        /**
         * Operation corresponding to nq_none.
         */
        virtual bool visitNone(network_queue_item&) = 0;

        /**
         * Operation corresponding to an unspecified/unknown action.
         */
        virtual bool visitDefault(network_queue_item&) = 0;

    public:
        network_queue_visitor() { }
        ~network_queue_visitor() { }

        /**
         * Selects the correct task for the given network queue item and performs it.
         */
        bool visit(network_queue_item& item) {
            switch(item.action) {
                case nq_recv: {
                    return visitRecv(item);
                }
                case nq_send_ack: {
                    return visitAck(item);
                }
                case nq_send: {
                    return visitSend(item);
                }
                case nq_none: {
                    return visitNone(item);
                }
                default: {
                    return visitDefault(item);
                }
            }
        }
};

#endif // SOFTWARE_QUEUE_VISITOR_HPP
