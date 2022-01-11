#!/bin/bash

if [[ ! -f "libbee6502.a" ]]; then
	echo "Run this script from the directory where you built the Bee6502 engine."
	exit 1
fi

mkdir -p dist

if [ -d "Bee6502-SDL2" ]; then
	for lib in $(ldd Bee6502-SDL2/Bee6502-SDL2.exe | grep mingw | sed "s/.*=> //" | sed "s/(.*)//"); do
		cp "${lib}" dist
	done
	cp Bee6502-SDL2/Bee6502-SDL2.exe dist
fi

if [[ -f "Bee6502-SDL2/Bee6502-SDL2.exe" ]]; then
	cp -r ../Bee6502-SDL2/roms dist
fi
