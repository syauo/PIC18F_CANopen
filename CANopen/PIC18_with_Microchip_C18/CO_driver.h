/*******************************************************************************

   CO_driver.h - Header for CO_driver.c - Processor specific
                  Processor: PIC18Fxxx
                  Compiler: MPLAB - C18 (Microchip) >= V3.00

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

*******************************************************************************/

#ifndef _CO_DRIVER_H
#define _CO_DRIVER_H

#include <p18cxxx.h>       //processor header file

/*******************************************************************************
   Processor specific hardware macros
*******************************************************************************/
   //Oscilator frequency, mode PLLx4 can be used (example: 32MHz frequency with 8MHz Quartz - set 32)
   #ifndef CO_OSCILATOR_FREQ
      #define CO_OSCILATOR_FREQ     20 //(4, 8, 16, 20, 24, 32 or 40)
   #endif

   #define SET_CAN_TRIS(){    \
      TRISBbits.TRISB2 = 0;   \
      TRISBbits.TRISB3 = 1;   \
   }

   #ifndef PCB_DISABLE_CAN_LEDS
      //Green RUN led
      #define PCB_RUN_LED_INIT()    {TRISBbits.TRISB4 = 0; PORTBbits.RB4 = 0;}
      #define PCB_RUN_LED(i)        PORTBbits.RB4 = i
      //Red ERROR led
      #define PCB_ERROR_LED_INIT()  {TRISBbits.TRISB5 = 0; PORTBbits.RB5 = 0;}
      #define PCB_ERROR_LED(i)      PORTBbits.RB5 = i
   #endif

/*******************************************************************************
   Processor specific software macros
*******************************************************************************/
   //qualifier for variables located in read-only memory (C18 compiler put those variables in
   //flash memory space (rom qualifier). If located in RAM, 'const' should be used instead of 'rom')
   //ROM Variables in Object Dictionary MUST have attribute ATTR_ROM appended. If
   //base attribute is ATTR_RW or ATTR_WO, variable can be changed via CAN network (with
   //writing to flash memory space in PIC18Fxxx microcontroller)
   #define ROM             rom

   //default qualifiers and types
   #define CO_DEFAULT_SPEC static         //on PIC18F local variables are faster if static
   #define CO_DEFAULT_TYPE unsigned char  //for 8-bit microcontroller, for 16-bit use unsigned int
   #define CO_DEFAULT_TYPE_SIZE 0xFF      //for 8-bit microcontroller, for 16-bit use 0xFFFF

   #define CO_MODIF_Timer1msIsr           //modifier for CO_Timer1msIsr function

   //Macro is used for writing CAN-ID and RTR bit into the identifier in CO_TXCAN or CO_RXCAN array.
   //Identifier is aligned with hardware registers. RTR must be 0 or 1.
   #define CO_IDENT_WRITE(CobId, RTR)            (((CobId)<<5) | ((RTR)<<4))
   #define CO_IDENT_READ_COB(CanMessage_Ident)   ((CanMessage_Ident.WORD[0]) >> 5)  //inverse function, read COB-ID
   #define CO_IDENT_READ_RTR(CanMessage_Ident)   (CanMessage_Ident.BYTEbits[0].bit3)//inverse function, read RTR

   //reset microcontroller
   #define CO_Reset()  Reset()

   //enable, disable interrupts - VERY IMPORTANT SECTION
      //disable/enable all interrupts from mainline procedure
      #define CO_DISABLE_ALL()            INTCONbits.GIEH = 0
      #define CO_ENABLE_ALL()             INTCONbits.GIEH = 1
      //disable/enable timer interrupt from mainline procedure
      #define CO_DISABLE_TMR()            INTCONbits.GIEL = 0
      #define CO_ENABLE_TMR()             INTCONbits.GIEL = 1
      //disable/enable CANtx interrupt from mainline procedure
      #define CO_DISABLE_CANTX()          INTCONbits.GIEL = 0
      #define CO_ENABLE_CANTX()           INTCONbits.GIEL = 1
      //disable/enable CANtx interrupt from timer procedure
      #define CO_DISABLE_CANTX_TMR()      //nothing, beacuse same level interrupt is used for timer and CANtx
      #define CO_ENABLE_CANTX_TMR()
      //disable/enable CANrx interrupt from mainline procedure
      #define CO_DISABLE_CANRX()          INTCONbits.GIEH = 0
      #define CO_ENABLE_CANRX()           INTCONbits.GIEH = 1
      //disable/enable CANrx interrupt from timer procedure
      #define CO_DISABLE_CANRX_TMR()      INTCONbits.GIEH = 0
      #define CO_ENABLE_CANRX_TMR()       INTCONbits.GIEH = 1

   //Read Transmit Buffer Status
      #define CO_CANTX_BUFFER_FREE()      (TXB0CONbits.TXREQ == 0)
   //Copy message to CAN buffers
      #define CO_TXCAN_COPY_TO_BUFFERS(INDEX){                                         \
         /*put address in buffers, set standard identifier*/                           \
         TXB0SIDL = CO_TXCAN[INDEX].Ident.BYTE[0];                                     \
         TXB0SIDH = CO_TXCAN[INDEX].Ident.BYTE[1];                                     \
         /*set data length*/                                                           \
         TXB0DLC = CO_TXCAN[INDEX].NoOfBytes;                                          \
         if(TXB0DLC > 8) TXB0DLC = 8;                                                  \
         /*copy data*/                                                                 \
         memcpy((void*)&TXB0D0, (void*)&CO_TXCAN[INDEX].Data, TXB0DLC);                \
         /*Set RTR bit*/                                                               \
         TXB0DLCbits.TXRTR = CO_IDENT_READ_RTR(CO_TXCAN[INDEX].Ident);                 \
         /*control register, transmit request*/                                        \
         TXB0CON = 0x08;                                                               \
      }

#endif
