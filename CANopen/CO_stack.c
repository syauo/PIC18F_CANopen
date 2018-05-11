/*******************************************************************************

   CO_stack.c - Functions for CANopenNode - processor independent

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

/*******************************************************************************
   VERIFY SIZES   确认设置
*******************************************************************************/
   #if CO_NO_SYNC != 0 && CO_NO_SYNC != 1
      #error defineCO_NO_SYNC (CO_NO_SYNC) not correct!
   #endif
   #if CO_NO_EMERGENCY != 0 && CO_NO_EMERGENCY != 1
      #error defineCO_NO_EMERGENCY (CO_NO_EMERGENCY) not correct!
   #endif
   #if CO_NO_RPDO < 0 || CO_NO_RPDO > 512
      #error define_CO_NO_RPDO (CO_NO_RPDO) not valid
   #endif
   #if CO_NO_TPDO < 0 || CO_NO_TPDO > 512
      #error define_CO_NO_TPDO (CO_NO_TPDO) not valid
   #endif
   #if CO_NO_SDO_SERVER < 0 || CO_NO_SDO_SERVER > 128
      #error define_CO_NO_SDO_SERVER (CO_NO_SDO_SERVER) not valid
   #endif
   #if CO_NO_SDO_CLIENT < 0 || CO_NO_SDO_CLIENT > 128
      #error define_CO_NO_SDO_CLIENT (CO_NO_SDO_CLIENT) not valid
   #endif
   #if CO_NO_CONS_HEARTBEAT < 0 || CO_NO_CONS_HEARTBEAT > 255
      #error define_CO_NO_CONS_HEARTBEAT (CO_NO_CONS_HEARTBEAT) not valid
   #endif
   #if CO_NO_USR_CAN_RX < 0
      #error define_CO_NO_USR_CAN_RX (CO_NO_USR_CAN_RX) not valid
   #endif
   #if CO_NO_USR_CAN_TX < 0
      #error define_CO_NO_USR_CAN_TX (CO_NO_USR_CAN_TX) not valid
   #endif
   #if CO_RXCAN_NO_MSGS > CO_DEFAULT_TYPE_SIZE
      #error number of receiving messages CO_RXCAN_NO_MSGS too large
   #endif
   #if CO_TXCAN_NO_MSGS > CO_DEFAULT_TYPE_SIZE
      #error number of transmiting messages CO_TXCAN_NO_MSGS too large
   #endif
   #if CO_NO_ERROR_FIELD < 0 || CO_NO_ERROR_FIELD > 254
      #error define_CO_NO_ERROR_FIELD (CO_NO_ERROR_FIELD) not valid
   #endif
   #if CO_MAX_OD_ENTRY_SIZE < 4 || CO_MAX_OD_ENTRY_SIZE > 256
      #error define_CO_MAX_OD_ENTRY_SIZE (CO_MAX_OD_ENTRY_SIZE) not valid
   #endif


/*******************************************************************************
   Functions, implemented by user, used in CO_stack.c

   CO_stack.c 中交由用户实现的函数 声明
*******************************************************************************/
   void User_ResetComm(void);
   void User_Process1msIsr(void);


/*******************************************************************************
   Functions from CO_driver.c used in CO_stack.c
   
   CO_driver.c 中的函数调用声明
*******************************************************************************/
   // 节点号 NodeId 与比特率
   void CO_Read_NodeId_BitRate(void);  //determine NodeId and BitRate
   // 控制器 CAN 配置
   void CO_SetupCAN(void);             //setup CAN controller
   // MCU 特定处理代码
   void CO_ProcessDriver(void);        //process microcontroller specific code
   //object dictionary read/write functions
   // 对象字典读写函数
   unsigned long CO_OD_Read(ROM CO_objectDictionaryEntry* pODE, void* pBuff, unsigned char BuffMaxSize);
   unsigned long CO_OD_Write(ROM CO_objectDictionaryEntry* pODE, void* pBuff, unsigned char dataSize);


/*******************************************************************************
   Variables  变量
*******************************************************************************/
//timer variables and macros (immune on overflow)
// 定时器变量
   // 16 位，每 1ms 自增
   volatile unsigned int CO_Timer16bit1ms = 0;   //16-bit variable, increments every 1ms
   // 8 位，每 100ms 自增
   volatile unsigned char CO_Timer8bit100ms = 0; //8-bit variable, increments every 100ms
   #define TMR8Z(ttimerVariable)   (unsigned char) (ttimerVariable = CO_Timer8bit100ms)
   #define TMR8(ttimerVariable)    (unsigned char) (CO_Timer8bit100ms - ttimerVariable)

//when CO_Timer1msIsr is executing, this variable is set to 1. This way some functions
//distinguish between being called from mainline or timer functions
// 当 CO_Timer1msIst 执行，该变量置1.可以区分函数是 mainline 还是定时器调用。
   volatile unsigned char CO_Timer1msIsr_executing = 0;

//Status LEDs
   volatile CO_StatusLED_struct CO_StatusLED = {1, 0};

//CANopen nodeID
   extern unsigned char CO_NodeID;

//CAN transmit
   volatile CO_DEFAULT_TYPE CO_TXCANcount;

//main variables for storing CAN messages, filled at CanInit, see CANopen.h for indexes
// 存储 CAN 消息的变量
   #pragma udata CO_udata_RXCAN
      volatile CO_CanMessage CO_RXCAN[CO_RXCAN_NO_MSGS];
   #pragma udata

   #pragma udata CO_udata_TXCAN
      volatile CO_CanMessage CO_TXCAN[CO_TXCAN_NO_MSGS];
   #pragma udata

//NMT operating state of node
   volatile unsigned char CO_NMToperatingState;

//SYNC
   #if CO_NO_SYNC > 0
      volatile unsigned int CO_SYNCcounter;  //variable is incremented after SYNC message
      volatile unsigned int CO_SYNCtime;     //variable is incremented every 1ms, after SYNC message it is set to 0
      volatile unsigned int CO_SYNCwindow;   //window [ms]
      //(CO_RXCAN[CO_RXCAN_SYNC].Data is not used by CAN receive). It is used here to save space
      #define CO_SYNCperiod         CO_RXCAN[CO_RXCAN_SYNC].Data.WORD[0]   //period [ms]
      #define CO_SYNCperiodTimeout  CO_RXCAN[CO_RXCAN_SYNC].Data.WORD[1]   //1,5 * CO_SYNCperiod [ms]
      #define CO_SYNCperiodEarly    CO_RXCAN[CO_RXCAN_SYNC].Data.WORD[2]   //0,5 * CO_SYNCperiod [ms]
      #define CO_SYNCproducer       CO_RXCAN[CO_RXCAN_SYNC].Data.BYTEbits[6].bit0   //if 1, node is SYNC producer
   #endif

//Emergency message control and Error codes (see CO_errors.h)
// 紧急消息控制和错误代号
   volatile struct{
      unsigned int CheckErrors     :1;
      unsigned int EmergencyToSend :1;
      unsigned int EmergencyErrorCode;
      unsigned char ErrorBit;
      unsigned int CodeVal;
   }ErrorControl;
   unsigned char CO_ErrorStatusBits[ERROR_NO_OF_BYTES];
   ROM unsigned int ErrorCodesTable[] = ERROR_CODES;

//SDO Server
   #if CO_NO_SDO_SERVER > 0
      struct{
         ROM CO_objectDictionaryEntry* pODE;          //Location of curent object dictionary entry
         unsigned char State;                         //state of SDO server
                     //bit1: 1=segmented download in progress
                     //bit2: 1=segmented upload in progress
                     //bit4: toggle bit in previous object
         #if CO_MAX_OD_ENTRY_SIZE > 0x04
            unsigned char Buff[CO_MAX_OD_ENTRY_SIZE]; //buffer for segmented transfer
            unsigned char BuffSize;                   //position in Buff of next data segment being read/written
            unsigned char tTimeout;                   //timeout timer
         #endif
      }CO_SDOserverVar[CO_NO_SDO_SERVER];
   #endif

//SDO Client
   #if CO_NO_SDO_CLIENT > 0
      struct{
         unsigned char State;                     //state of SDO client
                     //bit1: 1=segmented download in progress
                     //bit2: 1=segmented upload in progress
                     //bit4: toggle bit in previous object
                     //bit6: 1=download initiated
                     //bit7: 1=upload initiated
         unsigned char* pBuff;                     //pointer to data buffer supplied by user
         unsigned char BuffMaxSize;                //by download: size of data in buffer; by upload: size of buffer
         unsigned char BuffSize;                   //position in Buff of next data segment being read/written
         unsigned char tTimeout;                   //timeout timer
      }CO_SDOclientVar[CO_NO_SDO_CLIENT];
   #endif

//PDOs
   #if CO_NO_TPDO > 0
      #if CO_NO_SYNC > 0
         volatile unsigned char CO_TPDO_SyncTimer[CO_NO_TPDO];    //SYNC timer used for trigger PDO sending
      #endif
      #ifdef CO_TPDO_INH_EV_TIMER
         volatile unsigned int CO_TPDO_InhibitTimer[CO_NO_TPDO];  //Inhibit timer used for inhibit PDO sending
         volatile unsigned int CO_TPDO_EventTimer[CO_NO_TPDO];    //Event timer used for trigger PDO sending
      #endif
   #endif

//HeartBeat Consumer (CO_RXCAN[CO_RXCAN_CONS_HB].Data (except first byte) is not used by CAN receive)
   volatile unsigned char CO_HBcons_AllMonitoredOperational = 0;
   unsigned char CO_HBcons_AllMonitoredOperationalCpy;
   #if CO_NO_CONS_HEARTBEAT > 0
      //#define CO_HBcons_NMTstate(i)    CO_RXCAN[CO_RXCAN_CONS_HB+i].Data.BYTE[1]//defined in CANopen.h
      #define CO_HBcons_TimerValue(i)  CO_RXCAN[CO_RXCAN_CONS_HB+i].Data.WORD[1]
      #define CO_HBcons_MonStarted(i)  CO_RXCAN[CO_RXCAN_CONS_HB+i].Data.BYTE[4]
   #endif


/*******************************************************************************
   CO_ResetComm - RESET COMMUNICATION
   Function is called after start of program and after CANopen NMT command: Reset
   Communication. It resets CAN interface and majority of CANopen variables.
   It also calls User_ResetComm() function.

   复位 CAN 接口和 主要的 CANopen 变量。
*******************************************************************************/
void CO_ResetComm(void){
   CO_DEFAULT_SPEC CO_DEFAULT_TYPE i;
   unsigned int ii;

   // 绿灯灭，NMT 初始化状态
   PCB_RUN_LED(CO_StatusLED.Off);
   CO_NMToperatingState = NMT_INITIALIZING;
   
   // 读节点号和比特率
   CO_Read_NodeId_BitRate();

/* Clear arrays 
   清零接收和发送数组*/
   for(ii=0; ii<sizeof(CO_RXCAN); ii++)
      *(((unsigned char*)CO_RXCAN)+ii) = 0;
   for(ii=0; ii<sizeof(CO_TXCAN); ii++)
      *(((unsigned char*)CO_TXCAN)+ii) = 0;

/* Setup variables 
   设置变量        */

   //Sync
   #if CO_NO_SYNC > 0
      CO_SYNCperiod = (unsigned int)(ODE_Communication_Cycle_Period / 1000);
      CO_SYNCperiodTimeout = (unsigned int)((ODE_Communication_Cycle_Period * 3) / 2000);
      CO_SYNCperiodEarly = (unsigned int)((ODE_Communication_Cycle_Period) / 2000);
      CO_SYNCproducer = (ODE_SYNC_COB_ID & 0x40000000L)?1:0;
      CO_SYNCwindow = (unsigned int)(ODE_Synchronous_Window_Length / 1000);
      CO_SYNCcounter = 0;
      CO_SYNCtime = 0;
   #endif

   //Errors and Emergency
   ErrorControl.EmergencyToSend = 0;
   ErrorControl.CheckErrors = 1;
   for(i=0; i<ERROR_NO_OF_BYTES; i++)
      CO_ErrorStatusBits[i] = 0;
   ODE_Error_Register = 0;
   #if CO_NO_ERROR_FIELD > 0
      ODE_Pre_Defined_Error_Field_NoOfErrors = 0;
      for(i=0; i<CO_NO_ERROR_FIELD; i++)
         ODE_Pre_Defined_Error_Field[i] = 0;
   #endif

   //CAN transmit
   CO_TXCANcount = 0;

   //SDO Server
   #if CO_NO_SDO_SERVER > 0
      for(i=0; i<CO_NO_SDO_SERVER; i++) CO_SDOserverVar[i].State = 0;
   #endif

   //SDO Client
   #if CO_NO_SDO_CLIENT > 0
      for(i=0; i<CO_NO_SDO_CLIENT; i++) CO_SDOclientVar[i].State = 0;
   #endif

   //PDOs
   #if CO_NO_TPDO > 0
      for(i=0; i<CO_NO_TPDO; i++){
         #if CO_NO_SYNC > 0
            CO_TPDO_SyncTimer[i] = ODE_TPDO_Parameter[i].Transmission_type;
         #endif
         #ifdef CO_TPDO_INH_EV_TIMER
            CO_TPDO_InhibitTimer[i] = 0;
            CO_TPDO_EventTimer[i] = ODE_TPDO_Parameter[i].Event_Timer;
         #endif
      }
   #endif

/* User function */
   User_ResetComm();

/* Setup CO_RXCAN array */
   CO_RXCAN[CO_RXCAN_NMT].Ident.WORD[0] = CO_IDENT_WRITE(0, 0);
   CO_RXCAN[CO_RXCAN_NMT].NoOfBytes = 2;
   #if CO_NO_SYNC > 0
      CO_RXCAN[CO_RXCAN_SYNC].Ident.WORD[0] = CO_IDENT_WRITE((unsigned int)ODE_SYNC_COB_ID, 0);
      CO_RXCAN[CO_RXCAN_SYNC].NoOfBytes = 0;
      CO_RXCAN[CO_RXCAN_SYNC].Inhibit = 1;
   #endif
   #if CO_NO_RPDO > 0
      for(i=CO_RXCAN_RPDO; i<CO_RXCAN_RPDO+CO_NO_RPDO; i++){
         unsigned char length;
         #ifdef CO_PDO_MAPPING_IN_OD
            unsigned char j;
            //calculate length from mapping
            length = 7;  //round up to use whole byte
            for(j=0; j<8; j++)
               length += (unsigned char)ODE_RPDO_Mapping[i-CO_RXCAN_RPDO][j];
            length >>= 3;
            if(length > 8){
               length = 8;
               ErrorReport(ERROR_wrong_PDO_mapping, i-CO_RXCAN_RPDO);
            }
         #else
            length = 9; //size of RPDO is not important
         #endif
         if((ODE_RPDO_Parameter[i-CO_RXCAN_RPDO].COB_ID & 0x80000000L) == 0 && length){//is RPDO used
            CO_RXCAN[i].Ident.WORD[0] = CO_IDENT_WRITE((unsigned int)ODE_RPDO_Parameter[i-CO_RXCAN_RPDO].COB_ID, 0);
            CO_RXCAN[i].NoOfBytes = length;
            switch(i-CO_RXCAN_RPDO){
               case 0:  //First RPDO: is used standard predefined COB-ID?
                  #if ODD_RPDO_PAR_COB_ID_0 == 0
                     CO_RXCAN[i].Ident.WORD[0] += CO_IDENT_WRITE((unsigned int)CO_NodeID, 0);
                  #endif
                  break;
               case 1:  //Second RPDO: is used standard predefined COB-ID?
                  #if ODD_RPDO_PAR_COB_ID_1 == 0
                     CO_RXCAN[i].Ident.WORD[0] += CO_IDENT_WRITE((unsigned int)CO_NodeID, 0);
                  #endif
                  break;
               case 2:  //Third RPDO: is used standard predefined COB-ID?
                  #if ODD_RPDO_PAR_COB_ID_2 == 0
                     CO_RXCAN[i].Ident.WORD[0] += CO_IDENT_WRITE((unsigned int)CO_NodeID, 0);
                  #endif
                  break;
               case 3:  //Fourth RPDO: is used standard predefined COB-ID?
                  #if ODD_RPDO_PAR_COB_ID_3 == 0
                     CO_RXCAN[i].Ident.WORD[0] += CO_IDENT_WRITE((unsigned int)CO_NodeID, 0);
                  #endif
                  break;
            }
         }
      }
   #endif
   #if CO_NO_SDO_SERVER > 0
      for(i=CO_RXCAN_SDO_SRV; i<CO_RXCAN_SDO_SRV+CO_NO_SDO_SERVER; i++){
         if((ODE_Server_SDO_Parameter[i-CO_RXCAN_SDO_SRV].COB_ID_Client_to_Server & 0x80000000L) == 0 &&
            (ODE_Server_SDO_Parameter[i-CO_RXCAN_SDO_SRV].COB_ID_Server_to_Client & 0x80000000L) == 0){//is SDO used
            CO_RXCAN[i].Ident.WORD[0] = CO_IDENT_WRITE((unsigned int)ODE_Server_SDO_Parameter[i-CO_RXCAN_SDO_SRV].COB_ID_Client_to_Server, 0);
            CO_RXCAN[i].NoOfBytes = 8;
            CO_RXCAN[i].Inhibit = 1;
            if((i-CO_RXCAN_SDO_SRV) == 0) //first SDO? -> standard predefined COB-ID is used.
               CO_RXCAN[i].Ident.WORD[0] += CO_IDENT_WRITE((unsigned int)CO_NodeID, 0);
         }
      }
   #endif
   #if CO_NO_SDO_CLIENT > 0
      for(i=CO_RXCAN_SDO_CLI; i<CO_RXCAN_SDO_CLI+CO_NO_SDO_CLIENT; i++){
         if((ODE_Client_SDO_Parameter[i-CO_RXCAN_SDO_CLI].COB_ID_Client_to_Server & 0x80000000L) == 0 &&
            (ODE_Client_SDO_Parameter[i-CO_RXCAN_SDO_CLI].COB_ID_Server_to_Client & 0x80000000L) == 0){//is SDO used
            CO_RXCAN[i].Ident.WORD[0] = CO_IDENT_WRITE((unsigned int)ODE_Client_SDO_Parameter[i-CO_RXCAN_SDO_CLI].COB_ID_Server_to_Client, 0);
            CO_RXCAN[i].NoOfBytes = 8;
            CO_RXCAN[i].Inhibit = 1;
         }
      }
   #endif
   #if CO_NO_CONS_HEARTBEAT > 0
      for(i=CO_RXCAN_CONS_HB; i<CO_RXCAN_CONS_HB+CO_NO_CONS_HEARTBEAT; i++){
         if(ODE_Consumer_Heartbeat_Time[i-CO_RXCAN_CONS_HB] & 0x00FF0000L)
            CO_RXCAN[i].Ident.WORD[0] = CO_IDENT_WRITE((unsigned int)(((ODE_Consumer_Heartbeat_Time[i-CO_RXCAN_CONS_HB]>>16)&0xFF) | CAN_ID_HEARTBEAT), 0);
         else CO_RXCAN[i].Ident.WORD[0] = CO_IDENT_WRITE(0, 0);
         CO_RXCAN[i].NoOfBytes = 1;
         CO_RXCAN[i].Inhibit = 1;
      }
   #endif

/* Setup CO_TXCAN array */
   #if CO_NO_SYNC > 0
      CO_TXCAN[CO_TXCAN_SYNC].Ident.WORD[0] = CO_IDENT_WRITE((unsigned int)ODE_SYNC_COB_ID, 0);
      CO_TXCAN[CO_TXCAN_SYNC].NoOfBytes = 0;
   #endif
   #if CO_NO_EMERGENCY > 0
      CO_TXCAN[CO_TXCAN_EMERG].Ident.WORD[0] = CO_IDENT_WRITE((unsigned int)ODE_Emergency_COB_ID+CO_NodeID, 0);
      CO_TXCAN[CO_TXCAN_EMERG].NoOfBytes = 8;
   #endif
   #if CO_NO_TPDO > 0
      for(i=CO_TXCAN_TPDO; i<CO_TXCAN_TPDO+CO_NO_TPDO; i++){
         unsigned char length;
         #ifdef CO_PDO_MAPPING_IN_OD
            unsigned char j;
            //calculate length from mapping
            length = 7;  //round up to use whole byte
            for(j=0; j<8; j++)
               length += (unsigned char)ODE_TPDO_Mapping[i-CO_TXCAN_TPDO][j];
            length >>= 3;
            if(length > 8){
               length = 8;
               ErrorReport(ERROR_wrong_PDO_mapping, i-CO_TXCAN_TPDO);
            }
         #else
            length = 8;    //8 bytes long TPDO will be transmitted
         #endif
         if((ODE_TPDO_Parameter[i-CO_TXCAN_TPDO].COB_ID & 0x80000000L) == 0 && length){//is TPDO used
            CO_TXCAN[i].Ident.WORD[0] = CO_IDENT_WRITE((unsigned int)ODE_TPDO_Parameter[i-CO_TXCAN_TPDO].COB_ID, 0);
            CO_TXCAN[i].NoOfBytes = length;
            #if CO_NO_SYNC > 0
               if((ODE_TPDO_Parameter[i-CO_TXCAN_TPDO].Transmission_type-1) <= 239)
                  CO_TXCAN[i].Inhibit = 1; //mark Synchronous TPDOs
            #endif
            switch(i-CO_TXCAN_TPDO){
               case 0:  //First TPDO: is used standard predefined COB-ID?
                  #if ODD_TPDO_PAR_COB_ID_0 == 0
                     CO_TXCAN[i].Ident.WORD[0] += CO_IDENT_WRITE((unsigned int)CO_NodeID, 0);
                  #endif
                  break;
               case 1:  //Second TPDO: is used standard predefined COB-ID?
                  #if ODD_TPDO_PAR_COB_ID_1 == 0
                     CO_TXCAN[i].Ident.WORD[0] += CO_IDENT_WRITE((unsigned int)CO_NodeID, 0);
                  #endif
                  break;
               case 2:  //Third TPDO: is used standard predefined COB-ID?
                  #if ODD_TPDO_PAR_COB_ID_2 == 0
                     CO_TXCAN[i].Ident.WORD[0] += CO_IDENT_WRITE((unsigned int)CO_NodeID, 0);
                  #endif
                  break;
               case 3:  //Fourth TPDO: is used standard predefined COB-ID?
                  #if ODD_TPDO_PAR_COB_ID_3 == 0
                     CO_TXCAN[i].Ident.WORD[0] += CO_IDENT_WRITE((unsigned int)CO_NodeID, 0);
                  #endif
                  break;
            }
         }
      }
   #endif
   #if CO_NO_SDO_SERVER > 0
      for(i=CO_TXCAN_SDO_SRV; i<CO_TXCAN_SDO_SRV+CO_NO_SDO_SERVER; i++){
         if((ODE_Server_SDO_Parameter[i-CO_TXCAN_SDO_SRV].COB_ID_Client_to_Server & 0x80000000L) == 0 &&
            (ODE_Server_SDO_Parameter[i-CO_TXCAN_SDO_SRV].COB_ID_Server_to_Client & 0x80000000L) == 0){//is SDO used
            CO_TXCAN[i].Ident.WORD[0] = CO_IDENT_WRITE((unsigned int)ODE_Server_SDO_Parameter[i-CO_TXCAN_SDO_SRV].COB_ID_Server_to_Client, 0);
            CO_TXCAN[i].NoOfBytes = 8;
            if((i-CO_TXCAN_SDO_SRV) == 0) //first SDO? -> standard predefined COB-ID is used.
               CO_TXCAN[i].Ident.WORD[0] += CO_IDENT_WRITE((unsigned int)CO_NodeID, 0);
         }
      }
   #endif
   #if CO_NO_SDO_CLIENT > 0
      for(i=CO_TXCAN_SDO_CLI; i<CO_TXCAN_SDO_CLI+CO_NO_SDO_CLIENT; i++){
         if((ODE_Client_SDO_Parameter[i-CO_TXCAN_SDO_CLI].COB_ID_Client_to_Server & 0x80000000L) == 0 &&
            (ODE_Client_SDO_Parameter[i-CO_TXCAN_SDO_CLI].COB_ID_Server_to_Client & 0x80000000L) == 0){//is SDO used
            CO_TXCAN[i].Ident.WORD[0] = CO_IDENT_WRITE((unsigned int)ODE_Client_SDO_Parameter[i-CO_TXCAN_SDO_CLI].COB_ID_Client_to_Server, 0);
            CO_TXCAN[i].NoOfBytes = 8;
         }
      }
   #endif
   CO_TXCAN[CO_TXCAN_HB].Ident.WORD[0] = CO_IDENT_WRITE(CAN_ID_HEARTBEAT + CO_NodeID, 0);
   CO_TXCAN[CO_TXCAN_HB].NoOfBytes = 1;

/* Setup CAN bus 
   CAN 总线初始化 */
   CO_SetupCAN();
}


/*******************************************************************************
   CO_TXCANsend - MARK CAN MESSAGE FOR SENDING
   Function can be called from mainline or timer function. When message is prepared,
   it marks it for sending and switch on CANTX interrupt, which will send the
   message when buffers are ready.

   函数可由主线程序或定时器调用。当消息准备好时，设置发送标记并转到 CANTX 中断，在缓冲准备好时
   中断将发送这些消息。

   PARAM Index:   index of CO_TXCAN array to be send
                  要发送的 CO_TXCAN 数组索引
   RETURN:        0 = success
                  1 = error, previous message was not sent, buffer full
                      错误，之前的消息未发送，缓冲已满
*******************************************************************************/
char CO_TXCANsend(unsigned int index){
   if(CO_Timer1msIsr_executing) CO_DISABLE_CANTX_TMR(); else CO_DISABLE_CANTX();
   if(CO_TXCAN[index].NewMsg){      // 若之前的消息正等待发送
      ErrorReport(ERROR_CAN_TX_OVERFLOW, index);      // 报告缓冲满错误
      if(CO_Timer1msIsr_executing) CO_ENABLE_CANTX_TMR(); else CO_ENABLE_CANTX();
      return 1;   // 返回错误 之前的消息未发送
   }
   // 如果缓冲器及队列空闲，则直接发送消息
   if(CO_CANTX_BUFFER_FREE() && CO_TXCANcount==0){   //if buffer is free, send message directly
      #if CO_NO_SYNC > 0
         //messages with Inhibit flag set (synchronous PDOs) must be transmited inside preset window
         if(CO_TXCAN[index].Inhibit && CO_SYNCwindow && CO_SYNCtime > CO_SYNCwindow)
            ErrorReport(ERROR_TPDO_OUTSIDE_WINDOW, index);
         else
      #endif
         CO_TXCAN_COPY_TO_BUFFERS(index); // 拷贝消息到缓冲器
   }
   // 否则由中断发送
   else{                         //interrupt will send it
      CO_TXCAN[index].NewMsg = 1;
      CO_TXCANcount++;
   }
   if(CO_Timer1msIsr_executing) CO_ENABLE_CANTX_TMR(); else CO_ENABLE_CANTX();
   return 0;
}


/*******************************************************************************
   CO_TPDOsend - MARK TPDO FOR SENDING
   Function can be called from mainline or timer function. it marks TPDO for
   sending. If TPDO has to be prepared, this can be done here.

   函数可又主线或定时器调用。它标记需要发送的 TPDO。若 TPDO 需要准备发送，可以在这里完成。

   PARAM Index:   index of PDO to be send (0 = first PDO)
                  要发送的 PDO 索引
   RETURN:        0 = success
                  1 = error, previous TPDO was not sent, buffer full
                  2 = error, TPDO was inhibited
*******************************************************************************/
char CO_TPDOsend(unsigned int index){

   #ifdef CO_TPDO_INH_EV_TIMER
      if(CO_TPDO_InhibitTimer[index]) return 2;
      CO_TPDO_InhibitTimer[index] = ODE_TPDO_Parameter[index].Inhibit_Time / 10;
   #endif

   return CO_TXCANsend(CO_TXCAN_TPDO+index);
}


/*******************************************************************************
   CO_Timer1msIsr - 1 ms TIMER FUNCTION
   Function is executed every 1 ms. It is deterministic and has priority over
   mainline functions.
*******************************************************************************/
void CO_MODIF_Timer1msIsr CO_Timer1msIsr(void){
   CO_DEFAULT_SPEC CO_DEFAULT_TYPE i;
   static unsigned char timer100ms = 0;
   //Blinking bits
   static unsigned char tLEDflicker = 0;
   static unsigned int tLEDblink = 0, tLEDflash = 0;

/* information for some functions (when variable = 1, timer is executing and mainline is waiting)*/
   // 通知其他函数，为 1 时说明是定时器执行 主线等待
   CO_Timer1msIsr_executing = 1;

/* Increment timer variables */
   CO_Timer16bit1ms++;
   if(++timer100ms == 100){
      CO_Timer8bit100ms++;
      timer100ms = 0;
   }

/* SYNC timer/counter variables, SYNC producer */
   #if CO_NO_SYNC > 0
      if(CO_SYNCperiod && (CO_NMToperatingState==NMT_PRE_OPERATIONAL || CO_NMToperatingState==NMT_OPERATIONAL)){
         //increment SYNC timer variable
         CO_SYNCtime++;
         //Verify if new SYNC message received
         if(CO_RXCAN[CO_RXCAN_SYNC].NewMsg){
            //Verify is SYNC is to early (two producers)
            if(CO_SYNCtime<CO_SYNCperiodEarly && CO_NMToperatingState == NMT_OPERATIONAL)
               ErrorReport(ERROR_SYNC_EARLY, CO_SYNCtime);
            CO_SYNCcounter++;
            CO_SYNCtime = 0;
            CO_RXCAN[CO_RXCAN_SYNC].NewMsg = 0;
         }

         //SYNC producer (On CAN bus must NOT exist two equal SYNC producers)
         if(CO_SYNCproducer){
            static unsigned int tSYNCperiod = 0;
            if(++tSYNCperiod >= CO_SYNCperiod){
               CO_TXCANsend(CO_TXCAN_SYNC);
               CO_SYNCcounter++;
               CO_SYNCtime = 0;
               tSYNCperiod = 0;
            }
         }//end of SYNC producer
      }
   #endif

/* Erase RPDOs if not operational 
   非 Operational 状态时，擦除 RPDO */
   #if CO_NO_RPDO > 0
      if(CO_NMToperatingState != NMT_OPERATIONAL){
         for(i=0; i<CO_NO_RPDO; i++)
            CO_RPDO_New(i) = 0;
      }
   #endif

/* User timer procedure */
   User_Process1msIsr();

/* Send Synchronous PDOs and verify timeout of SYNC */
   #if CO_NO_SYNC > 0
      if(CO_SYNCperiod && CO_NMToperatingState == NMT_OPERATIONAL){
         #if CO_NO_TPDO > 0
            if(CO_SYNCtime == 0){
               for(i=0; i<CO_NO_TPDO; i++){
                  //is TPDO Synchronous (Inhibit flag) and is time to be send
                  if(CO_TXCAN[CO_TXCAN_TPDO+i].Inhibit && --CO_TPDO_SyncTimer[i] == 0){
                     CO_TPDO_SyncTimer[i] = ODE_TPDO_Parameter[i].Transmission_type;
                     CO_TPDOsend(i);
                  }
               }
            }
         #endif

         //Verify timeout of SYNC
         if(CO_SYNCtime>CO_SYNCperiodTimeout)
            ErrorReport(ERROR_SYNC_TIME_OUT, CO_SYNCtime);
      }
   #endif

/* PDO transmission - Inhibit and Event timer */
   #ifdef CO_TPDO_INH_EV_TIMER
      for(i=0; i<CO_NO_TPDO; i++){
         if(CO_TPDO_InhibitTimer[i]) CO_TPDO_InhibitTimer[i]--;
         if(ODE_TPDO_Parameter[i].Event_Timer && --CO_TPDO_EventTimer[i] == 0){
            CO_TPDO_EventTimer[i] = ODE_TPDO_Parameter[i].Event_Timer;
            if(CO_NMToperatingState == NMT_OPERATIONAL)
               CO_TPDOsend(i);
         }
      }
   #endif

/* Blinking bits */
   //10Hz
   if   (++tLEDflicker == 50)    CO_StatusLED.Flickering = 1;
   else if(tLEDflicker == 100){  CO_StatusLED.Flickering = 0; tLEDflicker = 0;}

   //2.5Hz
   if   (++tLEDblink == 200)     CO_StatusLED.Blinking = 1;
   else if(tLEDblink == 400){    CO_StatusLED.Blinking = 0; tLEDblink = 0;}

   //flashes
   switch(++tLEDflash){
      case 200:  CO_StatusLED.TripleFlash = CO_StatusLED.DoubleFlash = CO_StatusLED.SingleFlash = 1; break;
      case 400:  CO_StatusLED.TripleFlash = CO_StatusLED.DoubleFlash = CO_StatusLED.SingleFlash = 0; break;
      case 600:  CO_StatusLED.TripleFlash = CO_StatusLED.DoubleFlash = 1; break;
      case 800:  CO_StatusLED.TripleFlash = CO_StatusLED.DoubleFlash = 0; break;
      case 1000: CO_StatusLED.TripleFlash = 1; break;
      case 1200: CO_StatusLED.TripleFlash = 0; break;
      case 2000: tLEDflash = 0; break;
   }

/* information for some functions */
   CO_Timer1msIsr_executing = 0;

}


/*******************************************************************************
   ErrorReport - REPORT ERROR CONDITION
   Function is used to report any error occurred in program. It can be used from
   mainline or interrupt. It sets appropriate error bit and sends emergency if
   it is the first time for that bit. If critical bits are set, node will not be
   able to enter operational state. For detailed description see CO_errors.h

   函数用来报告程序中发生的任何错误。可由主线程序或中断使用。它置位适当的错误位，若该位为首次
   出现则发送紧急消息。如关键的位被置位，则节点将不能进入操作状态。细节描述见 CO_errors.h

   PARAM ErrorBit: specific error bit, use defined constants
                   特定的错误位，使用预定义
   PARAM Code: informative value, send as 4-th and 5-th byte in Emergency message
               错误代码值，作为紧急消息的第 4 和 5 字节发送
*******************************************************************************/
void ErrorReport(unsigned char ErrorBit, unsigned int Code){
   unsigned char index = ErrorBit >> 3;               // ErrorBit 的 bit5&4 是 ErrorStatusBits[] 的索引
   unsigned char bitmask = 1 << (ErrorBit & 0x7);     // ErrorBit 的 bit1-3 是 ErrorStatusBits[] 的内容

   //if ErrorBit value not supported, set ERROR_ErrorReport_ParametersNotSupp
   // 若 ErrorBit 值为未定义值，则设置 ERROR_ErrorReport_ParametersNotSupp (0x01, 按照上面两行 index = 0, bitmask = 2)
   if(index > (ERROR_NO_OF_BYTES-1)){
      index = 0; bitmask = 2;
   }
   if((CO_ErrorStatusBits[index]&bitmask) == 0){      // 若对应错误为首次设置
      if(ErrorBit) CO_ErrorStatusBits[index] |= bitmask; //set bit, if NO_ERROR just send emergency
      ErrorControl.CheckErrors = 1;
      if(!ErrorControl.EmergencyToSend){  //free
         ErrorControl.EmergencyToSend = 1;
         ErrorControl.EmergencyErrorCode = ErrorCodesTable[ErrorBit];
         ErrorControl.ErrorBit = ErrorBit;
         ErrorControl.CodeVal = Code;
      }
   }
}


/*******************************************************************************
   ErrorReset - RESET ERROR CONDITION
   Function is used to report any if error condition is no more present. It can
   be used from mainline or interrupt function. It resets appropriate error bit
   and sends emergency 'no error'.

   用来报告错误条件不再存在。可由主线和中断程序调用。复位相应错误位并发送『no error』

   For detailed description see CO_errors.h
   PARAM ErrorBit: specific error bit, use defined constants
                   特定错误位，使用预定义
   PARAM Code: informative value, send as 4-th and 5-th byte in Emergency message
               错误代码值，作为紧急消息的第 4 和 5 字节发送
*******************************************************************************/
void ErrorReset(unsigned char ErrorBit, unsigned int Code){
   unsigned char index = ErrorBit >> 3;
   unsigned char bitmask = 1 << (ErrorBit & 0x7);

   if(index > (ERROR_NO_OF_BYTES-1)){
      ErrorReport(ERROR_ErrorReport_ParametersNotSupp, ErrorBit);
      return;
   }
   if((CO_ErrorStatusBits[index]&bitmask) != 0){
      CO_ErrorStatusBits[index] &= ~bitmask; //erase bit
      ErrorControl.CheckErrors = 1;
      if(!ErrorControl.EmergencyToSend){  //free
         ErrorControl.EmergencyToSend = 1;
         ErrorControl.EmergencyErrorCode = 0x0000; //no error
         ErrorControl.ErrorBit = ErrorBit;
         ErrorControl.CodeVal = Code;
      }
   }
}


/*******************************************************************************
********************************************************************************

   MAINLINE FUNCTIONS

********************************************************************************
*******************************************************************************/

/*******************************************************************************
   CO_FindEntryInOD - SEARCH OBJECT DICTIONARY
   Function is used for searching object dictionary for entry with specified
   index and subindex. It searches OD from beginning to end and if matched, returns
   poiner to entry. It is usually called from SDO server.
   If Object Dictionary exist in multiple arrays, this function must search all.

   用以搜索对象字典特定索引和子索引的条目。从头到尾搜索 OD，若匹配则返回条目指针。常由 SDO server 调用。

   PARAM index, subindex: address of entry in object dictionary
                          对象字典条目地址
   RETURN: if found, pointer to entry, othervise 0
           若找到，条目指针，否则 0
*******************************************************************************/
ROM CO_objectDictionaryEntry* CO_FindEntryInOD(unsigned int index, unsigned char subindex){
   CO_DEFAULT_SPEC  unsigned int Index;
   CO_DEFAULT_SPEC unsigned char SubIndex;
   #ifdef CO_OD_IS_ORDERED
      //Fast search in ordered Object Dictionary. If indexes or subindexes are mixed, this won't work.
      //If CO_OD has up to 2^N entries, then N is max number of loop passes.
      CO_DEFAULT_SPEC unsigned int cur, min, max, CurIndex;
      CO_DEFAULT_SPEC unsigned char CurSubIndex;
      Index = index;
      SubIndex = subindex;
      min = 0;
      max = CO_OD_NoOfElements - 1;
      while(min < max){
         cur = (min + max) / 2;
         CurIndex = CO_OD[cur].index;
         CurSubIndex = CO_OD[cur].subindex;
         if(Index == CurIndex && SubIndex == CurSubIndex)   // 若匹配 返回条目指针
            return &CO_OD[cur];
         else if(Index < CurIndex || (Index == CurIndex && SubIndex < CurSubIndex)){
            max = cur;
            if(max) max--;
         }
         else
            min = cur + 1;
      }
      if(min == max){
         cur = min;
         CurIndex = CO_OD[cur].index;
         CurSubIndex = CO_OD[cur].subindex;
         if(Index == CurIndex && SubIndex == CurSubIndex)
            return &CO_OD[cur];
      }
   #else
      //search OD from first to last entry
      CO_DEFAULT_SPEC unsigned int i;
      ROM CO_objectDictionaryEntry* pODE;
      Index = index;
      SubIndex = subindex;
      pODE = &CO_OD[0];
      for(i = CO_OD_NoOfElements; i>0; i--){
         if(Index == pODE->index && SubIndex == pODE->subindex) return pODE;
         pODE++;
      }
   #endif
   return 0;  //object does not exist in OD
}


#if CO_NO_SDO_CLIENT > 0
/*******************************************************************************
   SDO CLIENT FUNCTIONS:
   CO_SDOclient_setup         - setup SDO Client channel (non-blocking)
   CO_SDOclientDownload_init  - initiate SDO download communication (non-blocking)
   CO_SDOclientDownload       - Proceed SDO download communication (non-blocking)
   CO_SDOclientUpload_init    - initiate SDO upload communication (non-blocking)
   CO_SDOclientUpload         - Proceed SDO upload communication (non-blocking)
   CO_SDOclientDownload_wait  - Complete SDO Client Download function (blocking)
   CO_SDOclientUpload_wait    - Complete SDO Client Upload function (blocking)

   SDO download means, that SDO client wants to WRITE to Object Dictionary in remote node
   SDO upload means, that SDO client wants to READ from Object Dictionary in remote node

   PARAM channel (in all SDO Client functions): Usually is used one channel (0). In this case
   only one SDO client is allowed in one network in same time. Anyway, CANopen
   allowes more channels to be used. In this case, COB-IDs must be set manually on
   both, client and server. All channels on one node can also 'work' simultaneously.

   RETURN (in all SDO Client functions)
      >0 - communication in progress
              1  waiting for RXCAN response
              2  responsed by server, new message sent
       0 - success, (end of communication for CO_SDOclientDownload() and CO_SDOclientUpload())
      <0 - Error:
             -1  wrong argument 'channel'
             -2  wrong other arguments
             -3  communication was not properly initiaded
             -10 error in SDO communication, SDO abort code is in value pointed by pSDOabortCode
             -11 timeout in SDO communication, SDO abort code is in value pointed by pSDOabortCode
*******************************************************************************/

   #if CO_NO_SDO_CLIENT == 1
      #define STATE     CO_SDOclientVar[0].State
      #define pBUFF     CO_SDOclientVar[0].pBuff
      #define BUFFMAX   CO_SDOclientVar[0].BuffMaxSize
      #define BUFFSIZE  CO_SDOclientVar[0].BuffSize
      #define tTIMEOUT  CO_SDOclientVar[0].tTimeout
      #define CLI_PARAM ODE_Client_SDO_Parameter[0]
      #define RXC_INDEX CO_RXCAN_SDO_CLI
      #define TXC_INDEX CO_TXCAN_SDO_CLI
   #else
      #define STATE     CO_SDOclientVar[channel].State
      #define pBUFF     CO_SDOclientVar[channel].pBuff
      #define BUFFMAX   CO_SDOclientVar[channel].BuffMaxSize
      #define BUFFSIZE  CO_SDOclientVar[channel].BuffSize
      #define tTIMEOUT  CO_SDOclientVar[channel].tTimeout
      #define CLI_PARAM ODE_Client_SDO_Parameter[channel]
      #define RXC_INDEX (CO_RXCAN_SDO_CLI + channel)
      #define TXC_INDEX (CO_TXCAN_SDO_CLI + channel)
   #endif

   //return macro
   #define RETURN(code)                \
   {  STATE = 0;                       \
      CO_RXCAN[RXC_INDEX].NewMsg = 0;  \
      return code;                     \
   }


/*******************************************************************************
   CO_SDOclient_setup - SETUP SDO CLIENT CHANNEL
   Function specifies SDO communication parameters for specified channel. It
   writes to  object dictionary CLI_PARAM and RXCAN and TXCAN variables.

   PARAM channel: see above.
   PARAM COB_ID_Client_to_Server, COB_ID_Server_to_Client: COB-IDs for communication,
         if zero, use default CANopen COB-IDs.
   PARAM NODE_ID_of_SDO_Server: NODE-ID of SDO server, if this node, then exchange
         data with this node
   RETURN: see above.
*******************************************************************************/
char CO_SDOclient_setup(unsigned char channel, unsigned long COB_ID_Client_to_Server,
      unsigned long COB_ID_Server_to_Client, unsigned char NODE_ID_of_SDO_Server){
   //verify parameters
   if(channel > (CO_NO_SDO_CLIENT-1)) return -1;
   if((COB_ID_Client_to_Server&0x7FFFF800L) || (COB_ID_Server_to_Client&0x7FFFF800L) ||
      NODE_ID_of_SDO_Server < 1 || NODE_ID_of_SDO_Server > 127) return -2;
   //Clear State
   STATE = 0;
   //setup Object Dictionary
   if(COB_ID_Client_to_Server == 0 || COB_ID_Server_to_Client == 0){
      CLI_PARAM.COB_ID_Client_to_Server = CAN_ID_RSDO + NODE_ID_of_SDO_Server;
      CLI_PARAM.COB_ID_Server_to_Client = CAN_ID_TSDO + NODE_ID_of_SDO_Server;
   }
   else{
      CLI_PARAM.COB_ID_Client_to_Server = COB_ID_Client_to_Server;
      CLI_PARAM.COB_ID_Server_to_Client = COB_ID_Server_to_Client;
   }
   CLI_PARAM.NODE_ID_of_SDO_Client_or_Server = NODE_ID_of_SDO_Server;
   //setup RXCAN and TXCAN variables
   CO_RXCAN[RXC_INDEX].NoOfBytes = 8;
   CO_RXCAN[RXC_INDEX].NewMsg = 0;
   CO_RXCAN[RXC_INDEX].Inhibit = 1;
   CO_TXCAN[TXC_INDEX].NoOfBytes = 8;
   CO_TXCAN[TXC_INDEX].NewMsg = 0;
   if((CLI_PARAM.COB_ID_Client_to_Server & 0x80000000L) == 0 &&
      (CLI_PARAM.COB_ID_Server_to_Client & 0x80000000L) == 0){//is SDO used
      CO_RXCAN[RXC_INDEX].Ident.WORD[0] = CO_IDENT_WRITE((unsigned int)CLI_PARAM.COB_ID_Server_to_Client, 0);
      CO_TXCAN[TXC_INDEX].Ident.WORD[0] = CO_IDENT_WRITE((unsigned int)CLI_PARAM.COB_ID_Client_to_Server, 0);
   }
   else{
      CO_RXCAN[RXC_INDEX].Ident.WORD[0] = CO_IDENT_WRITE(0, 0);
      CO_TXCAN[TXC_INDEX].Ident.WORD[0] = CO_IDENT_WRITE(0, 0);
   }
   return 0;
}


/*******************************************************************************
   CO_SDOclientDownload_init - INITIATE SDO CLIENT DOWNLOAD
   Function initiates SDO download communication with server specified in
   CO_SDOclient_setup() function. Data will be written to remote node.

   PARAM channel: see above.
   PARAM index, subindex: address of entry in object dictionary in remote node
   PARAM pBuff: pointer to buffer which contains data to be written. Buffer must
         be valid untill end of communication.
   PARAM dataSize: Size of data in buffer.
   RETURN: see above.
*******************************************************************************/
char CO_SDOclientDownload_init(unsigned char channel, unsigned int index, unsigned char subindex,
                               unsigned char* pBuff, unsigned char dataSize){
   //verify parameters
   if(channel > (CO_NO_SDO_CLIENT-1)) return -1;
   if(pBuff == 0 || dataSize == 0) return -2;
   //reset timeout timer
   TMR8Z(tTIMEOUT);
   //save parameters
   pBUFF = pBuff;
   BUFFMAX = dataSize;
   STATE = 0x20;
   //prepare dataBuff for CAN message
   CO_TXCAN[TXC_INDEX].Data.BYTE[1] = index & 0xFF;
   CO_TXCAN[TXC_INDEX].Data.BYTE[2] = index >> 8;
   CO_TXCAN[TXC_INDEX].Data.BYTE[3] = subindex;
   //if NODE_ID_of_SDO_Server == node-ID of this node, then exchange data with this node
   if(CLI_PARAM.NODE_ID_of_SDO_Client_or_Server == CO_NodeID){
      return 0;
   }
   //continue: prepare dataBuff for CAN message
   if(BUFFMAX <= 4){
      unsigned char i;
      //expedited transfer
      CO_TXCAN[TXC_INDEX].Data.BYTE[0] = 0x23 | ((4-BUFFMAX) << 2);
      CO_TXCAN[TXC_INDEX].Data.DWORD[1] = *((unsigned long*)pBUFF);
      for(i=BUFFMAX; i<4; i++) CO_TXCAN[TXC_INDEX].Data.BYTE[i+4] = 0;
   }
   else{
      //segmented transfer
      CO_TXCAN[TXC_INDEX].Data.BYTE[0] = 0x21;
      CO_TXCAN[TXC_INDEX].Data.DWORD[1] = BUFFMAX;
   }
   //empty receive buffer and send message
   CO_RXCAN[RXC_INDEX].NewMsg = 0;
   CO_TXCANsend(TXC_INDEX);
   return 0;
}


/*******************************************************************************
   CO_SDOclientDownload - PROCEED SDO CLIENT DOWNLOAD
   Function must be called cyclically untill it returns <=0. It Proceeds SDO
   download communication initiated with CO_SDOclientDownload_init() function.

   PARAM channel: see above.
   PARAM pSDOabortCode: pointer to user defined variable written by function
         in case of error in communication.
   RETURN: see above.
*******************************************************************************/
char CO_SDOclientDownload(unsigned char channel, unsigned long* pSDOabortCode){
   unsigned char i,j;

   //clear abort code
   *pSDOabortCode = 0;
   //verify parameters
   if(channel > (CO_NO_SDO_CLIENT-1)) RETURN(-1)
   if(!(STATE&0x22)) RETURN(-3) //communication was not properly initiaded
   //if NODE_ID_of_SDO_Server == node-ID of this node, then exchange data with this node
   if(CLI_PARAM.NODE_ID_of_SDO_Client_or_Server == CO_NodeID){
      ROM CO_objectDictionaryEntry* pODE;
      pODE = CO_FindEntryInOD(((unsigned int)CO_TXCAN[TXC_INDEX].Data.BYTE[2]<<8) | CO_TXCAN[TXC_INDEX].Data.BYTE[1],
                              CO_TXCAN[TXC_INDEX].Data.BYTE[3]);
      if(pODE == 0){
         *pSDOabortCode = 0x06020000L;  //object does not exist in OD
         RETURN(-10)
      }
      *pSDOabortCode = CO_OD_Write(pODE, (void*)pBUFF, BUFFMAX);
      if(*pSDOabortCode) RETURN(-10)
      RETURN(0)
   }
   //check if new SDO object received
   if(CO_RXCAN[RXC_INDEX].NewMsg){
      TMR8Z(tTIMEOUT);
      if(STATE & 0x20){
         //download initiated
         //verify response (byte0(bit4..0), index, subindex)
         //if((CO_RXCAN[RXC_INDEX].Data.BYTE[0]&0x1F) || CO_RXCAN[RXC_INDEX].Data.BYTE[1] != CO_TXCAN[TXC_INDEX].Data.BYTE[1] ||
         //    CO_RXCAN[RXC_INDEX].Data.BYTE[2] != CO_TXCAN[TXC_INDEX].Data.BYTE[2] || CO_RXCAN[RXC_INDEX].Data.BYTE[3] != CO_TXCAN[TXC_INDEX].Data.BYTE[3]){
         //   *pSDOabortCode = 0x06040043L; //General parameter incompatibility reason
         //   RETURN(-10)
         //}
         switch(CO_RXCAN[RXC_INDEX].Data.BYTE[0]>>5){  //Switch Server Command Specifier
            case 3:  //response OK
               if(BUFFMAX <= 4){
                  //expedited transfer
                  RETURN(0)
               }
               else{
                  //segmented transfer - prepare first segment
                  BUFFSIZE = 0;
                  STATE = 0x22;
                  //continue with segmented download
               }
               break;
            case 4:  //abort by server
               *pSDOabortCode = CO_RXCAN[RXC_INDEX].Data.DWORD[1];
               RETURN(-10)
            default:
               *pSDOabortCode = 0x05040001L; //Client/server command specifier not valid or unknown
               RETURN(-10)
         }
      }//end of download initiated
      if(STATE & 0x02){
         //segmented download in progress
         if(STATE & 0x20){   //is the first segment?
            STATE = 0x02;
         }
         else{
            //verify response from previous segment sent
            switch(CO_RXCAN[RXC_INDEX].Data.BYTE[0]>>5){  //Switch Server Command Specifier
               case 1:  //response OK
                  //verify toggle bit
                  if((CO_RXCAN[RXC_INDEX].Data.BYTE[0]&0x10) != (STATE&0x10)){
                     *pSDOabortCode = 0x05030000L;  //toggle bit not alternated
                     RETURN(-10)
                  }
                  //is end of transfer?
                  if(BUFFSIZE==BUFFMAX){
                     RETURN(0);
                  }
                  //alternate toggle bit
                  if(STATE&0x10) STATE &= 0xEF;
                  else STATE |= 0x10;
                  break;
               case 4:  //abort by server
                  //verify byte0(bit4..0), index, subindex
                  //if((CO_RXCAN[RXC_INDEX].Data.BYTE[0]&0x1F) || CO_RXCAN[RXC_INDEX].Data.BYTE[1] != CO_TXCAN[TXC_INDEX].Data.BYTE[1] ||
                  //    CO_RXCAN[RXC_INDEX].Data.BYTE[2] != CO_TXCAN[TXC_INDEX].Data.BYTE[2] || CO_RXCAN[RXC_INDEX].Data.BYTE[3] != CO_TXCAN[TXC_INDEX].Data.BYTE[3]){
                  //   *pSDOabortCode = 0x06040043L; //General parameter incompatibility reason
                  //}
                  *pSDOabortCode = CO_RXCAN[RXC_INDEX].Data.DWORD[1];
                  RETURN(-10)
               default:
                  *pSDOabortCode = 0x05040001L; //Client/server command specifier not valid or unknown
                  RETURN(-10)
            }
         }
         //calculate length to be sent
         j = BUFFMAX - BUFFSIZE;
         if(j > 7) j = 7;
         //fill data bytes
         for(i=0; i<j; i++)
            CO_TXCAN[TXC_INDEX].Data.BYTE[i+1] = pBUFF[BUFFSIZE+i];
         for(; i<7; i++)
            CO_TXCAN[TXC_INDEX].Data.BYTE[i+1] = 0;
         BUFFSIZE += j;
         CO_TXCAN[TXC_INDEX].Data.BYTE[0] = 0x00 | (STATE&0x10) | ((7-j)<<1);
         //is end of transfer?
         if(BUFFSIZE==BUFFMAX){
            CO_TXCAN[TXC_INDEX].Data.BYTE[0] |= 1;
         }
         CO_TXCANsend(TXC_INDEX);
         CO_RXCAN[RXC_INDEX].NewMsg = 0;
         return 2;
      }
   }
   else{
      //verify timeout
      if(TMR8(tTIMEOUT) >= CO_SDO_TIMEOUT_TIME){
         *pSDOabortCode = 0x05040000L;  //SDO protocol timed out
         RETURN(-11)
      }
      return 1;
   }
   return 0;
}


/*******************************************************************************
   CO_SDOclientUpload_init - INITIATE SDO CLIENT UPLOAD
   Function initiates SDO upload communication with server specified in
   CO_SDOclient_setup() function. Data will be read from remote node.

   PARAM channel: see above.
   PARAM index, subindex: address of entry in object dictionary in remote node
   PARAM pBuff: pointer to buffer where data will be written. Buffer must
         be valid untill end of communication.
   PARAM BuffMaxSize: Size of buffer (to prevent overflow).
   RETURN: see above.
*******************************************************************************/
char CO_SDOclientUpload_init(unsigned char channel, unsigned int index, unsigned char subindex,
                             unsigned char* pBuff, unsigned char BuffMaxSize){
   //verify parameters
   if(channel > (CO_NO_SDO_CLIENT-1)) return -1;
   if(pBuff == 0 || BuffMaxSize < 4) return -2;
   //reset timeout timer
   TMR8Z(tTIMEOUT);
   //save parameters
   pBUFF = pBuff;
   BUFFMAX = BuffMaxSize;
   STATE = 0x40;
   //prepare dataBuff for CAN message
   CO_TXCAN[TXC_INDEX].Data.BYTE[0] = 0x40;
   CO_TXCAN[TXC_INDEX].Data.BYTE[1] = index & 0xFF;
   CO_TXCAN[TXC_INDEX].Data.BYTE[2] = index >> 8;
   CO_TXCAN[TXC_INDEX].Data.BYTE[3] = subindex;
   CO_TXCAN[TXC_INDEX].Data.DWORD[1] = 0;
   //if NODE_ID_of_SDO_Server == node-ID of this node, then exchange data with this node
   if(CLI_PARAM.NODE_ID_of_SDO_Client_or_Server == CO_NodeID){
      return 0;
   }
   //empty receive buffer and send message
   CO_RXCAN[RXC_INDEX].NewMsg = 0;
   CO_TXCANsend(TXC_INDEX);
   return 0;
}


/*******************************************************************************
   CO_SDOclientUpload - PROCEED SDO CLIENT UPLOAD
   Function must be called cyclically untill it returns <=0. It Proceeds SDO
   upload communication initiated with CO_SDOclientUpload_init() function.

   PARAM channel: see above.
   PARAM pSDOabortCode: pointer to user defined variable written by function
         in case of error in communication.
   PARAM pDataSize: pointer to variable, where size of data (in *pBuff) will be written.
   RETURN: see above.
*******************************************************************************/
char CO_SDOclientUpload(unsigned char channel, unsigned long* pSDOabortCode, unsigned char* pDataSize){
   unsigned char i;

   //clear abort code
   *pSDOabortCode = 0;
   //verify parameters
   if(channel > (CO_NO_SDO_CLIENT-1)) RETURN(-1)
   if(!(STATE&0x44)) RETURN(-3) //communication was not properly initiaded
   //if NODE_ID_of_SDO_Server == node-ID of this node, then exchange data with this node
   if(CLI_PARAM.NODE_ID_of_SDO_Client_or_Server == CO_NodeID){
      ROM CO_objectDictionaryEntry* pODE;
      pODE = CO_FindEntryInOD(((unsigned int)CO_TXCAN[TXC_INDEX].Data.BYTE[2]<<8) | CO_TXCAN[TXC_INDEX].Data.BYTE[1],
                              CO_TXCAN[TXC_INDEX].Data.BYTE[3]);
      if(pODE == 0){
         *pSDOabortCode = 0x06020000L;  //object does not exist in OD
         RETURN(-10)
      }
      //copy data
      *pSDOabortCode = CO_OD_Read(pODE, (void*)pBUFF, BUFFMAX);
      if(*pSDOabortCode){
         RETURN(-10)
      }
      if(pODE->length <= 4 && (pODE->attribute & ATTR_ADD_ID)){
         //add node ID
         *((unsigned long*)pBUFF) += CO_NodeID;
      }
      //set data size
      *pDataSize = pODE->length;
      RETURN(0)
   }
   //check if new SDO object received
   if(CO_RXCAN[RXC_INDEX].NewMsg){
      TMR8Z(tTIMEOUT);
      if(STATE & 0x40){
         //upload initiated
         //verify response (byte0(bit4), index, subindex)
         //if((CO_RXCAN[RXC_INDEX].Data.BYTE[0]&0x10) || CO_RXCAN[RXC_INDEX].Data.BYTE[1] != CO_TXCAN[TXC_INDEX].Data.BYTE[1] ||
         //    CO_RXCAN[RXC_INDEX].Data.BYTE[2] != CO_TXCAN[TXC_INDEX].Data.BYTE[2] || CO_RXCAN[RXC_INDEX].Data.BYTE[3] != CO_TXCAN[TXC_INDEX].Data.BYTE[3]){
         //   *pSDOabortCode = 0x06040043L; //General parameter incompatibility reason
         //   RETURN(-10)
         //}
         switch(CO_RXCAN[RXC_INDEX].Data.BYTE[0]>>5){  //Switch Server Command Specifier
            case 2:  //response OK
               if(CO_RXCAN[RXC_INDEX].Data.BYTE[0] & 0x02){
                  //Expedited transfer
                  if(CO_RXCAN[RXC_INDEX].Data.BYTE[0] & 0x01)//is size indicated
                     i = 4 - ((CO_RXCAN[RXC_INDEX].Data.BYTE[0]>>2)&0x03);   //size
                  else i = 4;
                  //copy data and set size
                  memcpy((void*)pBUFF, (void*)&CO_RXCAN[RXC_INDEX].Data.BYTE[4], i);
                  *pDataSize = i;
                  RETURN(0)
               }
               else{
                  //segmented transfer - prepare first segment
                  BUFFSIZE = 0;
                  STATE = 0x44;
                  //continue with segmented upload
               }
               break;
            case 4:  //abort by server
               *pSDOabortCode = CO_RXCAN[RXC_INDEX].Data.DWORD[1];
               RETURN(-10)
            default:
               *pSDOabortCode = 0x05040001L; //Client/server command specifier not valid or unknown
               RETURN(-10)
         }
      }//end of upload initiated
      if(STATE & 0x04){
         //segmented upload in progress
         if(STATE & 0x40){   //is the first segment?
            STATE = 0x04;
         }
         else{
            //verify response from previous segment sent
            switch(CO_RXCAN[RXC_INDEX].Data.BYTE[0]>>5){  //Switch Server Command Specifier
               case 0:  //response OK
                  //verify toggle bit
                  if((CO_RXCAN[RXC_INDEX].Data.BYTE[0]&0x10) != (STATE&0x10)){
                     *pSDOabortCode = 0x05030000L;  //toggle bit not alternated
                     RETURN(-10)
                  }
                  //get size
                  i = 7 - ((CO_RXCAN[RXC_INDEX].Data.BYTE[0]>>1)&0x07);   //size
                  //verify length
                  if((BUFFSIZE + i) > BUFFMAX){
                     *pSDOabortCode = 0x05040005L;  //Out of memory
                     RETURN(-10)
                  }
                  //copy data to buffer
                  memcpy((void*)(pBUFF+BUFFSIZE), (void*)&CO_RXCAN[RXC_INDEX].Data.BYTE[1], i);
                  BUFFSIZE += i;
                  //If no more segments to be uploaded, finish communication
                  if(CO_RXCAN[RXC_INDEX].Data.BYTE[0] & 0x01){
                     *pDataSize = BUFFSIZE;
                     RETURN(0)
                  }
                  //alternate toggle bit
                  if(STATE&0x10) STATE &= 0xEF;
                  else STATE |= 0x10;
                  break;
               case 4:  //abort by server
                  //verify byte0(bit4..0), index, subindex
                  //if((CO_RXCAN[RXC_INDEX].Data.BYTE[0]&0x1F) || CO_RXCAN[RXC_INDEX].Data.BYTE[1] != CO_TXCAN[TXC_INDEX].Data.BYTE[1] ||
                  //    CO_RXCAN[RXC_INDEX].Data.BYTE[2] != CO_TXCAN[TXC_INDEX].Data.BYTE[2] || CO_RXCAN[RXC_INDEX].Data.BYTE[3] != CO_TXCAN[TXC_INDEX].Data.BYTE[3]){
                  //   *pSDOabortCode = 0x06040043L; //General parameter incompatibility reason
                  //}
                  *pSDOabortCode = CO_RXCAN[RXC_INDEX].Data.DWORD[1];
                  RETURN(-10)
               default:
                  *pSDOabortCode = 0x05040001L; //Client/server command specifier not valid or unknown
                  RETURN(-10)
            }
         }
         //prepare next segment
         CO_TXCAN[TXC_INDEX].Data.BYTE[0] = 0x60 | (STATE&0x10);
         CO_TXCAN[TXC_INDEX].Data.BYTE[1] = 0;
         CO_TXCAN[TXC_INDEX].Data.WORD[1] = 0;
         CO_TXCAN[TXC_INDEX].Data.DWORD[1] = 0;
         CO_TXCANsend(TXC_INDEX);
         CO_RXCAN[RXC_INDEX].NewMsg = 0;
         return 2;
      }
   }
   else{
      //verify timeout
      if(TMR8(tTIMEOUT) >= CO_SDO_TIMEOUT_TIME){
         *pSDOabortCode = 0x05040000L;  //SDO protocol timed out
         RETURN(-11)
      }
      return 1;
   }
   return 0;
}


/*******************************************************************************
   CO_SDOclientDownload_wait - SDO CLIENT DOWNLOAD
   This is blocking function and waits untill end of communication or timeout (500 ms).
   It writes data to object dictionary in the remote node. It uses default COB-IDs.
   It can be used in multitasking OS. For nonblocking implementation, this function
   can be an example.

   PARAM NODE_ID_of_SDO_Server: NODE-ID of SDO server, if this node, then exchange
         data with this node
   PARAM index, subindex: address of entry in object dictionary in the remote node
   PARAM pBuff: pointer to buffer which contains data to be written.
   PARAM dataSize: Size of data in buffer.
   PARAM pSDOabortCode: pointer to user defined variable written by function
         in case of error in communication.
   RETURN: see above.
*******************************************************************************/
char CO_SDOclientDownload_wait(unsigned char NODE_ID_of_SDO_Server, unsigned int index, unsigned char subindex,
                               unsigned char* pBuff, unsigned char dataSize, unsigned long* pSDOabortCode){
   char ret;
   ret = CO_SDOclient_setup(0, 0, 0, NODE_ID_of_SDO_Server);
   if(ret) return ret;
   ret = CO_SDOclientDownload_init(0, index, subindex, pBuff, dataSize);
   if(ret) return ret;
   do{
      ret = CO_SDOclientDownload(0, pSDOabortCode);
   }while(ret > 0);
   return ret;
}


/*******************************************************************************
   CO_SDOclientUpload_wait - SDO CLIENT UPLOAD
   This is blocking function and waits untill end of communication or timeout (500 ms).
   It reads data from object dictionary in the remote node. It uses default COB-IDs.
   It can be used in multitasking OS. For nonblocking implementation, this function
   can be an example.

   PARAM NODE_ID_of_SDO_Server: NODE-ID of SDO server, if this node, then exchange
         data with this node
   PARAM index, subindex: address of entry in object dictionary in the remote node
   PARAM pBuff: pointer to buffer where data will be written.
   PARAM BuffMaxSize: Size of buffer (to prevent overflow).
   PARAM pDataSize: pointer to variable, where size of data (in *pBuff) will be written.
   PARAM pSDOabortCode: pointer to user defined variable written by function
         in case of error in communication.
   RETURN: see above.
*******************************************************************************/
char CO_SDOclientUpload_wait(unsigned char NODE_ID_of_SDO_Server, unsigned int index, unsigned char subindex,
                             unsigned char* pBuff, unsigned char BuffMaxSize, unsigned char* pDataSize, unsigned long* pSDOabortCode){
   char ret;
   ret = CO_SDOclient_setup(0, 0, 0, NODE_ID_of_SDO_Server);
   if(ret) return ret;
   ret = CO_SDOclientUpload_init(0, index, subindex, pBuff, BuffMaxSize);
   if(ret) return ret;
   do{
      ret = CO_SDOclientUpload(0, pSDOabortCode, pDataSize);
   }while(ret > 0);
   return ret;
}

   #undef STATE
   #undef pBUFF
   #undef BUFFMAX
   #undef BUFFSIZE
   #undef tTIMEOUT
   #undef CLI_PARAM
   #undef RXC_INDEX
   #undef TXC_INDEX
   #undef RETURN

#endif   //end of SDO Client


/*******************************************************************************
   CO_ProcessMain - PROCESS CANOPEN MAINLINE
   This function is cyclycally called from main(). It is non blocking function.
   It is asynchronous. Here is longer and time consuming code.

   由 main() 函数循环调用。非阻塞函数。异步的。
*******************************************************************************/
void CO_ProcessMain(void){

/* variables */
   //multipurpose usage
   // 复用变量
   CO_DEFAULT_SPEC CO_DEFAULT_TYPE i, j;
   //SDO server
   #if CO_NO_SDO_SERVER > 1
      static unsigned char SDOserverChannel = 0;
   #endif
   //Heartbeat producer timer variable
   // 心跳生产者定时器变量
   static unsigned int tProducerHeartbeatTime = 0;

/* 16 bit mainline timer variable 
   16 位主线定时器变量 */
   unsigned int CO_Timer16bit1msCopy;
   CO_DISABLE_TMR();
   CO_Timer16bit1msCopy = CO_Timer16bit1ms;
   CO_ENABLE_TMR();
   #define TMR16Z(ttimerVariable)  (unsigned int)(ttimerVariable = CO_Timer16bit1msCopy)
   #define TMR16(ttimerVariable)   (unsigned int)(CO_Timer16bit1msCopy - ttimerVariable)

/* Process microcontroller specific code */
   CO_ProcessDriver();

/* Verify if new NMT message received */
   if(CO_RXCAN[CO_RXCAN_NMT].NewMsg){
      if(CO_RXCAN[CO_RXCAN_NMT].Data.BYTE[1] == 0 || CO_RXCAN[CO_RXCAN_NMT].Data.BYTE[1] == CO_NodeID){
         switch(CO_RXCAN[CO_RXCAN_NMT].Data.BYTE[0]){//switch NMT command
            case NMT_ENTER_OPERATIONAL:      CO_NMToperatingState = NMT_OPERATIONAL;      break;
            case NMT_ENTER_STOPPED:          CO_NMToperatingState = NMT_STOPPED;          break;
            case NMT_ENTER_PRE_OPERATIONAL:  CO_NMToperatingState = NMT_PRE_OPERATIONAL;  break;
            case NMT_RESET_NODE:             CO_Reset();                                  break;
            case NMT_RESET_COMMUNICATION:    CO_DISABLE_ALL();
                                             CO_ResetComm();
                                             CO_ENABLE_ALL();                             break;
            default: ErrorReport(ERROR_CO_RXMSG_NMTcmd, 0);      //Error in NMT Command
         }
      }
      CO_RXCAN[CO_RXCAN_NMT].NewMsg = 0;
   }

/* Error handling and Emergency message sending */
   //calculate Error register
   if(ErrorControl.CheckErrors){
      ErrorControl.CheckErrors = 0;
      //generic error
      if(ERROR_REGISTER_BIT0_CONDITION) ODE_Error_Register |= 0x01;
      else                              ODE_Error_Register &= 0xFE;
      //current
      if(ERROR_REGISTER_BIT1_CONDITION) ODE_Error_Register |= 0x02;
      else                              ODE_Error_Register &= 0xFD;
      //voltage
      if(ERROR_REGISTER_BIT2_CONDITION) ODE_Error_Register |= 0x04;
      else                              ODE_Error_Register &= 0xFB;
      //temperature
      if(ERROR_REGISTER_BIT3_CONDITION) ODE_Error_Register |= 0x08;
      else                              ODE_Error_Register &= 0xF7;
      //communication error (overrun, error state)
      if(ERROR_REGISTER_BIT4_CONDITION) ODE_Error_Register |= 0x10;
      else                              ODE_Error_Register &= 0xEF;
      //device profile specific error
      if(ERROR_REGISTER_BIT5_CONDITION) ODE_Error_Register |= 0x20;
      else                              ODE_Error_Register &= 0xDF;
      //manufacturer specific error
      if(ERROR_REGISTER_BIT7_CONDITION) ODE_Error_Register |= 0x80;
      else                              ODE_Error_Register &= 0x7F;

      //send emergency message
      if(CO_NMToperatingState==NMT_PRE_OPERATIONAL || CO_NMToperatingState==NMT_OPERATIONAL){
         //is new emergency, buffer free and no inhibit?
         if(ErrorControl.EmergencyToSend){
            #if CO_NO_EMERGENCY > 0
               static unsigned int tInhibitEmergency = 0;
               if(!CO_TXCAN[CO_TXCAN_EMERG].NewMsg && (TMR16(tInhibitEmergency) > (ODE_Inhibit_Time_Emergency/10))){
                  CO_TXCAN[CO_TXCAN_EMERG].Data.WORD[0] = ErrorControl.EmergencyErrorCode;
                  CO_TXCAN[CO_TXCAN_EMERG].Data.BYTE[2] = ODE_Error_Register;
                  CO_TXCAN[CO_TXCAN_EMERG].Data.BYTE[3] = ErrorControl.ErrorBit;
                  CO_TXCAN[CO_TXCAN_EMERG].Data.WORD[2] = ErrorControl.CodeVal;
                  CO_TXCAN[CO_TXCAN_EMERG].Data.BYTE[6] = ERROR_EMERGENCY_BYTE6;
                  CO_TXCAN[CO_TXCAN_EMERG].Data.BYTE[7] = ERROR_EMERGENCY_BYTE7;
                  CO_TXCANsend(CO_TXCAN_EMERG);
               }
               TMR16Z(tInhibitEmergency);
            #endif
            ErrorControl.EmergencyToSend = 0;
            //write to history
            #if CO_NO_ERROR_FIELD > 0
               if(ODE_Pre_Defined_Error_Field_NoOfErrors < CO_NO_ERROR_FIELD)
                  ODE_Pre_Defined_Error_Field_NoOfErrors++;
               for(i=ODE_Pre_Defined_Error_Field_NoOfErrors-1; i>0; i--)
                  ODE_Pre_Defined_Error_Field[i] = ODE_Pre_Defined_Error_Field[i-1];
               ODE_Pre_Defined_Error_Field[0] = ErrorControl.EmergencyErrorCode | (((unsigned long)ErrorControl.CodeVal)<<16);
            #endif
         }
      }
   }

   //in case of error enter pre-operational state
   if(ODE_Error_Register && (CO_NMToperatingState == NMT_OPERATIONAL)){
      if(ODE_Error_Register&0xEF){  //all, except communication error
         CO_NMToperatingState = NMT_PRE_OPERATIONAL;
      }
      if(ODE_Error_Register&0x10){  //communication error
         switch(ODE_Error_Behavior.Communication_Error){
            case 0x01:
               break;
            case 0x02:
               CO_NMToperatingState = NMT_STOPPED;
               break;
            default:
               CO_NMToperatingState = NMT_PRE_OPERATIONAL;
         }
      }
   }

#if CO_NO_SDO_SERVER > 0
/* SDO SERVER */
   //SDO server makes Object Dictionary of this node available to SDO client (Master on CAN bus)
   //SDO download means, that SDO client wants to WRITE to Object Dictionary of this node
   //SDO upload means, that SDO client wants to READ from Object Dictionary of this node

   if(CO_NMToperatingState==NMT_PRE_OPERATIONAL || CO_NMToperatingState==NMT_OPERATIONAL){
      #if CO_NO_SDO_SERVER == 1
         #define pODE      CO_SDOserverVar[0].pODE
         #define STATE     CO_SDOserverVar[0].State
         #define BUFF      CO_SDOserverVar[0].Buff
         #define BUFFSIZE  CO_SDOserverVar[0].BuffSize
         #define tTIMEOUT  CO_SDOserverVar[0].tTimeout
         #define RXC_INDEX CO_RXCAN_SDO_SRV
         #define TXC_INDEX CO_TXCAN_SDO_SRV
      #else
         #define pODE      CO_SDOserverVar[SDOserverChannel].pODE
         #define STATE     CO_SDOserverVar[SDOserverChannel].State
         #define BUFF      CO_SDOserverVar[SDOserverChannel].Buff
         #define BUFFSIZE  CO_SDOserverVar[SDOserverChannel].BuffSize
         #define tTIMEOUT  CO_SDOserverVar[SDOserverChannel].tTimeout
         #define RXC_INDEX (CO_RXCAN_SDO_SRV + SDOserverChannel)
         #define TXC_INDEX (CO_TXCAN_SDO_SRV + SDOserverChannel)
         i = SDOserverChannel;
         do{
            if(++SDOserverChannel == CO_NO_SDO_SERVER) SDOserverChannel = 0;
            if(CO_RXCAN[RXC_INDEX].NewMsg) break;
         }while(SDOserverChannel != i);
      #endif

      //Abort macro:
      #define SDO_ABORT(Code){                      \
         CO_TXCAN[TXC_INDEX].Data.BYTE[0] = 0x80;   \
         CO_TXCAN[TXC_INDEX].Data.DWORD[1] = Code;  \
         STATE = 0;                                 \
         CO_TXCANsend(TXC_INDEX);                   \
      }

      if(CO_RXCAN[RXC_INDEX].NewMsg && !CO_TXCAN[TXC_INDEX].NewMsg){  //New SDO object has to be processed and SDO CAN send buffer is free
         //setup variables for default response
         CO_TXCAN[TXC_INDEX].Data.BYTE[1] = CO_RXCAN[RXC_INDEX].Data.BYTE[1];
         CO_TXCAN[TXC_INDEX].Data.BYTE[2] = CO_RXCAN[RXC_INDEX].Data.BYTE[2];
         CO_TXCAN[TXC_INDEX].Data.BYTE[3] = CO_RXCAN[RXC_INDEX].Data.BYTE[3];
         CO_TXCAN[TXC_INDEX].Data.DWORD[1] = 0;

         switch(CO_RXCAN[RXC_INDEX].Data.BYTE[0]>>5){  //Switch Client Command Specifier
            case 1:   //Initiate SDO Download request
               //find pointer to object dictionary entry
               pODE = CO_FindEntryInOD(*((unsigned int*)&CO_RXCAN[RXC_INDEX].Data.BYTE[1]), CO_RXCAN[RXC_INDEX].Data.BYTE[3]);
               if(!pODE){
                  SDO_ABORT(0x06020000L)  //object does not exist in OD
                  break;
               }
               if(pODE->length > CO_MAX_OD_ENTRY_SIZE){  //length of ODE is not valid
                  SDO_ABORT(0x06040047L)   //general internal incompatibility in the device
                  break;
               }
               if((pODE->attribute&0x07) == ATTR_RO || (pODE->attribute&0x07) == ATTR_CO){
                  SDO_ABORT(0x06010002L)  //attempt to write a read-only object
                  break;
               }
               if(CO_RXCAN[RXC_INDEX].Data.BYTE[0] & 0x02){
                  //Expedited transfer
                  if(CO_RXCAN[RXC_INDEX].Data.BYTE[0] & 0x01)//is size indicated
                     i = 4 - ((CO_RXCAN[RXC_INDEX].Data.BYTE[0]>>2)&0x03);   //size
                  else i = 4;
                  //write to memory
                  CO_TXCAN[TXC_INDEX].Data.DWORD[1] = CO_OD_Write(pODE, (void*)&CO_RXCAN[RXC_INDEX].Data.BYTE[4], i);
                  if(CO_TXCAN[TXC_INDEX].Data.DWORD[1] != 0)//SDO_ABORT
                     CO_TXCAN[TXC_INDEX].Data.BYTE[0] = 0x80;
                  else
                     CO_TXCAN[TXC_INDEX].Data.BYTE[0] = 0x60;
                  STATE = 0;
                  CO_TXCANsend(TXC_INDEX);
               }
               else{
#if CO_MAX_OD_ENTRY_SIZE == 0x04
                  SDO_ABORT(0x06010000L)   //unsupported access to an object
               }
               break;
#else
                  //segmented transfer
                  if(CO_RXCAN[RXC_INDEX].Data.BYTE[0] & 0x01){//is size indicated
                     if(*((unsigned long*)&CO_RXCAN[RXC_INDEX].Data.BYTE[4]) != (unsigned long)pODE->length){
                        SDO_ABORT(0x06070010L)   //Length of service parameter does not match
                        break;
                     }
                  BUFFSIZE = 0;
                  TMR8Z(tTIMEOUT);
                  STATE = 0x02;
                  CO_TXCAN[TXC_INDEX].Data.BYTE[0] = 0x60;
                  CO_TXCANsend(TXC_INDEX);
                  }
               }
               break;

            case 0:   //Download SDO segment
               if(!(STATE&0x02)){//download SDO segment was not initiated
                  SDO_ABORT(0x05040001L) //command specifier not valid
                  break;
               }
               //verify toggle bit
               if((CO_RXCAN[RXC_INDEX].Data.BYTE[0]&0x10) != (STATE&0x10)){
                  SDO_ABORT(0x05030000L) //toggle bit not alternated
                  break;
               }
               //get size
               i = 7 - ((CO_RXCAN[RXC_INDEX].Data.BYTE[0]>>1)&0x07);   //size
               //verify length
               if((BUFFSIZE + i) > pODE->length){
                  SDO_ABORT(0x06070012L)   //Length of service parameter too high
                  break;
               }
               //copy data to buffer
               memcpy((void*)&BUFF[BUFFSIZE], (void*)&CO_RXCAN[RXC_INDEX].Data.BYTE[1], i);
               BUFFSIZE += i;
               //write response data (partial)
               CO_TXCAN[TXC_INDEX].Data.BYTE[0] = 0x20 | (STATE&0x10);
               //If no more segments to be downloaded, copy data to variable
               if(CO_RXCAN[RXC_INDEX].Data.BYTE[0] & 0x01){
                  CO_TXCAN[TXC_INDEX].Data.DWORD[1] = CO_OD_Write(pODE, (void*)&BUFF[0], BUFFSIZE);
                  if(CO_TXCAN[TXC_INDEX].Data.DWORD[1]){//send SDO_ABORT
                     CO_TXCAN[TXC_INDEX].Data.BYTE[0] = 0x80;
                     STATE = 0;
                     CO_TXCANsend(TXC_INDEX);
                     break;
                  }
                  STATE = 0;
               }
               else{
                  //reset timeout timer, alternate toggle bit
                  TMR8Z(tTIMEOUT);
                  if(STATE&0x10) STATE &= 0xEF;
                  else STATE |= 0x10;
               }
               //write other response data
               CO_TXCAN[TXC_INDEX].Data.BYTE[1] = 0;
               CO_TXCAN[TXC_INDEX].Data.BYTE[2] = 0;
               CO_TXCAN[TXC_INDEX].Data.BYTE[3] = 0;
               //download segment response
               CO_TXCANsend(TXC_INDEX);
               break;
#endif
            case 2:   //Initiate SDO Upload request
               //find pointer to object dictionary entry
               pODE = CO_FindEntryInOD(*((unsigned int*)&CO_RXCAN[RXC_INDEX].Data.BYTE[1]), CO_RXCAN[RXC_INDEX].Data.BYTE[3]);
               if(!pODE){
                  SDO_ABORT(0x06020000L)  //object does not exist in OD
                  break;
               }
               else if(pODE->length > CO_MAX_OD_ENTRY_SIZE){  //length of ODE is not valid
                  SDO_ABORT(0x06040047L)   //general internal incompatibility in the device
                  break;
               }
               if((pODE->attribute&0x07) == ATTR_WO){
                  SDO_ABORT(0x06010001L)   //attempt to read a write-only object
                  break;
               }
               if(pODE->length <= 4){
                  unsigned long ret;
                  //expedited transfer
                  ret = CO_OD_Read(pODE, (void*)&CO_TXCAN[TXC_INDEX].Data.DWORD[1], 4);
                  if(ret) SDO_ABORT(ret)
                  else{
                     if(pODE->attribute & ATTR_ADD_ID) CO_TXCAN[TXC_INDEX].Data.DWORD[1] += CO_NodeID;
                     CO_TXCAN[TXC_INDEX].Data.BYTE[0] = 0x43 | ((4-pODE->length) << 2);
                     STATE = 0;
                     CO_TXCANsend(TXC_INDEX);
                  }
               }
#if CO_MAX_OD_ENTRY_SIZE == 0x04
               break;
#else
               else{
                  unsigned long ret;
                  //segmented transfer
                  ret = CO_OD_Read(pODE, (void*)&BUFF[0], CO_MAX_OD_ENTRY_SIZE);
                  if(ret) SDO_ABORT(ret)
                  else{
                     BUFFSIZE = 0;  //indicates pointer to next data to be send
                     TMR8Z(tTIMEOUT);
                     STATE = 0x04;
                     CO_TXCAN[TXC_INDEX].Data.BYTE[0] = 0x41;
                     CO_TXCAN[TXC_INDEX].Data.DWORD[1] = (unsigned long)pODE->length;
                     CO_TXCANsend(TXC_INDEX);
                  }
               }
               break;

            case 3:   //Upload SDO segment
               if(!(STATE&0x04)){//upload SDO segment was not initiated
                  SDO_ABORT(0x05040001L) //command specifier not valid
                  break;
               }
               //verify toggle bit
               if((CO_RXCAN[RXC_INDEX].Data.BYTE[0]&0x10) != (STATE&0x10)){
                  SDO_ABORT(0x05030000L) //toggle bit not alternated
                  break;
               }
               //calculate length to be sent
               j = pODE->length - BUFFSIZE;
               if(j > 7) j = 7;
               //fill data bytes
               for(i=0; i<j; i++)
                  CO_TXCAN[TXC_INDEX].Data.BYTE[i+1] = BUFF[BUFFSIZE+i];
               for(; i<7; i++)
                  CO_TXCAN[TXC_INDEX].Data.BYTE[i+1] = 0;
               BUFFSIZE += j;
               CO_TXCAN[TXC_INDEX].Data.BYTE[0] = 0x00 | (STATE&0x10) | ((7-j)<<1);
               //is end of transfer?
               if(BUFFSIZE==pODE->length){
                  CO_TXCAN[TXC_INDEX].Data.BYTE[0] |= 1;
                  STATE = 0;
               }
               else{
                  //reset timeout timer, alternate toggle bit
                  TMR8Z(tTIMEOUT);
                  if(STATE&0x10) STATE &= 0xEF;
                  else STATE |= 0x10;
               }
               CO_TXCANsend(TXC_INDEX);
               break;
#endif
            case 4:   //Abort SDO transfer by client
#if CO_MAX_OD_ENTRY_SIZE > 0x04
               STATE = 0;
#endif
               break;

            default:
               SDO_ABORT(0x05040001L) //command specifier not valid
         }//end switch
         CO_RXCAN[RXC_INDEX].NewMsg = 0;//release
      }//end process new SDO object

   //verify timeout of segmented transfer
#if CO_MAX_OD_ENTRY_SIZE > 0x04
      if(STATE){ //Segmented SDO transfer in progress
         if(TMR8(tTIMEOUT) >= CO_SDO_TIMEOUT_TIME){
            SDO_ABORT(0x05040000L)  //SDO protocol timed out
         }
      }
#endif
   }
   else{ //not in (pre)operational state
      STATE = 0;
      CO_RXCAN[RXC_INDEX].NewMsg = 0;//release buffer
   }

   #undef pODE
   #undef STATE
   #undef BUFF
   #undef BUFFSIZE
   #undef tTIMEOUT
   #undef RXC_INDEX
   #undef TXC_INDEX

#endif //end of SDO server


/* Heartbeat consumer message handling */
   //DS 301: "Monitoring starts after the reception of the first HeartBeat. (Not bootup)"
   CO_HBcons_AllMonitoredOperationalCpy = 5;
#if CO_NO_CONS_HEARTBEAT > 0
   if(CO_NMToperatingState==NMT_PRE_OPERATIONAL || CO_NMToperatingState==NMT_OPERATIONAL){
      for(i=0; i<CO_NO_CONS_HEARTBEAT; i++){
         if((unsigned int)ODE_Consumer_Heartbeat_Time[i]){//is node monitored
            //Verify if new Consumer Heartbeat message received
            if(CO_RXCAN[CO_RXCAN_CONS_HB+i].NewMsg){
               CO_HBcons_NMTstate(i) = CO_RXCAN[CO_RXCAN_CONS_HB+i].Data.BYTE[0];
               if(CO_HBcons_NMTstate(i))//must not be a bootup message
                  TMR16Z(CO_HBcons_TimerValue(i));  //reset timer
               CO_RXCAN[CO_RXCAN_CONS_HB+i].NewMsg = 0;
            }
            //Verify timeout
            else{
               if(CO_HBcons_MonStarted(i)){//Monitoring starts after the reception of the first heartbeat (not Bootup)
                  if(TMR16(CO_HBcons_TimerValue(i)) > (unsigned int)ODE_Consumer_Heartbeat_Time[i]){
                     ErrorReport(ERROR_HEARTBEAT_CONSUMER, 0);
                     CO_HBcons_NMTstate(i) = 0;
                  }
               }
               else{ //monitoring did not yet sterted
                  if(CO_HBcons_NMTstate(i)) CO_HBcons_MonStarted(i) = 1;
               }
            }
            if(CO_HBcons_NMTstate(i) != NMT_OPERATIONAL)
               CO_HBcons_AllMonitoredOperationalCpy = 0;
         }
      }
   }
   else{ //not in (pre)operational state
      for(i=0; i<CO_NO_CONS_HEARTBEAT; i++){
         CO_HBcons_NMTstate(i) = 0;
         CO_RXCAN[CO_RXCAN_CONS_HB+i].NewMsg = 0;
         CO_HBcons_MonStarted(i) = 0;
      }
      CO_HBcons_AllMonitoredOperationalCpy = 0;
   }
#endif
   CO_HBcons_AllMonitoredOperational = CO_HBcons_AllMonitoredOperationalCpy;

/* Heartbeat producer message & Bootup message*/
   //Sent only if not in TX passive, bootup send always
   if((ODE_Producer_Heartbeat_Time && (TMR16(tProducerHeartbeatTime) >= ODE_Producer_Heartbeat_Time))
          || CO_NMToperatingState == NMT_INITIALIZING){
      TMR16Z(tProducerHeartbeatTime);
      CO_TXCAN[CO_TXCAN_HB].Data.BYTE[0] = CO_NMToperatingState;
      CO_TXCANsend(CO_TXCAN_HB);
      if(CO_NMToperatingState == NMT_INITIALIZING){
         if((ODE_NMT_Startup & 0x04) == 0) CO_NMToperatingState = NMT_OPERATIONAL;
         else                              CO_NMToperatingState = NMT_PRE_OPERATIONAL;
      }
   }

/* Status LEDs */
   //green RUN LED (DR 303-3)
   switch(CO_NMToperatingState){
      case NMT_STOPPED:
         PCB_RUN_LED(CO_StatusLED.SingleFlash);
         break;
      case NMT_PRE_OPERATIONAL:
         PCB_RUN_LED(CO_StatusLED.Blinking);
         break;
      case NMT_OPERATIONAL:
         PCB_RUN_LED(CO_StatusLED.On);
         break;
   }

   //red ERROR LED (DR 303-3)
   if(ERROR_BIT_READ(ERROR_CAN_TX_BUS_OFF))           PCB_ERROR_LED(CO_StatusLED.On);
   #if CO_NO_SYNC > 0
      else if(ERROR_BIT_READ(ERROR_SYNC_TIME_OUT))       PCB_ERROR_LED(CO_StatusLED.TripleFlash);
   #endif
   #if CO_NO_CONS_HEARTBEAT > 0
      else if(ERROR_BIT_READ(ERROR_HEARTBEAT_CONSUMER))  PCB_ERROR_LED(CO_StatusLED.DoubleFlash);
   #endif
   else if( ERROR_BIT_READ(ERROR_CAN_TX_BUS_PASSIVE)
         || ERROR_BIT_READ(ERROR_CAN_RX_BUS_PASSIVE)
         || ERROR_BIT_READ(ERROR_CAN_BUS_WARNING))    
            PCB_ERROR_LED(CO_StatusLED.SingleFlash);
   else if(ODE_Error_Register)                        
            PCB_ERROR_LED(CO_StatusLED.Blinking);//not in CiA standard
   else                                               
            PCB_ERROR_LED(CO_StatusLED.Off);

}
