#!/bin/zsh

extension="${1##*.}"

if [ "$extension" = "jack" ]; then

directory=$(dirname $1)

filename=$(basename $1)
filename="${filename%.*}"

if [ "$directory" = "." ]; then

./jack_compiler "$filename.jack"
./vm_translator "$filename.vm"
./hack_disassembler "$filename.asm"

# You can comment/uncomment to view files with extensions 
# of which you would like to see or not
#
rm -rf "$filename.vm"
# rm -rf "$filename.hack"
rm -rf "$filename.xml"
rm -rf "$filename""T.xml"
rm -rf "$filename.asm"

else

./jack_compiler "$directory/$filename.jack"
./vm_translator "$directory/$filename.vm"
./hack_disassembler "$directory/$filename.asm"

# You can comment/uncomment to view files with extensions 
# of which you would like to see or not
#
rm -rf "$directory/$filename.vm"
# rm -rf "$directory/$filename.hack"
rm -rf "$directory/$filename.xml"
rm -rf "$directory/$filename""T.xml"
rm -rf "$directory/$filename.asm"

fi

echo
echo "The code is ready to run!"

else

src="$1/OS"
dest="$1"

cp $src/* $dest

./jack_compiler $1
./vm_translator $1
./hack_disassembler $1/$1.asm

# You can comment/uncomment to view files with extensions 
# of which you would like to see or not
#
rm -rf $1/*.vm
rm -rf $1/*.asm
# rm -rf $1/*.hack
rm -rf $1/*.xml

echo
echo "The code is ready to run!"

fi
