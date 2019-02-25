/*
 * Defines interfaces for worker visitors for Luna and Titan.
 */

#ifndef WORKER_VISITOR_HPP
#define WORKER_VISITOR_HPP

#define SEND_TIME 500000 //Send every 500ms.

#include "../final/timed_item_list.hpp"
#include "../util/logger.hpp"

extern int engine_type;
extern int time_between_gitvc;
extern int gitvc_wait_time;
extern float pressure_slope;
extern float pressure_yint;
extern int pressure_min;
extern int pressure_max;
extern int preignite_us;
extern int hotflow_us;
extern bool ignition_on;
extern bool pressure_shutoff;
extern bool use_gitvc;
extern std::vector<int> gitvc_times;

struct adc_reading {
    timestamp_t dat;
    uint64_t t;
};

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

        Logger logger;
        network_queue_item nq_item;
        adc_reading adc_data;

        timestamp_t now;
        timed_item_list *ti_list;
        timestamp_t start_time_nitr;
        double pressure_avg;
        bool burn_on;

        /**
         * Operation corresponding to wq_process (a worker process).
         */
        virtual void visitProc(work_queue_item&);

        /**
         * Operation corresponding to wq_timed (a timed item).
         */
        virtual void visitTimed(work_queue_item&);

        /**
         * Operation corresponding to ign1 (ignition).
         */
        virtual void visitIgn(work_queue_item&);

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
                        , logger(Logger("logs/main_worker.log", "main_worker", LOG_DEBUG))
                        , ti_list(new timed_item_list(TI_COUNT, 12 << 17))
                        , nq_item({})
                        , adc_data({})
                        , now(0)
                        , start_time_nitr(0)
                        , pressure_avg(700)
                        , burn_on(false)
        {
            // TODO a somewhat hacky fix to prevent a circular include between
            // worker_visitor.hpp and timed_item_list.hpp
            ti_list->set_delay(ign2, preignite_us);
            ti_list->set_delay(ign3, hotflow_us);
            ti_list->set_delay(gitvc, gitvc_wait_time);
        }

        ~worker_visitor() = default;

        /**
         * Selects the correct task for the given work queue item and its queue,
         * and performs it.
         */
        void visit(work_queue_item& item);

        void check_ti_list(timestamp_t t, safe_queue<work_queue_item> &qw);
};

#endif // WORKER_VISITOR_HPP
