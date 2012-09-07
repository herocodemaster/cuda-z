#	\file pkg-win32.nsi
#	\brief Windows package generator.
#	\author Andriy Golovnya <andrew_golovnia@ukr.net> http://ag.embedded.org.ru/
#	\url http://cuda-z.sf.net/ http://sf.net/projects/cuda-z/
#	\license GPLv2 http://www.gnu.org/licenses/gpl-2.0.html

!define CZ_CUDART_DLL cudart32_42_9.dll
!define CZ_CUDAZ_EXE cuda-z.exe

!system "perl -pe $\"s/#/!/g$\" < src/build.h > build.nsi"
!system "perl -pe $\"s/#/!/g$\" < src/version.h | perl -pe $\"s/build.h/build.nsi/g$\" > version.nsi"
#!system "cd tmp\cert && signfile.bat ..\..\bin\${CZ_CUDAZ_EXE}"
!include version.nsi
!delfile build.nsi
!delfile version.nsi

!ifndef CZ_VER_BUILD
!define CZ_VER_BUILD "0"
!endif

!ifndef CZ_VER_STATE
!define CZ_VER_STATE ""
!endif

!define VERSION_NUM "${CZ_VER_STRING}.${CZ_VER_BUILD}"
!define VERSION_STR "${CZ_VER_STRING}.${CZ_VER_BUILD} ${CZ_VER_STATE}"
!define NAME "${CZ_NAME_SHORT} Container"

Name "${NAME} ${VERSION_STR}"
Icon res\img\icon.ico 
OutFile "${CZ_NAME_SHORT}-${VERSION_NUM}.exe"
SetCompressor /SOLID lzma
SetCompressorDictSize 8

LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"
VIProductVersion "${VERSION_NUM}.0"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "${CZ_ORG_NAME}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "${NAME}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${VERSION_STR}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "${CZ_COPY_INFO}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${CZ_NAME_SHORT}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${VERSION_STR}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "Comments" "Smart Container for ${CZ_NAME_SHORT}"

Function .onInit
	SetSilent silent
FunctionEnd

Section "-Go"
	GetTempFileName $OUTDIR
	Delete $OUTDIR
	CreateDirectory $OUTDIR
	SetOutPath $OUTDIR
	File /oname=${CZ_CUDAZ_EXE} bin\${CZ_CUDAZ_EXE}
	File /oname=${CZ_CUDART_DLL} bin\${CZ_CUDART_DLL}

	ExecWait '"$OUTDIR\${CZ_CUDAZ_EXE}"'

	Delete $OUTDIR\${CZ_CUDAZ_EXE}
	IfFileExists $OUTDIR\${CZ_CUDAZ_EXE} -1 0
	Delete $OUTDIR\${CZ_CUDART_DLL}
	IfFileExists $OUTDIR\${CZ_CUDART_DLL} -1 0
	SetOutPath $TEMP
	RMDir $OUTDIR
SectionEnd