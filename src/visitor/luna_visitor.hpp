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
        void visitProc(work_queue_item&) override;

        void visitTimed(work_queue_item&) override;

        void visitIgn(work_queue_item&) override;

        luna_visitor(safe_queue<work_queue_item>& qw,
                                safe_queue<network_queue_item>& qn,
                                adc_block& adcs,
                                main_network_worker* nw_ref)
//                                bool use_gitvc,
//                                int time_between_gitvc,
//                                int gitvc_wait_time,
//                                std::vector<int> *gitvc_times)
                : worker_visitor(qw, qn, adcs, nw_ref)
//                , use_gitvc(use_gitvc)
//                , time_between_gitvc(time_between_gitvc)
//                , gitvc_wait_time(gitvc_wait_time)
//                , gitvc_times(gitvc_times)
        {
                gitvc_count = 0;
        }
        ~luna_visitor() = default;

    private:
        bool gitvc_on;
//        bool use_gitvc;
//        int time_between_gitvc;
//        int gitvc_wait_time;
        int gitvc_count;
//        std::vector<int> *gitvc_times;
};

#endif //LUNA_VISITOR_HPP
