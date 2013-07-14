#!/bin/bash
cd ../src
nasm -f elf -o ../build/storm.o storm.asm
cd ../build
ld -T storm.elf.ld -melf_i386 -o storm.elf storm.o
rm storm.o
