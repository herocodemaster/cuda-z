#!/bin/sh

export MAKEFLAGS=-j2
#configure.exe -release -static -no-exceptions -no-accessibility -no-stl -no-sql-sqlite -no-qt3support -no-opengl -platform win32-msvc2005 -no-gif -no-libmng -no-libtiff -no-libjpeg -vcproj -dsp -no-incredibuild-xge -no-rtti -no-3dnow -no-sse -no-sse2 -no-openssl -no-dbus -no-phonon -no-webkit -no-style-windows -no-style-windowsxp -no-style-windowsvista -no-style-plastique -no-style-cleanlooks -no-style-motif -no-style-cde
nice ./configure -prefix /home/ag/Qt/4.6.2_s_mini -release -static \
    -no-exceptions -no-accessibility -no-stl -no-sql-sqlite -no-sql-sqlite2 \
    -no-sql-mysql -no-sql-psql -no-qt3support -no-xmlpatterns -no-phonon \
    -no-phonon-backend -no-svg -no-webkit -no-scripttools -no-opengl -no-gif \
    -no-libmng -no-libtiff -no-libjpeg -no-3dnow -no-sse -no-sse2 -no-openssl \
    -no-dbus -no-nis -no-iconv -no-cups -no-pch -no-nas-sound -no-opengl \
    -no-openvg -no-sm -no-xshape -no-xinerama -confirm-license -opensource
nice make sub-src
#make install

