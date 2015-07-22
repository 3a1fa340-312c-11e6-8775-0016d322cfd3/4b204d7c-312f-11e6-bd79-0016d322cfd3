#############################################################################
#
#       Makefile for Zero One Print Server with ARM 
#
#		LOADER.MAK
#
#############################################################################

SDIR = Z:\ZOT716U2W_SDK_0.01\loader\src
ODIR = Z:\ZOT716U2W_SDK_0.01\loader\obj
OBJS = $(ODIR)\STR8132.o	 $(ODIR)\spiflash.o	 $(ODIR)\INFLATE.o	 $(ODIR)\LOADER.o	 $(ODIR)\CRC32.o		 $(ODIR)\ALLOC.o

$(ODIR) :
	@if not exist "$(ODIR)" mkdir "$(ODIR)"

Z:\ZOT716U2W_SDK_0.01\loader\lib\LOADER.LIB : $(ODIR) $(LIB_DIR) $(OBJS)
	@ECHO Generating LOADER.LIB...
	$(LIB) -v <<
	-c $@ $(OBJS)
<<

{$(SDIR)}.s{$(ODIR)}.o :
	@ECHO %|fF%|eF
	$(ASM) -Errors E -via <<
	$(S_FLAGS) $< -o $@
<<

{$(SDIR)}.c{$(ODIR)}.o :
	@ECHO %|fF%|eF
	$(CC) -Errors E -via <<
	$(C_FLAGS) -O2 $(CODE_TYPE_FLAG) $< -o $@
<<

