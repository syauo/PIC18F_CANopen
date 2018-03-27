/*******************************************************************************

   memcpyram2flash.c - Copy contents from RAM to Flash Program space

   Copyright (C) 2004  Janez Paternoster, Slovenia

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


   Author: janez.paternoster@siol.net

   History:
   2004/06/25 [JP]  File Created
   2004/08/09 [JP]  Added Clear Watchdog instruction and line: while(EECON1bits.WR);

*******************************************************************************/


/*
This function copies contents of RAM variable into Flash program
memory space. It's use can be dangerous, since it may “reprogram”
the microcontroller if wrong used.It is tested to work with PIC18Fxx8.
Function takes about 10 ms, because writing to flash is slow. If reset
occurs between erasing and writing new values, up to 64 bytes of memory
locations may stay erased.

*/

#include <p18cxxx.h>
#include <string.h>


void memcpyram2flash(rom void* dest, void* src, unsigned int memsize){
   unsigned char romTemp[64];
   static unsigned char i, j; //multipurpose variables
   char GIEold = INTCONbits.GIE;

   j = (unsigned char)dest & 0x3F;  //offset in first block
   dest -= j;
   while(memsize){
      //read block
      memcpypgm2ram(romTemp, dest, 64);
      //determine size of data to be copied into block
      i = 64 - j;
      if(memsize < i) i = memsize;
      //modify block
      memcpy(romTemp+j, src, i);
      //prepare variables for next block
      memsize -= i;
      src += i;
      //wait if write to EEPROM is curently in progress
      while(EECON1bits.WR);
      //erase block
      TBLPTR = (unsigned short long)dest;
      INTCONbits.GIE = 0;
      ClrWdt();
      EECON1bits.EEPGD = 1;
      EECON1bits.CFGS = 0;
      EECON1bits.WREN = 1;
      EECON1bits.FREE = 1;
      EECON2 = 0x55;
      EECON2 = 0xAA;
      EECON1bits.WR = 1;
      Nop();
      if(GIEold) INTCONbits.GIE=1; else INTCONbits.GIE=0;
      j = 8;
      _asm
      TBLRDPOSTDEC
      _endasm
      for(i=0; i<64; i++){
         TABLAT = romTemp[i];
         _asm
         TBLWTPREINC
         _endasm
         if(--j == 0){
            INTCONbits.GIE = 0;
            ClrWdt();
            EECON1bits.EEPGD = 1;
            EECON1bits.CFGS = 0;
            EECON1bits.WREN = 1;
            EECON2 = 0x55;
            EECON2 = 0xAA;
            EECON1bits.WR = 1;
            Nop();
            if(GIEold) INTCONbits.GIE=1; else INTCONbits.GIE=0;
            j = 8;
         }
      }
      EECON1bits.WREN = 0;
      j = 0;   //offset is 0 for following blocks
      dest += 64;
   }
}
