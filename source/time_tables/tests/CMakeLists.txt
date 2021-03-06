SET(BOOST_LIBS ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY} ${Boost_SERIALIZATION_LIBRARY}
    ${Boost_DATE_TIME_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_REGEX_LIBRARY}
    ${Boost_IOSTREAMS_LIBRARY})


add_executable(thermometer_test thermometer.cpp)
target_link_libraries(thermometer_test time_tables ptreferential ed data fare routing utils pb_lib thermometer georef ${BOOST_LIBS} log4cplus protobuf)
ADD_BOOST_TEST(thermometer_test)


add_executable(departure_boards_test departure_boards_test.cpp)
target_link_libraries(departure_boards_test rt_handling time_tables ptreferential ed data fare routing utils pb_lib thermometer georef ${BOOST_LIBS} log4cplus pthread protobuf)
ADD_BOOST_TEST(departure_boards_test)



add_executable(route_schedules_test route_schedules_test.cpp)
target_link_libraries(route_schedules_test time_tables ptreferential ed data fare routing utils pb_lib thermometer georef ${BOOST_LIBS} log4cplus pthread protobuf)
ADD_BOOST_TEST(route_schedules_test)


add_executable(passages_test passages_test.cpp)
target_link_libraries(passages_test time_tables ptreferential ed data fare routing utils pb_lib thermometer georef ${BOOST_LIBS} log4cplus pthread protobuf)
ADD_BOOST_TEST(passages_test)
