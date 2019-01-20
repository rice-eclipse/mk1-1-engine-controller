//
// Created by rjcunningham on 12/1/17.
//

#ifndef SOFTWARE_MAIN_WORKER_HPP
#define SOFTWARE_MAIN_WORKER_HPP

#include "../util/circular_buffer.hpp"
#include "../server/worker.hpp"
#include "../adc/lib/adc_block.hpp"
#include "../util/timestamps.hpp"
#include "main_network_worker.hpp"
#include "../server/queue_visitor.hpp"

extern int time_between_gitvc;
extern int gitvc_wait_time;
extern float pressure_slope;
extern float pressure_yint;
extern int pressure_min;
extern int pressure_max;
extern int preignite_us;
extern int hotflow_us;
extern bool ignition_on;
extern bool pressure_shutoff;
extern bool use_gitvc;
extern std::vector<int> gitvc_times;

struct adc_reading {
    timestamp_t dat;
    uint64_t t;
};
extern struct adc_reading adcd;

class main_worker : public worker {
    public:
        circular_buffer &buff;
        adc_block &adcs;
        main_network_worker *nw_ref;

        main_work_queue_visitor *wqv;

        main_worker(safe_queue<network_queue_item> &my_qn, safe_queue<work_queue_item> &my_qw,
                       circular_buffer &buff, adc_block &adcs, main_network_worker *nw_ref)
                : worker(my_qn, my_qw)
                , buff(buff)
                , adcs(adcs)
                , nw_ref(nw_ref)
        {
            wqv = new main_work_queue_visitor(my_qw, my_qn, adcs, nw_ref);
        }

        ~main_worker() {
            delete wqv;
        }

        void start()
        {
            worker::start();
        }

        void worker_method();

        void stop() {
        }


};

#define CIRC_SIZE 1 << 16

extern network_queue_item null_nqi; //An item for null args to
extern work_queue_item null_wqi; //An object with the non-matching action to do nothing.

/*
BIG TODO
How this (timed items for scheduling stuff) currently works

List of timed items (structs, bad RJ) with a bunch of stuff in each of them:
	Timed item contains a flag corresponding to an action.
	Also possibly contains some data choices:
		ADC_INFO to sample
		Buffer that gets filled after sampling

		Crap related to scheduling sending the adc buffer.


One major simplification would be to change these to be classes
with a virtual method to process the object.

And then could have instances of these for each ADC.
Instance that accepts a lambda? -> This is a little trickier and it also still has that
chicken and egg problem of setting/unsetting another timed item.
    I don't have a good solution to this issue, but there are easish bandaid fixes for now
    that still greatly improve code readability.

Because none of these things have strict timing requirements we can easily make them virtual methods. If we need stricter timing we would need RT_PREEMPT and usage of real hardware timers.

I think the stricter timing stuff is best left to other parts of the code. For the most part it is completely sufficient to just do a spin loop that checks for more work to do periodically.
*/

#endif //SOFTWARE_MAIN_WORKER_HPP
