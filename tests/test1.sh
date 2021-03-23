#!/bin/bash

echo "================================= Normal Malloc, Realloc, Normal Free, Approx Free Test ================================="

mkdir output
gcc -o tests/lkmalloc.o src/lkmalloc.c -c -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
ar rcs tests/liblkmalloc.a tests/lkmalloc.o
gcc -c tests/lkmalloc_test_1.c -o tests/lkmalloc_test_1.o -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
gcc -o output/test1 tests/lkmalloc_test_1.o tests/liblkmalloc.a -lglib-2.0
output/test1
