/*******************************************************************************

   CO_errors.h - Header for error handling

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

   Error Status Bits indicate any error occurred in program. Specific bit is set
   by ErrorReport() function, when error occurs in program. It can also be reset
   by ErrorReset() function, if error is solved. On each change of any Error
   Status Bit Emergency message is sent. If critical bits are set, node will not
   be able to enter operational state.

   错误状态位指示程序中发生的任何错误。当程序出现错误时 ErrorReport() 函数设置特定对应
   位。如果错误得到解决，也可以通过 ErrorReset() 函数复位。任何错误状态位的每次更改都
   会发送紧急消息。如果重要位被置位，节点将不能进入运行状态。

****** standards ***************************************************************
         标准

   错误寄存器
   Error register 0x1001 (ds-301):
   bit      mandatory/opt      description
   -----------------------------------------------------------------------------
   0        M                  generic error    通用错误
   1        O                  current          电流
   2        O                  voltage          电压      
   3        O                  temperature      温度
   4        O                  communication error (overrun, error state) 通信错误
   5        O                  device profile specific      具体设备特定配置
   6        O                  Reserved (always 0)          保留
   7        O                  manufacturer specific        制造商特定配置

   紧急错误代码 ds-301
   Emergency error codes (ds-301):
   error code       description
   -----------------------------------------------------------------------------
   00xx             Error Reset or No Error     错误复位或没有错误
   10xx             Generic Error               通用错误
   20xx             Current                     电流
   21xx             Current, device input side  设备输入侧电流
   22xx             Current inside the device   设备内部电流
   23xx             Current, device output side 设备外部电流
   30xx             Voltage                     电压
   31xx             Mains Voltage               主回路电压
   32xx             Voltage inside the device   设备内部电压
   33xx             Output Voltage              输出电压
   40xx             Temperature                 温度
   41xx             Ambient Temperature         环境温度
   42xx             Device Temperature          设备温度
   50xx             Device Hardware             设备硬件
   60xx             Device Software             设备软件
   61xx             Internal Software           内部软件
   62xx             User Software               用户软件
   63xx             Data Set                    数据设置
   70xx             Additional Modules          附加模块
   80xx             Monitoring                  监控
   81xx             Communication               通信
   8110             CAN Overrun (Objects lost)  CAN 过载
   8120             CAN in Error Passive Mode   CAN 错误被动模式
   8130             Life Guard Error or Heartbeat Error     心跳监控错误
   8140             recovered from bus off      从总线关闭恢复
   8150             Transmit COB-ID collision   发送 COB-ID 冲突
   82xx             Protocol Error              协议错误
   8210             PDO not processed due to length error   因长度错误 PDO 未处理
   8220             PDO length exceeded         PDO 超长
   90xx             External Error              外部错误
   F0xx             Additional Functions        附加功能
   FFxx             Device specific             设备规定

   
   紧急错误代码 ds-401
   Emergency error codes (ds-401):
   error code       description
   -----------------------------------------------------------------------------
   2310             Current at outputs too high (overload)  输出电流过大（过载）
   2320             Short circuit at outputs    输出短路
   2330             Load dump at outputs        输出负载突降/空载
   3110             Input voltage too high      输入电压过高
   3120             Input voltage too low       输入电压过低
   3210             Internal voltage too high   内部电压过高
   3220             Internal voltage too low    内部电压过低
   3310             Output voltage too high     输出电压过高
   3320             Output voltage too low      输出电压过低

   紧急消息的组成
   Contents of Emergency message (COB-ID = 0x80 + Node-ID):
   byte no.         description
   -----------------------------------------------------------------------------
   byte 0..1        Error code      错误代码
   byte 2           Error Register (ODE_Error_Register, OD index 1001)  错误寄存器
   byte 3..7        Manufacturer specific:      制造商规定
   byte 3              ErrorControl.ErrorBit (see definitions of specific error bits)
   byte 4..5           ErrorControl.CodeVal  (code argument to ErrorReport() or ErrorReset())
   byte 6              ERROR_EMERGENCY_BYTE6 (see below)
   byte 7              ERROR_EMERGENCY_BYTE7 (see below)
   
   预定义错误区的组成
   Contents of Pre Defined Error Field (Index 1003):
   byte no.         description
   -----------------------------------------------------------------------------
   byte 0..1        Error code
   byte 3..4        Manufacturer specific:
                       ErrorControl.CodeVal  (code argument to ErrorReport() or ErrorReset())
*******************************************************************************/

#ifndef _ERRORS_H
#define _ERRORS_H

   #define ERROR_NO_OF_BYTES 4      //4...32


/***** Table of standard error codes (DS 301) *********************************/
/*     标准错误代码表  */
   //Each error code value corresponds one error bit
   // 每个错误代码对应一个错误位
   #define ERROR_CODES {                                                   \
         0x0000, 0x6100, 0x6110, 0x6120, 0x6000, 0x6300, 0     , 0     ,   \
         0x8110, 0x8140, 0x8120, 0x8120, 0x8110, 0x8100, 0x8130, 0x8100,   \
         0x1001, 0x1002, 0x1003, 0x1004, 0x1005, 0x1006, 0x1007, 0x1008,   \
         0x8210, 0x8200, 0x8200, 0x8200, 0x0000, 0x8100, 0x8100, 0         }


/***** definitions for Error Status Bits **************************************/
/*     错误状态位的定义 */

   //byte 0, important generic errors
   //字节 0，重要的通用错误
   #define ERROR_NO_ERROR                             0x00
   #define ERROR_ErrorReport_ParametersNotSupp        0x01
   #define ERROR_isr_low_WrongInterrupt               0x02
   #define ERROR_wrong_PDO_mapping                    0x03
   #define ERROR_isr_timer_overflow                   0x04
   #define ERROR_WrongNodeIDorBitRate                 0x05

   //byte 1, important communication errors
   //字节 1，重要的通信错误
   #define ERROR_CAN_RXB_OVERFLOW                     0x08
   #define ERROR_CAN_TX_BUS_OFF                       0x09
   #define ERROR_CAN_TX_BUS_PASSIVE                   0x0A
   #define ERROR_CAN_RX_BUS_PASSIVE                   0x0B
   #define ERROR_CAN_TX_OVERFLOW                      0x0C
   #define ERROR_TPDO_OUTSIDE_WINDOW                  0x0D
   #define ERROR_HEARTBEAT_CONSUMER                   0x0E
   #define ERROR_SYNC_TIME_OUT                        0x0F

   //byte 2, important or unimpotrant user defined errors
   //字节 2，重要或非重要的用户定义错误
   #define ERROR_USER_0_IMPORTANT                     0x10
   #define ERROR_USER_1_IMPORTANT                     0x11
   #define ERROR_USER_2_IMPORTANT                     0x12
   #define ERROR_USER_3_IMPORTANT                     0x13
   #define ERROR_USER_4                               0x14
   #define ERROR_USER_5                               0x15
   #define ERROR_USER_6                               0x16
   #define ERROR_USER_7                               0x17

   //byte 3, protocol errors, not important
   //字节 3，非重要的，协议错误
   #define ERROR_CO_RXMSG_PDOlength                   0x18
   #define ERROR_CO_RXMSG_Length                      0x19
   #define ERROR_CO_RXMSG_NMTcmd                      0x1A
   #define ERROR_CO_RXMSG_Inhibit                     0x1B
   #define ERROR_CAN_BUS_WARNING                      0x1C
   #define ERROR_SYNC_EARLY                           0x1D
   #define ERROR_CANRXTX_INT                          0x1E

/***** calculation of Error register 0x1001 ***********************************/
/*     错误寄存器的计算 */

   //generic error
   #define ERROR_REGISTER_BIT0_CONDITION (CO_ErrorStatusBits[0] & 0xFE)
   //current
   #define ERROR_REGISTER_BIT1_CONDITION (CO_ErrorStatusBits[0] & 0x00)
   //voltage
   #define ERROR_REGISTER_BIT2_CONDITION (CO_ErrorStatusBits[0] & 0x00)
   //temperature
   #define ERROR_REGISTER_BIT3_CONDITION (CO_ErrorStatusBits[0] & 0x00)
   //communication error
   #define ERROR_REGISTER_BIT4_CONDITION (CO_ErrorStatusBits[1])
   //device profile specific
   #define ERROR_REGISTER_BIT5_CONDITION (CO_ErrorStatusBits[0] & 0x00)
   //manufacturer specific
   #define ERROR_REGISTER_BIT7_CONDITION (CO_ErrorStatusBits[2] & 0x0F)

/***** other defines **********************************************************/
/*     其它定义 */
   extern unsigned char CO_ErrorStatusBits[];
   #define ERROR_BIT_READ(error_bit)   (CO_ErrorStatusBits[error_bit>>3] & (1<<(error_bit & 0x07)))

   //Defines for bytes 6 and 7 in emergency message
   //紧急消息中的第 6 和第 7 字节定义
   #define ERROR_EMERGENCY_BYTE6    CO_ErrorStatusBits[0]
   #define ERROR_EMERGENCY_BYTE7    CO_ErrorStatusBits[1]

/*******************************************************************************
   ErrorReport - REPORT ERROR CONDITION
   Function is used to report any error occurred in program. It can be used from
   mainline or interrupt. It sets appropriate error bit and sends emergency if
   it is the first time for that bit. If critical bits are set, node will not be
   able to enter operational state.

   ErrorReport - 报告错误状况
   函数用于报告程序中发生的任何错误。它可以从主线或中断使用。它设置适当的错误位，并在该
   位第一次被设置时发送紧急消息。如果关键位被设置，则节点将不能进入运行状态。

   PARAM ErrorBit: specific error bit, use defined constants
                   特定的错误位，使用定义的常量
   PARAM Code: informative value, send as 4-th and 5-th byte in Emergency message
               信息值，在紧急消息中作为第 4 和第 5 字节发送
*******************************************************************************/
   void ErrorReport(unsigned char ErrorBit, unsigned int Code);

/*******************************************************************************
   ErrorReset - RESET ERROR CONDITION
   Function is used to report any if error condition is no more present. It can
   be used from mainline or interrupt function. It resets appropriate error bit
   and sends emergency 'no error'. Parameters are same as above.

   ErrorReset - 复位错误状态
   函数用于报告任何错误情况是否不再存在。它可以从主线或中断功能使用。它将复位适当的错误
   位并发送『无错误』紧急消息。 参数与上面相同。
*******************************************************************************/
   void ErrorReset(unsigned char ErrorBit, unsigned int Code);

#endif
