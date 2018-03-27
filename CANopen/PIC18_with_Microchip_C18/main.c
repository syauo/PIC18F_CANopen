/*******************************************************************************

   main.c - Main and ISR functions - Processor specific
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

#include "CANopen.h"


/* Function prototypes ********************************************************/
   //from CO_driver.c
   void CO_CANrxIsr(void);       //ISR for receiving CAN messages
   void CO_CANtxIsr(void);       //ISR for transmiting CAN messages
   void CO_DriverInit(void);
   //from CO_stack.c
   void CO_ResetComm(void);      //Reset communication
   void CO_Timer1msIsr(void);    //Process CANopen from 1ms timer interrupt, synchronous
   void CO_ProcessMain(void);    //Process CANopen from main function, asynchronous
   //from user.c
   void User_Init(void);
   void User_ProcessMain(void);
   #ifdef USER_ISR_HIGH
      void User_IsrHigh(void);
   #endif
   #ifdef USER_ISR_LOW
      void User_IsrLow(void);
   #endif

/* Parameters for Timer2 (1ms interrupt) **************************************/
   #if CO_OSCILATOR_FREQ == 4
      #define TIMER2PRESCALE     0     //0=1:1, 1=1:4, 2=1:16
      #define TIMER2POSTSCALE    10    //1...16
      #define TIMER2PR2          99    //0...255
   #elif CO_OSCILATOR_FREQ == 8
      #define TIMER2PRESCALE     0     //0=1:1, 1=1:4, 2=1:16
      #define TIMER2POSTSCALE    16    //1...16
      #define TIMER2PR2          124   //0...255
   #elif CO_OSCILATOR_FREQ == 16
      #define TIMER2PRESCALE     0     //0=1:1, 1=1:4, 2=1:16
      #define TIMER2POSTSCALE    16    //1...16
      #define TIMER2PR2          249   //0...255
   #elif CO_OSCILATOR_FREQ == 20
      #define TIMER2PRESCALE     1     //0=1:1, 1=1:4, 2=1:16
      #define TIMER2POSTSCALE    10    //1...16
      #define TIMER2PR2          124   //0...255
   #elif CO_OSCILATOR_FREQ == 24
      #define TIMER2PRESCALE     1     //0=1:1, 1=1:4, 2=1:16
      #define TIMER2POSTSCALE    12    //1...16
      #define TIMER2PR2          124   //0...255
   #elif CO_OSCILATOR_FREQ == 32
      #define TIMER2PRESCALE     1     //0=1:1, 1=1:4, 2=1:16
      #define TIMER2POSTSCALE    16    //1...16
      #define TIMER2PR2          124   //0...255
   #elif CO_OSCILATOR_FREQ == 40
      #define TIMER2PRESCALE     1     //0=1:1, 1=1:4, 2=1:16
      #define TIMER2POSTSCALE    10    //1...16
      #define TIMER2PR2          249   //0...255
   #else
      #error define_CO_OSCILATOR_FREQ CO_OSCILATOR_FREQ not supported
   #endif

/* INTERRUPTS *****************************************************************/
#pragma interruptlow IsrLow
void IsrLow (void){
   if(PIR1bits.TMR2IF){
      PIR1bits.TMR2IF = 0;
      CO_Timer1msIsr();
      if(PIR1bits.TMR2IF) ErrorReport(ERROR_isr_timer_overflow, 0);
      }
  #ifdef USER_ISR_LOW
   User_IsrLow();
  #else
   else
      ErrorReport(ERROR_isr_low_WrongInterrupt, 0);
  #endif
}

#pragma code high_vector=0x08
void interrupt_high(void){
  #ifdef USER_ISR_HIGH
   _asm
      MOVF PIR3, 0, ACCESS
      ANDLW 0x3
      BZ M_LAB_1
      GOTO CO_CANrxIsr
     M_LAB_1:
      GOTO User_IsrHigh
   _endasm
  #else
   _asm
      GOTO CO_CANrxIsr
   _endasm
  #endif
}
#pragma code

#pragma code low_vector=0x18
void interrupt_low(void){
   _asm
      BTFSC PIR3, 0x2, ACCESS   //test TXB0 interrupt flag, skip line if cleared
      GOTO CO_CANtxIsr
      GOTO IsrLow
   _endasm
}
#pragma code


/* main() *********************************************************************/
void main (void){
   CO_DriverInit();  //CANopenNode driver init
   User_Init();      //User init
   CO_ResetComm();   //Reset communication

   //Timer 1ms
   //timer2 is used for 1ms interrupt. It is also time-base for PWM modules.
   //So if PWM is used, frequency is set here.
   T2CON = ((TIMER2POSTSCALE-1) << 3) | 0x04 | TIMER2PRESCALE;
   PR2 = TIMER2PR2;
   TMR2 = 0;

   //enable timer interrupts
   PIR1bits.TMR2IF = 0;
   IPR1bits.TMR2IP = 0;   //low priority interrupt
   PIE1bits.TMR2IE = 1;

   //interrupts
   RCONbits.IPEN   = 1;    //interrupt priority enable
   INTCONbits.GIEL = 1;    //global interrupt low enable
   INTCONbits.GIEH = 1;    //global interrupt high enable

   while(1){
      ClrWdt();
      CO_ProcessMain();
      ClrWdt();
      User_ProcessMain();
   }
}

