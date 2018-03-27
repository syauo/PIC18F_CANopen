/*******************************************************************************

   User functions for Generic Input/Output Example device profile

   Digital/Anlaog Inputs/Outputs are received asynchronicaly/transmitted on
   Change of State.
   File is Device independent -> connection with hardware ports must be made ->
   follow '//CHANGE THIS LINE -> ' comments.
   Analog values are 16 bit. If resolutoin of ADC/DAC is smaller, least
   significant bits are zero.

*******************************************************************************/

#include "CANopen.h"


/***** Device profile for Generic I/O *****************************************/
           #ifdef CO_IO_DIGITAL_INPUTS
/*0x6000*/ extern       tData4bytes    ODE_Read_Digital_Input;
           #endif

           #ifdef CO_IO_DIGITAL_OUTPUTS
/*0x6200*/ extern       tData4bytes    ODE_Write_Digital_Output;
           #endif

           #ifdef CO_IO_ANALOG_INPUTS
/*0x6401*/ extern       INTEGER16      ODE_Read_Analog_Input[];
           #endif

           #ifdef CO_IO_ANALOG_OUTPUTS
/*0x6411*/ extern       INTEGER16      ODE_Write_Analog_Output[];
           #endif


/*******************************************************************************
   SwitchOffNode - SWITCH OFF ALL NODE OUTPUTS

   Function is called after Node startup, Communication reset or after
   NMT_OPERATIONAL (this or monitored nodes) was lost.
*******************************************************************************/
void SwitchOffNode(void){

   #ifdef CO_IO_DIGITAL_OUTPUTS
      //CHANGE THIS LINE -> WRITE 0 TO ALL PORTS !!!
      ODE_Write_Digital_Output.DWORD[0] = 0;
   #endif

   #ifdef CO_IO_ANALOG_OUTPUTS
      //CHANGE THIS LINE -> WRITE 0 TO ALL PORTS !!!
      ODE_Write_Analog_Output[0] = 0;
      ODE_Write_Analog_Output[1] = 0;
   #endif

}


/*******************************************************************************
   User_Init - USER INITIALIZATION OF NODE
   Function is called after start of program.
*******************************************************************************/
void User_Init(void){
   ODE_EEPROM.PowerOnCounter++;

   #ifdef CO_IO_DIGITAL_INPUTS
      //CHANGE THIS LINE -> set ports as digital inputs
      ODE_Read_Digital_Input.DWORD[0] = 0;
   #endif

   #ifdef CO_IO_ANALOG_INPUTS
      //CHANGE THIS LINE -> set ports as analog inputs
      ODE_Read_Analog_Input[0] = 0;
      ODE_Read_Analog_Input[1] = 0;
      ODE_Read_Analog_Input[2] = 0;
      ODE_Read_Analog_Input[3] = 0;
      ODE_Read_Analog_Input[4] = 0;
      ODE_Read_Analog_Input[5] = 0;
      ODE_Read_Analog_Input[6] = 0;
      ODE_Read_Analog_Input[7] = 0;
   #endif

   #ifdef CO_IO_DIGITAL_OUTPUTS
      //CHANGE THIS LINE -> set ports as digital outputs
   #endif

   #ifdef CO_IO_ANALOG_OUTPUTS
      //CHANGE THIS LINE -> set ports as analog outputs
   #endif

   SwitchOffNode();

}


/*******************************************************************************
   User_Remove - USER EXECUTION ON EXIT OF PROGRAM
   Function is called before end of program. Not used in PIC.
*******************************************************************************/
void User_Remove(void){

}


/*******************************************************************************
   User_ResetComm - USER RESET COMMUNICATION
   Function is called after start of program and after CANopen NMT command: Reset
   Communication.
*******************************************************************************/
void User_ResetComm(void){

   SwitchOffNode();

}


/*******************************************************************************
   User_ProcessMain - USER PROCESS MAINLINE
   This function is cyclycally called from main(). It is non blocking function.
   It is asynchronous. Here is longer and time consuming code. Cycle time can
   be shorter than in User_Process1msIsr.
*******************************************************************************/
void User_ProcessMain(void){

}


/*******************************************************************************
   User_Process1msIsr - 1 ms USER TIMER FUNCTION
   Function is executed every 1 ms. It is deterministic and has priority over
   mainline functions.

   Function of the following code (each cycle):
    - Read from hardware
    - If Operational state:
       - If all Monitored nodes are Operational:
          - Copy all Received PDOs to their mapped location
       - Prepare all Transmit PDOs from their mapped location (and send them in case Change Of State)
    - If Operational state of this or monitored nodes is lost, node outputs are switched off.
    - Write to hardware

   If TPDO is configured for Change of State transmission (Transmission_type >= 254), then consider next
   situation: On network startup one node is started first and send PDO of curent state, which will not
   change soon. Another node is started later and missed PDO from first node??? Solution might be
   Event timer.

   Following code can also be moved into User_ProcessMain() function.
*******************************************************************************/
void User_Process1msIsr(void){
   static unsigned char LastStateOperationalGradePrev = 0;
   unsigned char LastStateOperationalGrade = 0;
   extern volatile unsigned int CO_TPDO_InhibitTimer[CO_NO_TPDO];  //Inhibit timer used for inhibit PDO sending - if 0, TPDO is not inhibited


   // Read from Hardware -------------------------------------------------------
   //CHANGE THIS LINE -> ODE_Read_Digital_Input.BYTE[0] = port_xxx
   //CHANGE THIS LINE -> ODE_Read_Digital_Input.BYTE[1] = port_xxx
   //CHANGE THIS LINE -> ODE_Read_Digital_Input.BYTE[2] = port_xxx
   //CHANGE THIS LINE -> ODE_Read_Digital_Input.BYTE[3] = port_xxx
   //CHANGE THIS LINE -> ODE_Read_Analog_Input[0...7] = ...


   //PDO Communication
   if(CO_NMToperatingState == NMT_OPERATIONAL){

      LastStateOperationalGrade ++;

      //verify operating state of monitored nodes
      #if CO_NO_CONS_HEARTBEAT > 0
      if(CO_HBcons_AllMonitoredOperational == NMT_OPERATIONAL){
      #endif

         LastStateOperationalGrade ++;


         //Read RPDOs -------------------------------------------------------------
         //Following code realizes Static RPDO Mapping
         #if CO_NO_RPDO > 0
            #ifdef CO_IO_DIGITAL_OUTPUTS
               if(CO_RPDO_New(0)){
                  CO_DISABLE_CANRX_TMR();
                  ODE_Write_Digital_Output.DWORD[0] = CO_RPDO(0).DWORD[0];
                  CO_RPDO_New(0) = 0;
                  CO_ENABLE_CANRX_TMR();
               }
            #endif
         #endif
         #if CO_NO_RPDO > 1
            #ifdef CO_IO_ANALOG_OUTPUTS
               if(CO_RPDO_New(1)){
                  CO_DISABLE_CANRX_TMR();
                  ODE_Write_Analog_Output[0] = CO_RPDO(1).WORD[0];
                  ODE_Write_Analog_Output[1] = CO_RPDO(1).WORD[1];
                  CO_RPDO_New(1) = 0;
                  CO_ENABLE_CANRX_TMR();
               }
            #endif
         #endif


      #if CO_NO_CONS_HEARTBEAT > 0
      }// end if(CO_HBcons_AllMonitoredOperational == NMT_OPERATIONAL)
      #endif


      //Write TPDOs ------------------------------------------------------------
      //Following code realizes Static TPDO Mapping
      //Transmission is Synchronous or Change of State, depends on Transmission_type.
      //Inhibit timer and Periodic Event Timer can be used.
      #if CO_NO_TPDO > 0
         #ifdef CO_IO_DIGITAL_INPUTS
            if(CO_TPDO_InhibitTimer[0] == 0 &&
               CO_TPDO(0).DWORD[0] != ODE_Read_Digital_Input.DWORD[0]){
      
               CO_TPDO(0).DWORD[0] = ODE_Read_Digital_Input.DWORD[0];
               if(ODE_TPDO_Parameter[0].Transmission_type >= 254)
                  CO_TPDOsend(0);
            }
         #endif
      #endif
      #if CO_NO_TPDO > 1
         #ifdef CO_IO_ANALOG_INPUTS
            if(CO_TPDO_InhibitTimer[1] == 0 && (
               CO_TPDO(1).WORD[0] != ODE_Read_Analog_Input[0] ||
               CO_TPDO(1).WORD[1] != ODE_Read_Analog_Input[1] ||
               CO_TPDO(1).WORD[2] != ODE_Read_Analog_Input[2] ||
               CO_TPDO(1).WORD[3] != ODE_Read_Analog_Input[3])){
   
               CO_TPDO(1).WORD[0] = ODE_Read_Analog_Input[0];
               CO_TPDO(1).WORD[1] = ODE_Read_Analog_Input[1];
               CO_TPDO(1).WORD[2] = ODE_Read_Analog_Input[2];
               CO_TPDO(1).WORD[3] = ODE_Read_Analog_Input[3];
               if(ODE_TPDO_Parameter[1].Transmission_type >= 254)
                  CO_TPDOsend(1);
            }
         #endif
      #endif
      #if CO_NO_TPDO > 2
         #ifdef CO_IO_ANALOG_INPUTS
            if(CO_TPDO_InhibitTimer[2] == 0 && (
               CO_TPDO(2).WORD[0] != ODE_Read_Analog_Input[4] ||
               CO_TPDO(2).WORD[1] != ODE_Read_Analog_Input[5] ||
               CO_TPDO(2).WORD[2] != ODE_Read_Analog_Input[6] ||
               CO_TPDO(2).WORD[3] != ODE_Read_Analog_Input[7])){
   
               CO_TPDO(2).WORD[0] = ODE_Read_Analog_Input[4];
               CO_TPDO(2).WORD[1] = ODE_Read_Analog_Input[5];
               CO_TPDO(2).WORD[2] = ODE_Read_Analog_Input[6];
               CO_TPDO(2).WORD[3] = ODE_Read_Analog_Input[7];
               if(ODE_TPDO_Parameter[2].Transmission_type >= 254)
                  CO_TPDOsend(2);
            }
         #endif
      #endif


   } //end if(CO_NMToperatingState == NMT_OPERATIONAL)


   if(LastStateOperationalGrade < LastStateOperationalGradePrev){ //NMT_OPERATIONAL (this or monitored nodes) was just lost
      SwitchOffNode();
   }
   LastStateOperationalGradePrev = LastStateOperationalGrade;


   // Write to Hardware --------------------------------------------------------
   //CHANGE THIS LINE -> port_xxx = ODE_Write_Digital_Output.BYTE[0];
   //CHANGE THIS LINE -> port_xxx = ODE_Write_Digital_Output.BYTE[1];
   //CHANGE THIS LINE -> port_xxx = ODE_Write_Digital_Output.BYTE[2];
   //CHANGE THIS LINE -> port_xxx = ODE_Write_Digital_Output.BYTE[3];
   //CHANGE THIS LINE -> AN_OUT_port_xxx = ODE_Write_Analog_Output[0];
   //CHANGE THIS LINE -> AN_OUT_port_xxx = ODE_Write_Analog_Output[1];

}

