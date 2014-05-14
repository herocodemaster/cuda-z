rem taskkill /IM mspdbsrv.exe
call C:\Qt\msvc2010\setup_x86.bat

set MAKEFLAGS=
set CL=-MP
set QMAKESPEC=win32-msvc2010_s_mini
set QTDIR=C:\Qt\qt-4.8.6-msvc2010_x86_s_mini
set PATH=%PATH:qt-current=qt-4.8.6-msvc2010_x86_s_mini%
rem set PATH=%PATH:openssl-current=openssl-1.0.1g-msvc2010_x86%
rem set PATH=C:\Qt\openssl-1.0.1g-msvc2010_x86\bin;%PATH%
rem set INCLUDE=%INCLUDE%;C:\Qt\openssl-1.0.1g-msvc2010_x86\include
rem set LIB=%LIB%;C:\Qt\openssl-1.0.1g-msvc2010_x86\lib
color c
