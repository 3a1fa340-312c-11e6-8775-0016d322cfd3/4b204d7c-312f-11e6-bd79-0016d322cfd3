@echo off
@echo *************************
@echo * DELETE TEMP DIRECTORY *
@echo *************************
..\deltree /Y ..\temp
MD ..\TEMP

:BUILD_TMP
@echo ****************************************
@echo * Parser %2 HTML to TEMP DIRECTORY
@echo ****************************************
..\exe\2DHTML.EXE ..\HTTP.CFG %2 ..\temp

if NOT ERRORLEVEL 1 goto CREATE_WEB_VER
@echo BUILD TEMPLET %2 ERROR !
goto END

:CREATE_WEB_VER
@echo ********************
@echo * WEB VERSION=2
@echo ********************
..\exe\webver1.exe 2 %1 %3

if NOT ERRORLEVEL 1 goto WWW_AUTH
@echo CREATE WEB VERSION FILE ERROR !
goto END

:WWW_AUTH
@echo ************************
@echo * BUILD AUTH FILE LIST *
@echo ************************
..\exe\WebAuth %2\WebAuth.TXT AUTH.TMP

if NOT ERRORLEVEL 1 goto WWW_MSG
@echo BUILD AUTH FILE ERROR !
goto END

:WWW_MSG
@echo ***************************
@echo * BUILD HTTP MESSAGE FILE *
@echo ***************************
..\exe\WebMsg %2\WebMsg.TXT MSG.TMP

if NOT ERRORLEVEL 1 goto WWW_IMAGE
@echo BUILD WWW MESSAGE ERROR !
goto END

:WWW_IMAGE
@echo *************************
@echo * BUILD HTTP IMAGE FILE *
@echo *************************
..\exe\W3TOBIN WWW.TMP ..\temp

if NOT ERRORLEVEL 1 goto WWW_MSG_IMAGE
@echo BUILD WWW IMAGE ERROR !
goto END

:WWW_MSG_IMAGE
@echo ***************************
@echo * Combine all *.TMP to %3
@echo ***************************
copy %3/b+AUTH.TMP/b+MSG.TMP+WWW.TMP/b  %3
del AUTH.TMP 
del MSG.TMP  
del WWW.TMP  

:SUCCESS
@echo "<<<HTTP IMAGE FILE = %3 ! SUCCESS >>>"

@echo ***************************
@echo * MKWEB %3 to %4
@echo ***************************
..\exe\mkweb %3 %4 %5

@echo ***************************
@echo * MAKECFG %3 to ..\ENDMARK.C
@echo ***************************
..\exe\MAKECFG %3 ..\ENDMARK.C

cd ..

goto END1

:END
del %3
@echo Please check error !!!

:END1