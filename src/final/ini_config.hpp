//
// Created by Cody Tapscott on 3/20/18.
//

// A simple config INI parser using Boost's Program Options library

#ifndef SOFTWARE_INI_CONFIG_H
#define SOFTWARE_INI_CONFIG_H

#include <boost/program_options/variables_map.hpp>
namespace po = boost::program_options;


po::variables_map init_config(unsigned int *port, bool *use_gitvc, std::vector<int> *gitvc_times, bool *pressure_shutoff, unsigned int *preignite_ms, unsigned int *hotflow_ms);

// Global variables_map for accessing config values
extern po::variables_map config_map;

#endif //SOFTWARE_INI_CONFIG_H
