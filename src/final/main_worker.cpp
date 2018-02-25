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

#define SEND_TIME 100000 //Send every 100ms.

network_queue_item null_nqi = {nq_none}; //An item for null args to
work_queue_item null_wqi = {wq_none}; //An object with the non-matching action to do nothing.
adc_reading adc_data = {};

static int ti_count = 0;
size_t buff_size = 2 << 20; // About 5 minutes
timestamp_t now = get_time();
timed_item ti_list[MAX_TIMED_LIST_LEN];

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
            if (ti_list[i].enabled && t > ti_list[i].scheduled && t - ti_list[i].scheduled > ti_list[i].time_delay) {
                // Add this to the list of items to process:
                wqi.action = wq_timed;
                wqi.extra_datap = &ti_list[i];
                qw.enqueue(wqi);
            }
        }
    }
    return;
}

// In case we need to reference the timed items individually
static timed_item lc_main_ti =
        timed_item(now, LC_MAIN_T, new circular_buffer(buff_size), (adc_info_t) {LC_ADC, true, 0}, lc_main, true, now);
static timed_item lc1_ti =
        timed_item(now, LC1_T, new circular_buffer(buff_size), (adc_info_t) {LC_ADC, true, 1}, lc1, true, now);
timed_item lc2_ti =
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
        timed_item(now, IGN2_T, nullptr, (adc_info_t) {}, ign2, false, now);
static timed_item ign3_ti =
        timed_item(now, IGN3_T, nullptr, (adc_info_t) {}, ign3, false, now);

static timestamp_t start_time_nitr = 0;
static double pressure_avg = 700;
static bool burn_on = false;

void main_worker::worker_method() {
    // Copy the values into ti_list. There might be a better way to do this.
    timed_item temp_ti_list[] = {lc_main_ti, lc1_ti, lc2_ti, pt_inje_ti, pt_comb_ti, pt_feed_ti, tc1_ti, tc2_ti,
                                 tc3_ti, ign2_ti, ign3_ti};
    std::copy(temp_ti_list, temp_ti_list + 12, ti_list);

    network_queue_item nq_item = {};
    work_queue_item wq_item = {};
    char c;

    // Create a Logger for this thread.
    Logger logger("logs/main_worker.log", "main_worker", LOG_INFO);

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

                    if (ti->a == 15) {
                        // todo calibrate adcd.dat first
                        // For y = mx+b, m=-0.36002  b=1412.207
                        double pt_cal = -0.36002 * adcd.dat + 1412.207;
                        pressure_avg = pressure_avg * 0.95 + pt_cal * 0.05;


                        if ((pressure_avg > 800 || pressure_avg < 300) && burn_on) {
                            // Start after 1000ms = 1s.
                            if (now - start_time_nitr > 1000000) {
                                //abcd
                                logger.error("Pressure shutoff. Closing main valve and unsetting ignition.", now);
                                bcm2835_gpio_write(MAIN_VALVE, LOW);
                                bcm2835_gpio_write(IGN_START, LOW);
                                burn_on = false;
                                break;
                            }
                        }
                    }

                    adcd.t = now;
                    ti->b->add_data(&adcd, sizeof(adcd));
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
                } else { // Handle the case of using ignition stuff. //todo this naming is confusing
                    if (ti->action == ign2) {
                        ign2_ti.disable();
                        ign3_ti.enable(now);
                        logger.info("Writing main valve on from timed item.", now);

                        bcm2835_gpio_write(MAIN_VALVE, HIGH);
                        start_time_nitr = now;
                        burn_on = true;
                        break;
                        // Enable the second igntion thing:
                        // TODO
                    }
                    if (ti->action == ign3) {
                        logger.info("Ending burn.", now);
                        burn_on = false;
                        logger.debug("Writing main valve off from timed item.", now);
                        bcm2835_gpio_write(MAIN_VALVE, LOW);

                        ign3_ti.disable();

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

                //todo original was timed_list[10], which is now ign2_ti?
                ign2_ti.enable(now);
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
