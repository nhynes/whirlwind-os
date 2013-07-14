#!/bin/bash
cd ../src
nasm -f bin gale.asm
cat gale > ../build/storm
rm gale

nasm -f elf -o ../build/storm.o storm.asm
cd ../build
ld -T storm.ld -melf_i386 -o storm.k storm.o
rm ../build/storm.o

cat storm.k >> storm
rm storm.k
