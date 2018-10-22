# A directory containing a couple simple mock objects that abstract away
# the real world raspberry PI. Should be sufficient for testing without
# starting the PI.

add_library(fake_adc ../../src/adc/lib/adc_block.hpp fake_adc_block.cpp)

add_library(fake_bcm fake_bcm.cpp)
