// TODO: deal with the worker queue qw

#include "queue_visitor.hpp"
#include <climits>
#include <unistd.h>
#include "../util/logger.hpp"
#include "../final/pins.hpp"
#include "../final/timed_item.hpp"
#include "../final/timed_item_list.hpp"
#include "../final/main_buff_logger.hpp"

Logger logger("logs/main_worker.log", "main_worker", LOG_DEBUG);

// TODO: plopping all this here for now. Probably want to move it to a more
// appropriate location at some point.

#define SEND_TIME 500000 //Send every 500ms.

network_queue_item null_nqi = {nq_none}; //An item for null args to
work_queue_item null_wqi = {wq_none}; //An object with the non-matching action to do nothing.
adc_reading adc_data = {};

static int ti_count = 13;
int gitvc_count = 0;
timestamp_t now = 0;

timed_item_list* ti_list = new timed_item_list(ti_count, 12 << 17);

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

static timestamp_t start_time_nitr = 0;
static double pressure_avg = 700;
static bool burn_on = false;

network_queue_item nq_item = { };

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

void main_work_queue_visitor::visitProc(work_queue_item& wq_item) {
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

void main_work_queue_visitor::visitTimed(work_queue_item& wq_item) {
    // Get the current time
    now = get_time();

    // Get the timed item that added this:
    timed_item *ti = wq_item.extra_datap;

    ti->scheduled = now;

    if (ti->buffer != NULL) {
        //TODO make a Logger call to debugv
        /*
        std::cout << "Reading adc please work" << ti->adc_info.pin << " "
                    << ti->adc_info.single_channel << " " << (int) ti->adc_info.channel << std::endl;

        std::cout << "Testing ADC read from ADC 2: " << \
            adcs.read_item(ti->adc_info.pin, true, ti->adc_info.channel) << std::endl;
        */
        adc_data.dat = adcs.read_item(ti->adc_info);
        //adc_data.dat = adcs.read_item(2, ti->adc_info.single_channel, ti->adc_info.channel);
        //std::cout << "Read value from ADC: " << adc_data.dat << std::endl;
        //adc_data.dat = count++;
        //usleep(100);
        //FIXME switch this.

        if (ti->action == pt_comb && pressure_shutoff) {
            double pt_cal = pressure_slope * adc_data.dat + pressure_yint;
            pressure_avg = pressure_avg * 0.95 + pt_cal * 0.05; // Running average

            if ((pressure_avg > pressure_max || pressure_avg < pressure_min) && burn_on) {
                // Start after 1000ms = 1s.
                if (now - start_time_nitr > 1000000) {
                    // GITVC is active low
                    logger.error("Pressure shutoff: " + std::to_string(pressure_avg) + " . Closing main valve and unsetting ignition.", now);
                    logger.error("Max/Min set to " + std::to_string(pressure_max) + "/" + std::to_string(pressure_min), now);
                    logger.error("Slope/y-int set to " + std::to_string(pressure_slope) + "/" + std::to_string(pressure_yint), now);

                    bcm2835_gpio_write(MAIN_VALVE, LOW);
                    bcm2835_gpio_write(IGN_START, LOW);
                    bcm2835_gpio_write(GITVC_VALVE, HIGH);
                    burn_on = false;
                }
            }
        }

        adc_data.t = now;
        ti->buffer->add_data(&adc_data, sizeof(adc_data));
        //TODO add some debugv info with information on this logger.(@patrickhan)
        // Now see if it has been long enough that we should send data:
        if (now - ti->last_send > SEND_TIME) {
            size_t bw = ti->buffer->bytes_written.load();
            logger.debugv("Will do sending", now);
            // TODO improve this logger (@patrickhan)

            //Send some data:
            nq_item.action = nq_send;
            nq_item.nbytes = bw - ti->nbytes_last_send;
            nq_item.total_bytes = ti->nbytes_last_send;
            nq_item.data[0] = ti->action; // Include info on which buffer this is from.
            nq_item.buff = ti->buffer;
            ti->nbytes_last_send = bw;
            ti->last_send = now;

            // Write the object if we have something to send and we are connected.
            if (nw_ref->connected && nq_item.nbytes > 0) {
                qn.enqueue(nq_item);
            }
            // Next we also write the data to a (binary) log file by just directly dumping it from buff.
            write_from_nqi(nq_item);
        }
    } else { // Handle the cases of using ignition stuff.
        if (ti->action == ign2) { // Open the main valve and initiative GITVC
            // ign2_ti.disable();
            // ign3_ti.enable(now);

            // ti_list->tis[10].disable();
            // ti_list->tis[11].enable(now);

            ti_list->disable(ign2);
            ti_list->enable(ign3, now);

            logger.info("Writing main valve on from timed item.", now);

            bcm2835_gpio_write(MAIN_VALVE, HIGH);
            start_time_nitr = now;
            burn_on = true;

            if (use_gitvc && gitvc_times.size() > gitvc_count) {
                ti_list->enable(gitvc, now); // GITVC delay is initially set to gitvc_wait_time in timed_item_list.cpp
                logger.info("Setting GITVC to start after " + std::to_string(gitvc_wait_time) + " microseconds", now);
                logger.info("Total " + std::to_string(gitvc_times.size()) + " gitvc opens", now);
                gitvc_on = false;
                // gitvc_count++;
            }

            // Enable the second igntion thing:
            // TODO
        }
        if (ti->action == ign3) { // End the burn and gitvc
            logger.info("Ending burn.", now);
            burn_on = false;
            logger.debug("Writing main valve off from timed item.", now);
            bcm2835_gpio_write(MAIN_VALVE, LOW);

            // ign3_ti.disable();
            // ti_list->tis[11].disable();
            ti_list->disable(ign3);

            logger.debug("Writing ignition off from timed item.", now);
            bcm2835_gpio_write(IGN_START, LOW);

            ti_list->disable(gitvc);
            gitvc_count = INT_MAX; // Added security for shutting off GITVC
            logger.debug("Ending GITVC from timed item.", now);
            bcm2835_gpio_write(GITVC_VALVE, HIGH);

            bcm2835_gpio_write(WATER_VALVE, LOW);
        }
        if (ti->action == gitvc) { // Should only reach here once GITVC is set initially

            if (gitvc_on) { // Currently on, so turn it off
                // ti_list->disable(gitvc);

                // Disable current GITVC
                bcm2835_gpio_write(GITVC_VALVE, HIGH);
                gitvc_on = false;
                logger.debug("Writing GITVC off from timed item for " + std::to_string(time_between_gitvc) + " microseconds", now);

                // Use ti to turn on new GITVC in after time_between_gitvc time passes
                ti_list->set_delay(gitvc, time_between_gitvc);
                ti_list->enable(gitvc, now);
            } else if (gitvc_times.size() > gitvc_count && burn_on){ // Currently off, so turn it on if we're still igniting
                // ti_list->disable(gitvc);

                // Re-enable GITVC
                bcm2835_gpio_write(GITVC_VALVE, LOW);
                gitvc_on = true;
                logger.debug("Writing GITVC on from timed item for " + std::to_string(gitvc_times.at(gitvc_count)) + " microseconds", now);

                // Use ti to turn off current GITVC after gitvc_times.at(gitvc_count) time passes
                ti_list->set_delay(gitvc, gitvc_times.at(gitvc_count));
                ti_list->enable(gitvc, now);

                gitvc_count++;
            }
        }
    }
}

void main_work_queue_visitor::visitIgn(work_queue_item& wq_item) {
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
}

void main_work_queue_visitor::visitNone(work_queue_item& wq_item) {
    now = get_time();
    check_ti_list(now, qw);
}

void main_work_queue_visitor::visitDefault(work_queue_item& wq_item) {
    logger.error("Work queue item not handled:" + std::to_string(wq_item.action), now);
}
