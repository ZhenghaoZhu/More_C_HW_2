#!/bin/bash

echo "================================= Normal Malloc, Realloc, Normal Free, Approx Free Test ================================="

gcc src/lkmalloc.c -c -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
ar rcs tests/lkmalloc.a lkmalloc.o
gcc -c tests/lkmalloc_test_1.c -o tests/lkmalloc_test_1.o -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
gcc -o output/test1 tests/lkmalloc_test_1.o tests/lkmalloc.a -lglib-2.0
output/test1
