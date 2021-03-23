#!/bin/bash

echo "================================= All Over the Place Allocs and Frees Test ================================="

gcc -c tests/lkmalloc_test_3.c -o tests/lkmalloc_test_3.o -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
gcc -o output/test3 tests/lkmalloc_test_3.o tests/liblkmalloc.a -lglib-2.0
output/test3