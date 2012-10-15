
#!/bin/sh

export MAKEFLAGS=-j4
nice ../qt-4.7.4/configure -prefix $PWD -debug-and-release -static \
	-xplatform macx-g++_s_mini  \
	-no-exceptions -no-accessibility -no-stl -no-sql-sqlite -no-sql-sqlite2 \
	-no-sql-mysql -no-sql-psql -no-qt3support -no-xmlpatterns -no-multimedia \
	-no-audio-backend -no-phonon -no-phonon-backend -no-svg -no-webkit \
	-no-javascript-jit -no-script -no-scripttools -no-declarative \
	-no-3dnow -no-sse -no-sse2 -no-sse3 -no-ssse3 -no-sse4.1 -no-sse4.2 \
	-no-avx -no-neon -no-gif -no-libtiff -qt-libpng -no-libmng -no-libjpeg \
	-no-openssl -no-nis -no-cups -no-iconv -no-dbus -reduce-relocations \
	-cocoa -arch x86 \
	-confirm-license -opensource
nice make sub-src
