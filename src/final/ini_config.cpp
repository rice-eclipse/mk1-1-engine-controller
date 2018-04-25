//
// Created by Cody Tapscott on 3/20/18.
//

#include "ini_config.hpp"

#include <boost/program_options.hpp>

#include <iostream>
#include <iterator>
#include <fstream>

// Global variables_map for config values
po::variables_map config_map;

// Configures the expected options to be read from the INI file, reads these, and then
// returns a variables_map containing the config parameters. See Boost::program_options
// documentation for more information about using po::variables_map
po::variables_map init_config(unsigned int *port, bool *use_gitvc, std::vector<int> *gitvc_times, bool *pressure_shutoff, unsigned int *preignite_ms, unsigned int *hotflow_ms)
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("Server.Port", po::value<unsigned int>(port)->required(), "set port for incoming connections")
        ("Server.Protocol", po::value<std::string>()->default_value("TCP"), "set protocol for data streaming (commands are always received over TCP)")
        ("Control.use_gitvc", po::value<bool>(use_gitvc)->default_value(false), "set if we are testing gitvc")
        ("Control.Gitvc_times", po::value<std::vector<int>>(gitvc_times)->required(), "array of times IN MICROSECONDS for GITVC")
        ("Control.Pressure_shutoff", po::value<bool>(pressure_shutoff)->default_value(true), "enable/disable pressure shutoff")
        ("Control.Preignite_ms",  po::value<unsigned int>(preignite_ms)->required(), "set milliseconds for preignition period")
        ("Control.Hotflow_ms",  po::value<unsigned int>(hotflow_ms)->required(), "set milliseconds before beginning hotflow")
    ;

    //todo change dir if needed
    // std::ifstream in("/home/eclipse/CLionProjects/mk1-1-engine-controller/cmake-build-debug/src/final/config.ini");
    std::ifstream in("src/final/config.ini");
    po::store(po::parse_config_file(in, desc), config_map);
    in.close();
    po::notify(config_map);

    return config_map;
}
