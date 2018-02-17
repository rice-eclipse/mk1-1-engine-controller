//
// Created by rjcunningham on 12/1/17.
//

#include <assert.h>
#include <iostream>
#include "unistd.h"
#include "main_worker.hpp"
#include "../util/timestamps.hpp"
#include "pins.hpp"
#include "../util/logger.hpp"
#include "main_buff_logger.hpp"
#include "timed_item.hpp"

#define SEND_TIME 100000 //Send every 100ms.

network_queue_item null_nqi = {nq_none}; //An item for null args to
work_queue_item null_wqi = {wq_none}; //An object with the non-matching action to do nothing.

adc_reading adcd = {};

static int ti_count = 0;
static timed_item ti_list[MAX_TIMED_LIST_LEN];// = {};

static void add_timed_item(timed_item &ti) {
    for (int i = 0; i < MAX_TIMED_LIST_LEN; i++) {
        if (ti_list[i].action == wq_none) {
            ti_list[i] = ti;
            ti_count++;
            return;
        }
    }
}

static void check_ti_list(timestamp_t t, safe_queue<work_queue_item> &qw) {
    int i, ti_seen = 0;
    work_queue_item wqi = {};
    for (i = 0; i < MAX_TIMED_LIST_LEN && ti_seen < ti_count; i++) {
        if (ti_list[i].action != wq_none) {
            ti_seen++;
            if (ti_list[i].enabled && t > ti_list[i].scheduled && t - ti_list[i].scheduled > ti_list[i].delay) {
                // Add this to the list of items to process:
                wqi.action = wq_timed;
                wqi.extra_datap = (void *) &ti_list[i];
                qw.enqueue(wqi);
            }
        }
    }
    return;
}

/*
static void enable_ti_item(timed_item *ti, timestamp_t now) {
    ti->scheduled = now;
    ti->enabled = true;
    std::cout << "Enabling timed item." << ti->action << std::endl;
}

static void disable_ti_item(timed_item *ti) {
    ti->enabled = false;
}*/

// Define all the timed items to be used. (If this just used a C++ class this wouldn't have been a mess).

size_t buff_size = 2 << 20; // About 5 minutes
timestamp_t now = get_time();

// todo put these into the array
// todo change delay defs to timestamp_t

static timed_item lc_main_ti =
        timed_item(now, LC_MAIN_T, new circular_buffer(buff_size), (adc_info_t) {LC_ADC, true, 0}, lc_main, true, now);
static timed_item lc1_ti =
        timed_item(now, LC1_T, new circular_buffer(buff_size), (adc_info_t) {LC_ADC, true, 1}, lc1, true, now);
static timed_item lc2_ti =
        timed_item(now, LC2_T, new circular_buffer(buff_size), (adc_info_t) {LC_ADC, true, 2}, lc2, true, now);
static timed_item lc3_ti =
        timed_item(now, LC3_T, new circular_buffer(buff_size), (adc_info_t) {LC_ADC, true, 3}, lc3, true, now);

static timed_item pt_inje_ti =
        timed_item(now, PT_FEED_T, new circular_buffer(buff_size), (adc_info_t) {PT_ADC, true, 1}, pt_feed, true, now);
static timed_item pt_comb_ti =
        timed_item(now, PT_INJE_T, new circular_buffer(buff_size), (adc_info_t) {PT_ADC, true, 2}, pt_inje, true, now);
static timed_item pt_feed_ti =
        timed_item(now, PT_COMB_T, new circular_buffer(buff_size), (adc_info_t) {PT_ADC, true, 0}, pt_comb, true, now);

static timed_item tc1_ti =
        timed_item(now, TC1_T, new circular_buffer(buff_size), (adc_info_t) {TC_ADC, true, 4}, tc1, true, now);
static timed_item tc2_ti =
        timed_item(now, TC2_T, new circular_buffer(buff_size), (adc_info_t) {TC_ADC, true, 5}, tc2, true, now);
static timed_item tc3_ti =
        timed_item(now, TC3_T, new circular_buffer(buff_size), (adc_info_t) {TC_ADC, true, 6}, tc3, true, now);

static timed_item ign2_ti =
        timed_item(now, IGN2_T, NULL, (adc_info_t) {}, ign2, false, now);
static timed_item ign3_ti =
        timed_item(now, IGN3_T, NULL, (adc_info_t) {}, ign3, false, now);

// uint16_t count = 0;

void main_worker::worker_method() {

    network_queue_item nq_item = {};
    work_queue_item wq_item = {};
    char c;

    // Create a Logger for this thread.
    Logger logger("logs/main_worker.log", "main_worker", LOG_INFO);

    // TODO should be using TI add for this. Make this into a C++ object with initialization.
    ti_count = 12;

    logger.info("Beginning main data worker.");

    while (1) {
        assert(ti_list[0].buffer != NULL);
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
                timed_item *ti = (timed_item *) wq_item.extra_datap;

                ti->scheduled = now;

                
                if (ti->buffer != NULL) {
                    //TODO make a Logger call to debugv
                    /*
                    std::cout << "Reading adc please work" << ti->adc_info.pin << " "
                                << ti->adc_info.single_channel << " " << ti->adc_info.channel << std::endl;
                                */
                    adcd.dat = adcs.read_item(ti->adc_info);
                    //adcd.dat = count++;
                    //usleep(100);
                    //FIXME switch this.

                    adcd.t = now;
                    ti->buffer->add_data(&adcd, sizeof(adcd));
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

                        // Next write the

                        // Write the object if we have something to send and we are connected.
                        if (nw_ref->connected && nq_item.nbytes > 0) {
                            qn.enqueue(nq_item);
                        }
                        // Next we also write the data to a (binary) log file by just directly dumping it from buff.
                        write_from_nqi(nq_item);
                        break;
                    }
                } else {
                    // Handle the case of using ignition stuff.
                    if (ti->action == ign2) {
                        //TODO allow it to enable without just some magic number for list entry.
                        disable_ti_item(&ti_list[10]);
                        ign2_ti.disable();
                        //enable_ti_item(&ign3_ti, now);
                        enable_ti_item(&ti_list[11], now);
                        ign3.enable(now);
                        logger.info("Writing main valve on from timed item.", now);
                        bcm2835_gpio_write(MAIN_VALVE, HIGH);
                        break;
                        // Enable the second igntion thing:
                        // TODO
                    }
                    if (ti->action == ign3) {
                        logger.info("Ending burn.", now);
                        logger.debug("Writing main valve off from timed item.", now);
                        bcm2835_gpio_write(MAIN_VALVE, LOW);

                        // Disable ignition 3.
                        disable_ti_item(&ti_list[11]);

                        logger.debug("Writing ignition off from timed item.", now);
                        bcm2835_gpio_write(IGN_START, LOW);
                        break;
                    }
                }
                break;
            }
            case ign1: {
                // Set the ignition on and then enable ign2.
                logger.info("Beginning ignition process", now);
                logger.debug("Writing ignition off.", now);
                bcm2835_gpio_write(IGN_START, HIGH);

                enable_ti_item(&ti_list[10], now);
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
    }

}
