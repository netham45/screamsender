#!/bin/bash
g++ screamsender.cpp -o screamsender
aarch64-linux-gnu-g++ --static screamsender.cpp -o screamsender_aarch64
