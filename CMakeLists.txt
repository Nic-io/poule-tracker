# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_cpp_world)

zephyr_include_directories(
    include
)
file(GLOB SRC_LIST src/*)
target_sources(app PRIVATE ${SRC_LIST})
