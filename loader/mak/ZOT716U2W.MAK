#############################################################################
#
#       Makefile for Zero One Print Server with ARM 
#
#		This kernal base on KA9Q TCP/IP
#
#############################################################################

!INCLUDE PATH.DEF

CC					= @C:\ARM251\BIN\ARMCC
ASM					= @C:\ARM251\BIN\ARMASM
LIB					= @C:\ARM251\BIN\ARMLIB
LINK				= @C:\ARM251\BIN\ARMLINK

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF

.SUFFIXES: .c .s

#############################################################################
#
# BUILD OPTIONS
#
#############################################################################

CPU_OPTIONS			= -processor ARM922 -arch 4T

ARMLIB				= C:\ARM251\LIB

#if you take off -DCONST_DATA, you must be check httpd.c diefin 

C_DEFINED			= -D__ARMC__ -DEX_UCLK -D$(TARGET) -DCONST_DATA -DLPD_TXT -DIPPD -DSNMPIPX -DWEBADMIN -DTELNETD -DNDS_PS -DATALKD -DSNMPD -DMIB -DHTTPD -DUDP_ALL -DNTUDP -DNETBEUI -DUNIXUTIL -DUNIX_PS -DNOVELL_PS -DWINDOWS_PS -DRAWTCPD -DMSDOS -DTCP_ALL
# -DGARBAGE_COLLECT -DHW_DEBUG

C_FLAGS				= -li -g -c -fc -apcs 3/32bit -Efpzc -za1 -zas1 $(C_DEFINED) $(CPU_OPTIONS)
S_FLAGS				= -li -g -apcs 3/32bit -PD "ROM_AT_ADDRESS_ZERO SETL {TRUE}" -PD "$(TARGET) SETL {TRUE}"

CODE_TYPE_FLAG		= -DLOADER
MEM_OPTIONS			= -RO 0x0 -RW 0x207E0000
S_FLAGS				= $(S_FLAGS) -PD "LOADER SETL {TRUE}"

#############################################################################
#
# TARGET SPECIAL OPTIONS
#
#############################################################################

MAIN_LIBS = \
    $(LIB_DIR)\LOADER.LIB       \
	$(OBJ_DIR)\INIT.o

ALL : $(BIN_DIR)\$(TARGET).AXF  $(BIN_DIR)\$(TARGET).BIN

CLEAN : 
	@-erase $(OBJ_DIR)\*.o /S /A
	@-if exist $(LIB_DIR)\*.LIB erase $(LIB_DIR)\*.LIB /A
	@-rd $(OBJ_DIR) /S /Q

#############################################################################
# $(LIB_DIR)
#############################################################################

$(LIB_DIR) : 
    @if not exist "$(LIB_DIR)/$(NULL)" mkdir "$(LIB_DIR)"

#############################################################################
# $(BIN_DIR)
#############################################################################

$(BIN_DIR) : 
    @if not exist "$(BIN_DIR)/$(NULL)" mkdir "$(BIN_DIR)"

#############################################################################
# LIBRARYS
#############################################################################

!INCLUDE LOADER.MAK


#############################################################################
# BIN & AXF
#############################################################################

$(BIN_DIR)\$(TARGET).BIN : $(BIN_DIR) $(MAIN_LIBS)
	@ECHO Linking...
	$(LINK) -via <<
	-first INIT.o(Init) -bin -nozeropad $(MEM_OPTIONS)
	-Errors E
	-Symbols $*.SYM
	-map -list $*.LST
	-Output $@ 
	$(MAIN_LIBS)
<<

$(BIN_DIR)\$(TARGET).AXF : $(BIN_DIR) $(CFG_FILE) $(MAIN_LIBS)
	@ECHO Linking...
	$(LINK) -via <<
	-first INIT.o(Init) -elf -debug -nozeropad $(MEM_OPTIONS)
	-Errors E
	-Output $@ 
	$(MAIN_LIBS)
<<

