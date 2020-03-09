/*
 *  Copyright (C) 2002-2019  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA.
 */


#include "dosbox.h"
#include "inout.h"
#include "vga.h"

#define seq(blah) vga.seq.blah

Bitu read_p3c4(Bitu /*port*/,Bitu /*iolen*/) {
	return seq(index);
}

void write_p3c4(Bitu /*port*/,Bitu val,Bitu /*iolen*/) {
	// FIXME: Standard VGA behavior (based on observation) is to latch
	//        the sequencer register based only on the lowest 4 bits.
	//        Thus, the map mask register is accessible not only by index 0x02,
	//        but also from 0x12, 0x22, etc..
	//
	//        This isn't quite universal, as WHATVGA documentation suggests
	//        there are SVGA chipsets with extended registers in the 0x07-0x17
	//        and 0x80-0x9F ranges. But register snapshots so far seem to suggest
	//        most cards just mask off the index to 4 bits and put the extended
	//        regs elsewhere.
	//
	//        The yet to be answered question is: if the card only latches the
	//        low 4 bits, then what will you see when you read back the sequencer
	//        index register? Will you see the full 8-bit value, or the low 4 bits
	//        it actually decoded?
	//
	// FIXME: I don't have an EGA to test with, but, what masking behavior here
	//        do EGA and compatibles do with the index?

	/* Emulating the mask behavior fixes the following problems with games & demoscene entries:
	 *
	 * - "Spiral" by Dataction: SPIRAL.EXE appears to use VGA Mode-X 256-color mode, but it relies
	 *   on sequencer alias register 0x12 for masking bitplanes instead of properly writing
	 *   Map Mask Register (register 0x02). Older VGA chipsets only decoded 3 or 4 bits of the sequencer
	 *   index register, so this happened to work anyway since (0x12 & 0x0F) == 0x02, but it also means
	 *   the demo will not render Mode X properly on newer SVGA chipsets that decode more than 4 bits of
	 *   the sequencer index register. Adding this masking behavior, and running the demo
	 *   with machine=svga_et4000 allows the demo to display properly instead of rendering as
	 *   a blocky low-res mess.
	 *   [http://www.pouet.net/prod.php?which=12630]
	 */

	if (machine == MCH_VGA) {
		if (svgaCard == SVGA_S3Trio)
			val &= 0x3F;	// observed behavior: only the low 6 bits
		else
			val &= 0x0F;	// FIXME: reasonable guess

        /* Paradise/Western Digital sequencer registers appear to repeat every 0x40 aka decoding bits [5:0] */
	}

	seq(index)=(Bit8u)val;
}

void write_p3c5(Bitu /*port*/,Bitu val,Bitu /*iolen*/) {
//	LOG_MSG("SEQ WRITE reg %X val %X",seq(index),val);
	switch(seq(index)) {
	case 1:		/* Clocking Mode */
		if (val!=seq(clocking_mode)) {
			// don't resize if only the screen off bit was changed
			if ((val&(~0x20u))!=(seq(clocking_mode)&(~0x20u))) {
				seq(clocking_mode)=(Bit8u)val;
				VGA_StartResize();
			} else {
				seq(clocking_mode)=(Bit8u)val;
			}
		}
		/* TODO Figure this out :)
			0	If set character clocks are 8 dots wide, else 9.
			2	If set loads video serializers every other character
				clock cycle, else every one.
			3	If set the Dot Clock is Master Clock/2, else same as Master Clock
				(See 3C2h bit 2-3). (Doubles pixels). Note: on some SVGA chipsets
				this bit also affects the Sequencer mode.
			4	If set loads video serializers every fourth character clock cycle,
				else every one.
			5	if set turns off screen and gives all memory cycles to the CPU
				interface.
		*/
		break;
	default:
		{
			LOG(LOG_VGAMISC,LOG_NORMAL)("VGA:SEQ:Write to illegal index %2X",seq(index));
		}
		break;
	}
}


Bitu read_p3c5(Bitu /*port*/,Bitu /*iolen*/) {
//	LOG_MSG("VGA:SEQ:Read from index %2X",seq(index));
	switch(seq(index)) {
	case 1:			/* Clocking Mode */
		return seq(clocking_mode);
		break;
	default:
		break;
	}
	return 0;
}


void VGA_SetupSEQ(void) {
	if (IS_EGAVGA_ARCH) {
		IO_RegisterWriteHandler(0x3c4,write_p3c4,IO_MB);
		IO_RegisterWriteHandler(0x3c5,write_p3c5,IO_MB);
		if (IS_VGA_ARCH) {
			IO_RegisterReadHandler(0x3c4,read_p3c4,IO_MB);
			IO_RegisterReadHandler(0x3c5,read_p3c5,IO_MB);
		}
	}
}

void VGA_UnsetupSEQ(void) {
    IO_FreeWriteHandler(0x3c4,IO_MB);
    IO_FreeReadHandler(0x3c4,IO_MB);
    IO_FreeWriteHandler(0x3c5,IO_MB);
    IO_FreeReadHandler(0x3c5,IO_MB);
}

