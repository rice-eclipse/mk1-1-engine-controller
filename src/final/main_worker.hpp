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

struct adc_reading {
    uint16_t dat;
    uint64_t t;
};
extern struct adc_reading adcd;

class main_worker : public worker {
    public:
        circular_buffer &buff;
        adc_block &adcs;
        main_network_worker *nw_ref;
        main_worker(safe_queue<network_queue_item> &my_qn, safe_queue<work_queue_item> &my_qw,
                       circular_buffer &buff, adc_block &adcs, main_network_worker *nw_ref)
                : worker(my_qn, my_qw)
                , buff(buff)
                , adcs(adcs)
                , nw_ref(nw_ref)
        {
        };

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

// TODO please put these times somewhere less stupid.
// Times used for setting timed actions.
#define LC_MAIN_T 500

#define LC1_T 1000
#define LC2_T LC1_T
#define LC3_T LC1_T

#define PT_FEED_T 1000
#define PT_INJE_T PT_FEED_T
#define PT_COMB_T PT_FEED_T

#define TC1_T 20000
#define TC2_T TC1_T
#define TC3_T TC1_T

#define IGN2_T 250000 //250ms
#define IGN3_T 3000000 // 3000 ms

#define MAX_TIMED_LIST_LEN 20
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

struct timed_item {
    timestamp_t scheduled;
    timestamp_t delay;
    circular_buffer *buffer; // The output buffer used by this item.
    adc_info_t adc_info; // The adc info used to call the sampler:
    work_queue_item_action action;
    bool enabled;
    timestamp_t last_send; // A dumb value used to track when it was last sent.
    size_t nbytes_last_send; // The number of bytes that had been written into the circular buffer before the last send.
}; typedef struct timed_item timed_item;

#endif //SOFTWARE_MAIN_WORKER_HPP
