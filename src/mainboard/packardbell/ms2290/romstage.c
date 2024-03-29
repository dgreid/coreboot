/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007-2009 coresystems GmbH
 * Copyright (C) 2011 Sven Schnelle <svens@stackframe.org>
 * Copyright (C) 2013 Vladimir Serbinenko <phcoder@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdint.h>
#include <arch/io.h>
#include <device/pci_ops.h>
#include <device/pci_def.h>
#include <cpu/x86/lapic.h>
#include <romstage_handoff.h>
#include <console/console.h>
#include <cpu/intel/romstage.h>
#include <ec/acpi/ec.h>
#include <timestamp.h>
#include <arch/acpi.h>

#include <southbridge/intel/ibexpeak/pch.h>
#include <northbridge/intel/nehalem/nehalem.h>

#include <northbridge/intel/nehalem/raminit.h>
#include <southbridge/intel/ibexpeak/me.h>

static void pch_enable_lpc(void)
{
	/* Enable EC, PS/2 Keyboard/Mouse */
	pci_write_config16(PCH_LPC_DEV, LPC_EN,
			   CNF2_LPC_EN | CNF1_LPC_EN | MC_LPC_EN | KBC_LPC_EN |
			   COMA_LPC_EN);

	pci_write_config32(PCH_LPC_DEV, LPC_GEN1_DEC, (0x68 & ~3) | 0x00040001);

	pci_write_config16(PCH_LPC_DEV, LPC_IO_DEC, 0x10);

	pci_write_config32(PCH_LPC_DEV, 0xd0, 0x0);
	pci_write_config32(PCH_LPC_DEV, 0xdc, 0x8);

	pci_write_config8(PCH_LPC_DEV, GEN_PMCON_3,
			  (pci_read_config8(PCH_LPC_DEV, GEN_PMCON_3) & ~2) | 1);

	pci_write_config32(PCH_LPC_DEV, ETR3,
			   pci_read_config32(PCH_LPC_DEV, ETR3) & ~ETR3_CF9GR);
}

static void rcba_config(void)
{
	southbridge_configure_default_intmap();

	static const u32 rcba_dump3[] = {
		/* 3310 */ 0x02060100, 0x0000000f, 0x01020000, 0x80000000,
		/* 3320 */ 0x00000000, 0x04000000, 0x00000000, 0x00000000,
		/* 3330 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3340 */ 0x000fffff, 0x00000000, 0x00000000, 0x00000000,
		/* 3350 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3360 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3370 */ 0x00000000, 0x00000000, 0x7f8fdfff, 0x00000000,
		/* 3380 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3390 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 33a0 */ 0x00003900, 0x00000000, 0x00000000, 0x00000000,
		/* 33b0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 33c0 */ 0x00010000, 0x00000000, 0x00000000, 0x0001004b,
		/* 33d0 */ 0x06000008, 0x00010000, 0x00000000, 0x00000000,
		/* 33e0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 33f0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3400 */ 0x0000001c, 0x00000080, 0x00000000, 0x00000000,
		/* 3410 */ 0x00000c61, 0x00000000, 0x16fc1fe1, 0xbf4f001f,
		/* 3420 */ 0x00000000, 0x00060010, 0x0000001d, 0x00000000,
		/* 3430 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3440 */ 0xdeaddeed, 0x00000000, 0x00000000, 0x00000000,
		/* 3450 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3460 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3470 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3480 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3490 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 34a0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 34b0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 34c0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 34d0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 34e0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 34f0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3500 */ 0x20000557, 0x2000055f, 0x2000074b, 0x2000074b,
		/* 3510 */ 0x20000557, 0x2000014b, 0x2000074b, 0x2000074b,
		/* 3520 */ 0x2000074b, 0x2000074b, 0x2000055f, 0x2000055f,
		/* 3530 */ 0x20000557, 0x2000055f, 0x00000000, 0x00000000,
		/* 3540 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3550 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3560 */ 0x00000001, 0x000026a3, 0x00040002, 0x01000052,
		/* 3570 */ 0x02000772, 0x16000f8f, 0x1800ff4f, 0x0001d630,
		/* 3580 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3590 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 35a0 */ 0xfc000201, 0x3c000201, 0x00000000, 0x00000000,
		/* 35b0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 35c0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 35d0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 35e0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 35f0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3600 */ 0x0a001f00, 0x00000000, 0x00000000, 0x00000001,
		/* 3610 */ 0x00010000, 0x00000000, 0x00000000, 0x00000000,
		/* 3620 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3630 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3640 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3650 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3660 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3670 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3680 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3690 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 36a0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 36b0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 36c0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 36d0 */ 0x00000000, 0x089c0018, 0x00000000, 0x00000000,
		/* 36e0 */ 0x11111111, 0x00000000, 0x00000000, 0x00000000,
		/* 36f0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3700 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3710 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		/* 3720 */ 0x00000000, 0x4e564d49, 0x00000000, 0x00000000,
	};
	unsigned i;

	for (i = 0; i < sizeof(rcba_dump3) / 4; i++) {
		RCBA32(4 * i + 0x3310) = rcba_dump3[i];
		(void)RCBA32(4 * i + 0x3310);
	}
}

static inline void write_acpi32(u32 addr, u32 val)
{
	outl(val, DEFAULT_PMBASE | addr);
}

static inline void write_acpi16(u32 addr, u16 val)
{
	outw(val, DEFAULT_PMBASE | addr);
}

static inline u32 read_acpi32(u32 addr)
{
	return inl(DEFAULT_PMBASE | addr);
}

// unused func - used for RE
#if 0
static inline u16 read_acpi16(u32 addr)
{
	return inw(DEFAULT_PMBASE | addr);
}
#endif

void mainboard_romstage_entry(void)
{
	u32 reg32;
	int s3resume = 0;
	const u8 spd_addrmap[4] = { 0x50, 0, 0x52, 0 };

	/* SERR pin is confused on reset. Clear NMI.  */
	outb(4, 0x61);
	outb(0, 0x61);

	enable_lapic();

	nehalem_early_initialization(NEHALEM_MOBILE);

	pch_enable_lpc();

	/* Enable GPIOs */
	pci_write_config32(PCH_LPC_DEV, GPIO_BASE, DEFAULT_GPIOBASE | 1);
	pci_write_config8(PCH_LPC_DEV, GPIO_CNTL, 0x10);
	outl (0x796bd9c3, DEFAULT_GPIOBASE);
	outl (0x86fec7c2, DEFAULT_GPIOBASE + 4);
	outl (0xe4e8d7fe, DEFAULT_GPIOBASE + 0xc);
	outl (0, DEFAULT_GPIOBASE + 0x18);
	outl (0x00004182, DEFAULT_GPIOBASE + 0x2c);
	outl (0x123360f8, DEFAULT_GPIOBASE + 0x30);
	outl (0x1f47bfa8, DEFAULT_GPIOBASE + 0x34);
	outl (0xfffe7fb6, DEFAULT_GPIOBASE + 0x38);


	/* This should probably go away. Until now it is required
	 * and mainboard specific
	 */
	rcba_config();

	console_init();

	/* Read PM1_CNT */
	reg32 = inl(DEFAULT_PMBASE + 0x04);
	printk(BIOS_DEBUG, "PM1_CNT: %08x\n", reg32);
	if (((reg32 >> 10) & 7) == 5) {
		u8 reg8;
		reg8 = pci_read_config8(PCI_DEV(0, 0x1f, 0), 0xa2);
		printk(BIOS_DEBUG, "a2: %02x\n", reg8);
		if (!(reg8 & 0x20)) {
			outl(reg32 & ~(7 << 10), DEFAULT_PMBASE + 0x04);
			printk(BIOS_DEBUG, "Bad resume from S3 detected.\n");
		} else {
			if (acpi_s3_resume_allowed()) {
				printk(BIOS_DEBUG, "Resume from S3 detected.\n");
				s3resume = 1;
			} else {
				printk(BIOS_DEBUG,
				       "Resume from S3 detected, but disabled.\n");
			}
		}
	}

	/* Enable SMBUS. */
	enable_smbus();

	write_acpi16(0x2, 0x0);
	write_acpi32(0x28, 0x0);
	write_acpi32(0x2c, 0x0);
	if (!s3resume) {
		read_acpi32(0x4);
		read_acpi32(0x20);
		read_acpi32(0x34);
		write_acpi16(0x0, 0x900);
		write_acpi32(0x20, 0xffff7ffe);
		write_acpi32(0x34, 0x56974);
		pci_write_config8(PCH_LPC_DEV, GEN_PMCON_3,
				  pci_read_config8(PCH_LPC_DEV, GEN_PMCON_3) | 2);
	}

	early_thermal_init();

	timestamp_add_now(TS_BEFORE_INITRAM);

	chipset_init(s3resume);
	raminit(s3resume, spd_addrmap);

	timestamp_add_now(TS_AFTER_INITRAM);

	intel_early_me_status();

	if (s3resume) {
		/* Clear SLP_TYPE. This will break stage2 but
		 * we care for that when we get there.
		 */
		reg32 = inl(DEFAULT_PMBASE + 0x04);
		outl(reg32 & ~(7 << 10), DEFAULT_PMBASE + 0x04);
	}

	romstage_handoff_init(s3resume);
}
