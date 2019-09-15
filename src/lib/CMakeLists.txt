file(GLOB HEADER_LIST "./*.h")
file(GLOB SOURCE_LIST "*.cpp")

add_library(api_library ${SOURCE_LIST} ${HEADER_LIST})
find_package(Boost COMPONENTS system REQUIRED)

target_include_directories(api_library PUBLIC "./")
target_link_libraries(api_library  ${Boost_LIBRARIES} ${CURL_LIBRARIES} ${CPR_LIBRARIES})

