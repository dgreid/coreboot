##
## This file is part of the coreboot project.
##
## Copyright (C) 2014 The ChromiumOS Authors.  All rights reserved.
## Copyright (C) 2018 Siemens AG
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; version 2 of the License.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##

ifeq ($(CONFIG_VBOOT),y)

bootblock-y += bootmode.c
romstage-y += bootmode.c
ramstage-y += bootmode.c
verstage-y += bootmode.c
postcar-y += bootmode.c

# When VBOOT_STARTS_IN_ROMSTAGE is selected, DRAM is already up by
# the time verstage runs.
ifneq ($(CONFIG_VBOOT_STARTS_IN_ROMSTAGE),y)
verstage-generic-ccopts += -D__PRE_RAM__
endif

verstage-generic-ccopts += -D__VERSTAGE__

ramstage-y += gbb.c

bootblock-y += vbnv.c
verstage-y += vbnv.c
romstage-y += vbnv.c
ramstage-y += vbnv.c

bootblock-$(CONFIG_VBOOT_VBNV_CMOS) += vbnv_cmos.c
verstage-$(CONFIG_VBOOT_VBNV_CMOS) += vbnv_cmos.c
romstage-$(CONFIG_VBOOT_VBNV_CMOS) += vbnv_cmos.c
ramstage-$(CONFIG_VBOOT_VBNV_CMOS) += vbnv_cmos.c

bootblock-$(CONFIG_VBOOT_VBNV_CMOS_BACKUP_TO_FLASH) += vbnv_flash.c
verstage-$(CONFIG_VBOOT_VBNV_CMOS_BACKUP_TO_FLASH) += vbnv_flash.c
romstage-$(CONFIG_VBOOT_VBNV_CMOS_BACKUP_TO_FLASH) += vbnv_flash.c
ramstage-$(CONFIG_VBOOT_VBNV_CMOS_BACKUP_TO_FLASH) += vbnv_flash.c

bootblock-$(CONFIG_VBOOT_VBNV_EC) += vbnv_ec.c
verstage-$(CONFIG_VBOOT_VBNV_EC) += vbnv_ec.c
romstage-$(CONFIG_VBOOT_VBNV_EC) += vbnv_ec.c
ramstage-$(CONFIG_VBOOT_VBNV_EC) += vbnv_ec.c

bootblock-$(CONFIG_VBOOT_VBNV_FLASH) += vbnv_flash.c
verstage-$(CONFIG_VBOOT_VBNV_FLASH) += vbnv_flash.c
romstage-$(CONFIG_VBOOT_VBNV_FLASH) += vbnv_flash.c
ramstage-$(CONFIG_VBOOT_VBNV_FLASH) += vbnv_flash.c

bootblock-y += vboot_loader.c
romstage-y += vboot_loader.c
ramstage-y += vboot_loader.c
verstage-y += vboot_loader.c
postcar-y += vboot_loader.c

bootblock-y += vboot_common.c
verstage-y += vboot_common.c
romstage-y += vboot_common.c
ramstage-y += vboot_common.c
postcar-y += vboot_common.c

ifeq ($(CONFIG_VBOOT_MEASURED_BOOT),y)
bootblock-y += vboot_crtm.c
verstage-y += vboot_crtm.c
romstage-y += vboot_crtm.c
ramstage-y += vboot_crtm.c
postcar-y += vboot_crtm.c
endif

bootblock-y += common.c
verstage-y += vboot_logic.c
verstage-y += common.c
verstage-$(CONFIG_VBOOT_SEPARATE_VERSTAGE) += verstage.c
ifeq (${CONFIG_VBOOT_MOCK_SECDATA},y)
verstage-y += secdata_mock.c
romstage-$(CONFIG_VBOOT_SEPARATE_VERSTAGE) += secdata_mock.c
else
verstage-y += secdata_tpm.c
romstage-$(CONFIG_VBOOT_SEPARATE_VERSTAGE) += secdata_tpm.c
endif

ifneq ($(CONFIG_TPM1)$(CONFIG_TPM2),)
verstage-y += tpm_common.c
endif

romstage-y += vboot_logic.c
romstage-y += common.c

ramstage-y += common.c
postcar-y += common.c

romstage-$(CONFIG_FSP2_0_USES_TPM_MRC_HASH) += mrc_cache_hash_tpm.c

vboot-fixup-includes = $(patsubst -I%,-I$(top)/%,\
		       $(patsubst $(src)/%.h,$(top)/$(src)/%.h,\
		       $(filter-out -I$(obj),$(1))))

# call with $1 = stage name to create rules for building the library
# for the stage and adding it to the stage's set of object files.
define vboot-for-stage
VB2_LIB_$(1) = $(obj)/external/vboot_reference-$(1)/vboot_fw20.a
VBOOT_CFLAGS_$(1) += $$(call vboot-fixup-includes,$$(CPPFLAGS_$(1)))
VBOOT_CFLAGS_$(1) += $$(CFLAGS_$(1))
VBOOT_CFLAGS_$(1) += $$(call vboot-fixup-includes,$$($(1)-c-ccopts))
VBOOT_CFLAGS_$(1) += -I$(abspath $(obj)) -Wno-missing-prototypes
VBOOT_CFLAGS_$(1) += -DVBOOT_DEBUG

$$(VB2_LIB_$(1)): $(obj)/config.h
	printf "    MAKE       $(subst $(obj)/,,$(@))\n"
	+FIRMWARE_ARCH=$$(ARCHDIR-$$(ARCH-$(1)-y)) \
	CC="$$(CC_$(1))" \
	CFLAGS="$$(VBOOT_CFLAGS_$(1))" VBOOT2="y" \
	$(MAKE) -C $(VBOOT_SOURCE) \
		BUILD=$$(abspath $$(dir $$(VB2_LIB_$(1)))) \
		V=$(V) \
		fwlib20

$(1)-srcs += $$(VB2_LIB_$(1))

endef # vboot-for-stage

CFLAGS_common += -I3rdparty/vboot/firmware/2lib/include

$(eval $(call vboot-for-stage,bootblock))
$(eval $(call vboot-for-stage,romstage))
$(eval $(call vboot-for-stage,ramstage))
$(eval $(call vboot-for-stage,postcar))

ifeq ($(CONFIG_VBOOT_SEPARATE_VERSTAGE),y)

$(eval $(call vboot-for-stage,verstage))

cbfs-files-$(CONFIG_VBOOT_SEPARATE_VERSTAGE) += $(CONFIG_CBFS_PREFIX)/verstage
$(CONFIG_CBFS_PREFIX)/verstage-file := $(objcbfs)/verstage.elf
$(CONFIG_CBFS_PREFIX)/verstage-type := stage
$(CONFIG_CBFS_PREFIX)/verstage-compression := $(CBFS_PRERAM_COMPRESS_FLAG)

ifeq ($(CONFIG_ARCH_VERSTAGE_X86_32)$(CONFIG_ARCH_VERSTAGE_X86_64),y)
$(CONFIG_CBFS_PREFIX)/verstage-options := -a 64 -S ".car.data"

# If CAR does not support execution of code, verstage on x86 is expected to be
# xip.
ifneq ($(CONFIG_NO_XIP_EARLY_STAGES),y)
$(CONFIG_CBFS_PREFIX)/verstage-options += --xip
endif

endif

else # CONFIG_VBOOT_SEPARATE_VERSTAGE
ifeq ($(CONFIG_VBOOT_STARTS_IN_BOOTBLOCK),y)
postinclude-hooks += $$(eval bootblock-srcs += $$(verstage-srcs))
else
postinclude-hooks += $$(eval romstage-srcs += $$(verstage-srcs))
endif
endif # CONFIG_VBOOT_SEPARATE_VERSTAGE

#RO-Partition is always there!
VBOOT_PARTITIONS := COREBOOT
# Check for RW_A partition
ifeq ($(CONFIG_VBOOT_SLOTS_RW_A),y)
VBOOT_PARTITIONS += FW_MAIN_A
endif
# Check for RW_B partition
ifeq ($(CONFIG_VBOOT_SLOTS_RW_AB),y)
VBOOT_PARTITIONS += FW_MAIN_B
endif

# Define a list of files that need to be in RO only.
# All other files will be installed into RO and RW regions
# Use $(sort) to cut down on extra spaces that would be translated to commas
regions-for-file = $(subst $(spc),$(comma),$(sort \
	$(if $(filter \
		$(if $(filter y,$(CONFIG_VBOOT_STARTS_IN_BOOTBLOCK)),, \
			%/romstage) \
		mts \
		%/verstage \
		locales \
		locale_%.bin \
		font.bin \
		vbgfx.bin \
		rmu.bin \
		cmos_layout.bin \
		cmos.default \
		$(call strip_quotes,$(CONFIG_RO_REGION_ONLY)) \
		,$(1)),COREBOOT,$(VBOOT_PARTITIONS))))

CONFIG_GBB_HWID := $(call strip_quotes,$(CONFIG_GBB_HWID))
CONFIG_GBB_BMPFV_FILE := $(call strip_quotes,$(CONFIG_GBB_BMPFV_FILE))
CONFIG_VBOOT_KEYBLOCK := $(call strip_quotes,$(CONFIG_VBOOT_KEYBLOCK))
CONFIG_VBOOT_FIRMWARE_PRIVKEY := $(call strip_quotes,$(CONFIG_VBOOT_FIRMWARE_PRIVKEY))
CONFIG_VBOOT_KERNEL_KEY := $(call strip_quotes,$(CONFIG_VBOOT_KERNEL_KEY))
CONFIG_VBOOT_FWID_MODEL := $(call strip_quotes,$(CONFIG_VBOOT_FWID_MODEL))
CONFIG_VBOOT_FWID_VERSION := $(call strip_quotes,$(CONFIG_VBOOT_FWID_VERSION))

# bool-to-mask(var, value)
# return "value" if var is "y", 0 otherwise
bool-to-mask = $(if $(filter y,$(1)),$(2),0)

GBB_FLAGS := $(call int-add, \
	$(call bool-to-mask,$(CONFIG_GBB_FLAG_DEV_SCREEN_SHORT_DELAY),0x1) \
	$(call bool-to-mask,$(CONFIG_GBB_FLAG_LOAD_OPTION_ROMS),0x2) \
	$(call bool-to-mask,$(CONFIG_GBB_FLAG_ENABLE_ALTERNATE_OS),0x4) \
	$(call bool-to-mask,$(CONFIG_GBB_FLAG_FORCE_DEV_SWITCH_ON),0x8) \
	$(call bool-to-mask,$(CONFIG_GBB_FLAG_FORCE_DEV_BOOT_USB),0x10) \
	$(call bool-to-mask,$(CONFIG_GBB_FLAG_DISABLE_FW_ROLLBACK_CHECK),0x20) \
	$(call bool-to-mask,$(CONFIG_GBB_FLAG_ENTER_TRIGGERS_TONORM),0x40) \
	$(call bool-to-mask,$(CONFIG_GBB_FLAG_FORCE_DEV_BOOT_LEGACY),0x80) \
	$(call bool-to-mask,$(CONFIG_GBB_FLAG_FAFT_KEY_OVERIDE),0x100) \
	$(call bool-to-mask,$(CONFIG_GBB_FLAG_DISABLE_EC_SOFTWARE_SYNC),0x200) \
	$(call bool-to-mask,$(CONFIG_GBB_FLAG_DEFAULT_DEV_BOOT_LEGACY),0x400) \
	$(call bool-to-mask,$(CONFIG_GBB_FLAG_DISABLE_PD_SOFTWARE_SYNC),0x800) \
	$(call bool-to-mask,$(CONFIG_GBB_FLAG_DISABLE_LID_SHUTDOWN),0x1000) \
	$(call bool-to-mask,$(CONFIG_GBB_FLAG_FORCE_DEV_BOOT_FASTBOOT_FULL_CAP),0x2000) \
	$(call bool-to-mask,$(CONFIG_GBB_FLAG_FORCE_MANUAL_RECOVERY),0x4000) \
	$(call bool-to-mask,$(CONFIG_GBB_FLAG_DISABLE_FWMP),0x8000) \
	)

ifneq ($(CONFIG_GBB_BMPFV_FILE),)
$(obj)/gbb.sizetmp: $(obj)/coreboot.rom
	$(CBFSTOOL) $< read -r GBB -f $@

$(obj)/gbb.stub: $(obj)/coreboot.rom $(FUTILITY) $(obj)/gbb.sizetmp
	@printf "    CREATE GBB (with BMPFV)\n"
	$(FUTILITY) gbb_utility -c 0x100,0x1000,$(call int-subtract,$(call file-size,$(obj)/gbb.sizetmp) 0x2180),0x1000 $@.tmp
	mv $@.tmp $@
else
$(obj)/gbb.stub: $(obj)/coreboot.rom $(FUTILITY)
	@printf "    CREATE GBB (without BMPFV)\n"
	$(FUTILITY) gbb_utility -c 0x100,0x1000,0,0x1000 $@.tmp
	mv $@.tmp $@
endif

$(obj)/gbb.region: $(obj)/gbb.stub
	@printf "    SETUP GBB\n"
	cp $< $@.tmp
	$(FUTILITY) gbb_utility -s \
		--hwid="$(CONFIG_GBB_HWID)" \
		--rootkey="$(CONFIG_VBOOT_ROOT_KEY)" \
		--recoverykey="$(CONFIG_VBOOT_RECOVERY_KEY)" \
		--flags=$(GBB_FLAGS) \
		$@.tmp
ifneq ($(CONFIG_GBB_BMPFV_FILE),)
	$(FUTILITY) gbb_utility -s \
		--bmpfv="$(CONFIG_GBB_BMPFV_FILE)" \
		$@.tmp
endif
	mv $@.tmp $@

$(obj)/fwid.version:
	echo -n "$(CONFIG_VBOOT_FWID_VERSION)" > $@

$(obj)/fwid.region: $(obj)/fwid.version
	printf "%s%s\0" \
		"$(CONFIG_VBOOT_FWID_MODEL)" \
		"$$(cat "$(obj)/fwid.version")" > $@

build_complete:: $(obj)/gbb.region $(obj)/fwid.region
	@printf "    WRITE GBB\n"
	$(CBFSTOOL) $(obj)/coreboot.rom write -u -r GBB -i 0 -f $(obj)/gbb.region
	$(CBFSTOOL) $(obj)/coreboot.rom write -u -r RO_FRID -i 0 -f $(obj)/fwid.region
ifeq ($(CONFIG_VBOOT_SLOTS_RW_A),y)
	$(CBFSTOOL) $(obj)/coreboot.rom write -u -r RW_FWID_A -i 0 -f $(obj)/fwid.region
endif
ifeq ($(CONFIG_VBOOT_SLOTS_RW_AB),y)
	$(CBFSTOOL) $(obj)/coreboot.rom write -u -r RW_FWID_B -i 0 -f $(obj)/fwid.region
endif

ifneq ($(shell grep "SHARED_DATA" "$(CONFIG_FMDFILE)"),)
build_complete::
	printf "\0" > $(obj)/shared_data.region
	$(CBFSTOOL) $(obj)/coreboot.rom write -u -r SHARED_DATA -i 0 -f $(obj)/shared_data.region
endif

# Extract FW_MAIN_? region and minimize it if the last file is empty, so it
# doesn't contain this empty file (that can have a significant size),
# improving a lot on hash times due to a smaller amount of data loaded from
# firmware storage.
# When passing the minimized image to vbutil_firmware, its length is recorded
# in the keyblock, and coreboot's vboot code clips the region_device to match,
# which prevents any potential extension attacks.
$(obj)/FW_MAIN_%.bin: $(obj)/coreboot.rom
	$(CBFSTOOL) $< truncate -r $(basename $(notdir $@)) > $@.tmp.size
	$(CBFSTOOL) $< read -r $(basename $(notdir $@)) -f $@.tmp
	head -c $$( printf "%d" $$(cat $@.tmp.size)) $@.tmp > $@.tmp2
	mv $@.tmp2 $@
	rm -f $@.tmp $@.tmp.size

$(obj)/VBLOCK_%.bin: $(obj)/FW_MAIN_%.bin $(FUTILITY)
	$(FUTILITY) vbutil_firmware \
		--vblock $@ \
		--keyblock "$(CONFIG_VBOOT_KEYBLOCK)" \
		--signprivate "$(CONFIG_VBOOT_FIRMWARE_PRIVKEY)" \
		--version $(CONFIG_VBOOT_KEYBLOCK_VERSION) \
		--fv $< \
		--kernelkey "$(CONFIG_VBOOT_KERNEL_KEY)" \
		--flags $(CONFIG_VBOOT_KEYBLOCK_PREAMBLE_FLAGS)

ifeq ($(CONFIG_VBOOT_SLOTS_RW_AB),y)
files_added:: $(obj)/VBLOCK_A.bin $(obj)/VBLOCK_B.bin
	$(CBFSTOOL) $(obj)/coreboot.rom write -u -r VBLOCK_A -f $(obj)/VBLOCK_A.bin
	$(CBFSTOOL) $(obj)/coreboot.rom write -u -r VBLOCK_B -f $(obj)/VBLOCK_B.bin
	@printf "    FLASHMAP Layout generated for RO, A and B partition.\n"
else ifeq ($(CONFIG_VBOOT_SLOTS_RW_A),y)
files_added:: $(obj)/VBLOCK_A.bin
	$(CBFSTOOL) $(obj)/coreboot.rom write -u -r VBLOCK_A -f $(obj)/VBLOCK_A.bin
	@printf "    FLASHMAP Layout generated for RO and A partition.\n"
else
files_added::
	@printf "    FLASHMAP Layout generated for RO partition only.\n"
	@printf "             Beware that there is no failure safety in case of update now!\n"
endif

endif # CONFIG_VBOOT
