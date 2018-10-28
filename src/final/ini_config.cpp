//
// Created by Cody Tapscott on 3/20/18.
//

#include "ini_config.hpp"

#include <boost/program_options.hpp>

#include <iostream>
#include <iterator>
#include <fstream>
#include <string>
#include <exception>
#include "../util/useful_exceptions.hpp"

// Global variables_map for config values
po::variables_map config_map;
// Name of the config file, e.g. coldflow
std::string filename;

// Configures the expected options to be read from the INI file, reads these, and then
// returns a variables_map containing the config parameters. See Boost::program_options
// documentation for more information about using po::variables_map
po::variables_map init_config(unsigned int *port,
			      bool *use_gitvc,
			      int *time_between_gitvc,
			      int *gitvc_wait_time,
			      std::vector<int> *gitvc_times,
			      bool *pressure_shutoff,
			      float *pressure_slope,
			      float *pressure_yint,
			      int *pressure_max,
			      int *pressure_min,
			      int *preignite_ms,
			      int *hotflow_ms,
			      bool *ignition_on,
			      char* filename)
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("Server.Port", po::value<unsigned int>(port)->required(), "set port for incoming connections")
        ("Server.Protocol", po::value<std::string>()->default_value("TCP"), "set protocol for data streaming (commands are always received over TCP)")
        ("Control.use_gitvc", po::value<bool>(use_gitvc)->default_value(false), "set if we are testing gitvc")
        ("Control.Time_between_gitvc", po::value<int>(time_between_gitvc)->default_value(0), "set time between gitvc valve actuations")
        ("Control.Gitvc_wait_time", po::value<int>(gitvc_wait_time)->default_value(0), "set length of ignition before gitvc")
        ("Control.Gitvc_times", po::value<std::vector<int>>(gitvc_times)->multitoken(), "array of times IN MICROSECONDS for GITVC")
        ("Control.Ignition_on", po::value<bool>(ignition_on)->required(), "set ignition on or off")
	    ("Control.Preignite_ms",  po::value<int>(preignite_ms)->required(), "set milliseconds for preignition period")
        ("Control.Hotflow_ms",  po::value<int>(hotflow_ms)->required(), "set milliseconds before beginning hotflow")
        ("Pressure.Pressure_shutoff", po::value<bool>(pressure_shutoff)->default_value(true), "enable/disable pressure cutoff")
        ("Pressure.Pressure_slope", po::value<float>(pressure_slope)->required(), "set pt_comb slope")
        ("Pressure.Pressure_yint", po::value<float>(pressure_yint)->required(), "set pt_comb y intercept")
        ("Pressure.Pressure_max", po::value<int>(pressure_max)->default_value(800), "set max pressure cutoff")
        ("Pressure.Pressure_min", po::value<int>(pressure_min)->default_value(300), "set min pressure cutoff");

    //todo change dir if needed
    std::ifstream in("../../../src/final/" + std::string(filename) + ".ini");
    if (!in) {
		// ini not found, throw exception to be handled in main
		throw new fileNotFoundException(new std::string(filename));
	}
    po::store(po::parse_config_file(in, desc), config_map);
    in.close();
    po::notify(config_map);

    return config_map;
}
