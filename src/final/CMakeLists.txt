project(final)

add_executable(${PROJECT_NAME} main.cpp initialization.cpp initialization.hpp main_network_worker.cpp main_network_worker.hpp
        ../server/network_worker.cpp ../server/network_worker.hpp ../server/worker.hpp pins.hpp main_worker.cpp main_worker.hpp main_buff_logger.cpp main_buff_logger.hpp timed_item.hpp timed_item.cpp ini_config.cpp ini_config.hpp timed_item_list.cpp timed_item_list.hpp)

SET(FINAL_LIBS_NO_FAKED timestamps listener circ_buff logger)
SET(FINAL_LIBS ${FINAL_LIBS_NO_FAKED} adc_lib)
SET(FINAL_LIBS_MOCKS ${FINAL_LIBS_NO_FAKED} fake_adc fake_bcm)

SET(FINAL_LIBS_EXT bcm2835 pthread)

add_dependencies(${PROJECT_NAME} ${FINAL_LIBS})


# A mocked version of the final code that can be used for debugging.
# TODO include some modification of the logging so that we clearly observe the difference.

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS program_options REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
add_executable(final_mocked main.cpp ../../test/mocks/fake_initialization.cpp main_network_worker.cpp main_network_worker.hpp
        ../server/network_worker.cpp main_worker.cpp main_buff_logger.cpp timed_item.cpp ini_config.cpp timed_item_list.cpp timed_item_list.hpp)

#add_dependencies(final_mocked ${Boost_LIBRARIES} ${FINAL_LIBS_MOCKS})
target_link_libraries(final_mocked ${Boost_LIBRARIES} ${FINAL_LIBS_MOCKS} pthread)
target_link_libraries(${PROJECT_NAME} ${FINAL_LIBS} ${FINAL_LIBS_EXT} ${Boost_LIBRARIES})
