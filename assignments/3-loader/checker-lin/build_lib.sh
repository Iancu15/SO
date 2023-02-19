#!/bin/bash

cd ../skel-lin
make
cp libso_loader.so ../checker-lin
cd ../checker-lin
make -f Makefile.checker