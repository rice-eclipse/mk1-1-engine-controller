#include "worker_visitor.hpp"
#include "../final/timed_item.hpp"

int engine_type;
int time_between_gitvc;
int gitvc_wait_time;
float pressure_slope;
float pressure_yint;
int pressure_min;
int pressure_max;
int preignite_us;
int hotflow_us;
bool ignition_on;
bool pressure_shutoff;
bool use_gitvc;
std::vector<int> gitvc_times;

void worker_visitor::check_ti_list(timestamp_t t, safe_queue<work_queue_item> &qw) {
    int i, ti_seen = 0;
    work_queue_item wqi = {};
    for (i = 0; i < MAX_TIMED_LIST_LEN && ti_seen < TI_COUNT; i++) {
        if (ti_list->tis[i].action != wq_none) {
            ti_seen++;
            if (ti_list->tis[i].enabled && t > ti_list->tis[i].scheduled && t - ti_list->tis[i].scheduled > ti_list->tis[i].time_delay) {
                // Add this to the list of items to process:
                wqi.action = wq_timed;
                wqi.extra_datap = &ti_list->tis[i];
                qw.enqueue(wqi);
            }
        }
    }
}

void worker_visitor::visitNone(work_queue_item& wq_item) {
    now = get_time();
    check_ti_list(now, qw);
}

void worker_visitor::visitDefault(work_queue_item& wq_item) {
    logger.error("Work queue item not handled:" + std::to_string(wq_item.action), now);
}

void worker_visitor::visit(work_queue_item& item) {
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