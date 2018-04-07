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
po::variables_map init_config(unsigned int *port, unsigned int *preignite_ms, unsigned int *hotflow_ms)
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("Server.Port", po::value<unsigned int>(port)->required(), "set port for incoming connections")
        ("Server.Protocol", po::value<std::string>()->default_value("TCP"), "set protocol for data streaming (commands are always received over TCP)")
        ("Control.Preignite_ms",  po::value<unsigned int>(preignite_ms)->required(), "set milliseconds for preignition period")
        ("Control.Hotflow_ms",  po::value<unsigned int>(hotflow_ms)->required(), "set milliseconds before beginning hotflow")
    ;

    std::ifstream in("config.ini");
    po::store(po::parse_config_file(in, desc), config_map);
    in.close();
    po::notify(config_map);

    return config_map;
}