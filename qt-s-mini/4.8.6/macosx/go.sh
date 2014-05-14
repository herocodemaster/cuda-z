#!/bin/sh

CURRENT_PATH=`pwd`
INSTALL_PATH=$CURRENT_PATH
BUILD_PATH=${INSTALL_PATH}-build
SOURCE_PATH=$INSTALL_PATH/../qt-4.8.6

export MAKEFLAGS=-j4

mkdir $INSTALL_PATH/mkspecs
cp -r $SOURCE_PATH/mkspecs $INSTALL_PATH/mkspecs

mkdir $BUILD_PATH
cd $BUILD_PATH

nice $SOURCE_PATH/configure -prefix $INSTALL_PATH -debug-and-release -static \
	-xplatform macx-g++_s_mini  \
	-nomake examples -nomake demos -nomake docs -nomake translations \
	-no-exceptions -no-accessibility -no-stl -no-sql-sqlite -no-sql-sqlite2 \
	-no-sql-mysql -no-sql-psql -no-qt3support -no-xmlpatterns -no-multimedia \
	-no-audio-backend -no-phonon -no-phonon-backend -no-svg -no-webkit \
	-no-javascript-jit -no-script -no-scripttools -no-declarative \
	-no-3dnow -no-sse -no-sse2 -no-sse3 -no-ssse3 -no-sse4.1 -no-sse4.2 \
	-no-avx -no-neon -no-gif -no-libtiff -qt-libpng -no-libmng -no-libjpeg \
	-no-openssl -no-nis -no-cups -no-iconv -no-dbus -no-rtti \
	-reduce-relocations -cocoa -arch x86 \
	-confirm-license -opensource
make && make install

cd $INSTALL_PATH
