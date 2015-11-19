@REM Copyright 2006, ZOT.

@REM Batch file to launch bash shell for ZOT IPS
@echo off

@REM Change to the cygwin/bin directory in your system
C:
chdir D:\cygwin\bin

@REM The root of the source tree.
@SET ROOT_DIR=/cygdrive/Z/ZOT716U2W_SDK_0.01

@REM Global environment variables.
@SET GCC_DIR=/cygdrive/Z/cygwin/tools/H-i686-pc-cygwin/bin

@REM eCos install folder.
@SET PKG_INSTALL_DIR=%ROOT_DIR%/ecos/ecos_install

@REM Product tree.
@SET PROD_DIR=%ROOT_DIR%/prod

@REM Apps tree.
@SET APPS_DIR=%ROOT_DIR%/apps

@REM Product name.
@SET PROD_NAME=zot716u2w

@REM Product build directory.
@SET PROD_BUILD_DIR=%PROD_DIR%/%PROD_NAME%/build

@REM Where hdr.mak and ftr.mak are
@SET HDR_MAK=%PROD_BUILD_DIR%/hdr.mak
@SET FTR_MAK=%PROD_BUILD_DIR%/ftr.mak

@SET TARGET_DEF=%PROD_BUILD_DIR%/Target.def

@REM Do not edit below this line
bash --rcfile %PROD_BUILD_DIR%/env/env.sh
