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


#include <string.h>
#include "control.h"
#include "dosbox.h"
#include "inout.h"
#include "setup.h"
#include "cpu.h"
#include "../src/cpu/lazyflags.h"
#include "callback.h"

//#define ENABLE_PORTLOG

#include <vector>

IO_WriteHandler * io_writehandlers[3][IO_MAX];
IO_ReadHandler * io_readhandlers[3][IO_MAX];

#if C_DEBUG
void DEBUG_EnableDebugger(void);
#endif

static Bitu IO_ReadBlocked(Bitu /*port*/,Bitu /*iolen*/) {
	return ~0ul;
}

static void IO_WriteBlocked(Bitu /*port*/,Bitu /*val*/,Bitu /*iolen*/) {
}

static Bitu IO_ReadDefault(Bitu port,Bitu iolen) {
	switch (iolen) {
	case 1:
		LOG(LOG_IO,LOG_WARN)("Read from port %04X",(int)port);
		io_readhandlers[0][port]=IO_ReadBlocked;
		return 0xff;
	case 2:
		return
			(io_readhandlers[0][port+0](port+0,1) << 0) |
			(io_readhandlers[0][port+1](port+1,1) << 8);
	case 4:
		return
			(io_readhandlers[1][port+0](port+0,2) << 0) |
			(io_readhandlers[1][port+2](port+2,2) << 16);
	}
	return ~0ul;
}

void IO_WriteDefault(Bitu port,Bitu val,Bitu iolen) {
	switch (iolen) {
	case 1:
		LOG(LOG_IO,LOG_WARN)("Writing %02X to port %04X",(int)val,(int)port);
		io_writehandlers[0][port]=IO_WriteBlocked;
		break;
	case 2:
		io_writehandlers[0][port+0](port+0,(val >> 0) & 0xff,1);
		io_writehandlers[0][port+1](port+1,(val >> 8) & 0xff,1);
		break;
	case 4:
		io_writehandlers[1][port+0](port+0,(val >> 0 ) & 0xffff,2);
		io_writehandlers[1][port+2](port+2,(val >> 16) & 0xffff,2);
		break;
	}
}

void IO_RegisterReadHandler(Bitu port,IO_ReadHandler * handler,Bitu mask,Bitu range) {
    assert((port+range) <= IO_MAX);
	while (range--) {
		if (mask&IO_MB) io_readhandlers[0][port]=handler;
		if (mask&IO_MW) io_readhandlers[1][port]=handler;
		if (mask&IO_MD) io_readhandlers[2][port]=handler;
		port++;
	}
}

void IO_RegisterWriteHandler(Bitu port,IO_WriteHandler * handler,Bitu mask,Bitu range) {
    assert((port+range) <= IO_MAX);
	while (range--) {
		if (mask&IO_MB) io_writehandlers[0][port]=handler;
		if (mask&IO_MW) io_writehandlers[1][port]=handler;
		if (mask&IO_MD) io_writehandlers[2][port]=handler;
		port++;
	}
}

void IO_FreeReadHandler(Bitu port,Bitu mask,Bitu range) {
    assert((port+range) <= IO_MAX);
	while (range--) {
		if (mask&IO_MB) io_readhandlers[0][port]=IO_ReadDefault;
		if (mask&IO_MW) io_readhandlers[1][port]=IO_ReadDefault;
		if (mask&IO_MD) io_readhandlers[2][port]=IO_ReadDefault;
		port++;
	}
}

void IO_FreeWriteHandler(Bitu port,Bitu mask,Bitu range) {
    assert((port+range) <= IO_MAX);
	while (range--) {
		if (mask&IO_MB) io_writehandlers[0][port]=IO_WriteDefault;
		if (mask&IO_MW) io_writehandlers[1][port]=IO_WriteDefault;
		if (mask&IO_MD) io_writehandlers[2][port]=IO_WriteDefault;
		port++;
	}
}

void IO_InvalidateCachedHandler(Bitu port,Bitu range) {
    assert((port+range) <= IO_MAX);
    for (Bitu mb=0;mb <= 2;mb++) {
        Bitu p = port;
        Bitu r = range;
        while (r--) {
            io_writehandlers[mb][p]=IO_WriteBlocked;
            io_readhandlers[mb][p]=IO_ReadBlocked;
            p++;
        }
    }
}

void IO_ReadHandleObject::Install(Bitu port,IO_ReadHandler * handler,Bitu mask,Bitu range) {
	if(!installed) {
		installed=true;
		m_port=port;
		m_mask=mask;
		m_range=range;
		IO_RegisterReadHandler(port,handler,mask,range);
	} else E_Exit("IO_readHandler already installed port %x",(int)port);
}

void IO_ReadHandleObject::Uninstall(){
	if(!installed) return;
	IO_FreeReadHandler(m_port,m_mask,m_range);
	installed=false;
}

IO_ReadHandleObject::~IO_ReadHandleObject(){
	Uninstall();
}

void IO_WriteHandleObject::Install(Bitu port,IO_WriteHandler * handler,Bitu mask,Bitu range) {
	if(!installed) {
		installed=true;
		m_port=port;
		m_mask=mask;
		m_range=range;
		IO_RegisterWriteHandler(port,handler,mask,range);
	} else E_Exit("IO_writeHandler already installed port %x",(int)port);
}

void IO_WriteHandleObject::Uninstall() {
	if(!installed) return;
	IO_FreeWriteHandler(m_port,m_mask,m_range);
	installed=false;
}

IO_WriteHandleObject::~IO_WriteHandleObject(){
	Uninstall();
	//LOG_MSG("FreeWritehandler called with port %X",m_port);
}


/* Some code to make io operations take some virtual time. Helps certain
 * games with their timing of certain operations
 */

/* how much delay to add to I/O in nanoseconds */
int io_delay_ns[3] = {-1,-1,-1};

/* nonzero if we're in a callback */
extern unsigned int last_callback;

inline void IO_USEC_read_delay(const unsigned int szidx) {
	if (io_delay_ns[szidx] > 0 && last_callback == 0/*NOT running within a callback function*/) {
		Bits delaycyc = (CPU_CycleMax * io_delay_ns[szidx]) / 1000000;
		CPU_Cycles -= delaycyc;
		CPU_IODelayRemoved += delaycyc;
	}
}

inline void IO_USEC_write_delay(const unsigned int szidx) {
	if (io_delay_ns[szidx] > 0 && last_callback == 0/*NOT running within a callback function*/) {
		Bits delaycyc = (CPU_CycleMax * io_delay_ns[szidx] * 3) / (1000000 * 4);
		CPU_Cycles -= delaycyc;
		CPU_IODelayRemoved += delaycyc;
	}
}

#ifdef ENABLE_PORTLOG
static Bit8u crtc_index = 0;
const char* const len_type[] = {" 8","16","32"};
void log_io(Bitu width, bool write, Bitu port, Bitu val) {
	switch(width) {
	case 0:
		val&=0xff;
		break;
	case 1:
		val&=0xffff;
		break;
	}
	if (write) {
		// skip the video cursor position spam
		if (port==0x3d4) {
			if (width==0) crtc_index = (Bit8u)val;
			else if(width==1) crtc_index = (Bit8u)(val>>8);
		}
		if (crtc_index==0xe || crtc_index==0xf) {
			if((width==0 && (port==0x3d4 || port==0x3d5))||(width==1 && port==0x3d4))
				return;
		}

		switch(port) {
		//case 0x020: // interrupt command
		//case 0x040: // timer 0
		//case 0x042: // timer 2
		//case 0x043: // timer control
		//case 0x061: // speaker control
		case 0x3c8: // VGA palette
		case 0x3c9: // VGA palette
		// case 0x3d4: // VGA crtc
		// case 0x3d5: // VGA crtc
		// case 0x3c4: // VGA seq
		// case 0x3c5: // VGA seq
			break;
		default:
			LOG_MSG("iow%s % 4x % 4x, cs:ip %04x:%04x", len_type[width],
				(int)port, (int)val, (int)SegValue(cs), (int)reg_eip);
			break;
		}
	} else {
		switch(port) {
		//case 0x021: // interrupt status
		//case 0x040: // timer 0
		//case 0x042: // timer 2
		//case 0x061: // speaker control
		case 0x201: // joystick status
		case 0x3c9: // VGA palette
		// case 0x3d4: // VGA crtc index
		// case 0x3d5: // VGA crtc
		case 0x3da: // display status - a real spammer
			// don't log for the above cases
			break;
		default:
			LOG_MSG("ior%s % 4x % 4x,\t\tcs:ip %04x:%04x", len_type[width],
				(int)port, (int)val, (int)SegValue(cs), (int)reg_eip);
			break;
		}
	}
}
#else
#define log_io(W, X, Y, Z)
#endif


void IO_WriteB(Bitu port,Bit8u val) {
	log_io(0, true, port, val);
	if (GCC_UNLIKELY(GETFLAG(VM) && (CPU_IO_Exception(port,1)))) {
		CPU_ForceV86FakeIO_Out(port,val,1);
	}
	else {
		IO_USEC_write_delay(0);
		io_writehandlers[0][port](port,val,1);
	}
}

void IO_WriteW(Bitu port,Bit16u val) {
	log_io(1, true, port, val);
	if (GCC_UNLIKELY(GETFLAG(VM) && (CPU_IO_Exception(port,2)))) {
		CPU_ForceV86FakeIO_Out(port,val,2);
	}
	else {
		IO_USEC_write_delay(1);
		io_writehandlers[1][port](port,val,2);
	}
}

void IO_WriteD(Bitu port,Bit32u val) {
	log_io(2, true, port, val);
	if (GCC_UNLIKELY(GETFLAG(VM) && (CPU_IO_Exception(port,4)))) {
		CPU_ForceV86FakeIO_Out(port,val,4);
	}
	else {
		IO_USEC_write_delay(2);
		io_writehandlers[2][port](port,val,4);
	}
}

Bit8u IO_ReadB(Bitu port) {
	Bit8u retval;
	if (GCC_UNLIKELY(GETFLAG(VM) && (CPU_IO_Exception(port,1)))) {
		return (Bit8u)CPU_ForceV86FakeIO_In(port,1);
	}
	else {
		IO_USEC_read_delay(0);
		retval = (Bit8u)io_readhandlers[0][port](port,1);
	}
	log_io(0, false, port, retval);
	return retval;
}

Bit16u IO_ReadW(Bitu port) {
	Bit16u retval;
	if (GCC_UNLIKELY(GETFLAG(VM) && (CPU_IO_Exception(port,2)))) {
		return (Bit16u)CPU_ForceV86FakeIO_In(port,2);
	}
	else {
		IO_USEC_read_delay(1);
		retval = (Bit16u)io_readhandlers[1][port](port,2);
	}
	log_io(1, false, port, retval);
	return retval;
}

Bit32u IO_ReadD(Bitu port) {
	Bit32u retval;
	if (GCC_UNLIKELY(GETFLAG(VM) && (CPU_IO_Exception(port,4)))) {
		return (Bit32u)CPU_ForceV86FakeIO_In(port,4);
	}
	else {
		IO_USEC_read_delay(2);
		retval = (Bit32u)io_readhandlers[2][port](port,4);
	}
	log_io(2, false, port, retval);
	return retval;
}

void IO_Reset(Section * /*sec*/) { // Reset or power on
	Section_prop * section=static_cast<Section_prop *>(control->GetSection("dosbox"));

	io_delay_ns[0] = section->Get_int("iodelay");
	if (io_delay_ns[0] < 0) { // 8-bit transfers are said to have a transfer cycle with 4 wait states
		// TODO: How can we emulate Intel 440FX chipsets that allow 8-bit PIO with 1 wait state, or zero wait state devices?
		double t = (1000000000.0 * clockdom_ISA_BCLK.freq_div * 8.5) / clockdom_ISA_BCLK.freq;
		io_delay_ns[0] = (int)floor(t);
	}

	io_delay_ns[1] = section->Get_int("iodelay16");
	if (io_delay_ns[1] < 0) { // 16-bit transfers are said to have a transfer cycle with 1 wait state
		// TODO: How can we emulate ISA devices that support zero wait states?
		double t = (1000000000.0 * clockdom_ISA_BCLK.freq_div * 5.5) / clockdom_ISA_BCLK.freq;
		io_delay_ns[1] = (int)floor(t);
	}

	io_delay_ns[2] = section->Get_int("iodelay32");
	if (io_delay_ns[2] < 0) { // assume ISA bus details that turn 32-bit PIO into two 16-bit I/O transfers
		// TODO: If the device is a PCI device, then 32-bit PIO should take the same amount of time as 8/16-bit PIO
		double t = (1000000000.0 * clockdom_ISA_BCLK.freq_div * (5.5 + 5.5)) / clockdom_ISA_BCLK.freq;
		io_delay_ns[2] = (int)floor(t);
	}

	LOG(LOG_IO,LOG_DEBUG)("I/O 8-bit delay %uns",io_delay_ns[0]);
	LOG(LOG_IO,LOG_DEBUG)("I/O 16-bit delay %uns",io_delay_ns[1]);
	LOG(LOG_IO,LOG_DEBUG)("I/O 32-bit delay %uns",io_delay_ns[2]);
}

void IO_Init() {
	LOG(LOG_MISC,LOG_DEBUG)("Initializing I/O port handler system");

	/* init the ports, rather than risk I/O jumping to random code */
	IO_FreeReadHandler(0,IO_MA,IO_MAX);
	IO_FreeWriteHandler(0,IO_MA,IO_MAX);

	/* please call our reset function on power-on and reset */
	AddVMEventFunction(VM_EVENT_RESET,AddVMEventFunctionFuncPair(IO_Reset));
}

