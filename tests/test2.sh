#!/bin/bash

echo "================================= Normal Malloc, Normal and Approx Free Test ================================="

gcc -c tests/lkmalloc_test_2.c -o tests/lkmalloc_test_2.o -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
gcc -o output/test2 tests/lkmalloc_test_2.o tests/liblkmalloc.a -lglib-2.0
output/test2