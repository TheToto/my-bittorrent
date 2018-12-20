cmake_minimum_required(VERSION 3.0)
project (my-bittorrent)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra -std=c99 -pedantic -g")
set(CMAKE_C_STANDARD_LIBRARIES "-lcurl -ljansson -lcrypto -lssl")
include_directories(src/include)
include_directories(include/bencode)
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.c")

add_executable(my-bittorrent ${SOURCES})
target_link_libraries(my-bittorrent "${CMAKE_SOURCE_DIR}/lib/libbencode.a")
add_custom_target(doc "doxygen" "${CMAKE_SOURCE_DIR}/doc/configDoxygen")
add_custom_target(check "${CMAKE_SOURCE_DIR}/test/test.sh")
