#!/bin/usr/env bash

for n in {1..5}; 
do
    #\time -f "Program: %C\nTotal time: %E\nUser Mode (s) %U\nKernel Mode (s) %S\nCPU: %P" ./mnt/DATA/Repositories/RobotteknikA/speed-test/test1 test
    #\time -f "Program: %C\nTotal time: %E\nUser Mode (s) %U\nKernel Mode (s) %S\nCPU: %P" ./mnt/DATA/Repositories/RobotteknikA/speed-test/test2 test
    time ./mnt/DATA/Repositories/RobotteknikA/speed-test/test1 test
    time ./mnt/DATA/Repositories/RobotteknikA/speed-test/test2 test
done