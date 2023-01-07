#!/bin/zsh

clang jack_compiler.c -o jack_compiler 
clang vm_translator.c -o vm_translator
clang -g hack_disassembler.c -o hack_disassembler 
