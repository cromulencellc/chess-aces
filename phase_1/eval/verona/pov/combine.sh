#!/bin/sh

cp sploit.s sploit-combined.s
echo "" >> sploit-combined.s
cat shellcode/encoded.s >> sploit-combined.s