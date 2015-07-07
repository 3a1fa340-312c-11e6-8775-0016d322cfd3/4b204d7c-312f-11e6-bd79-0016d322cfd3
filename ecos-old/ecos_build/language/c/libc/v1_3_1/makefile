# eCos makefile

# This is a generated file - do not edit

export REPOSITORY := /ecos-c/ecos-1.3.1/packages
export PREFIX := /ecos-c/ZOT716U2_SDK_0.01/ecos/ecos_install
export COMMAND_PREFIX := arm-elf-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export HOST := CYGWIN
export AR := $(COMMAND_PREFIX)ar

PACKAGE := language/c/libc/v1_3_1
OBJECT_PREFIX := language_c_libc
CFLAGS := -mcpu=arm9 -Wall -Wpointer-arith -Wstrict-prototypes -Winline -Wundef -Woverloaded-virtual -g -O2 -fno-schedule-insns -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -fvtable-gc -finit-priority
LDFLAGS := -Wl,-Map,map -mcpu=arm9 -g -nostdlib -Wl,--gc-sections -Wl,-static
VPATH := $(REPOSITORY)/$(PACKAGE)
INCLUDE_PATH := $(INCLUDE_PATH) -I$(PREFIX)/include $(foreach dir,$(VPATH),-I$(dir) -I$(dir)/src -I$(dir)/tests) -I.
MLT := $(wildcard $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.ldi $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.h)
TESTS := tests/ctype/ctype tests/i18n/setlocale tests/setjmp/setjmp tests/signal/signal1 tests/signal/signal2 tests/stdio/sprintf1 tests/stdio/sprintf2 tests/stdio/sscanf tests/stdio/stdiooutput tests/stdlib/abs tests/stdlib/atexit tests/stdlib/atoi tests/stdlib/atol tests/stdlib/bsearch tests/stdlib/div tests/stdlib/getenv tests/stdlib/labs tests/stdlib/ldiv tests/stdlib/qsort tests/stdlib/malloc1 tests/stdlib/malloc2 tests/stdlib/malloc3 tests/stdlib/rand1 tests/stdlib/rand2 tests/stdlib/rand3 tests/stdlib/rand4 tests/stdlib/realloc tests/stdlib/srand tests/stdlib/strtol tests/stdlib/strtoul tests/string/memchr tests/string/memcmp1 tests/string/memcmp2 tests/string/memcpy1 tests/string/memcpy2 tests/string/memmove1 tests/string/memmove2 tests/string/memset tests/string/strcat1 tests/string/strcat2 tests/string/strchr tests/string/strcmp1 tests/string/strcmp2 tests/string/strcoll1 tests/string/strcoll2 tests/string/strcpy1 tests/string/strcpy2 tests/string/strcspn tests/string/strcspn tests/string/strlen tests/string/strncat1 tests/string/strncat2 tests/string/strncpy1 tests/string/strncpy2 tests/string/strpbrk tests/string/strrchr tests/string/strspn tests/string/strstr tests/string/strtok tests/string/strxfrm1 tests/string/strxfrm2 tests/time/asctime tests/time/clock tests/time/ctime tests/time/gmtime tests/time/localtime tests/time/mktime tests/time/strftime tests/time/time

build: headers libtarget.a.stamp

LIBRARY := libtarget.a
COMPILE := src/ctype/isalnum.cxx src/ctype/isalpha.cxx src/ctype/iscntrl.cxx src/ctype/isdigit.cxx src/ctype/isgraph.cxx src/ctype/islower.cxx src/ctype/isprint.cxx src/ctype/ispunct.cxx src/ctype/isspace.cxx src/ctype/isupper.cxx src/ctype/isxdigit.cxx src/ctype/tolower.cxx src/ctype/toupper.cxx src/errno/errno.cxx src/i18n/locale.cxx src/setjmp/longjmp.cxx src/signal/raise.cxx src/signal/siginit.cxx src/signal/signal.cxx src/stdio/common/fclose.cxx src/stdio/common/fflush.cxx src/stdio/common/fopen.cxx src/stdio/common/freopen.cxx src/stdio/common/setvbuf.cxx src/stdio/common/snprintf.cxx src/stdio/common/sprintf.cxx src/stdio/common/sscanf.cxx src/stdio/common/stderr.cxx src/stdio/common/stdin.cxx src/stdio/common/stdiofiles.cxx src/stdio/common/stdioinlines.cxx src/stdio/common/stdiosupp.cxx src/stdio/common/stdout.cxx src/stdio/common/stream.cxx src/stdio/common/streambuf.cxx src/stdio/common/ungetc.cxx src/stdio/common/vsnprintf.cxx src/stdio/common/vsscanf.cxx src/stdio/input/fgetc.cxx src/stdio/input/fgets.cxx src/stdio/input/fread.cxx src/stdio/input/fscanf.cxx src/stdio/input/gets.cxx src/stdio/input/scanf.cxx src/stdio/input/vfscanf.cxx src/stdio/output/fnprintf.cxx src/stdio/output/fprintf.cxx src/stdio/output/fputc.cxx src/stdio/output/fputs.cxx src/stdio/output/fwrite.cxx src/stdio/output/printf.cxx src/stdio/output/vfnprintf.cxx src/stdlib/_exit.cxx src/stdlib/abort.cxx src/stdlib/abs.cxx src/stdlib/atexit.cxx src/stdlib/atof.cxx src/stdlib/atoi.cxx src/stdlib/atol.cxx src/stdlib/bsearch.cxx src/stdlib/div.cxx src/stdlib/exit.cxx src/stdlib/getenv.cxx src/stdlib/itoa.cxx src/stdlib/labs.cxx src/stdlib/ldiv.cxx src/stdlib/malloc.cxx src/stdlib/qsort.cxx src/stdlib/rand.cxx src/stdlib/strtod.cxx src/stdlib/strtol.cxx src/stdlib/strtoul.cxx src/stdlib/system.cxx src/string/memchr.cxx src/string/memcmp.cxx src/string/memmove.cxx src/string/strcat.cxx src/string/strchr.cxx src/string/strcmp.cxx src/string/strcoll.cxx src/string/strcpy.cxx src/string/strcspn.cxx src/string/strlen.cxx src/string/strncat.cxx src/string/strncmp.cxx src/string/strncpy.cxx src/string/strpbrk.cxx src/string/strrchr.cxx src/string/strspn.cxx src/string/strstr.cxx src/string/strsuppt.cxx src/string/strtok.cxx src/string/strxfrm.cxx src/support/cstartup.cxx src/support/environ.cxx src/support/invokemain.cxx src/support/main.cxx src/support/mainthread.cxx src/time/asctime.cxx src/time/asctime_r.cxx src/time/clock.cxx src/time/ctime.cxx src/time/ctime_r.cxx src/time/difftime.cxx src/time/gmtime.cxx src/time/gmtime_r.cxx src/time/localtime.cxx src/time/localtime_r.cxx src/time/mktime.cxx src/time/settime.cxx src/time/strftime.cxx src/time/time.cxx src/time/timeutil.cxx
OBJECTS := $(COMPILE:.cxx=.o.d)
OBJECTS := $(OBJECTS:.c=.o.d)
OBJECTS := $(OBJECTS:.S=.o.d)

$(LIBRARY).stamp: $(OBJECTS)
	$(AR) rcs $(PREFIX)/lib/$(@:.stamp=) $(foreach obj,$?,$(dir $(obj))$(OBJECT_PREFIX)_$(notdir $(obj:.o.d=.o)))
	@cat $^ > $(@:.stamp=.deps)
	@touch $@

headers: mlt_headers $(PREFIX)/include/assert.h $(PREFIX)/include/ctype.h $(PREFIX)/include/ctype.inl $(PREFIX)/include/errno.h $(PREFIX)/include/locale.h $(PREFIX)/include/setjmp.h $(PREFIX)/include/setjmp.inl $(PREFIX)/include/signal.h $(PREFIX)/include/signal.inl $(PREFIX)/include/stdio.h $(PREFIX)/include/stdio.inl $(PREFIX)/include/stdlib.h $(PREFIX)/include/stdlib.inl $(PREFIX)/include/string.h $(PREFIX)/include/string.inl $(PREFIX)/include/time.h $(PREFIX)/include/time.inl $(PREFIX)/include/sys/cstartup.h $(PREFIX)/include/sys/timeutil.h

$(PREFIX)/include/assert.h: $(REPOSITORY)/$(PACKAGE)/include/assert.h
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ctype.h: $(REPOSITORY)/$(PACKAGE)/include/ctype.h
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ctype.inl: $(REPOSITORY)/$(PACKAGE)/include/ctype.inl
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/errno.h: $(REPOSITORY)/$(PACKAGE)/include/errno.h
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/locale.h: $(REPOSITORY)/$(PACKAGE)/include/locale.h
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/setjmp.h: $(REPOSITORY)/$(PACKAGE)/include/setjmp.h
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/setjmp.inl: $(REPOSITORY)/$(PACKAGE)/include/setjmp.inl
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/signal.h: $(REPOSITORY)/$(PACKAGE)/include/signal.h
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/signal.inl: $(REPOSITORY)/$(PACKAGE)/include/signal.inl
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/stdio.h: $(REPOSITORY)/$(PACKAGE)/include/stdio.h
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/stdio.inl: $(REPOSITORY)/$(PACKAGE)/include/stdio.inl
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/stdlib.h: $(REPOSITORY)/$(PACKAGE)/include/stdlib.h
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/stdlib.inl: $(REPOSITORY)/$(PACKAGE)/include/stdlib.inl
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/string.h: $(REPOSITORY)/$(PACKAGE)/include/string.h
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/string.inl: $(REPOSITORY)/$(PACKAGE)/include/string.inl
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/time.h: $(REPOSITORY)/$(PACKAGE)/include/time.h
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/time.inl: $(REPOSITORY)/$(PACKAGE)/include/time.inl
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/sys/cstartup.h: $(REPOSITORY)/$(PACKAGE)/include/sys/cstartup.h
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/sys/timeutil.h: $(REPOSITORY)/$(PACKAGE)/include/sys/timeutil.h
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

include $(REPOSITORY)/pkgconf/rules.mak

