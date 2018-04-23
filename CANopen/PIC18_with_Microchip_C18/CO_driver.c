/*******************************************************************************

   CO_driver.c - Functions for CANopenNode - Processor specific
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

#include <string.h>

#include "CANopen.h"
#ifdef CO_SAVE_ROM
   #include "memcpyram2flash.h"
#endif

/*******************************************************************************
   Variables and functions      变量和函数
*******************************************************************************/
   //Variables used in inline assembly
   //内联汇编（嵌入）中使用的变量
   #pragma udata access CO_udata_Assembly
      //variables used in CO_CANrxIsr inline assembly
      // 匹配 CANid 的索引
      volatile near CO_DEFAULT_TYPE CO_IsrHighIndex;        //index of matched CANid
      // IsrHigh 以错误的长度写入收到的消息的索引
      volatile near CO_DEFAULT_TYPE CO_IsrHighErrorIndex;   //IsrHigh writes index of received message with wrong length
      volatile near unsigned char CO_IsrHighSignal;         //temp variable and IsrHigh signals to ProcessMain:
                                                            // IsrHigh 到 ProcessMain 的信号，临时变量   
                                                               //0xFF: Error in length of received message, index is stored
                                                               //      接收消息长度错误，索引已保存
                                                               //0xFE: Error - message arrives, but is inhibited
                                                               //      错误 - 消息已到达，但是被抑制
   #pragma udata
   
   // 变量必须在这里初始化
   //Node-ID
   unsigned char CO_NodeID;                                 //Variable must be intialized here
   unsigned char CO_BitRate;                                //Variable must be intialized here

   //eeprom
   #ifdef CO_SAVE_EEPROM
      extern unsigned char* CO_EEPROM_pointer;
      extern ROM unsigned int CO_EEPROM_size;
   #endif

   //function for verifying values at write to Object Dictionary
   //函数在写入对象字典时验证值
   unsigned long CO_OD_VerifyWrite(ROM CO_objectDictionaryEntry* pODE, void* data);

   //other
   extern volatile CO_DEFAULT_TYPE CO_TXCANcount;
   #if CO_NO_SYNC > 0
      extern volatile unsigned int CO_SYNCwindow;
   #endif

/*******************************************************************************
   CAN bit rates - Registers setup
   CAN 比特率 - 寄存器配置
*******************************************************************************/
ROM struct{
   unsigned char BRP;         //(1...64)Baud Rate Prescaler - 波特率预分频 (2n)/Fosc
   unsigned char SJW;         //(1...4) SJW time - 同步跳转宽度位 nxTQ
   unsigned char PROP;        //(1...8) PROP time - Propagation Time Select bits
   unsigned char PhSeg1;      //(1...8) Phase Segment 1 time
   unsigned char PhSeg2;      //(1...8) Phase Segment 2 time
}CO_BitRateData[8] =
#if CO_OSCILATOR_FREQ == 4
  {{10,  1, 8, 8, 3}, //CAN=10kbps
   {5,   1, 8, 8, 3}, //CAN=20kbps
   {2,   1, 8, 8, 3}, //CAN=50kbps
   {1,   1, 5, 8, 2}, //CAN=125kbps
   {1,   1, 2, 4, 1}, //CAN=250kbps
   {1,   1, 1, 1, 1}, //CAN=500kbps
   {1,   1, 1, 1, 1}, //CAN=800kbps     //Not possible
   {1,   1, 1, 1, 1}};//CAN=1000kbps    //Not possible
#elif CO_OSCILATOR_FREQ == 8
  {{25,  1, 5, 8, 2}, //CAN=10kbps
   {10,  1, 8, 8, 3}, //CAN=20kbps
   {5,   1, 5, 8, 2}, //CAN=50kbps
   {2,   1, 5, 8, 2}, //CAN=125kbps
   {1,   1, 5, 8, 2}, //CAN=250kbps
   {1,   1, 2, 4, 1}, //CAN=500kbps
   {1,   1, 1, 2, 1}, //CAN=800kbps
   {1,   1, 1, 1, 1}};//CAN=1000kbps
#elif CO_OSCILATOR_FREQ == 16
  {{50,  1, 5, 8, 2}, //CAN=10kbps
   {25,  1, 5, 8, 2}, //CAN=20kbps
   {10,  1, 5, 8, 2}, //CAN=50kbps
   {4,   1, 5, 8, 2}, //CAN=125kbps
   {2,   1, 5, 8, 2}, //CAN=250kbps
   {1,   1, 5, 8, 2}, //CAN=500kbps
   {1,   1, 3, 4, 2}, //CAN=800kbps
   {1,   1, 2, 3, 2}};//CAN=1000kbps
#elif CO_OSCILATOR_FREQ == 20
  {{50,  1, 8, 8, 3}, //CAN=10kbps
   {25,  1, 8, 8, 3}, //CAN=20kbps
   {10,  1, 8, 8, 3}, //CAN=50kbps
   {5,   1, 5, 8, 2}, //CAN=125kbps
   {2,   1, 8, 8, 3}, //CAN=250kbps
   {1,   1, 8, 8, 3}, //CAN=500kbps
   {1,   1, 8, 8, 3}, //CAN=800kbps     //Not possible
   {1,   1, 3, 4, 2}};//CAN=1000kbps
#elif CO_OSCILATOR_FREQ == 24
  {{63,  1, 8, 8, 2}, //CAN=10kbps
   {40,  1, 4, 8, 2}, //CAN=20kbps
   {15,  1, 5, 8, 2}, //CAN=50kbps
   {6,   1, 5, 8, 2}, //CAN=125kbps
   {3,   1, 5, 8, 2}, //CAN=250kbps
   {2,   1, 3, 6, 2}, //CAN=500kbps
   {1,   1, 5, 7, 2}, //CAN=800kbps
   {1,   1, 3, 6, 2}};//CAN=1000kbps
#elif CO_OSCILATOR_FREQ == 32
  {{64,  1, 8, 8, 8}, //CAN=10kbps
   {50,  1, 5, 8, 2}, //CAN=20kbps
   {20,  1, 5, 8, 2}, //CAN=50kbps
   {8,   1, 5, 8, 2}, //CAN=125kbps
   {4,   1, 5, 8, 2}, //CAN=250kbps
   {2,   1, 5, 8, 2}, //CAN=500kbps
   {2,   1, 3, 4, 2}, //CAN=800kbps
   {2,   1, 2, 3, 2}};//CAN=1000kbps
#elif CO_OSCILATOR_FREQ == 40
  {{50,  1, 8, 8, 3}, //CAN=10kbps      //Not possible
   {50,  1, 8, 8, 3}, //CAN=20kbps
   {25,  1, 5, 8, 2}, //CAN=50kbps
   {10,  1, 5, 8, 2}, //CAN=125kbps
   {5,   1, 5, 8, 2}, //CAN=250kbps
   {2,   1, 8, 8, 3}, //CAN=500kbps
   {1,   1, 8, 8, 8}, //CAN=800kbps
   {2,   1, 2, 5, 2}};//CAN=1000kbps
#else
   #error define_CO_OSCILATOR_FREQ CO_OSCILATOR_FREQ not supported
#endif

/*******************************************************************************
   CO_DriverInit - CANopenNode DRIVER INITIALIZATION
   This is mainline function and is called only in the startup of the program.

   CO_DriverInit - CANopenNode 驱动初始化
   属于主线程序，在程序启动时调用
*******************************************************************************/
void CO_DriverInit(void){
   CO_DEFAULT_SPEC unsigned int i;

   //Init Printed Board
   // 初始化状态显示（LED）
   PCB_RUN_LED_INIT();
   PCB_ERROR_LED_INIT();

   #ifdef CO_SAVE_EEPROM
      //Initialize Eeprom variables. Variables are copied from EEPROM to RAM.
      // 初始化 EEPROM 变量，将变量从 EEPROM 复制到 RAM。
      EECON1bits.EEPGD = 0;
      EECON1bits.CFGS = 0;
      i=CO_EEPROM_size;
      do{
         i--;
         EEADR = i;
         EECON1bits.RD = 1;
         *(CO_EEPROM_pointer + i) = EEDATA;
      }while(i>0);
   #endif
}

/*******************************************************************************
   CO_ProcessDriver - PROCESS MICROCONTROLLER SPECIFIC CODE
   This is mainline function and is called cyclically from CO_ProcessMain().

   CO_ProcessDriver - 处理 MCU 特定代码
   属于主线程序，从 CO_ProcessMain() 中循环调用。
*******************************************************************************/
void CO_ProcessDriver(void){
   static unsigned char COMSTATold = 0;
   static unsigned int EEPROM_index = 0;

   /* Verify CAN Errors *****************/
   /* 确认 CAN 错误       */
   if(COMSTAT != COMSTATold){   // 通信状态寄存器（COMSTAT）与旧值比较，是否变化
      if(COMSTAT & 0xC0){       // 接收缓冲溢出
         ErrorReport(ERROR_CAN_RXB_OVERFLOW, COMSTAT);
         COMSTAT &= 0x3F;       // 清零最高位
      }

      if(COMSTATbits.TXBO)      // 发送错误计数器 > 255, Bus-Off 置位
         ErrorReport(ERROR_CAN_TX_BUS_OFF, COMSTAT);  // 报错
      else if(ERROR_BIT_READ(ERROR_CAN_TX_BUS_OFF))
         ErrorReset(ERROR_CAN_TX_BUS_OFF, COMSTAT);   // 复位

      if(COMSTATbits.TXBP)      // 发送错误计数器 > 127, Bus Passive 置位
         ErrorReport(ERROR_CAN_TX_BUS_PASSIVE, COMSTAT);  // 报错
      else if(ERROR_BIT_READ(ERROR_CAN_TX_BUS_PASSIVE)){
         ErrorReset(ERROR_CAN_TX_BUS_PASSIVE, COMSTAT);   // 复位
         ErrorReset(ERROR_CAN_TX_OVERFLOW, 0);
         }

      if(COMSTATbits.RXBP)      // 接收错误计数器 > 127, Bus Passive 置位
         ErrorReport(ERROR_CAN_RX_BUS_PASSIVE, COMSTAT);  // 报错
      else if(ERROR_BIT_READ(ERROR_CAN_RX_BUS_PASSIVE))
         ErrorReset(ERROR_CAN_RX_BUS_PASSIVE, COMSTAT);   // 复位

      if(COMSTAT & 0x07)        // 发送、接收、错误警告位是否置位
         ErrorReport(ERROR_CAN_BUS_WARNING, COMSTAT);   // 报错
      else if(ERROR_BIT_READ(ERROR_CAN_BUS_WARNING)) 
         ErrorReset(ERROR_CAN_BUS_WARNING, COMSTAT);    // 置位

      COMSTATold = COMSTAT;
   }

   //verify errors from IsrHigh
   // 确认从 IsrHigh 传过来的错误信息
   if(CO_IsrHighSignal == 0xFF){
      if(CO_IsrHighErrorIndex >= CO_RXCAN_RPDO && CO_IsrHighErrorIndex < (CO_RXCAN_RPDO+CO_NO_RPDO))
           ErrorReport(ERROR_CO_RXMSG_PDOlength, CO_IsrHighErrorIndex - CO_RXCAN_RPDO);
      else ErrorReport(ERROR_CO_RXMSG_Length, CO_IsrHighErrorIndex);
      CO_IsrHighSignal = 0;
   }
   else if (CO_IsrHighSignal == 0xFE){
      ErrorReport(ERROR_CO_RXMSG_Inhibit, CO_IsrHighErrorIndex);
      CO_IsrHighSignal = 0;
   }

   /* Update EEprom variables ***********/
   /* 刷新 EEprom 变量              */
   #ifdef CO_SAVE_EEPROM
      //update one byte at a cycle 每个循环更新一个字节
      if(EECON1bits.WR == 0){ //write is not in progress  上次写入完成
         if(EEPROM_index == 0) EEPROM_index = CO_EEPROM_size;
         EEPROM_index--;
         //read eeprom
         EEADR = EEPROM_index;
         EECON1bits.EEPGD = 0;  // 访问 EEPROM 存储器
         EECON1bits.CFGS = 0;   // 访问 EEPROM 或 Flash 存储器
         EECON1bits.RD = 1;     // 读控制
         if(EEDATA != *(CO_EEPROM_pointer + EEPROM_index)){
            //write to EEPROM
            EEDATA = *(CO_EEPROM_pointer + EEPROM_index);
            EECON1bits.WREN = 1;  // 写使能
            CO_DISABLE_ALL();     // 关中断
            EECON2 = 0x55;
            EECON2 = 0xAA;
            EECON1bits.WR = 1;
            CO_ENABLE_ALL();      // 开中断
            EECON1bits.WREN = 0;  // 写失能
         }
      }
   #endif
}

/*******************************************************************************
   CO_Read_NodeId_BitRate - READ NODE-ID AND CAN BITRATE
   This is mainline function and is called from Communication reset. Usually
   NodeID and BitRate are read from DIP switches.

   CO_Read_NodeId_BitRate - 读 NODE-ID 和 CAN 比特率
   属于主线程序，在复位通信时调用。通常 NODE-ID 和比特率从 DIP 开关读取。
*******************************************************************************/
void CO_Read_NodeId_BitRate(void){
   CO_NodeID = ODE_CANnodeID;    //range 1 to 127
   CO_BitRate = ODE_CANbitRate;  //range 0 to 7
                                    // 0 = 10 kbps    1 = 20 kbps
                                    // 2 = 50 kbps    3 = 125 kbps
                                    // 4 = 250 kbps   5 = 500 kbps
                                    // 6 = 800 kbps   7 = 1000 kbps

   if(CO_NodeID==0 || CO_NodeID>127 || CO_BitRate > 7){
      ErrorReport(ERROR_WrongNodeIDorBitRate, (CO_NodeID<<8)|CO_BitRate);
      CO_NodeID = 1;
      if(CO_BitRate > 7) CO_BitRate = 1;
   }
}

/*******************************************************************************
   CO_SetupCAN - INIT CAN controller interface
   This is mainline function and is called from Communication reset.

   CO_SetupCAN - 初始化 CAN 控制器接口
   属于主线程序，在复位通信时调用
*******************************************************************************/
void CO_SetupCAN(void){

/* Setup driver variables 设置驱动变量 */
   CO_IsrHighSignal = 0;

//////////////////////////
SET_CAN_TRIS();
//TRISBbits.TRISB3 = 1;
// TRISBbits.TRISB3 = 0;
// LATBbits.LATB3 = 1; 
// CANCONbits.ABAT=1;
//////////////////////////
/* Setup CAN bus 配置 CAN bus */
   CANCON = 0x80;       //request configuration mode
                        // 请求配置模式
//////////////////////////
// CANCONbits.ABAT=0;
//////////////////////////
   while((CANSTAT & 0xE0) != 0x80); //wait until configuration mode is set
                                    // 等待，直到运行模式状态位被设置。
   // 波特率控制寄存器配置
   BRGCON1 = ((CO_BitRateData[CO_BitRate].SJW-1) <<6 ) | (CO_BitRateData[CO_BitRate].BRP-1);
   BRGCON2 = 0x80 | ((CO_BitRateData[CO_BitRate].PhSeg1-1)<<3) | (CO_BitRateData[CO_BitRate].PROP-1);
   BRGCON3 = (CO_BitRateData[CO_BitRate].PhSeg2-1);

   // 引脚设置
   SET_CAN_TRIS();

   // 接收、发送缓冲清零
   RXB0CONbits.RXFUL = 0;  //clear rx and tx buffers
   RXB1CONbits.RXFUL = 0;   // 接收缓冲打开以接收新消息
   TXB0CONbits.TXREQ = 0;   // 发送请求状态位，消息成功发送后自动清零
   TXB1CONbits.TXREQ = 0;
   TXB2CONbits.TXREQ = 0;

   PIE3 = 0;            //CAN interrupts disable          中断失能
   PIR3 = 0;            //CAN interrupts flags clear      中断标志位清零
   IPR3 = 0;            //CAN interrupts low priority set 低优先级
   CIOCON = 0x20;       //can I/O control register        CAN IO 控制寄存器
                        // bit 5: ENDRHI - 1，CANTX 引脚在 recessive 时为 VDD
                        // bit 4: CANCAP - 0，CAN 捕捉禁用，RC2/CCP1 输入 为 CCP1 模式
   // 接收缓冲控制寄存器
   RXB0CON = 0x24;      //receive valid messages with standard identifier, if overflow, move to buffer1
                        // bit 6-5: RXM1-0: 01 = 仅接收标准过滤器的有效消息
                        // bit 2: RXB0DBEN: 1  = 接收缓冲 0 溢出则写入接收缓冲 1
   RXB1CON = 0x20;      //receive valid messages with standard identifier
                        // bit 6-5: RXM1-0: 01 = 仅接收标准过滤器的有效消息，RXFnSIDL 中的 EXIDEN 位必须为 0
   

   //set filters
   // 配置过滤器
   RXF0SIDH = 0; RXF0SIDL = 0;
   RXF1SIDH = 0; RXF1SIDL = 0;
   RXF2SIDH = 0; RXF2SIDL = 0;
   RXF3SIDH = 0; RXF3SIDL = 0;
   RXF4SIDH = 0; RXF4SIDL = 0;
   RXF5SIDH = 0; RXF5SIDL = 0;
   #if (CO_RXCAN_NO_MSGS <= 6)//is enough hardware filters?
      RXM0SIDH = 0xFF; RXM0SIDL = 0xFF;
      RXM1SIDH = 0xFF; RXM1SIDL = 0xFF;
      #if(CO_RXCAN_NO_MSGS > 0)
         RXF0SIDH = CO_RXCAN[0].Ident.BYTE[1]; RXF0SIDL = CO_RXCAN[0].Ident.BYTE[0];
      #endif
      #if(CO_RXCAN_NO_MSGS > 1)
         RXF1SIDH = CO_RXCAN[1].Ident.BYTE[1]; RXF1SIDL = CO_RXCAN[1].Ident.BYTE[0];
      #endif
      #if(CO_RXCAN_NO_MSGS > 2)
         RXF2SIDH = CO_RXCAN[2].Ident.BYTE[1]; RXF2SIDL = CO_RXCAN[2].Ident.BYTE[0];
      #endif
      #if(CO_RXCAN_NO_MSGS > 3)
         RXF3SIDH = CO_RXCAN[3].Ident.BYTE[1]; RXF3SIDL = CO_RXCAN[3].Ident.BYTE[0];
      #endif
      #if(CO_RXCAN_NO_MSGS > 4)
         RXF4SIDH = CO_RXCAN[4].Ident.BYTE[1]; RXF4SIDL = CO_RXCAN[4].Ident.BYTE[0];
      #endif
      #if(CO_RXCAN_NO_MSGS > 5)
         RXF5SIDH = CO_RXCAN[5].Ident.BYTE[1]; RXF5SIDL = CO_RXCAN[5].Ident.BYTE[0];
      #endif
   #else
      RXM0SIDH = 0; RXM0SIDL = 0;
      RXM1SIDH = 0; RXM1SIDL = 0;
   #endif

   CANCON = 0x00;       //request normal mode
                        // 请求正常模式

   while((CANSTAT & 0xE0) != 0x00); //wait until normal mode is set
                        // 等待，直到正常模式状态位被设置

   PIE3 = 0x07;         //enable two receive interrupts and TX0 interrupt
                        // 使能 TXB0IE,RXB1IE, RXB0IE,两个接收中断和一个发送中断
   IPR3 = 0x03;         //receive interrupts are high priority
                        // 使能 RXB1IP,RXB0IP, 接收中断高优先级
}

 /*******************************************************************************
   CO_CANrxIsr - CAN RECEIVE INTERRUPT
   This function is invoked by interrupt, when new can message receive from CAN
   network. It searches CO_RXCAN[] array and compares COB-ID. If found, copy data
   to appropriate buffer.

   CO_CANrxIsr - CAN 接收中断处理函数
   当从网络接收到新的 CAN 消息时，该函数被中断函数调用。它查找 CO_RXCAN[] 数组并比较
   COB-ID，如果找到，则复制数据到适当的缓冲器。
*******************************************************************************/
#pragma interrupt CO_CANrxIsr save = CANCON
void CO_CANrxIsr(void){
   if(PIR3bits.RXB0IF){ //RXB0 occurred    接收中断 0 产生
      PIR3bits.RXB0IF = 0;
      CANCONbits.WIN2 = 1; CANCONbits.WIN1 = 1; CANCONbits.WIN0 = 0;//set RXB0 to access memory
                                                                    // 设置接收缓冲 0 访问存储器
   }
   else{ //RXB1 occurred
      PIR3bits.RXB1IF = 0;
      CANCONbits.WIN2 = 1; CANCONbits.WIN1 = 0; CANCONbits.WIN0 = 1;//set RXB1 to access memory
                                                                    // 设置接收缓冲 1 访问存储器   
   }

  /* Equivalent code in C:
   // 等效 C 代码：
   //search entry in CO_RXCAN[] array with matched COB_ID
   // 查找 CO_RXCAN[] 数组中的条目，是否有匹配的 COB-ID
   for(CO_IsrHighIndex=0; CO_IsrHighIndex<CO_RXCAN_NO_MSGS; CO_IsrHighIndex++)
      if(CO_RXCAN[CO_IsrHighIndex].Ident.BYTE[0] == RXB0SIDL &&
         CO_RXCAN[CO_IsrHighIndex].Ident.BYTE[1] == RXB0SIDH) break;
   //Verify, if message is matched
   // 验证，如果消息匹配
   if(CO_IsrHighIndex < CO_RXCAN_NO_MSGS){
      //Verify length
      // 验证长度
      if((CO_RXCAN[CO_IsrHighIndex].NoOfBytes <= 8) &&
         (CO_RXCAN[CO_IsrHighIndex].NoOfBytes != (RXB0DLC&0x0F))){
         CO_IsrHighSignal = 0xFF;
         CO_IsrHighErrorIndex = CO_IsrHighIndex;
      }
      //Verify inhibit
      // 验证抑制
      else if(CO_RXCAN[CO_IsrHighIndex].NewMsg &&
              CO_RXCAN[CO_IsrHighIndex].Inhibit){
         CO_IsrHighSignal = 0xFE;
         CO_IsrHighErrorIndex = CO_IsrHighIndex;
      }
      else{
         //Set flag and copy data
         // 设置标志位并复制数据
         CO_RXCAN[CO_IsrHighIndex].NewMsg = 1;
         memcpy((void*)&CO_RXCAN[CO_IsrHighIndex].Data, (void*)&RXB0D0, RXB0DLC&0x0F);
      }
   }
   //release receive buffer
   // 释放接收缓冲
   RXB0CONbits.RXFUL = 0;
  */

   //CO_CanMessage structure must not be changed
   _asm
      //search entry in CO_RXCAN[] array with matched COB_ID
      // 查找 CO_RXCAN[] 数组中的条目，是否有匹配的 COB-ID
         LFSR     0, CO_RXCAN
         CLRF     CO_IsrHighIndex, 0
      CO_IH_1:
         //compare  比较
         MOVF     POSTINC0, 0, ACCESS
         SUBWF    RXB0SIDL, 0, ACCESS
         BNZ      CO_IH_2

         MOVF     INDF0, 0, ACCESS
         SUBWF    RXB0SIDH, 0, ACCESS
         BNZ      CO_IH_2

         BRA      CO_IH_3
      CO_IH_2:
         //increment indexes for next entry
         // 索引自增，下一条目
         MOVLW    0x0A
         ADDWF    FSR0L, 1, ACCESS
         MOVLW    0
         ADDWFC   FSR0H, 1, ACCESS
         INCF     CO_IsrHighIndex, 1, ACCESS
         //end of array?
         // 数组尾？
         MOVLW    CO_RXCAN_NO_MSGS
         SUBWF    CO_IsrHighIndex, 0, ACCESS
         BNZ      CO_IH_1
      CO_IH_3:
      //Verify, if message is matched
      // 验证，如果消息匹配  
         MOVLW    CO_RXCAN_NO_MSGS
         SUBWF    CO_IsrHighIndex, 0, ACCESS
         BNC      CO_IH_10
         BRA      CO_IH_END
      CO_IH_10:
      //Verify length
      // 验证长度
         MOVLW    0x0F
         ANDWF    PREINC0, 0, ACCESS         //FSR0 now points to CO_RXCAN[CO_IsrHighIndex].NoOfBytes (complete byte)
         MOVWF    CO_IsrHighSignal, ACCESS   //store length temporary
         SUBLW    0x08
         BNC      CO_IH_11

         MOVLW    0x0F
         ANDWF    RXB0DLC, 0, ACCESS
         SUBWF    CO_IsrHighSignal, 0, ACCESS
         BZ       CO_IH_11

         MOVLW    0xFF
         MOVWF    CO_IsrHighSignal, ACCESS
         MOVF     CO_IsrHighIndex, 0, ACCESS
         MOVWF    CO_IsrHighErrorIndex, ACCESS
         BRA      CO_IH_END
      CO_IH_11:
      //Verify inhibit
         MOVLW    0x10        //NewMsg bit
         ANDWF    INDF0, 0, ACCESS
         BZ       CO_IH_20

         MOVLW    0x20        //Inhibit bit
         ANDWF    INDF0, 0, ACCESS
         BZ       CO_IH_20

         MOVLW    0xFE
         MOVWF    CO_IsrHighSignal, ACCESS
         MOVF     CO_IsrHighIndex, 0, ACCESS
         MOVWF    CO_IsrHighErrorIndex, ACCESS
         BRA      CO_IH_END
      CO_IH_20:
      //Set flag and copy data
         BSF      POSTINC0, 4, ACCESS  //FSR0 now points to CO_RXCAN[CO_IsrHighIndex].Data

         MOVLW    0x0F
         ANDWF    RXB0DLC, 0, ACCESS
         BZ       CO_IH_30
         MOVWF    CO_IsrHighSignal, ACCESS   //store length temporary
         MOVFF    RXB0D0, POSTINC0
         DCFSNZ   CO_IsrHighSignal, 1, ACCESS
         BRA      CO_IH_30
         MOVFF    RXB0D1, POSTINC0
         DCFSNZ   CO_IsrHighSignal, 1, ACCESS
         BRA      CO_IH_30
         MOVFF    RXB0D2, POSTINC0
         DCFSNZ   CO_IsrHighSignal, 1, ACCESS
         BRA      CO_IH_30
         MOVFF    RXB0D3, POSTINC0
         DCFSNZ   CO_IsrHighSignal, 1, ACCESS
         BRA      CO_IH_30
         MOVFF    RXB0D4, POSTINC0
         DCFSNZ   CO_IsrHighSignal, 1, ACCESS
         BRA      CO_IH_30
         MOVFF    RXB0D5, POSTINC0
         DCFSNZ   CO_IsrHighSignal, 1, ACCESS
         BRA      CO_IH_30
         MOVFF    RXB0D6, POSTINC0
         DCFSNZ   CO_IsrHighSignal, 1, ACCESS
         BRA      CO_IH_30
         MOVFF    RXB0D7, POSTINC0
         CLRF     CO_IsrHighSignal, ACCESS
      CO_IH_30:
      CO_IH_END:
      //release receive buffer
         BCF      RXB0CON, 0x7, ACCESS
   _endasm

}

/*******************************************************************************
   CO_CANtxIsr - CAN TRANSMIT INTERRUPT
   This function is invoked by interrupt, when previous message has been
   successfully sent. It searches CO_TXCAN[] array
   and finds which message has to be sent. Then copies data to CAN buffer.

   CO_CANtxIsr - CAN 发送中断处理函数
   当之前的消息成功发送后，该函数被中断函数调用。它查询 CO_TXCAN[] 数组并找出要发送那条
   消息，然后将数据复制到缓冲器。
*******************************************************************************/
#pragma interruptlow CO_CANtxIsr
void CO_CANtxIsr(void){
   PIR3bits.TXB0IF = 0;
   if(CO_TXCANcount > 0 && CO_CANTX_BUFFER_FREE()){    //are new messages present
      CO_DEFAULT_SPEC CO_DEFAULT_TYPE i;
      CO_TXCANcount--;
      for(i=0; i<CO_TXCAN_NO_MSGS; i++){
         if(CO_TXCAN[i].NewMsg){
            #if CO_NO_SYNC > 0
               //messages with Inhibit flag set (synchronous PDOs) must be transmited inside preset window
               if(CO_TXCAN[i].Inhibit && CO_SYNCwindow && CO_SYNCtime > CO_SYNCwindow)
                  ErrorReport(ERROR_TPDO_OUTSIDE_WINDOW, i);
               else
            #endif
               CO_TXCAN_COPY_TO_BUFFERS(i);
            //release buffer
            CO_TXCAN[i].NewMsg = 0;
            break;
         }
      }
   }
}

/*******************************************************************************
   CO_OD_Read - OBJECT DICTIONARY READ
   This is mainline function. It is called from SDO server and reads data from
   object dictionary. Since object dictionary variables can use many types of
   memory, some code is processor specific.

   CO_OD_Read - 读取对象字典
   属于主线程序。被 SDO 服务器调用，从对象字典中读取数据。因为对象字典变量可以使用多种
   存储类型，故代码是处理器特定的。

   PARAM pODE: pointer to object dictionary entry, found by CO_FindEntryInOD().
               指向对象字典条目的指针
   PARAM pBuff: pointer to buffer where data will be written.
                指向数据写入缓冲位置的指针
   PARAM BuffMaxSize: Size of buffer (to prevent overflow).
                      缓冲长度，为防止溢出
   RETURN: 0 if success, otherwise abort code
   返回值：0 = 成功，否则返回终止代码
*******************************************************************************/
unsigned long CO_OD_Read(ROM CO_objectDictionaryEntry* pODE, void* pBuff, unsigned char sizeOfBuff){
   if(pODE->length > sizeOfBuff) return 0x05040005L;  //Out of memory
                                                      // 超出存储范围
   if((pODE->attribute&0x07) == ATTR_WO) return 0x06010001L;  //attempt to read a write-only object
                                                              // 所读对象为只写属性
   //read data from memory (processor specific code)
   // 从存储器读数据（控制器特定代码）
   if((unsigned int)pODE->pData>=0x1000 && (pODE->attribute&ATTR_ROM)){
      CO_DISABLE_ALL();
      memcpypgm2ram(pBuff, pODE->pData, pODE->length);
      CO_ENABLE_ALL();
   }
   else if((unsigned int)pODE->pData<0x1000 && !(pODE->attribute&ATTR_ROM)){
      CO_DISABLE_ALL();
      memcpy(pBuff, (void*)pODE->pData, pODE->length);
      CO_ENABLE_ALL();
   }
   else return 0x06040047L;  //General internal incompatibility in the device
                              // 设备内部不兼容
   return 0;
}

/*******************************************************************************
   CO_OD_Write - OBJECT DICTIONARY WRITE
   This is mainline function. It is called from SDO server and writes data to
   object dictionary. Since object dictionary variables can use many types of
   memory, some code is processor specific.

   CO_OD_Write - 写入对象字典
   属于主线程序。被 SDO 服务器调用，向对象字典写入数据。因为对象字典变量可以使用多种
   存储类型，故代码是处理器特定的。

   PARAM pODE: pointer to object dictionary entry, found by CO_FindEntryInOD().
               指向对象字典条目的指针
   PARAM pBuff: pointer to buffer which contains data to be written.
                指向被写入数据缓存的指针
   PARAM dataSize: Size of data in buffer.
                   缓存长度
   RETURN: 0 if success, otherwise abort code
   返回值： 0 = 成功，否则返回终止代码
*******************************************************************************/
unsigned long CO_OD_Write(ROM CO_objectDictionaryEntry* pODE, void* data, unsigned char length){
   #ifdef CO_VERIFY_OD_WRITE
      unsigned long AbortCode;
   #endif
   if((pODE->attribute&0x07) == ATTR_RO || (pODE->attribute&0x07) == ATTR_CO) return 0x06010002L;   //attempt to write a read-only object
                                                                                                    //尝试写入一个只读对象
   if(length != pODE->length) return 0x06070010L;   //Length of service parameter does not match
                                                    // 服务参数长度不匹配
   #ifdef CO_VERIFY_OD_WRITE
      //verify value
      // 验证值
      AbortCode = CO_OD_VerifyWrite(pODE, data);
      if(AbortCode) return AbortCode;
   #endif

   //write data to memory (processor specific code)
   // 想存储器写入数据（处理器特定代码）
   //RAM data
   if((unsigned int)pODE->pData<0x1000 && (pODE->attribute&ATTR_ROM) == 0){
      CO_DISABLE_ALL();
      memcpy((void*)pODE->pData, data, length);
      CO_ENABLE_ALL();
   }
   //ROM data (all ROM data are above address 0x1000 in PIC)
   else if((unsigned int)pODE->pData>=0x1000 && (pODE->attribute&ATTR_ROM)){
      #ifdef CO_SAVE_ROM
         CO_DISABLE_ALL();
         //following function takes ~32 miliseconds (1ms timer does not count them)
         // 下面的函数需要占用约 32 毫秒（1 ms 定时器不计入它们）
         memcpyram2flash((rom void*)pODE->pData, data, length);
         CO_ENABLE_ALL();
      #else
         return 0x06010002L;     //Attempt to write a read only object
                                 // 尝试写入一个只读对象
      #endif
   }
   else return 0x06040047L;  //General internal incompatibility in the device
                              // 设备内部不兼容
   return 0;
}


