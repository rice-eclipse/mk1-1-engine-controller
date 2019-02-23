/*
 * Defines interface for Luna visitor.
 */

#include "worker_visitor.hpp"

#ifndef LUNA_VISITOR_HPP
#define LUNA_VISITOR_HPP

/**
 * Work queue visitor that performs the standard tasks for Luna.
 */
class luna_visitor : public worker_visitor {
    public:
        void visitProc(work_queue_item&);

        void visitTimed(work_queue_item&);

        void visitIgn(work_queue_item&);

        luna_visitor(safe_queue<work_queue_item>& qw,
                                safe_queue<network_queue_item>& qn,
                                adc_block& adcs,
                                main_network_worker* nw_ref)
                : worker_visitor(qw, qn, adcs, nw_ref)
        {

        }
        ~luna_visitor() { }
};

#endif //LUNA_VISITOR_HPP
