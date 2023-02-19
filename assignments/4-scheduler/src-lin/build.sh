#!/bin/sh

make
cp libscheduler.so ../checker-lin
cd ../checker-lin
make -f Makefile.checker
cd ../src-lin