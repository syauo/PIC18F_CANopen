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
                           //处理器头文件

/*******************************************************************************
   Processor specific hardware macros
   处理器特定的硬件配置??
*******************************************************************************/
   //Oscilator frequency, mode PLLx4 can be used (example: 32MHz frequency with 8MHz Quartz - set 32)
   //晶振频率，可以使用PLLx4模式（如??MHz 晶振设置 PLLx4 后得??32MHz??
   #ifndef CO_OSCILATOR_FREQ
      #define CO_OSCILATOR_FREQ     20 //(4, 8, 16, 20, 24, 32 or 40)
   #endif

   #define SET_CAN_TRIS(){    \
      TRISBbits.TRISB2 = 0;   \
      TRISBbits.TRISB3 = 1;   \
   }

   #ifndef PCB_DISABLE_CAN_LEDS
      //Green RUN led         绿色 RUN LED
      #define PCB_RUN_LED_INIT()    {TRISBbits.TRISB1 = 0; LATBbits.LATB1 = 0;}
      #define PCB_RUN_LED(i)        LATBbits.LATB1 = i //PORTEbits.RE1 = i
      //Red ERROR led         红色 ERROR LED
      #define PCB_ERROR_LED_INIT()  {TRISBbits.TRISB4 = 0; LATBbits.LATB4 = 0;}
      #define PCB_ERROR_LED(i)      LATBbits.LATB4 = i //PORTEbits.RE2 = i
   #endif

/*******************************************************************************
   Processor specific software macros
   处理器特定的软件配置??
*******************************************************************************/
   //qualifier for variables located in read-only memory (C18 compiler put those variables in
   //flash memory space (rom qualifier). If located in RAM, 'const' should be used instead of 'rom')
   //ROM Variables in Object Dictionary MUST have attribute ATTR_ROM appended. If
   //base attribute is ATTR_RW or ATTR_WO, variable can be changed via CAN network (with
   //writing to flash memory space in PIC18Fxxx microcontroller)
   /**
    * 位于只读存储器中的变量的限定符（C18 编译器将这些变量放在 flash 空间中（rom 限定符）??
    * 如果位于RAM中，应使??const'来代??rom'）。对象字典中??ROM 变量必须追加 ATTR_ROM 属性??
    * 如果 base 属性为 ATTR_RW ??ATTR_WO，则可通过 CAN 网络更改变量（写??PIC18Fxxx 微控制器
    * 中的闪存空间??
    */
   
   #define ROM             rom

   //default qualifiers and types   默认的限定符和类??
   // ??PIC18F 上，本地变量如果是静态的话更??
   #define CO_DEFAULT_SPEC static         //on PIC18F local variables are faster if static
   #define CO_DEFAULT_TYPE unsigned char  //for 8-bit microcontroller, for 16-bit use unsigned int
   #define CO_DEFAULT_TYPE_SIZE 0xFF      //for 8-bit microcontroller, for 16-bit use 0xFFFF
   #define CO_MODIF_Timer1msIsr           //modifier for CO_Timer1msIsr function

   //Macro is used for writing CAN-ID and RTR bit into the identifier in CO_TXCAN or CO_RXCAN array.
   //Identifier is aligned with hardware registers. RTR must be 0 or 1.
   // 用以??CAN-ID ??RTR 位写入位??CO_TXCAN ??CO_RXCAN 数组的标识符??
   // 标识符与硬件寄存器对齐，RTR 位必须为 0 ??1
   #define CO_IDENT_WRITE(CobId, RTR)            (((CobId)<<5) | ((RTR)<<4))
   // 反函??用以读取 COB-ID
   #define CO_IDENT_READ_COB(CanMessage_Ident)   ((CanMessage_Ident.WORD[0]) >> 5)  //inverse function, read COB-ID
   // 反函??用以读取 RTR ??
   #define CO_IDENT_READ_RTR(CanMessage_Ident)   (CanMessage_Ident.BYTEbits[0].bit3)//inverse function, read RTR

   //reset microcontroller
   // 复位控制??
   #define CO_Reset()  Reset()

   //enable, disable interrupts - VERY IMPORTANT SECTION
   //使能、失能中??—??非常重要
      //disable/enable all interrupts from mainline procedure
      // 失能、使能所有中断，从主线程序中
      #define CO_DISABLE_ALL()            INTCONbits.GIEH = 0
      #define CO_ENABLE_ALL()             INTCONbits.GIEH = 1
      //disable/enable timer interrupt from mainline procedure
      // 失能、使能定时器中断，从主线程序??
      #define CO_DISABLE_TMR()            INTCONbits.GIEL = 0
      #define CO_ENABLE_TMR()             INTCONbits.GIEL = 1
      //disable/enable CANtx interrupt from mainline procedure
      // 失能、使能CAN发送中断，从主线程序中
      #define CO_DISABLE_CANTX()          INTCONbits.GIEL = 0
      #define CO_ENABLE_CANTX()           INTCONbits.GIEL = 1
      //disable/enable CANtx interrupt from timer procedure
      // 失能、使能CAN发送中断，从定时器程序??
      // 因为发送中断与定时器中断使用相同的等级，所以这里是空的
      #define CO_DISABLE_CANTX_TMR()      //nothing, beacuse same level interrupt is used for timer and CANtx
      #define CO_ENABLE_CANTX_TMR()
      //disable/enable CANrx interrupt from mainline procedure
      // 失能、使能CAN接收中断，从主线程序??
      #define CO_DISABLE_CANRX()          INTCONbits.GIEH = 0
      #define CO_ENABLE_CANRX()           INTCONbits.GIEH = 1
      //disable/enable CANrx interrupt from timer procedure
      // 使能、使能CAN接收中断，从定时器程序中
      #define CO_DISABLE_CANRX_TMR()      INTCONbits.GIEH = 0
      #define CO_ENABLE_CANRX_TMR()       INTCONbits.GIEH = 1

   //Read Transmit Buffer Status
   // 读取发送缓冲状??
      #define CO_CANTX_BUFFER_FREE()      (TXB0CONbits.TXREQ == 0)
   //Copy message to CAN buffers
   // 复制消息??CAN 发送缓冲器
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
