set CURRENT_PATH=%~dp0
set INSTALL_PATH=%CURRENT_PATH:~0,-1%
set BUILD_PATH=%INSTALL_PATH%-build
set SOURCE_PATH=%INSTALL_PATH%\..\qt-4.8.6

mkdir %INSTALL_PATH%\mkspecs
xcopy /s /q /y /i %SOURCE_PATH%\mkspecs %INSTALL_PATH%\mkspecs

mkdir %BUILD_PATH%
cd %BUILD_PATH%

%SOURCE_PATH%\configure.exe -platform win32-msvc2010 ^
	-xplatform %QMAKESPEC% -debug-and-release -static ^
	-prefix %INSTALL_PATH% ^
	-nomake examples -nomake demos -nomake docs -nomake translations ^
	-no-exceptions -no-accessibility -no-stl -no-sql-sqlite -no-sql-sqlite2 ^
	-no-sql-mysql -no-sql-psql -no-qt3support -no-xmlpatterns -no-multimedia ^
	-no-audio-backend -no-phonon -no-phonon-backend -no-webkit -no-script ^
	-no-scripttools -no-declarative -no-3dnow -no-sse -no-sse2 -no-gif ^
	-no-libtiff -qt-libpng -no-libmng -no-libjpeg -no-openssl -no-dbus ^
	-no-style-plastique -no-style-cleanlooks -no-style-motif -no-style-cde ^
	-no-rtti ^
	-confirm-license -opensource
nmake && nmake install

cd %INSTALL_PATH%
