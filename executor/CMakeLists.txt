cmake_minimum_required(VERSION 3.12)

project(syz-executor CXX)

execute_process(COMMAND	git rev-parse HEAD 	OUTPUT_VARIABLE GIT_REV)
string (REGEX REPLACE "[\n\t\r]" "" GIT_REV ${GIT_REV})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGIT_REVISION=\\\"${GIT_REV}\\\"")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGOOS_linux=1 -DGOARCH_amd64=1 -DHOSTGOOS_linux=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -m64 -pthread -Wall -Wparentheses -Wunused-const-variable -Wframe-larger-than=16384 -Wno-stringop-overflow -Wno-array-bounds -Wno-format-overflow -Wno-unused-but-set-variable -Wno-unused-command-line-argument")

add_executable(syz-executor executor.cc)
target_link_libraries(syz-executor PRIVATE /usr/local/lib/libxdc.a /usr/local/lib64/libcapstone.a)

install(TARGETS syz-executor DESTINATION /root/syz-executor-test)