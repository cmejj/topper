#!/bin/bash

g++ -std=c++11 -I ../include -o hello_world -g -O3 \
    hello_world.cc \
    ../src/response.cc \
    ../src/parameter.cc


