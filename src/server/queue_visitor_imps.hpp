/*
 * Contains classes for specific implementations of the queue visitors,
 * e.g. for Luna, for Titan, etc.
 */

#include "queue_visitor.hpp"

#ifndef SOFTWARE_QUEUE_VISITOR_IMPS_HPP
#define SOFTWARE_QUEUE_VISITOR_IMPS_HPP

/**
 * Work queue visitor that performs the standard tasks for Luna.
 */
class main_work_queue_visitor : public work_queue_visitor {
    public:
        virtual void visitProc(work_queue_item&); // virtual because overridden by titan_work_queue_visitor

        virtual void visitTimed(work_queue_item&); // virtual because overridden by titan_work_queue_visitor

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
 * Work queue visitor for Titan. Inherits most of its code from
 * main_work_queue_visitor, just with different ignition procedures.
 */
class titan_work_queue_visitor : public main_work_queue_visitor {
    public:
        void visitProc(work_queue_item&);
        
        void visitTimed(work_queue_item&);

        titan_work_queue_visitor(safe_queue<work_queue_item>& qw,
                                 safe_queue<network_queue_item>& qn,
                                 adc_block& adcs,
                                 main_network_worker* nw_ref)
                : main_work_queue_visitor(qw, qn, adcs, nw_ref)
        {

        }
        ~titan_work_queue_visitor() { }
};

#endif //SOFTWARE_QUEUE_VISITOR_IMPS_HPP
