project(final)

add_executable(${PROJECT_NAME} main.cpp initialization.cpp initialization.hpp main_network_worker.cpp main_network_worker.hpp
        ../server/network_worker.cpp ../server/network_worker.hpp ../server/worker.hpp pins.hpp main_worker.cpp main_worker.hpp pins.cpp main_buff_logger.cpp main_buff_logger.hpp)

SET(FINAL_LIBS_NO_FAKED timestamps.a listener.a circ_buff.a logger.a)
SET(FINAL_LIBS ${FINAL_LIBS_NO_FAKED} adc_lib.a)
SET(FINAL_LIBS_MOCKS ${FINAL_LIBS_NO_FAKED} fake_adc.a fake_bcm)

SET(FINAL_LIBS_EXT bcm2835 pthread)

add_dependencies(${PROJECT_NAME} ${FINAL_LIBS})



target_link_libraries(${PROJECT_NAME} ${FINAL_LIBS} ${FINAL_LIBS_EXT})

# A mocked version of the final code that can be used for debugging.
# TODO include some modification of the logging so that we clearly observe the difference.

add_executable(final_mocked main.cpp ../../test/mocks/fake_initialization.cpp main_network_worker.cpp
        ../server/network_worker.cpp main_worker.cpp pins.cpp main_buff_logger.cpp)

add_dependencies(final_mocked ${FINAL_LIBS_MOCKS})
target_link_libraries(final_mocked ${FINAL_LIBS_MOCKS} pthread)

