#!/bin/sh

#
# Point to the compiled instance of mbed-edge
#
# Repo: https://github.com/ARMmbed/mbed-edge-confidential-w
#
# export EDGE_REPO=${HOME}/edge/mbed-edge-confidential-w
export EDGE_REPO=${HOME}/edge/mbed-edge

#
# build the sample
#
make clean
make all
