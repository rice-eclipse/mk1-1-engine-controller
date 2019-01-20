#ifndef SOFTWARE_QUEUE_VISITOR_HPP
#define SOFTWARE_QUEUE_VISITOR_HPP

#include "queue_items.hpp"
#include "safe_queue.hpp"
#include "../adc/lib/adc_block.hpp"
#include "../final/main_network_worker.hpp"

/**
 * An interface to "visit" an item in a work queue and perform the appropriate
 * task based on the item's action.
 *
 * Based on a design by Arne Mertz.
 * See https://arne-mertz.de/2016/04/visitor-pattern-part-2-enum-based-visitor/
 */
class work_queue_visitor {
    public:
        safe_queue<work_queue_item>& qw; // the work queue from the containing worker
        safe_queue<network_queue_item>& qn; // the network queue from the containing worker
        adc_block& adcs;
        main_network_worker* nw_ref;

        /**
         * Operation corresponding to wq_process (a worker process).
         */
        virtual void visitProc(work_queue_item&) = 0;

        /**
         * Operation corresponding to wq_timed (a timed item).
         */
        virtual void visitTimed(work_queue_item&) = 0;

        /**
         * Operation corresponding to ign1 (ignition).
         */
        virtual void visitIgn(work_queue_item&) = 0;

        /**
         * Operation corresponding to wq_none (an empty queue).
         */
        virtual void visitNone(work_queue_item&) = 0;

        /**
         * Operation for an unknown/unspecified action.
         */
        virtual void visitDefault(work_queue_item&) = 0;

        work_queue_visitor(safe_queue<work_queue_item>& qw,
                           safe_queue<network_queue_item>& qn,
                           adc_block& adcs,
                           main_network_worker* nw_ref)
                           : qw(qw), qn(qn), adcs(adcs), nw_ref(nw_ref)
        {

        }
        ~work_queue_visitor() { }

        /**
         * Selects the correct task for the given work queue item and its queue,
         * and performs it.
         */
        void visit(work_queue_item& item) {
            switch (item.action) {
                case wq_process: {
                    visitProc(item);
                    break;
                }
                case wq_timed: {
                    visitTimed(item);
                    break;
                }
                case ign1: {
                    visitIgn(item);
                    break;
                }
                case wq_none: {
                    visitNone(item);
                    break;
                }
                default: {
                    visitDefault(item);
                    break;
                }
            }
        }
};

/**
 * Work queue visitor that performs the standard tasks for each action.
 */
class main_work_queue_visitor : public work_queue_visitor {
    public:
        void visitProc(work_queue_item&);

        void visitTimed(work_queue_item&);

        void visitIgn(work_queue_item&);

        void visitNone(work_queue_item&);

        void visitDefault(work_queue_item&);

        main_work_queue_visitor(safe_queue<work_queue_item>& qw,
                                safe_queue<network_queue_item>& qn,
                                adc_block& adcs,
                                main_network_worker* nw_ref)
                                : work_queue_visitor(qw, qn, adcs, nw_ref)
        {

        }
        ~main_work_queue_visitor() { }
};

/**
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
