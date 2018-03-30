/*******************************************************************************

   CO_OD.h - Definitions for default values in Object Dictionary

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

********************************************************************************

   For short description of standard Object Dictionary entries see CO_OD.txt

*******************************************************************************/

#ifndef _CO_OD_H
#define _CO_OD_H

/*******************************************************************************
   Setup CANopen    配置
*******************************************************************************/
   //(0 or 1), 是否使用 SYNC （生产者或消费者）
   #define CO_NO_SYNC            0  //(0 or 1), is SYNC (producer and consumer) used or not.
   //(0 or 1), 是否使用紧急消息生产者.
   #define CO_NO_EMERGENCY       1  //(0 or 1), is Emergency message producer used or not.
   //(0 to 512*), 接收 PDO 的个数
   #define CO_NO_RPDO            4  //(0 to 512*), number of receive PDOs.
   //(0 to 512*), 发送 PDO 的个数
   #define CO_NO_TPDO            4  //(0 to 512*), number of transmit PDOs.
   //(0 to 128*), SDO server 通道个数
   #define CO_NO_SDO_SERVER      1  //(0 to 128*), number of SDO server channels.
   //(0 to 128*), SDO client 通道个数
   #define CO_NO_SDO_CLIENT      0  //(0 to 128*), number of SDO client channels.
   //(0 to 255*), 消费者心跳条目个数
   #define CO_NO_CONS_HEARTBEAT  4  //(0 to 255*), number of consumer heartbeat entries.
   //(0 to ...), 用户接收 CAN 消息数量
   #define CO_NO_USR_CAN_RX      0  //(0 to ...), number of user CAN RX messages.
   //(0 to ...), 用户发送 CAN 消息数量
   #define CO_NO_USR_CAN_TX      0  //(0 to ...), number of user CAN TX messages.
   //(4 to 256), 对象字典中变量的最大字节长度
   #define CO_MAX_OD_ENTRY_SIZE  20 //(4 to 256), max size of variable in Object Dictionary in bytes.
   //[in 100*ms] SDO 通信超时设置
   #define CO_SDO_TIMEOUT_TIME   10 //[in 100*ms] Timeout in SDO communication.
   //(0 to 254*), 在索引 0x1003 预定义的错误区大小 
   #define CO_NO_ERROR_FIELD     8  //(0 to 254*), size of Pre Defined Error Fields at index 0x1003.
   //定义后，将在对象字典中使用 PDO parameters
   #define CO_PDO_PARAM_IN_OD       //if defined, PDO parameters will be in Object Dictionary.
   //定义后，将在 OD 中使用 PDO 映射；未定义，则 PDO 大小固定为 8 字节
   #define CO_PDO_MAPPING_IN_OD     //if defined, PDO mapping will be in Object Dictionary. If not defined, PDO size will be fixed to 8 bytes.
   //定义后，将在 TPDO 发送时使用抑制和事件定时器
   #define CO_TPDO_INH_EV_TIMER     //if defined, Inhibit and Event timer will be used for TPDO transmission.
   //定义后，SDO 写对象字典时将验证
   #define CO_VERIFY_OD_WRITE       //if defined, SDO write to Object Dictionary will be verified.
   //若 CO_OD 中的条目是排序的（从最低到最高索引，然后是子索引），则启用此宏。
   //这对于更长的对象字典搜索要快得多。
   //若没有排序，则禁用此宏。
   #define CO_OD_IS_ORDERED         //enable this macro, if entries in CO_OD are ordered (from lowest to highest index, then subindex). For longer Object Dictionaries searching is much faster. If entries are not ordered, disable macro.
   //定义后，将保存 ODE_EEPROM 数据
   #define CO_SAVE_EEPROM           //if defined, ODE_EEPROM data will be saved.
   //定义后，ROM 变量可从外部写入
   #define CO_SAVE_ROM              //if defined, ROM variables will be writeable from outside.

// * For some macros have written only 1 or up to 8 entries in CO_OD.c/CO_OD.h.
//   If you get error message and if you need more than that, write additional entries.

// * 对于一些宏，在 CO_OD.c / CO_OD.h 中只写入了 1 或 8 个条目。
//   如果收到错误消息，或者需要更多的话，再写附加条目。

/*******************************************************************************
   Device profile for Generic I/O   设备配置
*******************************************************************************/
   #define CO_IO_DIGITAL_INPUTS     //4 * 8 digital inputs
   #define CO_IO_DIGITAL_OUTPUTS    //4 * 8 digital outputs
   #define CO_IO_ANALOG_INPUTS      //8 * 16bit analog inputs
   #define CO_IO_ANALOG_OUTPUTS     //2 * 16bit analog outputs


/*******************************************************************************
   Default values for object dictionary 对象字典缺省值
*******************************************************************************/
   #define ODD_DEVICE_TYPE       0x000F0191L    /*index 0x1000, RO*/ //See standard
   #define ODD_SYNC_COB_ID       0x00000080L    /*index 0x1005*/     //if bit30=1, node produces SYNC
   #define ODD_COMM_CYCLE_PERIOD 0L             /*index 0x1006*/     //in micro seconds
   #define ODD_SYNCHR_WINDOW_LEN 0L             /*index 0x1007*/     //in micro seconds
   #define ODD_MANUF_DEVICE_NAME "HYPITCH"      /*index 0x1008, RO*/
   #define ODD_MANUF_HW_VERSION  "1.00"         /*index 0x1009, RO*/
   #define ODD_MANUF_SW_VERSION  "1.00"         /*index 0x100A, RO*/
   #define ODD_INHIBIT_TIME_EMER 50             /*index 0x1015*/     //time in 100 micro seconds
                                                /*index 0x1016*/     //see below
   #define ODD_PROD_HEARTBEAT    1000           /*index 0x1017*/     //time in ms
   #define ODD_IDENT_VENDOR_ID   0x00000000L    /*index 0x1018, RO*/ //See standard
   #define ODD_IDENT_PROD_CODE   0x00000000L
   #define ODD_IDENT_REVISION_NR 0x00000000L
   #define ODD_IDENT_SERIAL_NR   0x00000000L
   #define ODD_ERROR_BEH_COMM    0x00           /*index 0x1029*/     //If Communication error is present in Operational state: 0x00-switch to pre-operational; 0x01-do nothing; 0x02-switch to stopped.
   #define ODD_NMT_STARTUP       0x00000000L    /*index 0x1F80*/     //only bit2 implemented, if bit2=1, Node will NOT start operational

/* 0x1016 Heartbeat consumer **************************************************/
/*        心跳消费者          */
   //00NNTTTT: N=NodeID, T=time in ms
   #define ODD_CONS_HEARTBEAT_0  0x00000000L
   #define ODD_CONS_HEARTBEAT_1  0x00000000L
   #define ODD_CONS_HEARTBEAT_2  0x00000000L
   #define ODD_CONS_HEARTBEAT_3  0x00000000L
   #define ODD_CONS_HEARTBEAT_4  0x00000000L
   #define ODD_CONS_HEARTBEAT_5  0x00000000L
   #define ODD_CONS_HEARTBEAT_6  0x00000000L
   #define ODD_CONS_HEARTBEAT_7  0x00000000L

/* 0x1400 Receive PDO parameters **********************************************/
/*        接收 PDO 参数           */
   //COB-ID: if(bit31==1) PDO is not used; bit30=1; bits(10..0)=COB-ID;
   #define ODD_RPDO_PAR_COB_ID_0 0  //if 0, predefined value will be used (0x200+NODE-ID - read-only)
   #define ODD_RPDO_PAR_T_TYPE_0 255
   #define ODD_RPDO_PAR_COB_ID_1 0  //if 0, predefined value will be used (0x300+NODE-ID - read-only)
   #define ODD_RPDO_PAR_T_TYPE_1 255
   #define ODD_RPDO_PAR_COB_ID_2 0  //if 0, predefined value will be used (0x400+NODE-ID - read-only)
   #define ODD_RPDO_PAR_T_TYPE_2 255
   #define ODD_RPDO_PAR_COB_ID_3 0  //if 0, predefined value will be used (0x500+NODE-ID - read-only)
   #define ODD_RPDO_PAR_T_TYPE_3 255
   #define ODD_RPDO_PAR_COB_ID_4 0xC0000000L
   #define ODD_RPDO_PAR_T_TYPE_4 255
   #define ODD_RPDO_PAR_COB_ID_5 0xC0000000L
   #define ODD_RPDO_PAR_T_TYPE_5 255
   #define ODD_RPDO_PAR_COB_ID_6 0xC0000000L
   #define ODD_RPDO_PAR_T_TYPE_6 255
   #define ODD_RPDO_PAR_COB_ID_7 0xC0000000L
   #define ODD_RPDO_PAR_T_TYPE_7 255

/* 0x1600 Receive PDO mapping *************************************************/
/*        接收 PDO 映射        */
   //0xIIIISSDD IIII = index from OD, SS = subindex, DD = data length in bits
   //                  OD索引               子索引         数据位长
   //DD must be byte aligned, max value 0x40 (8 bytes)
   //DD - 数据位长必须字节对齐（8*），最大值为 0x40 即 8 字节 64 位
   #define ODD_RPDO_MAP_0_1      0x62000108L
   #define ODD_RPDO_MAP_0_2      0x62000208L
   #define ODD_RPDO_MAP_0_3      0x62000308L
   #define ODD_RPDO_MAP_0_4      0x62000408L
   #define ODD_RPDO_MAP_0_5      0x00000000L
   #define ODD_RPDO_MAP_0_6      0x00000000L
   #define ODD_RPDO_MAP_0_7      0x00000000L
   #define ODD_RPDO_MAP_0_8      0x00000000L

   #define ODD_RPDO_MAP_1_1      0x64110110L
   #define ODD_RPDO_MAP_1_2      0x64110210L
   #define ODD_RPDO_MAP_1_3      0x00000000L
   #define ODD_RPDO_MAP_1_4      0x00000000L
   #define ODD_RPDO_MAP_1_5      0x00000000L
   #define ODD_RPDO_MAP_1_6      0x00000000L
   #define ODD_RPDO_MAP_1_7      0x00000000L
   #define ODD_RPDO_MAP_1_8      0x00000000L

   #define ODD_RPDO_MAP_2_1      0x00000040L
   #define ODD_RPDO_MAP_2_2      0x00000000L
   #define ODD_RPDO_MAP_2_3      0x00000000L
   #define ODD_RPDO_MAP_2_4      0x00000000L
   #define ODD_RPDO_MAP_2_5      0x00000000L
   #define ODD_RPDO_MAP_2_6      0x00000000L
   #define ODD_RPDO_MAP_2_7      0x00000000L
   #define ODD_RPDO_MAP_2_8      0x00000000L

   #define ODD_RPDO_MAP_3_1      0x00000040L
   #define ODD_RPDO_MAP_3_2      0x00000000L
   #define ODD_RPDO_MAP_3_3      0x00000000L
   #define ODD_RPDO_MAP_3_4      0x00000000L
   #define ODD_RPDO_MAP_3_5      0x00000000L
   #define ODD_RPDO_MAP_3_6      0x00000000L
   #define ODD_RPDO_MAP_3_7      0x00000000L
   #define ODD_RPDO_MAP_3_8      0x00000000L

   #define ODD_RPDO_MAP_4_1      0x00000040L
   #define ODD_RPDO_MAP_4_2      0x00000000L
   #define ODD_RPDO_MAP_4_3      0x00000000L
   #define ODD_RPDO_MAP_4_4      0x00000000L
   #define ODD_RPDO_MAP_4_5      0x00000000L
   #define ODD_RPDO_MAP_4_6      0x00000000L
   #define ODD_RPDO_MAP_4_7      0x00000000L
   #define ODD_RPDO_MAP_4_8      0x00000000L

   #define ODD_RPDO_MAP_5_1      0x00000040L
   #define ODD_RPDO_MAP_5_2      0x00000000L
   #define ODD_RPDO_MAP_5_3      0x00000000L
   #define ODD_RPDO_MAP_5_4      0x00000000L
   #define ODD_RPDO_MAP_5_5      0x00000000L
   #define ODD_RPDO_MAP_5_6      0x00000000L
   #define ODD_RPDO_MAP_5_7      0x00000000L
   #define ODD_RPDO_MAP_5_8      0x00000000L

   #define ODD_RPDO_MAP_6_1      0x00000040L
   #define ODD_RPDO_MAP_6_2      0x00000000L
   #define ODD_RPDO_MAP_6_3      0x00000000L
   #define ODD_RPDO_MAP_6_4      0x00000000L
   #define ODD_RPDO_MAP_6_5      0x00000000L
   #define ODD_RPDO_MAP_6_6      0x00000000L
   #define ODD_RPDO_MAP_6_7      0x00000000L
   #define ODD_RPDO_MAP_6_8      0x00000000L

   #define ODD_RPDO_MAP_7_1      0x00000040L
   #define ODD_RPDO_MAP_7_2      0x00000000L
   #define ODD_RPDO_MAP_7_3      0x00000000L
   #define ODD_RPDO_MAP_7_4      0x00000000L
   #define ODD_RPDO_MAP_7_5      0x00000000L
   #define ODD_RPDO_MAP_7_6      0x00000000L
   #define ODD_RPDO_MAP_7_7      0x00000000L
   #define ODD_RPDO_MAP_7_8      0x00000000L

/* 0x1800 Transmit PDO parameters *********************************************/
/*        发送 PDO 参数            */
   //COB-ID: if(bit31==1) PDO is not used; bit30=1; bits(10..0)=COB-ID;
   //T_TYPE: 1-240...transmission after every (T_TYPE)-th SYNC object;
   //                在每第 (T_TYPE) 次 SYNC 对象后发送
   //        254...manufacturer specific
   //              制造商规定
   //        255...Device Profile specific, default transmission is Change of State
   //              设备配置规定，缺省为「状态改变」
   //I_TIME: 0...65535 Inhibit time in 100µs is minimum time between PDO transmission
   //                  以 100µs 为单位，是PDO 发送最小间隔时间
   //E_TIME: 0...65535 Event timer in ms - PDO is periodically transmitted (0 == disabled)
   //                  时间定时器，单位 ms，PDO 定时周期性发送（0 = 禁用）
   #define ODD_TPDO_PAR_COB_ID_0 0  //if 0, predefined value will be used (0x180+NODE-ID - read-only)
   #define ODD_TPDO_PAR_T_TYPE_0 255
   #define ODD_TPDO_PAR_I_TIME_0 0
   #define ODD_TPDO_PAR_E_TIME_0 0
   #define ODD_TPDO_PAR_COB_ID_1 0  //if 0, predefined value will be used (0x280+NODE-ID - read-only)
   #define ODD_TPDO_PAR_T_TYPE_1 255
   #define ODD_TPDO_PAR_I_TIME_1 0
   #define ODD_TPDO_PAR_E_TIME_1 0
   #define ODD_TPDO_PAR_COB_ID_2 0  //if 0, predefined value will be used (0x380+NODE-ID - read-only)
   #define ODD_TPDO_PAR_T_TYPE_2 255
   #define ODD_TPDO_PAR_I_TIME_2 0
   #define ODD_TPDO_PAR_E_TIME_2 0
   #define ODD_TPDO_PAR_COB_ID_3 0  //if 0, predefined value will be used (0x480+NODE-ID - read-only)
   #define ODD_TPDO_PAR_T_TYPE_3 255
   #define ODD_TPDO_PAR_I_TIME_3 0
   #define ODD_TPDO_PAR_E_TIME_3 0
   #define ODD_TPDO_PAR_COB_ID_4 0xC0000000L
   #define ODD_TPDO_PAR_T_TYPE_4 254
   #define ODD_TPDO_PAR_I_TIME_4 0
   #define ODD_TPDO_PAR_E_TIME_4 0
   #define ODD_TPDO_PAR_COB_ID_5 0xC0000000L
   #define ODD_TPDO_PAR_T_TYPE_5 254
   #define ODD_TPDO_PAR_I_TIME_5 0
   #define ODD_TPDO_PAR_E_TIME_5 0
   #define ODD_TPDO_PAR_COB_ID_6 0xC0000000L
   #define ODD_TPDO_PAR_T_TYPE_6 254
   #define ODD_TPDO_PAR_I_TIME_6 0
   #define ODD_TPDO_PAR_E_TIME_6 0
   #define ODD_TPDO_PAR_COB_ID_7 0xC0000000L
   #define ODD_TPDO_PAR_T_TYPE_7 254
   #define ODD_TPDO_PAR_I_TIME_7 0
   #define ODD_TPDO_PAR_E_TIME_7 0

/* 0x1A00 Transmit PDO mapping ************************************************/
/*        发送 PDO 映射 */
   //0xIIIISSDD IIII = index from OD, SS = subindex, DD = data length in bits
   //                  对象字典索引          子索引         数据位长
   //DD must be byte aligned, max value 0x40 (8 bytes)
   //DD 必须是字节对齐的，最大值 0x40 即 8 字节 64 位。
   #define ODD_TPDO_MAP_0_1      0x60000108L
   #define ODD_TPDO_MAP_0_2      0x60000208L
   #define ODD_TPDO_MAP_0_3      0x60000308L
   #define ODD_TPDO_MAP_0_4      0x60000408L
   #define ODD_TPDO_MAP_0_5      0x00000000L
   #define ODD_TPDO_MAP_0_6      0x00000000L
   #define ODD_TPDO_MAP_0_7      0x00000000L
   #define ODD_TPDO_MAP_0_8      0x00000000L

   #define ODD_TPDO_MAP_1_1      0x64010110L
   #define ODD_TPDO_MAP_1_2      0x64010210L
   #define ODD_TPDO_MAP_1_3      0x64010310L
   #define ODD_TPDO_MAP_1_4      0x64010410L
   #define ODD_TPDO_MAP_1_5      0x00000000L
   #define ODD_TPDO_MAP_1_6      0x00000000L
   #define ODD_TPDO_MAP_1_7      0x00000000L
   #define ODD_TPDO_MAP_1_8      0x00000000L

   #define ODD_TPDO_MAP_2_1      0x64010510L
   #define ODD_TPDO_MAP_2_2      0x64010610L
   #define ODD_TPDO_MAP_2_3      0x64010710L
   #define ODD_TPDO_MAP_2_4      0x64010810L
   #define ODD_TPDO_MAP_2_5      0x00000000L
   #define ODD_TPDO_MAP_2_6      0x00000000L
   #define ODD_TPDO_MAP_2_7      0x00000000L
   #define ODD_TPDO_MAP_2_8      0x00000000L

   #define ODD_TPDO_MAP_3_1      0x00000040L
   #define ODD_TPDO_MAP_3_2      0x00000000L
   #define ODD_TPDO_MAP_3_3      0x00000000L
   #define ODD_TPDO_MAP_3_4      0x00000000L
   #define ODD_TPDO_MAP_3_5      0x00000000L
   #define ODD_TPDO_MAP_3_6      0x00000000L
   #define ODD_TPDO_MAP_3_7      0x00000000L
   #define ODD_TPDO_MAP_3_8      0x00000000L

   #define ODD_TPDO_MAP_4_1      0x00000040L
   #define ODD_TPDO_MAP_4_2      0x00000000L
   #define ODD_TPDO_MAP_4_3      0x00000000L
   #define ODD_TPDO_MAP_4_4      0x00000000L
   #define ODD_TPDO_MAP_4_5      0x00000000L
   #define ODD_TPDO_MAP_4_6      0x00000000L
   #define ODD_TPDO_MAP_4_7      0x00000000L
   #define ODD_TPDO_MAP_4_8      0x00000000L

   #define ODD_TPDO_MAP_5_1      0x00000040L
   #define ODD_TPDO_MAP_5_2      0x00000000L
   #define ODD_TPDO_MAP_5_3      0x00000000L
   #define ODD_TPDO_MAP_5_4      0x00000000L
   #define ODD_TPDO_MAP_5_5      0x00000000L
   #define ODD_TPDO_MAP_5_6      0x00000000L
   #define ODD_TPDO_MAP_5_7      0x00000000L
   #define ODD_TPDO_MAP_5_8      0x00000000L

   #define ODD_TPDO_MAP_6_1      0x00000040L
   #define ODD_TPDO_MAP_6_2      0x00000000L
   #define ODD_TPDO_MAP_6_3      0x00000000L
   #define ODD_TPDO_MAP_6_4      0x00000000L
   #define ODD_TPDO_MAP_6_5      0x00000000L
   #define ODD_TPDO_MAP_6_6      0x00000000L
   #define ODD_TPDO_MAP_6_7      0x00000000L
   #define ODD_TPDO_MAP_6_8      0x00000000L

   #define ODD_TPDO_MAP_7_1      0x00000040L
   #define ODD_TPDO_MAP_7_2      0x00000000L
   #define ODD_TPDO_MAP_7_3      0x00000000L
   #define ODD_TPDO_MAP_7_4      0x00000000L
   #define ODD_TPDO_MAP_7_5      0x00000000L
   #define ODD_TPDO_MAP_7_6      0x00000000L
   #define ODD_TPDO_MAP_7_7      0x00000000L
   #define ODD_TPDO_MAP_7_8      0x00000000L


/*******************************************************************************
   Default values for user Object Dictionary Entries
   用户对象字典条目的缺省值
*******************************************************************************/
/*0x2101*/
   // (1 to 127), 默认节点号node ID
   #define ODD_CANnodeID    0x06 //(1 to 127), default node ID
/*0x2102*/
   // (0 to 7), 默认比特率
   #define ODD_CANbitRate   3    //(0 to 7), default CAN bit rate
                                    // 0 = 10 kbps    1 = 20 kbps
                                    // 2 = 50 kbps    3 = 125 kbps
                                    // 4 = 250 kbps   5 = 500 kbps
                                    // 6 = 800 kbps   7 = 1000 kbps


/*******************************************************************************
   CANopen basic Data Types
   CANopen 基本数据类型
*******************************************************************************/
   #define UNSIGNED8  unsigned char
   #define UNSIGNED16 unsigned int
   #define UNSIGNED32 unsigned long
   #define INTEGER8   char
   #define INTEGER16  int
   #define INTEGER32  long


/*******************************************************************************
   CANopen Variables from Object Dictionary
   对象字典中的 CANopen 变量
*******************************************************************************/
/*0x1001*/ extern       UNSIGNED8      ODE_Error_Register;
/*0x1002*/ extern       UNSIGNED32     ODE_Manufacturer_Status_Register;

           #if CO_NO_ERROR_FIELD > 0
/*0x1003*/ extern       UNSIGNED8      ODE_Pre_Defined_Error_Field_NoOfErrors;
           extern       UNSIGNED32     ODE_Pre_Defined_Error_Field[];
           #endif

           #if CO_NO_SYNC > 0
/*0x1005*/ extern ROM   UNSIGNED32     ODE_SYNC_COB_ID;
/*0x1006*/ extern ROM   UNSIGNED32     ODE_Communication_Cycle_Period;
/*0x1007*/ extern ROM   UNSIGNED32     ODE_Synchronous_Window_Length;
           #endif

           #if CO_NO_EMERGENCY > 0
/*0x1014*/ extern ROM   UNSIGNED32     ODE_Emergency_COB_ID;
/*0x1015*/ extern ROM   UNSIGNED16     ODE_Inhibit_Time_Emergency;
           #endif

           #if CO_NO_CONS_HEARTBEAT > 0
/*0x1016*/ extern ROM   UNSIGNED32     ODE_Consumer_Heartbeat_Time[];
           #endif

/*0x1017*/ extern ROM   UNSIGNED16     ODE_Producer_Heartbeat_Time;

           typedef struct{
              UNSIGNED8    NoOfEntries;
              UNSIGNED8    Communication_Error;
           }               ODEs_Error_Behavior;
/*0x1029*/ extern ROM   ODEs_Error_Behavior  ODE_Error_Behavior;

           #if CO_NO_SDO_SERVER > 0 || CO_NO_SDO_CLIENT > 0
           typedef struct{
              UNSIGNED32   COB_ID_Client_to_Server;
              UNSIGNED32   COB_ID_Server_to_Client;
              UNSIGNED8    NODE_ID_of_SDO_Client_or_Server;
           }               CO_sSDO_Param;
           #endif

           #if CO_NO_SDO_SERVER > 0
/*0x1200*/ extern ROM   CO_sSDO_Param  ODE_Server_SDO_Parameter[];
           #endif

           #if CO_NO_SDO_CLIENT > 0
/*0x1280*/ extern       CO_sSDO_Param  ODE_Client_SDO_Parameter[];
           #endif

           #if CO_NO_RPDO > 0
           typedef struct{
              UNSIGNED32   COB_ID;
              UNSIGNED8    Transmission_type;
           }               CO_sRPDO_param;
/*0x1400*/ extern ROM   CO_sRPDO_param ODE_RPDO_Parameter[];
           #ifdef CO_PDO_MAPPING_IN_OD
/*0x1600*/ extern ROM   UNSIGNED32     ODE_RPDO_Mapping[][8];
           #endif
           #endif

           #if CO_NO_TPDO > 0
           typedef struct{
              UNSIGNED32   COB_ID;
              UNSIGNED8    Transmission_type;
              UNSIGNED16   Inhibit_Time;
              //UNSIGNED8    Compatibility_Entry; - not needed
              UNSIGNED16   Event_Timer;
           }               CO_sTPDO_param;
/*0x1800*/ extern ROM   CO_sTPDO_param ODE_TPDO_Parameter[];
           #ifdef CO_PDO_MAPPING_IN_OD
/*0x1A00*/ extern ROM   UNSIGNED32     ODE_TPDO_Mapping[][8];
           #endif
           #endif

/*0x1F80*/ extern ROM   UNSIGNED32     ODE_NMT_Startup;


/***** Manufacturer specific variables ****************************************/
/*0x2101*/ extern ROM   UNSIGNED8      ODE_CANnodeID;
/*0x2102*/ extern ROM   UNSIGNED8      ODE_CANbitRate;


/***** Manufacturer specific EEPROM DATA **************************************/
           typedef struct{
/*0x2106*/     UNSIGNED32     PowerOnCounter;   //Incremented every time, when chip is powered.


           }sODE_EEPROM;

           extern sODE_EEPROM ODE_EEPROM;


#endif
