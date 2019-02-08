//
// Created by rjcunningham on 11/29/17.
//

#include <cstdio>
#include <iostream>
#include <bcm2835.h>
#include "initialization.hpp"
#include "../adc/lib/adc_block.hpp"
#include "pins.hpp"
#include "../server/queue_items.hpp"
#include "../server/safe_queue.hpp"
#include "../server/queue_visitor_imps.hpp"
#include "main_worker.hpp"
#include "ini_config.hpp"
#include "main_network_worker.hpp"
#include "timed_item_list.hpp"

circular_buffer buff(CIRC_SIZE);

/**
 * A target that runs the final code and does everything.
 *
 * Usage final <Port>
 *
 * Requires the following arguments:
 *   <Port>
 *      Sets the port on which the program listens for connections.
 *
 * @return 0 unless an error occurs.
 */
int main(int argc, char **argv) {
    unsigned int port; 
    int result, preignite_ms, hotflow_ms;
    char* filename;

    if (argc < 2) {
	    std::cout << "Please provide a config file filename!\n";
	    return 1;
    } else {
	    filename = argv[1];
    }

    // Store the config values in these variables
    init_config(&port,
                &engine_type,
                &use_gitvc,
                &time_between_gitvc,
                &gitvc_wait_time,
                &gitvc_times,
                &pressure_shutoff,
                &pressure_slope,
                &pressure_yint,
                &pressure_max,
                &pressure_min,
                &preignite_ms,
                &hotflow_ms,
                &ignition_on,
                filename);

    std::cout << "Reading config options from file: " << argv[1] << '\n';

    std::cout << "Engine type: " << engine_type << '\n';
    std::cout << "Hotflow time: " << hotflow_ms << '\n';
    std::cout << "Ignition:" << ignition_on << std::endl;
    std::cout << "Use pressure shutoff: " << pressure_shutoff << '\n';
    std::cout << "pressure slope: " << pressure_slope << '\n';
    std::cout << "Pressure y-intercept: " << pressure_yint << '\n';
    std::cout << "Pressure max: " << pressure_max << '\n';
    std::cout << "Pressure min: " << pressure_min << '\n';
    std::cout << "Time before ignition: " << preignite_ms << '\n';
    std::cout << "Use GITVC: " << use_gitvc << '\n';

    if (use_gitvc) {
    	std::cout << "\nTime between GITVC: " << time_between_gitvc << '\n';

    	for(int i = 0; i < gitvc_times.size(); i++)
		std::cout << "    " << gitvc_times[i] << '\n';
    }

    preignite_us = preignite_ms * 1000;
    hotflow_us = hotflow_ms * 1000;

    if (preignite_us < 0 || preignite_us > 5000000) {
        std::cerr << "Incorrect preignite time." << std::endl;
        return 1;
    }

    if (hotflow_ms < 0 || hotflow_ms > 15000000) {
        std::cerr << "Incorrect hotflow time." << std::endl;
        return 1;
    }

    if (port <= 0) {
        std::cerr << "Port must be larger than zero." << std::endl;
        return 1;
    }


    if (!bcm2835_init()) {
        std::cerr << "bcm2835_init failed. Are you running as root??\n" << std::endl;
        return 1;
    }

    if (engine_type == 0) {
        // use the Luna pin init
        initialize_pins();
    } else if (engine_type == 1) {
        // use the Titan pin init
        titan_initialize_pins();
    } else {
        // huh?!
        std::cerr << "Unknown engine type \'" << engine_type << "\'." << std::endl;
        return 1;
    }

    if (initialize_spi() != 0) {
        std::cerr << "Could not initialize SPI." << std::endl;
        return 1;
    };

    result = atexit(initialize_pins);
    if (result != 0) {
        std::cerr << "Could not register exit function." << std::endl;
        return 1;
    }

    adc_block adcs = adc_block(3);
    adcs.register_pin(0, ADC_0_CS);
    adcs.register_pin(1, ADC_1_CS);
    adcs.register_pin(2, ADC_2_CS);

    // Check that we can read from an ADC:
    std::cout << "Testing ADC read from ADC 2: " << adcs.read_item(2, true, 0) << std::endl;
    std::cout << "Testing ADC read from LC_ADC alt: " << adcs.read_item(adc_info_t(LC_ADC, true, 0)) << std::endl;

    // Set the base time so that we have no risk of overflow.
    set_base_time();


    // Now we create our network and hardware workers:
    safe_queue<network_queue_item> qn (null_nqi);
    safe_queue<work_queue_item> qw (null_wqi);

    network_queue_item initial = {};
    initial.action = nq_recv;

    qn.enqueue(initial);

    main_network_worker nw(qn, qw, port, buff);

    work_queue_visitor *wqv;
    if (engine_type == 0) {
        wqv = new main_work_queue_visitor(qw, qn, adcs, &nw);
    } else if (engine_type == 1) {
        wqv = new titan_work_queue_visitor(qw, qn, adcs, &nw);
    }
    main_worker cw(qn, qw, buff, adcs, &nw, wqv);

    nw.start();
    cw.start();
    nw.wait();
    cw.wait();
}
