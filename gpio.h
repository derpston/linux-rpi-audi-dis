/*

Minimal set of functions for driving the GPIO port on a Raspberry Pi.

(Heavily modified from:
Device driver for the Raspberry Pi utilities.

Copyright (c) 2013 Viewing Ltd.
All Rights Reserved.)

This software is released under a Dual BSD (3 Clause) / GPL license

BSD:

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Viewing Ltd nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

GPL:

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/



// ARM TRM section 3-83, 3-84
// Need a read barrier after the last read from peripheral, and a write barrier before first write to peripheral.
// Not so sure which of these to use but its leaning towards the sync.
#define ARM_DATA_SYNC_BARRIER __asm__ __volatile__ ("mcr p15, 0, r0, c7, c10, 4" : : : "memory")
#define ARM_DATA_MEM_BARRIER  __asm__ __volatile__ ("mcr p15, 0, r0, c7, c10, 5" : : : "memory")

#define  GPFSEL0     0x00200000  // GPIO Function Select 0, R/W
#define  GPFSEL1     0x00200004  // GPIO Function Select 1, R/W
#define  GPFSEL2     0x00200008  // GPIO Function Select 2, R/W

#define GPFSEL_REG(P)         (GPFSEL0 + ((GPFSEL1 - GPFSEL0) * ((P) / 10)))
#define GPFSEL_MASK(P)        (0x7 << (((P) % 10) * 3))
#define GPFSEL_VALSHIFT(P,V)  ((V) << (((P) % 10) * 3))

#define  GPFSEL_INPUT   0
#define  GPFSEL_OUTPUT  1
#define  GPFSEL_ALT0    4
#define  GPFSEL_ALT1    5
#define  GPFSEL_ALT2    6
#define  GPFSEL_ALT3    7
#define  GPFSEL_ALT4    3
#define  GPFSEL_ALT5    2


// setting

#define GPSET_REG(P)       (GPSET0 + ((GPSET1 - GPSET0) * ((P) / 32)))

#define  GPSET0      0x0020001C  // GPIO Pin Output Set 0, W
#define  GPSET1      0x00200020  // GPIO Pin Output Set 1, W
#define  GPCLR0      0x00200028  // GPIO Pin Output Clear 0, W
#define  GPCLR1      0x0020002C  // GPIO Pin Output Clear 1, W

#define GPSET_VALSHIFT(P,V)      ((V) ? (1 << ((P) % 32)) : 0)

// clear

#define GPCLR_REG(P)       (GPCLR0 + ((GPCLR1 - GPCLR0) * ((P) / 32)))
#define GPCLR_VALSHIFT(P,V)      ((V) ? (1 << ((P) % 32)) : 0)

#define KERN_VIRT_ADDR 0xF2000000

void set_output(int);
void set_pin(int);
void clear_pin(int);

// Configures a pin to be an output.
void set_output(int pin)
{
	volatile unsigned *p_mem = NULL;
    unsigned tmp;

    unsigned address = KERN_VIRT_ADDR | GPFSEL_REG(pin);
    unsigned mask = GPFSEL_MASK(pin);
    unsigned value = GPFSEL_VALSHIFT(pin, GPFSEL_OUTPUT);

    ARM_DATA_SYNC_BARRIER;
    p_mem = (unsigned *) address;
	tmp = *p_mem;
	tmp &= ~mask;
	tmp |= (value & mask);
	*p_mem = tmp;
    ARM_DATA_SYNC_BARRIER;
}

// Sets a pin high.
void set(int pin)
{
	volatile unsigned *p_mem = NULL;
    unsigned address = KERN_VIRT_ADDR | GPSET_REG(pin);
    unsigned value = GPSET_VALSHIFT(pin, 1);
    
    ARM_DATA_SYNC_BARRIER;
    p_mem = (unsigned *) address;
    *p_mem = value;
    ARM_DATA_SYNC_BARRIER;
}

// Sets a pin low.
void clear(int pin)
{
	volatile unsigned *p_mem = NULL;
    unsigned address = KERN_VIRT_ADDR | GPCLR_REG(pin);
    unsigned value = GPCLR_VALSHIFT(pin, 1);

    ARM_DATA_SYNC_BARRIER;
    p_mem = (unsigned *) address;
    *p_mem = value;
    ARM_DATA_SYNC_BARRIER;
}


