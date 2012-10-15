set MAKEFLAGS=
..\qt-4.7.4\configure.exe -platform win32-msvc2010 ^
	-xplatform win32-msvc2010_s_mini -debug-and-release -static ^
	-no-exceptions -no-accessibility -no-stl -no-sql-sqlite -no-sql-sqlite2 ^
	-no-sql-mysql -no-sql-psql -no-qt3support -no-opengl -no-openvg -no-gif ^
	-qt-libpng -no-libmng -no-libtiff -no-libjpeg -no-dsp -no-vcproj ^
	-no-incredibuild-xge -no-plugin-manifests -no-rtti -no-3dnow -no-sse ^
	-no-sse2 -no-openssl -no-dbus -no-phonon -no-phonon-backend ^
	-no-multimedia -no-audio-backend -no-webkit -no-script -no-scripttools ^
	-no-declarative -no-declarative-debug -no-style-plastique ^
	-no-style-cleanlooks -no-style-motif -no-style-cde ^
	-confirm-license -opensource
nmake sub-src
