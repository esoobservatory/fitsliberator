#!/bin/sh
# Photoshop FITS Liberator
#
# Script for compiling libtiff as universal binary (32/64 bit).
cd ../library
chmod a+x configure

./configure --with-apple-opengl-framework --disable-pixarlog --enable-cxx CFLAGS="-arch i386" CPPFLAGS="-arch i386" LDFLAGS="-arch i386" CC=gcc-4.2 CXX=g++-4.2
make
cp -f libtiff/.libs/libtiff.a ../binaries/libtiff_i386.a
make clean

./configure --with-apple-opengl-framework --disable-pixarlog --enable-cxx CFLAGS="-arch x86_64" CPPFLAGS="-arch x86_64" LDFLAGS="-arch x86_64" CC=gcc-4.2 CXX=g++-4.2
make
cp -f libtiff/.libs/libtiff.a ../binaries/libtiff_x86_64.a
make clean

cd ../binaries/
rm -f libtiff.a
lipo -create libtiff_i386.a libtiff_x86_64.a -output libtiff.a

