/*
 * Defines interfaces for worker visitors for Luna and Titan.
 */

#ifndef SOFTWARE_QUEUE_VISITOR_HPP
#define SOFTWARE_QUEUE_VISITOR_HPP

#include "../server/queue_items.hpp"
#include "../server/safe_queue.hpp"
#include "../adc/lib/adc_block.hpp"
#include "../final/main_network_worker.hpp"

/**
 * An interface to "visit" an item in a work queue and perform the appropriate
 * task based on the item's action.
 *
 * Based on a design by Arne Mertz.
 * See https://arne-mertz.de/2016/04/visitor-pattern-part-2-enum-based-visitor/
 */
class worker_visitor {
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
        void visitNone(work_queue_item&);

        /**
         * Operation for an unknown/unspecified action.
         */
        void visitDefault(work_queue_item&);

        worker_visitor(safe_queue<work_queue_item>& qw,
                           safe_queue<network_queue_item>& qn,
                           adc_block& adcs,
                           main_network_worker* nw_ref)
                           : qw(qw), qn(qn), adcs(adcs), nw_ref(nw_ref)
        {

        }
        ~worker_visitor() { }

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

#endif // SOFTWARE_QUEUE_VISITOR_HPP
