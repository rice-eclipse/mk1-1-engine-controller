// TODO: deal with the worker queue qw

#include "queue_visitor.hpp"
#include <unistd.h>
#include "../util/logger.hpp"
#include "../final/pins.hpp"
#include "../final/timed_item.hpp"
#include "../final/timed_item_list.hpp"

Logger logger("logs/main_worker.log", "main_worker", LOG_DEBUG);

// TODO: plopping all this here for now. Probably want to move it to a more
// appropriate location at some point.

#define SEND_TIME 500000 //Send every 500ms.

network_queue_item null_nqi = {nq_none}; //An item for null args to
work_queue_item null_wqi = {wq_none}; //An object with the non-matching action to do nothing.
adc_reading adc_data = {};

timed_item_list* ti_list;

static int ti_count = 13;
int gitvc_count = 0;
timestamp_t now = 0;

// These variables will be initialized from config.ini
int time_between_gitvc;
int gitvc_wait_time;
float pressure_slope;
float pressure_yint;
int pressure_min;
int pressure_max;
bool gitvc_on;
int preignite_us;
int hotflow_us;
bool ignition_on;
bool pressure_shutoff;
bool use_gitvc;
std::vector<int> gitvc_times;

static void add_timed_item(timed_item &ti) {
    for (int i = 0; i < MAX_TIMED_LIST_LEN; i++) {
        if (ti_list->tis[i].action == wq_none) {
            ti_list->tis[i] = ti;
            ti_count++;
            return;
        }
    }
}

static void check_ti_list(timestamp_t t, safe_queue<work_queue_item> &qw) {
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

void main_work_queue_visitor::visitProc(work_queue_item& wq_item, safe_queue<work_queue_item>& qw) override {
    logger.info("In process case");
    char c = wq_item.data[0];

    logger.debug("Processing request on worker.");

    switch (c) {
        // TODO is this doing anything. I don't think so.
        case '0': {
            wq_item.action = wq_start;
            qw.enqueue(wq_item);
            break;
        }
        case '1': {
            wq_item.action = wq_stop;
            qw.enqueue(wq_item);
            break;
        }
        case unset_valve: {
            logger.info("Writing main valve off on pin " + std::to_string(MAIN_VALVE), now);
            bcm2835_gpio_write(MAIN_VALVE, LOW);
            break;
        }
        case set_valve: {
            logger.info("Writing main valve on on pin " + std::to_string(MAIN_VALVE), now);
            bcm2835_gpio_write(MAIN_VALVE, HIGH);
            break;
        }
        case set_water: {
            logger.info("Turning water on on pin " + std::to_string(WATER_VALVE), now);
            bcm2835_gpio_write(WATER_VALVE, HIGH);
            break;
        }
        case unset_water: {
            logger.info("Turning water off on pin " + std::to_string(WATER_VALVE), now);
            bcm2835_gpio_write(WATER_VALVE, LOW);
            break;
        }
        case set_gitvc: {
            logger.info("Turning gitvc on on pin " + std::to_string(GITVC_VALVE), now);
            bcm2835_gpio_write(GITVC_VALVE, LOW);
            break;
        }
        case unset_gitvc:{
            logger.info("turning gitvc off on pin " + std::to_string(GITVC_VALVE), now);
            bcm2835_gpio_write(GITVC_VALVE, HIGH);
            break;
        }
        case unset_ignition: {
            logger.info("Writing ignition off.", now);
            bcm2835_gpio_write(IGN_START, LOW);
            logger.info("Writing main valve off.", now);
            bcm2835_gpio_write(MAIN_VALVE, LOW); // TODO ensure this gets done elsewhere.
            break;
        }
        case set_ignition: {
            logger.info("Writing ignition on.", now);
            bcm2835_gpio_write(IGN_START, HIGH);
            break;
        }
        case ign_normal: {
            logger.info("Beginning ignition process.", now);
            wq_item.action = ign1;
            qw.enqueue(wq_item);
            break;
        }
        default: {
            wq_item.action = wq_none;
            qw.enqueue(wq_item);
            break;
        }
    }
}

// TODO: timed_item

void main_work_queue_visitor::visitIgn(work_queue_item& wq_item, safe_queue<work_queue_item>& qw) override {
    now = get_time();
    logger.info("Beginning ignition process", now);
    if (ignition_on) {
        bcm2835_gpio_write(IGN_START, HIGH);
        logger.info("Hotflow on", now);
    } else {
        logger.info("Hotflow not on due to configuration", now);
    }

    //todo original was timed_list[10], which is now ign2_ti?
    // ign2_ti.enable(now);
    // ti_list->tis[10].enable(now);

    // Enable main valve on after the preignite period
    std::cout << "main_worker setting preignite_us and hotflow_us: " << preignite_us << "    " << hotflow_us << '\n';
    ti_list->set_delay(ign2, preignite_us);
    ti_list->enable(ign2, now);

    ti_list->set_delay(ign3, hotflow_us);
    bcm2835_gpio_write(WATER_VALVE, HIGH);
    break;
}

void main_work_queue_visitor::visitNone(work_queue_item& wq_item, safe_queue<work_queue_item>& qw) override {
    now = get_time();
    check_ti_list(now, qw);
}

void main_work_queue_visitor::visitDefault(work_queue_item& wq_item, safe_queue<work_queue_item>& qw) override {
    logger.error("Work queue item not handled:" + std::to_string(wq_item.action), now);
}
