//
// Created by Cody Tapscott on 3/20/18.
//

// A simple config INI parser using Boost's Program Options library

#ifndef SOFTWARE_INI_CONFIG_H
#define SOFTWARE_INI_CONFIG_H

#include <boost/program_options/variables_map.hpp>
#include <vector>

namespace po = boost::program_options;

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
			      unsigned int *preignite_ms,
			      unsigned int *hotflow_ms,
			      bool *ignition_on,
			      char *filename);

// Global variables_map for accessing config values
extern po::variables_map config_map;

#endif //SOFTWARE_INI_CONFIG_H
