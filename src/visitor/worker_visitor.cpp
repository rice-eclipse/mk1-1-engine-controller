void worker_visitor::add_timed_item(timed_item &ti) {
    for (int i = 0; i < MAX_TIMED_LIST_LEN; i++) {
        if (ti_list->tis[i].action == wq_none) {
            ti_list->tis[i] = ti;
            ti_count++;
            return;
        }
    }
}

void worker_visitor::check_ti_list(timestamp_t t, safe_queue<work_queue_item> &qw) {
    int i, ti_seen = 0;
    work_queue_item wqi = {};
    for (i = 0; i < MAX_TIMED_LIST_LEN && ti_seen < ti_count; i++) {
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
    return;
}

void worker_visitor::visitNone(work_queue_item& wq_item) {
    now = get_time();
    check_ti_list(now, qw);
}

void worker_visitor::visitDefault(work_queue_item& wq_item) {
    logger.error("Work queue item not handled:" + std::to_string(wq_item.action), now);
}
