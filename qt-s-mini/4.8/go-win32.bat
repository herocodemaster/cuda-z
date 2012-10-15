set MAKEFLAGS=
..\qt-4.8.2\configure.exe -platform win32-msvc2010 ^
	-xplatform win32-msvc2010_s_mini -debug-and-release -static ^
	-no-exceptions -no-accessibility -no-stl -no-sql-sqlite -no-sql-sqlite2 ^
	-no-sql-mysql -no-sql-psql -no-qt3support -no-xmlpatterns -no-multimedia ^
	-no-audio-backend -no-phonon -no-phonon-backend -no-webkit -no-script ^
	-no-scripttools -no-declarative -no-3dnow -no-sse -no-sse2 -no-gif ^
	-no-libtiff -qt-libpng -no-libmng -no-libjpeg -no-openssl -no-dbus ^
	-no-style-plastique -no-style-cleanlooks -no-style-motif -no-style-cde ^
	-confirm-license -opensource
nmake sub-src
