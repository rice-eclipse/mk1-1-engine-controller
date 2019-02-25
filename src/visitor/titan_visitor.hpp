/*
 * Defines interface for the Titan visitor.
 */

#include "worker_visitor.hpp"

#ifndef TITAN_VISITOR_HPP
#define TITAN_VISITOR_HPP

/**
 * Work queue visitor for Titan.
 */
class titan_visitor : public worker_visitor {
    public:
        void visitProc(work_queue_item&) override;
        
        void visitTimed(work_queue_item&) override;

        titan_visitor(safe_queue<work_queue_item>& qw,
                                 safe_queue<network_queue_item>& qn,
                                 adc_block& adcs,
                                 main_network_worker* nw_ref)
                : worker_visitor(qw, qn, adcs, nw_ref)
        {

        }

        ~titan_visitor() = default;
};

#endif //TITAN_VISITOR_HPP
