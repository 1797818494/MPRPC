#!/bin/bash
set -e
rm -rf `pwd`/build/*
cd `pwd`/build && cmake .. && make -j4

cd ..
cp -r `pwd`/src/include `pwd`/lib