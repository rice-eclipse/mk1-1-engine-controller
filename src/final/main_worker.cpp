//
// Created by rjcunningham on 12/1/17.
//

#include <assert.h>
#include <iostream>
#include "unistd.h"
#include "main_worker.hpp"
#include "pins.hpp"
#include "../util/logger.hpp"
#include "main_buff_logger.hpp"
#include "timed_item.hpp"
#include "timed_item_list.hpp"

#define SEND_TIME 500000 //Send every 500ms.

network_queue_item null_nqi = {nq_none}; //An item for null args to
work_queue_item null_wqi = {wq_none}; //An object with the non-matching action to do nothing.
adc_reading adc_data = {};

static int ti_count = 13;
int gitvc_count = 0;
int time_between_gitvc = 200000;
bool gitvc_on;
timestamp_t now = 0;
// timed_item ti_list[MAX_TIMED_LIST_LEN];
timed_item_list* ti_list = new timed_item_list(ti_count, 2 << 20);

int preignite_us = 750000;
int hotflow_us = 7000000;
bool pressure_shutoff = true;
bool use_gitvc = false;
std::vector<int> gitvc_times{};

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

static timestamp_t start_time_nitr = 0;
static double pressure_avg = 700;
static bool burn_on = false;
int dont_crash = 0;

void main_worker::worker_method() {
    network_queue_item nq_item = {};
    work_queue_item wq_item = {};
    char c;
    int count = 0;

    // Create a Logger for this thread.
    Logger logger("logs/main_worker.log", "main_worker", LOG_DEBUG);

    logger.info("Beginning main data worker.");

    // assert(ti_list[0].adc_info.pin == LC_ADC);
    while (true) {
        // assert(ti_list[0].buffer != NULL);
        //std::cout << "Backworker entering loop:\n";
        wq_item = qw.poll();
        logger.debugv("Data worker got work item: " + std::to_string(wq_item.action));
        now = get_time();

        switch (wq_item.action) {
            // A case (that should be deprecated) that can be used to handle actions. Ideally remove this code asap.
            case (wq_process): {
                c = wq_item.data[0];

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
                        logger.info("Writing main valve off.", now);
                        bcm2835_gpio_write(MAIN_VALVE, LOW);
                        break;
                    }
                    case set_valve: {
                        logger.info("Writing main valve on.", now);
                        bcm2835_gpio_write(MAIN_VALVE, HIGH);
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
                break;
            }

            // The case for handling a request from a timed item.
            // This can be left as is, but ideally the logic would be moved into a future timed_item class.
            case (wq_timed): {
                // Get the timed item that added this:
                timed_item *ti = wq_item.extra_datap;

                ti->scheduled = now;

                if (ti->buffer != NULL) {
                    //TODO make a Logger call to debugv
                    /*
                    std::cout << "Reading adc please work" << ti->adc_info.pin << " "
                                << ti->adc_info.single_channel << " " << ti->adc_info.channel << std::endl;
                                */
                    adc_data.dat = adcs.read_item(ti->adc_info);
                    //adc_data.dat = count++;
                    //usleep(100);
                    //FIXME switch this.

                    if (ti->action == pt_comb && pressure_shutoff) {
                        // todo calibrate adcd.dat first
                        // For y = mx+b, m=-0.36002  b=1412.207
                        double pt_cal = -0.2810327855 * adc_data.dat + 1068.22;
                        pressure_avg = pressure_avg * 0.95 + pt_cal * 0.05;

                        if ((pressure_avg > 800 || pressure_avg < 300) && burn_on) {
                            // Start after 1000ms = 1s.
                            if (now - start_time_nitr > 1000000) {
                                //abcd
                                logger.error("Pressure shutoff. Closing main valve and unsetting ignition.", now);
                                bcm2835_gpio_write(MAIN_VALVE, LOW);
                                bcm2835_gpio_write(IGN_START, LOW);
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
                        break;
                    }
                } else { // Handle the case of using ignition stuff.
                    if (ti->action == ign2) {
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
                            ti_list->set_delay(gitvc, gitvc_times.at(0));
                            ti_list->enable(gitvc, now + 2000000);
                            logger.info("Setting first GITVC for " + std::to_string(gitvc_times.at(0)) + " microseconds.", now);
                            gitvc_on = true;
                            gitvc_count++;
                          }

                        break;
                        // Enable the second igntion thing:
                        // TODO
                    }
                    if (ti->action == ign3) {
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
                        logger.debug("Writing GITVC off from timed item.", now);
                        bcm2835_gpio_write(GITVC_VALVE, LOW);
                        break;
                    }
                    if (ti->action == gitvc) { // Should only reach here once GITVC is set initially

                        if (gitvc_on) { // Currently on, so turn it off
                            // ti_list->disable(gitvc);

                            // Disable current GITVC
                            bcm2835_gpio_write(GITVC_VALVE, LOW);
                            gitvc_on = false;
                            logger.debug("Writing GITVC off from timed item for " + std::to_string(time_between_gitvc) + " microseconds", now);

                            // Use ti to turn on new GITVC in after time_between_gitvc time passes
                            ti_list->set_delay(gitvc, time_between_gitvc);
                            ti_list->enable(gitvc, now);
                        } else if (gitvc_times.size() > gitvc_count){ // Currently off, so turn it on
                            // ti_list->disable(gitvc);

                            // Re-enable GITVC
                            bcm2835_gpio_write(GITVC_VALVE, HIGH);
                            gitvc_on = true;
                            logger.debug("Writing GITVC on from timed item for " + std::to_string(gitvc_times.at(gitvc_count)) + " microseconds", now);

                            // Use ti to turn off current GITVC after gitvc_times.at(gitvc_count) time passes
                            ti_list->set_delay(gitvc, gitvc_times.at(gitvc_count));
                            ti_list->enable(gitvc, now);

                            gitvc_count++;
                        }
                    }
                }
                break;
            }
            case ign1: {
                // Set the ignition on and then enable ign2.
                logger.info("Beginning ignition process", now);
                logger.debug("Writing ignition off.", now);
                bcm2835_gpio_write(IGN_START, HIGH);

                //todo original was timed_list[10], which is now ign2_ti?
                // ign2_ti.enable(now);
                // ti_list->tis[10].enable(now);
                ti_list->enable(ign2, now);
                break;
            }
            case (wq_none): {
                check_ti_list(now, qw);
                break;
            }

            default: {
                logger.error("Work queue item not handled:" + std::to_string(wq_item.action), now);
                break;
            }
        }
        dont_crash++;
    }
}
