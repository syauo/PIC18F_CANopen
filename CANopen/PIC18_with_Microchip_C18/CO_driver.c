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
   Variables and functions
*******************************************************************************/
   //Variables used in inline assembly
   #pragma udata access CO_udata_Assembly
      //variables used in CO_CANrxIsr inline assembly
      volatile near CO_DEFAULT_TYPE CO_IsrHighIndex;        //index of matched CANid
      volatile near CO_DEFAULT_TYPE CO_IsrHighErrorIndex;   //IsrHigh writes index of received message with wrong length
      volatile near unsigned char CO_IsrHighSignal;         //temp variable and IsrHigh signals to ProcessMain:
                                                               //0xFF: Error in length of received message, index is stored
                                                               //0xFE: Error - message arrives, but is inhibited
   #pragma udata

   //Node-ID
   unsigned char CO_NodeID;                                 //Variable must be intialized here
   unsigned char CO_BitRate;                                //Variable must be intialized here

   //eeprom
   #ifdef CO_SAVE_EEPROM
      extern unsigned char* CO_EEPROM_pointer;
      extern ROM unsigned int CO_EEPROM_size;
   #endif

   //function for verifying values at write to Object Dictionary
   unsigned long CO_OD_VerifyWrite(ROM CO_objectDictionaryEntry* pODE, void* data);

   //other
   extern volatile CO_DEFAULT_TYPE CO_TXCANcount;
   #if CO_NO_SYNC > 0
      extern volatile unsigned int CO_SYNCwindow;
   #endif

/*******************************************************************************
   CAN bit rates - Registers setup
*******************************************************************************/
ROM struct{
   unsigned char BRP;         //(1...64)Baud Rate Prescaler
   unsigned char SJW;         //(1...4) SJW time
   unsigned char PROP;        //(1...8) PROP time
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
*******************************************************************************/
void CO_DriverInit(void){
   CO_DEFAULT_SPEC unsigned int i;

   //Init Printed Board
   PCB_RUN_LED_INIT();
   PCB_ERROR_LED_INIT();

   #ifdef CO_SAVE_EEPROM
      //Initialize Eeprom variables. Variables are copied from EEPROM to RAM.
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
*******************************************************************************/
void CO_ProcessDriver(void){
   static unsigned char COMSTATold = 0;
   static unsigned int EEPROM_index = 0;

   /* Verify CAN Errors *****************/
   if(COMSTAT != COMSTATold){
      if(COMSTAT & 0xC0){
         ErrorReport(ERROR_CAN_RXB_OVERFLOW, COMSTAT);
         COMSTAT &= 0x3F;
      }

      if(COMSTATbits.TXBO)
         ErrorReport(ERROR_CAN_TX_BUS_OFF, COMSTAT);
      else if(ERROR_BIT_READ(ERROR_CAN_TX_BUS_OFF))
         ErrorReset(ERROR_CAN_TX_BUS_OFF, COMSTAT);

      if(COMSTATbits.TXBP)
         ErrorReport(ERROR_CAN_TX_BUS_PASSIVE, COMSTAT);
      else if(ERROR_BIT_READ(ERROR_CAN_TX_BUS_PASSIVE)){
         ErrorReset(ERROR_CAN_TX_BUS_PASSIVE, COMSTAT);
         ErrorReset(ERROR_CAN_TX_OVERFLOW, 0);
         }

      if(COMSTATbits.RXBP)
         ErrorReport(ERROR_CAN_RX_BUS_PASSIVE, COMSTAT);
      else if(ERROR_BIT_READ(ERROR_CAN_RX_BUS_PASSIVE))
         ErrorReset(ERROR_CAN_RX_BUS_PASSIVE, COMSTAT);

      if(COMSTAT & 0x07)
         ErrorReport(ERROR_CAN_BUS_WARNING, COMSTAT);
      else if(ERROR_BIT_READ(ERROR_CAN_BUS_WARNING))
         ErrorReset(ERROR_CAN_BUS_WARNING, COMSTAT);

      COMSTATold = COMSTAT;
   }

   //verify errors from IsrHigh
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
   #ifdef CO_SAVE_EEPROM
      //update one byte at a cycle
      if(EECON1bits.WR == 0){ //write is not in progress
         if(EEPROM_index == 0) EEPROM_index = CO_EEPROM_size;
         EEPROM_index--;
         //read eeprom
         EEADR = EEPROM_index;
         EECON1bits.EEPGD = 0;
         EECON1bits.CFGS = 0;
         EECON1bits.RD = 1;
         if(EEDATA != *(CO_EEPROM_pointer + EEPROM_index)){
            //write to EEPROM
            EEDATA = *(CO_EEPROM_pointer + EEPROM_index);
            EECON1bits.WREN = 1;
            CO_DISABLE_ALL();
            EECON2 = 0x55;
            EECON2 = 0xAA;
            EECON1bits.WR = 1;
            CO_ENABLE_ALL();
            EECON1bits.WREN = 0;
         }
      }
   #endif
}

/*******************************************************************************
   CO_Read_NodeId_BitRate - READ NODE-ID AND CAN BITRATE
   This is mainline function and is called from Communication reset. Usually
   NodeID and BitRate are read from DIP switches.
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
*******************************************************************************/
void CO_SetupCAN(void){

/* Setup driver variables */
   CO_IsrHighSignal = 0;

/* Setup CAN bus */
   CANCON = 0x80;       //request configuration mode
   while((CANSTAT & 0xE0) != 0x80); //wait until configuration mode is set

   BRGCON1 = ((CO_BitRateData[CO_BitRate].SJW-1) <<6 ) | (CO_BitRateData[CO_BitRate].BRP-1);
   BRGCON2 = 0x80 | ((CO_BitRateData[CO_BitRate].PhSeg1-1)<<3) | (CO_BitRateData[CO_BitRate].PROP-1);
   BRGCON3 = (CO_BitRateData[CO_BitRate].PhSeg2-1);

   SET_CAN_TRIS();

   RXB0CONbits.RXFUL = 0;  //clear rx and tx buffers
   RXB1CONbits.RXFUL = 0;
   TXB0CONbits.TXREQ = 0;
   TXB1CONbits.TXREQ = 0;
   TXB2CONbits.TXREQ = 0;

   PIE3 = 0;            //CAN interrupts disable
   PIR3 = 0;            //CAN interrupts flags clear
   IPR3 = 0;            //CAN interrupts low priority set
   CIOCON = 0x20;       //can I/O control register

   RXB0CON = 0x24;      //receive valid messages with standard identifier, if overflow, move to buffer1
   RXB1CON = 0x20;      //receive valid messages with standard identifier

   //set filters
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
   while((CANSTAT & 0xE0) != 0x00); //wait until normal mode is set

   PIE3 = 0x07;         //enable two receive interrupts and TX0 interrupt
   IPR3 = 0x03;         //receive interrupts are high priority
}

 /*******************************************************************************
   CO_CANrxIsr - CAN RECEIVE INTERRUPT
   This function is invoked by interrupt, when new can message receive from CAN
   network. It searches CO_RXCAN[] array and compares COB-ID. If found, copy data
   to appropriate buffer.
*******************************************************************************/
#pragma interrupt CO_CANrxIsr save = CANCON
void CO_CANrxIsr(void){
   if(PIR3bits.RXB0IF){ //RXB0 occurred
      PIR3bits.RXB0IF = 0;
      CANCONbits.WIN2 = 1; CANCONbits.WIN1 = 1; CANCONbits.WIN0 = 0;//set RXB0 to access memory
   }
   else{ //RXB1 occurred
      PIR3bits.RXB1IF = 0;
      CANCONbits.WIN2 = 1; CANCONbits.WIN1 = 0; CANCONbits.WIN0 = 1;//set RXB1 to access memory
   }

   /* Equivalent code in C:
   //search entry in CO_RXCAN[] array with matched COB_ID
   for(CO_IsrHighIndex=0; CO_IsrHighIndex<CO_RXCAN_NO_MSGS; CO_IsrHighIndex++)
      if(CO_RXCAN[CO_IsrHighIndex].Ident.BYTE[0] == RXB0SIDL &&
         CO_RXCAN[CO_IsrHighIndex].Ident.BYTE[1] == RXB0SIDH) break;
   //Verify, if message is matched
   if(CO_IsrHighIndex < CO_RXCAN_NO_MSGS){
      //Verify length
      if((CO_RXCAN[CO_IsrHighIndex].NoOfBytes <= 8) &&
         (CO_RXCAN[CO_IsrHighIndex].NoOfBytes != (RXB0DLC&0x0F))){
         CO_IsrHighSignal = 0xFF;
         CO_IsrHighErrorIndex = CO_IsrHighIndex;
      }
      //Verify inhibit
      else if(CO_RXCAN[CO_IsrHighIndex].NewMsg &&
              CO_RXCAN[CO_IsrHighIndex].Inhibit){
         CO_IsrHighSignal = 0xFE;
         CO_IsrHighErrorIndex = CO_IsrHighIndex;
      }
      else{
         //Set flag and copy data
         CO_RXCAN[CO_IsrHighIndex].NewMsg = 1;
         memcpy((void*)&CO_RXCAN[CO_IsrHighIndex].Data, (void*)&RXB0D0, RXB0DLC&0x0F);
      }
   }
   //release receive buffer
   RXB0CONbits.RXFUL = 0;
   */

   //CO_CanMessage structure must not be changed
   _asm
      //search entry in CO_RXCAN[] array with matched COB_ID
         LFSR     0, CO_RXCAN
         CLRF     CO_IsrHighIndex, 0
      CO_IH_1:
         //compare
         MOVF     POSTINC0, 0, ACCESS
         SUBWF    RXB0SIDL, 0, ACCESS
         BNZ      CO_IH_2

         MOVF     INDF0, 0, ACCESS
         SUBWF    RXB0SIDH, 0, ACCESS
         BNZ      CO_IH_2

         BRA      CO_IH_3
      CO_IH_2:
         //increment indexes for next entry
         MOVLW    0x0A
         ADDWF    FSR0L, 1, ACCESS
         MOVLW    0
         ADDWFC   FSR0H, 1, ACCESS
         INCF     CO_IsrHighIndex, 1, ACCESS
         //end of array?
         MOVLW    CO_RXCAN_NO_MSGS
         SUBWF    CO_IsrHighIndex, 0, ACCESS
         BNZ      CO_IH_1
      CO_IH_3:
      //Verify, if message is matched
         MOVLW    CO_RXCAN_NO_MSGS
         SUBWF    CO_IsrHighIndex, 0, ACCESS
         BNC      CO_IH_10
         BRA      CO_IH_END
      CO_IH_10:
      //Verify length
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

   PARAM pODE: pointer to object dictionary entry, found by CO_FindEntryInOD().
   PARAM pBuff: pointer to buffer where data will be written.
   PARAM BuffMaxSize: Size of buffer (to prevent overflow).
   RETURN: 0 if success, otherwise abort code
*******************************************************************************/
unsigned long CO_OD_Read(ROM CO_objectDictionaryEntry* pODE, void* pBuff, unsigned char sizeOfBuff){
   if(pODE->length > sizeOfBuff) return 0x05040005L;  //Out of memory
   if((pODE->attribute&0x07) == ATTR_WO) return 0x06010001L;  //attempt to read a write-only object

   //read data from memory (processor specific code)
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
   return 0;
}

/*******************************************************************************
   CO_OD_Write - OBJECT DICTIONARY WRITE
   This is mainline function. It is called from SDO server and writes data to
   object dictionary. Since object dictionary variables can use many types of
   memory, some code is processor specific.

   PARAM pODE: pointer to object dictionary entry, found by CO_FindEntryInOD().
   PARAM pBuff: pointer to buffer which contains data to be written.
   PARAM dataSize: Size of data in buffer.
   RETURN: 0 if success, otherwise abort code
*******************************************************************************/
unsigned long CO_OD_Write(ROM CO_objectDictionaryEntry* pODE, void* data, unsigned char length){
   #ifdef CO_VERIFY_OD_WRITE
      unsigned long AbortCode;
   #endif
   if((pODE->attribute&0x07) == ATTR_RO || (pODE->attribute&0x07) == ATTR_CO) return 0x06010002L;   //attempt to write a read-only object
   if(length != pODE->length) return 0x06070010L;   //Length of service parameter does not match
   #ifdef CO_VERIFY_OD_WRITE
      //verify value
      AbortCode = CO_OD_VerifyWrite(pODE, data);
      if(AbortCode) return AbortCode;
   #endif

   //write data to memory (processor specific code)
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
         memcpyram2flash((rom void*)pODE->pData, data, length);
         CO_ENABLE_ALL();
      #else
         return 0x06010002L;     //Attempt to write a read only object
      #endif
   }
   else return 0x06040047L;  //General internal incompatibility in the device
   return 0;
}


