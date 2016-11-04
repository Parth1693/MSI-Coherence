#!/bin/bash
# My first script

echo "Hello World"
make clean
make
###################################################
#####################################################
./smp_cache 262144 8 64 4 0 trace/canneal.04t.longTrace > cachesize_long.txt
./smp_cache 524288 8 64 4 0 trace/canneal.04t.longTrace >> cachesize_long.txt
./smp_cache 1048576 8 64 4 0 trace/canneal.04t.longTrace >> cachesize_long.txt
./smp_cache 2097152 8 64 4 0 trace/canneal.04t.longTrace >> cachesize_long.txt
####################################################
./smp_cache 262144 8 64 4 1 trace/canneal.04t.longTrace >> cachesize_long.txt
./smp_cache 524288 8 64 4 1 trace/canneal.04t.longTrace >> cachesize_long.txt
./smp_cache 1048576 8 64 4 1 trace/canneal.04t.longTrace >> cachesize_long.txt
./smp_cache 2097152 8 64 4 1 trace/canneal.04t.longTrace >> cachesize_long.txt
#####################################################
./smp_cache 262144 8 64 4 2 trace/canneal.04t.longTrace >> cachesize_long.txt
./smp_cache 524288 8 64 4 2 trace/canneal.04t.longTrace >> cachesize_long.txt
./smp_cache 1048576 8 64 4 2 trace/canneal.04t.longTrace >> cachesize_long.txt
./smp_cache 2097152 8 64 4 2 trace/canneal.04t.longTrace >> cachesize_long.txt
#####################################################
#####################################################
./smp_cache 1048576 4 64 4 0 trace/canneal.04t.longTrace > assoc_long.txt
./smp_cache 1048576 8 64 4 0 trace/canneal.04t.longTrace >> assoc_long.txt
./smp_cache 1048576 16 64 4 0 trace/canneal.04t.longTrace >> assoc_long.txt
#####################################################
./smp_cache 1048576 4 64 4 1 trace/canneal.04t.longTrace >> assoc_long.txt
./smp_cache 1048576 8 64 4 1 trace/canneal.04t.longTrace >> assoc_long.txt
./smp_cache 1048576 16 64 4 1 trace/canneal.04t.longTrace >> assoc_long.txt
#####################################################
./smp_cache 1048576 4 64 4 2 trace/canneal.04t.longTrace >> assoc_long.txt
./smp_cache 1048576 8 64 4 2 trace/canneal.04t.longTrace >> assoc_long.txt
./smp_cache 1048576 16 64 4 2 trace/canneal.04t.longTrace >> assoc_long.txt
#####################################################
#####################################################
./smp_cache 1048576 8 64 4 0 trace/canneal.04t.longTrace > blocksize_long.txt
./smp_cache 1048576 8 128 4 0 trace/canneal.04t.longTrace >> blocksize_long.txt
./smp_cache 1048576 8 256 4 0 trace/canneal.04t.longTrace >> blocksize_long.txt
#####################################################
./smp_cache 1048576 8 64 4 1 trace/canneal.04t.longTrace >> blocksize_long.txt
./smp_cache 1048576 8 128 4 1 trace/canneal.04t.longTrace >> blocksize_long.txt
./smp_cache 1048576 8 256 4 1 trace/canneal.04t.longTrace >> blocksize_long.txt
#####################################################
./smp_cache 1048576 8 64 4 2 trace/canneal.04t.longTrace >> blocksize_long.txt
./smp_cache 1048576 8 128 4 2 trace/canneal.04t.longTrace >> blocksize_long.txt
./smp_cache 1048576 8 256 4 2 trace/canneal.04t.longTrace >> blocksize_long.txt
