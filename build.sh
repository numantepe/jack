#!/bin/zsh

clang jack_compiler.cc -o jack_compiler 
clang vm_translator.cc -o vm_translator
clang -g hack_disassembler.cc -o hack_disassembler 
