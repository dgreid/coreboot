/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2018 Arthur Heymans <arthur@aheymans.xyz>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <device/pci_ops.h>
#include <console/console.h>
#include <southbridge/intel/i82801jx/i82801jx.h>
#include <southbridge/intel/common/gpio.h>
#include <southbridge/intel/common/pmclib.h>
#include <northbridge/intel/x4x/x4x.h>
#include <cpu/intel/romstage.h>
#include <superio/winbond/w83667hg-a/w83667hg-a.h>
#include <superio/winbond/common/winbond.h>
#include <northbridge/intel/x4x/iomap.h>

#define SERIAL_DEV PNP_DEV(0x2e, W83667HG_A_SP1)
#define LPC_DEV PCI_DEV(0, 0x1f, 0)

/* Early mainboard specific GPIO setup.
 * We should use standard gpio.h eventually
 */

static void mb_gpio_init(void)
{
	/* Set the value for GPIO base address register and enable GPIO. */
	pci_write_config32(LPC_DEV, D31F0_GPIO_BASE, (DEFAULT_GPIOBASE | 1));
	pci_write_config8(LPC_DEV,  D31F0_GPIO_CNTL, 0x10);

	setup_pch_gpios(&mainboard_gpio_map);

	/* Set default GPIOs on superio: TODO (here or in ramstage) */

	/* Enable IOAPIC */
	RCBA8(0x31ff) = 0x03;
	RCBA8(0x31ff);

	/* TODO? */
	RCBA32(RCBA_CG) = 0xbf7f001f;
	RCBA32(0x3430) = 0x00000002;
	RCBA32(0x3f00) = 0x00000038;
}

static void ich10_enable_lpc(void)
{
	/* Configure serial IRQs.*/
	pci_write_config16(LPC_DEV, D31F0_LPC_IODEC, 0x0010);
	pci_write_config16(LPC_DEV, D31F0_LPC_EN, CNF2_LPC_EN | CNF1_LPC_EN
			   | KBC_LPC_EN | FDD_LPC_EN | LPT_LPC_EN | COMB_LPC_EN
			   | COMA_LPC_EN);
	/* HW EC */
	pci_write_config32(LPC_DEV, D31F0_GEN1_DEC, 0x00000295);
	/* ????? */
	pci_write_config32(LPC_DEV, D31F0_GEN2_DEC, 0x001c4701);
}

void mainboard_romstage_entry(void)
{
	const u8 spd_addrmap[4] = { 0x50, 0x51, 0x52, 0x53 };
	u8 boot_path = 0;
	u8 s3_resume;

	/* Set southbridge and Super I/O GPIOs. */
	ich10_enable_lpc();
	mb_gpio_init();
	winbond_enable_serial(SERIAL_DEV, CONFIG_TTYS0_BASE);

	console_init();

	enable_smbus();

	x4x_early_init();

	s3_resume = southbridge_detect_s3_resume();
	if (s3_resume)
		boot_path = BOOT_PATH_RESUME;
	if (MCHBAR32(PMSTS_MCHBAR) & PMSTS_WARM_RESET)
		boot_path = BOOT_PATH_WARM_RESET;

	sdram_initialize(boot_path, spd_addrmap);

	x4x_late_init(s3_resume);

	printk(BIOS_DEBUG, "x4x late init complete\n");
}
