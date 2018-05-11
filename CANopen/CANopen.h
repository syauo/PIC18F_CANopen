/*******************************************************************************

   CANopen.h - Main header file for CANopenNode

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

#ifndef _CAN_OPEN_H
#define _CAN_OPEN_H

#include "CO_driver.h"
#include "CO_OD.h"
#include "CO_errors.h"

/*******************************************************************************
   CANopen NMT constants    NMT 常量
*******************************************************************************/
   //States   状态
   #define NMT_INITIALIZING    0      // 初始化
   #define NMT_PRE_OPERATIONAL 127    // 预运行
   #define NMT_OPERATIONAL     5      // 运行
   #define NMT_STOPPED         4      // 停止
   //commands 命令
   #define NMT_ENTER_OPERATIONAL       1    // 进入运行状态命令
   #define NMT_ENTER_STOPPED           2    // 停止命令
   #define NMT_ENTER_PRE_OPERATIONAL   128  // 进入预运行状态
   #define NMT_RESET_NODE              129  // 复位节点
   #define NMT_RESET_COMMUNICATION     130  // 复位通信

/*******************************************************************************
   COB-IDs in CANopen   通信对象ID 预定义连接集
*******************************************************************************/
   #define CAN_ID_NMT_SERVICE  0x000
   #define CAN_ID_SYNC         0x080
   #define CAN_ID_EMERGENCY    0x080   // + NODE ID
   #define CAN_ID_TIME_STAMP   0x100
   #define CAN_ID_TPDO0        0x180   // + NODE ID
   #define CAN_ID_RPDO0        0x200   // + NODE ID
   #define CAN_ID_TPDO1        0x280   // + NODE ID
   #define CAN_ID_RPDO1        0x300   // + NODE ID
   #define CAN_ID_TPDO2        0x380   // + NODE ID
   #define CAN_ID_RPDO2        0x400   // + NODE ID
   #define CAN_ID_TPDO3        0x480   // + NODE ID
   #define CAN_ID_RPDO3        0x500   // + NODE ID
   #define CAN_ID_TSDO         0x580   // + NODE ID
   #define CAN_ID_RSDO         0x600   // + NODE ID
   #define CAN_ID_HEARTBEAT    0x700   // + NODE ID

/*******************************************************************************
   Universal structures and Macros for data access  用于数据访问的通用结构体和宏
*******************************************************************************/
/* 八位 */
   typedef struct{
      unsigned int bit0 :1;
      unsigned int bit1 :1;
      unsigned int bit2 :1;
      unsigned int bit3 :1;
      unsigned int bit4 :1;
      unsigned int bit5 :1;
      unsigned int bit6 :1;
      unsigned int bit7 :1;
      }tData8bits;

/* 单字节 */
   typedef union{
      unsigned char BYTE[1];
      tData8bits    BYTEbits[1];
      }tData1byte;

/* 双字节 */
   typedef union{
      unsigned int  WORD[1];
      unsigned char BYTE[2];
      tData8bits    BYTEbits[2];
      }tData2bytes;

/* 四字节 */
   typedef union{
      unsigned long DWORD[1];
      unsigned int  WORD[2];
      unsigned char BYTE[4];
      tData8bits    BYTEbits[4];
      }tData4bytes;//0x 12 34 56 78

/* 八字节 */
   typedef union{
      unsigned long DWORD[2];
      unsigned int  WORD[4];
      unsigned char BYTE[8];
      tData8bits    BYTEbits[8];
      }tData8bytes;

/*******************************************************************************
   Usefull bits for implementation status leds (variable can be read)
   实现led状态的位（可以读取变量）
*******************************************************************************/
   typedef struct {
      unsigned int On          :1;
      unsigned int Off         :1;
      unsigned int Flickering  :1;
      unsigned int Blinking    :1;
      unsigned int SingleFlash :1;
      unsigned int DoubleFlash :1;
      unsigned int TripleFlash :1;
   }CO_StatusLED_struct;
   extern volatile CO_StatusLED_struct CO_StatusLED;

/*******************************************************************************
   Object Dictionary      对象字典
*******************************************************************************/
   //One entry in Object Dictionary
   // 对象字典中的一个条目
   typedef struct {
      unsigned int   index;               // Index of OD entry    OD条目索引
      unsigned char  subindex;            // Subindex of OD entry OD条目子索引
      unsigned char  attribute;           // Attributes           属性
      unsigned char  length;              // Data length in bytes 数据长度n字节
      ROM void*      pData;               // POINTER to data (RAM or ROM memory) 指向的数据
   } CO_objectDictionaryEntry;

   //access attributes for object dictionary
   // 对象字典的访问属性
   #define ATTR_RW      0x00    //attribute: read/write
   #define ATTR_WO      0x01    //attribute: write/only
   #define ATTR_RO      0x02    //attribute: read/only (TPDO may read from that entry)
   #define ATTR_CO      0x03    //attribute: read/only, constant value
   #define ATTR_RWR     0x04    //attribute: read/write on process input (TPDO may read from that entry)
   #define ATTR_RWW     0x05    //attribute: read/write on process output (RPDO may write to that entry)
   #define ATTR_RES1    0x06    //attribute: Reserved 1
   #define ATTR_RES2    0x07    //attribute: Reserved 2
   //additional attributes, must be '|' with access attributes
   // 附加属性，必须与访问属性『|』
   #define ATTR_RES3    0x80    //attribute: Reserved 3
   #define ATTR_ROM     0x10    //attribute: ROM variable is saved in retentive memory
   #define ATTR_ADD_ID  0x20    //attribute: add NODE-ID to variable value (sizeof(variable)<=4)

   //Object Dictionary 对象字典
   extern ROM CO_objectDictionaryEntry CO_OD[];
   //Number of Elements in Object Dictionary  对象字典中的元素个数
   extern ROM unsigned int CO_OD_NoOfElements;
   //Function for search Object Dictionary    查找对象字典函数
   ROM CO_objectDictionaryEntry* CO_FindEntryInOD(unsigned int index, unsigned char subindex);

/*******************************************************************************
   CAN message Structure    CAN 消息结构体
*******************************************************************************/
   typedef struct{
      tData2bytes  Ident;        //Can message identifier aligned with hardware registers
                                 //11 bit COB-ID and Remote Transfer Request bit are included
                                 //see CO_IDENT_WRITE macro in CO_driver.h
                                 //Remote transfer request bit:
                                 //Reception: receive message only if matched RTR
                                 //Transmission: send normal or RTR message
      unsigned int NoOfBytes :4; //length of message, if>8, length is not monitored at receive
      unsigned int NewMsg    :1; //flag bit, if new message has received (is waiting to be sent)
      unsigned int Inhibit   :1; //Reception: Data are not copied, if Inhibit==1 AND NewMsg==1
                                 //Transmission: message is not sent if Inhibit==1 AND time is outside
                                 //CO_SYNCwindow (Flag for synchronous TPDO messages)
      tData8bytes  Data;         //Data
   }CO_CanMessage;

/*******************************************************************************
   Main variables for storing CAN messages  存储 CAN 消息的主要变量
*******************************************************************************/
   extern volatile CO_CanMessage CO_RXCAN[];                         //Receive  接收消息
   #define CO_RXCAN_NMT       0                                      //index for NMT message
   #define CO_RXCAN_SYNC      1                                      //index for SYNC message
   #define CO_RXCAN_RPDO     (CO_RXCAN_SYNC+CO_NO_SYNC)              //start index for RPDO messages
   #define CO_RXCAN_SDO_SRV  (CO_RXCAN_RPDO+CO_NO_RPDO)              //start index for SDO server message (request)
   #define CO_RXCAN_SDO_CLI  (CO_RXCAN_SDO_SRV+CO_NO_SDO_SERVER)     //start index for SDO client message (response)
   #define CO_RXCAN_CONS_HB  (CO_RXCAN_SDO_CLI+CO_NO_SDO_CLIENT)     //start index for Heartbeat Consumer messages
   #define CO_RXCAN_USER     (CO_RXCAN_CONS_HB+CO_NO_CONS_HEARTBEAT) //start index for user defined CANrx messages

   extern volatile CO_CanMessage CO_TXCAN[];                         //Transmit 发送消息
   #define CO_TXCAN_SYNC      0                                      //index for SYNC message
   #define CO_TXCAN_EMERG    (CO_TXCAN_SYNC+CO_NO_SYNC)              //index for Emergency message
   #define CO_TXCAN_TPDO     (CO_TXCAN_EMERG+CO_NO_EMERGENCY)        //start index for TPDO messages
   #define CO_TXCAN_SDO_SRV  (CO_TXCAN_TPDO+CO_NO_TPDO)              //start index for SDO server message (response)
   #define CO_TXCAN_SDO_CLI  (CO_TXCAN_SDO_SRV+CO_NO_SDO_SERVER)     //start index for SDO client message (request)
   #define CO_TXCAN_HB       (CO_TXCAN_SDO_CLI+CO_NO_SDO_CLIENT)     //index for Heartbeat message
   #define CO_TXCAN_USER     (CO_TXCAN_HB+1)                         //start index for user defined CANtx messages

   //total number of received/transmited CAN messages
   // 接收／发送CAN消息的总个数
   #define CO_RXCAN_NO_MSGS (1+CO_NO_SYNC+CO_NO_RPDO+CO_NO_SDO_SERVER+CO_NO_SDO_CLIENT+CO_NO_CONS_HEARTBEAT+CO_NO_USR_CAN_RX)
   #define CO_TXCAN_NO_MSGS (CO_NO_SYNC+CO_NO_EMERGENCY+CO_NO_TPDO+CO_NO_SDO_SERVER+CO_NO_SDO_CLIENT+1+CO_NO_USR_CAN_TX)

/*******************************************************************************
   Macros and Variables from CO_stack.c useful for user program
   CO_stack.c 中用于用户程序的变量和宏
*******************************************************************************/
   extern unsigned char CO_NodeID;                    //CANopen nodeID
   extern unsigned char CO_BitRate;                   //CANopen Bit rate  比特率
   extern volatile unsigned char CO_NMToperatingState;//Operating state of this node  该节点运行状态

   #if CO_NO_SYNC > 0
      // 在 SYNC 消息后，变量自增
      extern volatile unsigned int CO_SYNCcounter;       //variable is incremented after SYNC message
      // 变量每毫秒自增，SYNC 消息后清零
      extern volatile unsigned int CO_SYNCtime;          //variable is incremented every 1ms, after SYNC message it is set to 0
   #endif

 /*PDO Data (type tData8bytes)    PDO 数据（类型 tData8bytes 八字节）*/
   // 接收
   #if CO_NO_RPDO > 0
      #define CO_RPDO(i)            CO_RXCAN[CO_RXCAN_RPDO+i].Data            //(0 <= i < CO_NO_RPDO)
      #define CO_RPDO_New(i)        CO_RXCAN[CO_RXCAN_RPDO+i].NewMsg
   #endif

   // 发送
   #if CO_NO_TPDO > 0
      #define CO_TPDO(i)               CO_TXCAN[CO_TXCAN_TPDO+i].Data            //(0 <= i < CO_NO_TPDO)
   #endif

 /*Heartbeat consumer - operating state of monitored nodes (type unsigned char)
   心跳消费者 - 被监控节点的运行状态 （类型 unsigned char）*/
   // 如果没有被监控节点，变量仍是正确的
   extern volatile unsigned char CO_HBcons_AllMonitoredOperational;//if no monitored nodes, variable is still true
   #if CO_NO_CONS_HEARTBEAT > 0
      #define CO_HBcons_NMTstate(i)    CO_RXCAN[CO_RXCAN_CONS_HB+i].Data.BYTE[1] //(0 <= i < CO_NO_CONS_HEARTBEAT)
   #endif

/*******************************************************************************
   Functions for sending messages directly
   用于直接发送消息的函数
*******************************************************************************/
   //Send CAN message (can be called from mainline or timer interrupt)
   // 发送 CAN 消息（可以在主线程序或定时器中断中调用）
   //   INPUT:   Index - index of CO_TXCAN array to be send
   //                    要发送的 CO_TXCAN 数组索引
   //   OUTPUT:  0 = success, 1 = error, previous message was not sent
   char CO_TXCANsend(unsigned int index);

   //Send TPDO (can be called from mainline or timer interrupt)
   // 发送 TPDO（可以在主线程序或定时器中断中调用）
   //   INPUT:   Index - index of PDO to be send (0 = first TPDO)
   //                    要发送的PDO的索引（0 = 第一个TPDO）
   //   OUTPUT:  0 = success, 1 = error, previous message was not sent
   char CO_TPDOsend(unsigned int index);

/*******************************************************************************
   Functions for SDO client            SDO 客户端的函数
*******************************************************************************/
   #if CO_NO_SDO_CLIENT > 0
      char CO_SDOclient_setup(unsigned char channel, unsigned long COB_ID_Client_to_Server,
            unsigned long COB_ID_Server_to_Client, unsigned char NODE_ID_of_SDO_Server);

      char CO_SDOclientDownload_init(unsigned char channel, unsigned int index, unsigned char subindex,
                                     unsigned char* pBuff, unsigned char dataSize);
      char CO_SDOclientDownload(unsigned char channel, unsigned long* pSDOabortCode);

      char CO_SDOclientUpload_init(unsigned char channel, unsigned int index, unsigned char subindex,
                                   unsigned char* pBuff, unsigned char BuffMaxSize);
      char CO_SDOclientUpload(unsigned char channel, unsigned long* pSDOabortCode, unsigned char* dataSize);

      char CO_SDOclientDownload_wait(unsigned char NODE_ID_of_SDO_Server, unsigned int index, unsigned char subindex,
                                     unsigned char* pBuff, unsigned char BuffMaxSize, unsigned long* pSDOabortCode);
      char CO_SDOclientUpload_wait(unsigned char NODE_ID_of_SDO_Server, unsigned int index, unsigned char subindex,
                                   unsigned char* pBuff, unsigned char BuffMaxSize, unsigned char* dataSize, unsigned long* pSDOabortCode);
   #endif

#endif
