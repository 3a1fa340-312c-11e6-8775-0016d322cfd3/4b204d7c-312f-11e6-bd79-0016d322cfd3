# eCos makefile

# This is a generated file - do not edit

export REPOSITORY := /ecos-c/ecos-1.3.1/packages
export HOST := CYGWIN
export PREFIX := /ecos-c/ZOT716U2_SDK_0.01/ecos/ecos_install
export COMMAND_PREFIX := arm-elf-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export AR := $(COMMAND_PREFIX)ar

.PHONY: default build clean tests headers

build: headers $(PREFIX)/include/pkgconf/ecos.mak
	$(MAKE) -r -C hal/arm/arch/v1_3_1 arm.inc
	$(MAKE) -r -C hal/common/v1_3_1 $@
	$(MAKE) -r -C io/common/v1_3_1 $@
	$(MAKE) -r -C infra/v1_3_1 $@
	$(MAKE) -r -C kernel/v1_3_1 $@
	$(MAKE) -r -C language/c/libc/v1_3_1 $@
	$(MAKE) -r -C language/c/libm/v1_3_1 $@
	$(MAKE) -r -C devs/wallclock/v1_3_1 $@
	$(MAKE) -r -C error/v1_3_1 $@
	$(MAKE) -r -C hal/arm/arch/v1_3_1 $@
	$(MAKE) -r -C hal/arm/zot716u2/v1_3_1 $@
	$(MAKE) -r -C io/serial/v1_3_1 $@
	$(MAKE) -r -C hal/common/v1_3_1 $(PREFIX)/lib/extras.o
	$(MAKE) -r -C hal/arm/arch/v1_3_1 $(PREFIX)/lib/vectors.o
	$(MAKE) -r -C hal/arm/arch/v1_3_1 $(PREFIX)/lib/target.ld
	@echo $@ finished

clean:
	$(MAKE) -r -C hal/common/v1_3_1 $@
	$(MAKE) -r -C io/common/v1_3_1 $@
	$(MAKE) -r -C infra/v1_3_1 $@
	$(MAKE) -r -C kernel/v1_3_1 $@
	$(MAKE) -r -C language/c/libc/v1_3_1 $@
	$(MAKE) -r -C language/c/libm/v1_3_1 $@
	$(MAKE) -r -C devs/wallclock/v1_3_1 $@
	$(MAKE) -r -C error/v1_3_1 $@
	$(MAKE) -r -C hal/arm/arch/v1_3_1 $@
	$(MAKE) -r -C hal/arm/zot716u2/v1_3_1 $@
	$(MAKE) -r -C io/serial/v1_3_1 $@
	@echo $@ finished

tests: build
	$(MAKE) -r -C hal/common/v1_3_1 $@
	$(MAKE) -r -C io/common/v1_3_1 $@
	$(MAKE) -r -C infra/v1_3_1 $@
	$(MAKE) -r -C kernel/v1_3_1 $@
	$(MAKE) -r -C language/c/libc/v1_3_1 $@
	$(MAKE) -r -C language/c/libm/v1_3_1 $@
	$(MAKE) -r -C devs/wallclock/v1_3_1 $@
	$(MAKE) -r -C error/v1_3_1 $@
	$(MAKE) -r -C hal/arm/arch/v1_3_1 $@
	$(MAKE) -r -C hal/arm/zot716u2/v1_3_1 $@
	$(MAKE) -r -C io/serial/v1_3_1 $@
	@echo $@ finished

headers:
	$(MAKE) -r -C hal/common/v1_3_1 $@
	$(MAKE) -r -C io/common/v1_3_1 $@
	$(MAKE) -r -C infra/v1_3_1 $@
	$(MAKE) -r -C kernel/v1_3_1 $@
	$(MAKE) -r -C language/c/libc/v1_3_1 $@
	$(MAKE) -r -C language/c/libm/v1_3_1 $@
	$(MAKE) -r -C devs/wallclock/v1_3_1 $@
	$(MAKE) -r -C error/v1_3_1 $@
	$(MAKE) -r -C hal/arm/arch/v1_3_1 $@
	$(MAKE) -r -C hal/arm/zot716u2/v1_3_1 $@
	$(MAKE) -r -C io/serial/v1_3_1 $@
	@echo $@ finished

$(PREFIX)/include/pkgconf/ecos.mak: makefile
	@echo 'ECOS_GLOBAL_CFLAGS = -mcpu=arm9 -Wall -Wpointer-arith -Wstrict-prototypes -Winline -Wundef -Woverloaded-virtual -g -O2 -fno-schedule-insns -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -fvtable-gc -finit-priority' > $@
	@echo 'ECOS_GLOBAL_LDFLAGS = -Wl,-Map,map -mcpu=arm9 -g -nostdlib -Wl,--gc-sections -Wl,-static' >> $@
	@echo 'ECOS_COMMAND_PREFIX = $(COMMAND_PREFIX)' >> $@

