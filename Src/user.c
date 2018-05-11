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

/*-------------------------------------------------------------*/
      // DO???
      #define	do_port_VALVE_22_23_X1	   LATDbits.LATD0	
      #define	do_port_VALVE_31_33_X1	   LATDbits.LATD1	
      #define	do_port_VALVE_30_X1	   LATDbits.LATD2	
      #define	do_port_VALVE_PDR         LATDbits.LATD3	
      #define	do_port_BOX_HEAT          LATDbits.LATD5  
      #define     do_port_BOX_FAN           LATDbits.LATD6  
      #define     do_port_PITCH_LUB         LATDbits.LATD7	
      #define	do_port_ROTOR_LUB         LATEbits.LATE1  
      #define	do_port_SYS_SC            LATEbits.LATE2  
      #define     TRIS_DO()               TRISD &= 0x10; TRISE &= 0xF9

      // DI???
      #define	di_port_PITCH_LUB_OIL     	PORTCbits.RC0	
      #define	di_port_PITCH_LUB_JAM        PORTCbits.RC1	
      #define	di_port_ROTOR_LUB_OIL     	PORTCbits.RC2	
      #define	di_port_ROTOR_LUB_JAM        PORTCbits.RC3	
      #define	di_port_MANUAL_0D_X1         PORTBbits.RB5	
      #define	di_port_MANUAL_90D_X1        PORTBbits.RB6	
      #define	di_port_MANUAL_MODE_ON       PORTBbits.RB7 	
      #define	di_port_SPD_GRP_OK           PORTCbits.RC4 	
      #define	di_port_MAIN_SC_OK           PORTBbits.RB0    
      #define     TRIS_DI()               TRISC |= 0x1F; TRISA |= 0x13; TRISB |= 0xE1


/***** Device profile for Generic I/O *****************************************/
// /*     ????¡è?¨¦?????    ?????? CO_OD.c         */
//            #ifdef CO_IO_DIGITAL_INPUTS
// /*0x6000*/ extern       tData4bytes    ODE_Read_Digital_Input;
//            #endif

//            #ifdef CO_IO_DIGITAL_OUTPUTS
// /*0x6200*/ extern       tData4bytes    ODE_Write_Digital_Output;
//            #endif

//            #ifdef CO_IO_ANALOG_INPUTS
// /*0x6401*/ extern       INTEGER16      ODE_Read_Analog_Input[];
//            #endif

//            #ifdef CO_IO_ANALOG_OUTPUTS
// /*0x6411*/ extern       INTEGER16      ODE_Write_Analog_Output[];
//            #endif
////////////////////////////////////////////////////////////////////////////////
/*0xa100*/ 
        extern     tData2bytes     ODE_out_system_state;
        extern     tData2bytes     ODE_out_system_error;
        extern     tData2bytes     ODE_out_DI_status;
        extern     tData2bytes     ODE_out_DO_status;
        extern     INTEGER16       ODE_out_actual_position[3];
        extern     INTEGER16       ODE_out_LVDT_out[3];
        extern     INTEGER16       ODE_out_actual_pressure[3];
        extern     INTEGER16       ODE_out_temperature[2];


/*0xa580*/
        extern     tData2bytes     ODE_in_control_word;
        extern     INTEGER16       ODE_in_target_position[3];
        extern     INTEGER16       ODE_in_speed[3];


/*0xa100*/ 
        #define can_out_system_state        CO_TPDO(0).WORD[0]
        #define can_out_system_error        CO_TPDO(0).WORD[1]        
        #define can_out_DI_status           CO_TPDO(0).WORD[2]
        #define can_out_DO_status           CO_TPDO(0).WORD[3]

        #define can_out_actual_position_1   CO_TPDO(1).WORD[0]
        #define can_out_actual_position_2   CO_TPDO(1).WORD[1]
        #define can_out_actual_position_3   CO_TPDO(1).WORD[2]
        #define can_out_LVDT_out_1          CO_TPDO(1).WORD[3]

        #define can_out_LVDT_out_2          CO_TPDO(2).WORD[0]
        #define can_out_LVDT_out_3          CO_TPDO(2).WORD[1]
        #define can_out_actual_pressure_1   CO_TPDO(2).WORD[2]
        #define can_out_actual_pressure_2   CO_TPDO(2).WORD[3]

        #define can_out_actual_pressure_3   CO_TPDO(3).WORD[0]
        #define can_out_cab_temperature     CO_TPDO(3).WORD[1]
        #define can_out_hub_temperature     CO_TPDO(3).WORD[2]

         #define ODE_out_a1_normal_mode             ODE_out_system_state.BYTEbits[0].bit0
         #define ODE_out_a1_speedctrl_mode          ODE_out_system_state.BYTEbits[0].bit1
         #define ODE_out_a1_ctrled_emer_stop_mode   ODE_out_system_state.BYTEbits[0].bit2
         #define ODE_out_a1_emergency_mode          ODE_out_system_state.BYTEbits[0].bit3
         #define ODE_out_a1_manual_mode             ODE_out_system_state.BYTEbits[0].bit4
         #define ODE_out_a2_normal_mode             ODE_out_system_state.BYTEbits[0].bit5
         #define ODE_out_a2_speedctrl_mode          ODE_out_system_state.BYTEbits[0].bit6
         #define ODE_out_a2_ctrled_emer_stop_mode   ODE_out_system_state.BYTEbits[0].bit7
         #define ODE_out_a2_emergency_mode          ODE_out_system_state.BYTEbits[1].bit0
         #define ODE_out_a2_manual_mode             ODE_out_system_state.BYTEbits[1].bit1
         #define ODE_out_a3_normal_mode             ODE_out_system_state.BYTEbits[1].bit2
         #define ODE_out_a3_speedctrl_mode          ODE_out_system_state.BYTEbits[1].bit3
         #define ODE_out_a3_ctrled_emer_stop_mode   ODE_out_system_state.BYTEbits[1].bit4
         #define ODE_out_a3_emergency_mode          ODE_out_system_state.BYTEbits[1].bit5
         #define ODE_out_a3_manual_mode             ODE_out_system_state.BYTEbits[1].bit6
         #define ODE_out_heartbeat_mirrored         ODE_out_system_state.BYTEbits[1].bit7 
        
         #define ODE_out_di_lub_PI_oil          ODE_out_DI_status.BYTEbits[0].bit0
         #define ODE_out_di_lub_PI_jam          ODE_out_DI_status.BYTEbits[0].bit1  
         #define ODE_out_di_lub_MB_oil          ODE_out_DI_status.BYTEbits[0].bit2
         #define ODE_out_di_lub_MB_jam          ODE_out_DI_status.BYTEbits[0].bit3
         #define ODE_out_di_a1_manual_0d        ODE_out_DI_status.BYTEbits[0].bit4
         #define ODE_out_di_a1_manual_90d       ODE_out_DI_status.BYTEbits[0].bit5
         #define ODE_out_di_a2_manual_0d        ODE_out_DI_status.BYTEbits[0].bit6
         #define ODE_out_di_a2_manual_90d       ODE_out_DI_status.BYTEbits[0].bit7
         #define ODE_out_di_a3_manual_0d        ODE_out_DI_status.BYTEbits[1].bit0
         #define ODE_out_di_a3_manual_90d       ODE_out_DI_status.BYTEbits[1].bit1
         #define ODE_out_di_manual_mode         ODE_out_DI_status.BYTEbits[1].bit2
         #define ODE_out_di_SPD_group           ODE_out_DI_status.BYTEbits[1].bit3
         #define ODE_out_di_sc_ext              ODE_out_DI_status.BYTEbits[1].bit4

         #define ODE_out_do_a1_valve_22_23      ODE_out_DO_status.BYTEbits[0].bit0
         #define ODE_out_do_a1_valve_31_33      ODE_out_DO_status.BYTEbits[0].bit1
         #define ODE_out_do_a1_valve_30         ODE_out_DO_status.BYTEbits[0].bit2	
         #define ODE_out_do_a2_valve_22_23      ODE_out_DO_status.BYTEbits[0].bit3
         #define ODE_out_do_a2_valve_31_33      ODE_out_DO_status.BYTEbits[0].bit4
         #define ODE_out_do_a2_valve_30         ODE_out_DO_status.BYTEbits[0].bit5	
         #define ODE_out_do_a3_valve_22_23      ODE_out_DO_status.BYTEbits[0].bit6
         #define ODE_out_do_a3_valve_31_33      ODE_out_DO_status.BYTEbits[0].bit7
         #define ODE_out_do_a3_valve_30         ODE_out_DO_status.BYTEbits[1].bit0	
         #define ODE_out_do_valve_PDR           ODE_out_DO_status.BYTEbits[1].bit1	
         #define ODE_out_do_cab_heater          ODE_out_DO_status.BYTEbits[1].bit2  
         #define ODE_out_do_cab_cooler          ODE_out_DO_status.BYTEbits[1].bit3 
         #define ODE_out_do_lub_PI              ODE_out_DO_status.BYTEbits[1].bit4	
         #define ODE_out_do_lub_MB              ODE_out_DO_status.BYTEbits[1].bit5 
         #define ODE_out_do_sc_int              ODE_out_DO_status.BYTEbits[1].bit6

/*0xa580*/
        #define can_in_control_word         CO_RPDO(0).WORD[0]
        #define can_in_target_position_1    CO_RPDO(0).WORD[1]
        #define can_in_target_position_2    CO_RPDO(0).WORD[2]
        #define can_in_target_position_3    CO_RPDO(0).WORD[3]

        #define can_in_speed_1              CO_RPDO(1).WORD[0]
        #define can_in_speed_2              CO_RPDO(1).WORD[1]
        #define can_in_speed_3              CO_RPDO(1).WORD[2]
         
         #define ODE_in_normal_operation     ODE_in_control_word.BYTEbits[0].bit0
         #define ODE_in_normal_speedctrl     ODE_in_control_word.BYTEbits[0].bit1
         #define ODE_in_normal_stop          ODE_in_control_word.BYTEbits[0].bit2
         #define ODE_in_ctrled_emer_stop     ODE_in_control_word.BYTEbits[0].bit3
         #define ODE_in_emergency_operation  ODE_in_control_word.BYTEbits[0].bit4
         #define ODE_in_reset_errors         ODE_in_control_word.BYTEbits[0].bit5
      //    #define ODE_in_referencing          ODE_in_control_word.BYTEbits[0].bit6
         #define ODE_in_cab_heat             ODE_in_control_word.BYTEbits[0].bit7 
         #define ODE_in_lubrication_PI       ODE_in_control_word.BYTEbits[1].bit0
         #define ODE_in_lubrication_MB       ODE_in_control_word.BYTEbits[1].bit1   
         #define ODE_in_heartbeat            ODE_in_control_word.BYTEbits[1].bit7 
////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
   SwitchOffNode - SWITCH OFF ALL NODE OUTPUTS
   Function is called after Node startup, Communication reset or after
   NMT_OPERATIONAL (this or monitored nodes) was lost.

   SwitchOffNode - ????????????????????????
   ??????????????¡§????¡è????¨¦????????NMT_OPERATIONAL???????????????????????¡ì?????????????¡è????????¡ã???¡§????????¡ã???
*******************************************************************************/
void SwitchOffNode(void){

//    #ifdef CO_IO_DIGITAL_OUTPUTS
//       //CHANGE THIS LINE -> WRITE 0 TO ALL PORTS !!!
//       // ????????¡è?????????????????????????????? 0
//       ODE_Write_Digital_Output.DWORD[0] = 0;
//    #endif

//    #ifdef CO_IO_ANALOG_OUTPUTS
//       //CHANGE THIS LINE -> WRITE 0 TO ALL PORTS !!!
//       // ????????¡è?????????????????????????????? 0      
//       ODE_Write_Analog_Output[0] = 0;
//       ODE_Write_Analog_Output[1] = 0;
//    #endif

      ODE_out_system_state.WORD[0] = 0;
      ODE_out_system_error.WORD[0] = 0;
      ODE_out_DI_status.WORD[0] = 0;
      ODE_out_DO_status.WORD[0] = 0;
      ODE_out_actual_position[0] = 0xff;
      ODE_out_actual_position[1] = 0xff;
      ODE_out_actual_position[2] = 0xff;

      ODE_out_LVDT_out[0] = 0;
      ODE_out_LVDT_out[1] = 0;
      ODE_out_LVDT_out[2] = 0;
      ODE_out_actual_pressure[0] = 0;
      ODE_out_actual_pressure[1] = 0;
      ODE_out_actual_pressure[2] = 0;
      ODE_out_temperature[0] = 0;
      ODE_out_temperature[1] = 0;

}


/*******************************************************************************
   User_Init - USER INITIALIZATION OF NODE
   Function is called after start of program.

   User_Init - ??¡§??¡¦????¡ì??????????
   ??¡§?¡§?????????¡§????¡ã???¡§
*******************************************************************************/
void User_Init(void){
   ODE_EEPROM.PowerOnCounter++;

//    #ifdef CO_IO_DIGITAL_INPUTS
//       //CHANGE THIS LINE -> set ports as digital inputs
//       // ????????¡è??????????????¡ã???¨¦??????????????
//       ODE_Read_Digital_Input.DWORD[0] = 0;
//    #endif

//    #ifdef CO_IO_ANALOG_INPUTS
//       //CHANGE THIS LINE -> set ports as analog inputs
//       // ????????¡è?????????????¡§????¨¦??????????????
//       ODE_Read_Analog_Input[0] = 0;
//       ODE_Read_Analog_Input[1] = 0;
//       ODE_Read_Analog_Input[2] = 0;
//       ODE_Read_Analog_Input[3] = 0;
//       ODE_Read_Analog_Input[4] = 0;
//       ODE_Read_Analog_Input[5] = 0;
//       ODE_Read_Analog_Input[6] = 0;
//       ODE_Read_Analog_Input[7] = 0;
//    #endif

//    #ifdef CO_IO_DIGITAL_OUTPUTS
//       //CHANGE THIS LINE -> set ports as digital outputs
//       // ????????¡è??????????????¡ã???¨¦??????????????
//    #endif

//    #ifdef CO_IO_ANALOG_OUTPUTS
//       //CHANGE THIS LINE -> set ports as analog outputs
//       // ????????¡è?????????????¡§????¨¦??????????????
//    #endif

      // DIO ??????????¡ì????
      TRIS_DI();
      TRIS_DO();

      // ????¡ì???????
      do_port_SYS_SC = 0;  
      do_port_VALVE_22_23_X1 = 0;	
      do_port_VALVE_31_33_X1 = 0;	
      do_port_VALVE_30_X1 = 0;	
      do_port_VALVE_PDR = 0;	
      do_port_BOX_HEAT = 0;  
      do_port_BOX_FAN = 0;  
      do_port_PITCH_LUB = 0;	
      do_port_ROTOR_LUB = 0; 
  
      SwitchOffNode();

}


/*******************************************************************************
   User_Remove - USER EXECUTION ON EXIT OF PROGRAM
   Function is called before end of program. Not used in PIC.

   ???PIC ???????????¡§
*******************************************************************************/
void User_Remove(void){

}


/*******************************************************************************
   User_ResetComm - USER RESET COMMUNICATION
   Function is called after start of program and after CANopen NMT command: Reset
   Communication.

   User_ResetComm - ??¡§??¡¦?¡è????¨¦?????
   ??¡§?¡§?????????¡§??????NMT ?¡è????¨¦?????????????¡è????¡ã???¡§   
*******************************************************************************/
void User_ResetComm(void){

   SwitchOffNode();

}


/*******************************************************************************
   User_ProcessMain - USER PROCESS MAINLINE
   This function is cyclycally called from main(). It is non blocking function.
   It is asynchronous. Here is longer and time consuming code. Cycle time can
   be shorter than in User_Process1msIsr.
   
   User_ProcessMain - ??¡§??¡¦?¡è??????????
   ????????¡ã??? main() ?????????????¡ã???¡§?????????¨¦??¨¦?????????????¡ã????????????????????????¨¦????????????????????????????????
   ??¡§??????????????¡§ User_Process1msIsr ????????????
*******************************************************************************/
void User_ProcessMain(void){

}


/*******************************************************************************
   User_Process1msIsr - 1 ms USER TIMER FUNCTION
   Function is executed every 1 ms. It is deterministic and has priority over
   mainline functions.
   User_Process1msIsr - 1 ms ????????¡§??¡§??¡¦??????
   ?????¡ã???1 ms ??¡ì??????????????????????????¡ì???????????¡¦??????mainline ?¡§???????¨¦?????????????¡ì???

   Function of the following code (each cycle):
    - Read from hardware
    - If Operational state:
       - If all Monitored nodes are Operational:
          - Copy all Received PDOs to their mapped location
       - Prepare all Transmit PDOs from their mapped location (and send them in case Change Of State)
    - If Operational state of this or monitored nodes is lost, node outputs are switched off.
    - Write to hardware
   ?????¡ã????????¡ì??????¨¦?????????????????????????????
    * ?????????
    * ????????¡§???????????????
       - ????????????????????¡ì????????¡§???????????????
          - ?¡ã???????????????? PDO ?¡è??????¡ã????¡ã??????¡ã???
       - ?¡ã??????????¨¦??PDO ????????????????¡ã???????????¡è?????????????¨¦??????????????????????????????????????
    * ????????????????????????????????¡ì??????????????????????¡è?????????????????????????
    * ?????????

   If TPDO is configured for Change of State transmission (Transmission_type >= 254), then consider next
   situation: On network startup one node is started first and send PDO of curent state, which will not
   change soon. Another node is started later and missed PDO from first node??? Solution might be
   Event timer.
   ????¡ã? TPDO ¨¦????????????????????????????????Transmission_type >= 254??????¨¦??????????????????????????
   ??¡§???????????¡§??????¨¦??????????¡§?????????????????????¨¦????????????????? PDO??????????????????¨¦???????????????????????
   ?????????????¡§?????????¡§??????¨¦????????????????????????????????PDO????¡ì?????????????????????????????????¡§???

   Following code can also be moved into User_ProcessMain() function.
   ???¨¦?????????????????????¡ì???¡§???User_ProcessMain() ?????¡ã??????
*******************************************************************************/
void User_Process1msIsr(void){
   static unsigned char LastStateOperationalGradePrev = 0;
   unsigned char LastStateOperationalGrade = 0;
   extern volatile unsigned int CO_TPDO_InhibitTimer[CO_NO_TPDO];  //Inhibit timer used for inhibit PDO sending - if 0, TPDO is not inhibited


   // Read from Hardware -------------------------------------------------------
   // ?????????
   //CHANGE THIS LINE -> ODE_Read_Digital_Input.BYTE[0] = port_xxx
   //CHANGE THIS LINE -> ODE_Read_Digital_Input.BYTE[1] = port_xxx
   //CHANGE THIS LINE -> ODE_Read_Digital_Input.BYTE[2] = port_xxx
   //CHANGE THIS LINE -> ODE_Read_Digital_Input.BYTE[3] = port_xxx
   //CHANGE THIS LINE -> ODE_Read_Analog_Input[0...7] = ...
   ODE_out_di_lub_PI_oil = di_port_PITCH_LUB_OIL;	
   ODE_out_di_lub_PI_jam = di_port_PITCH_LUB_JAM;  	
   ODE_out_di_lub_MB_oil = di_port_ROTOR_LUB_OIL;	
   ODE_out_di_lub_MB_jam = di_port_ROTOR_LUB_JAM; 	
   ODE_out_di_a1_manual_0d = di_port_MANUAL_0D_X1;   	
   ODE_out_di_a1_manual_90d = di_port_MANUAL_90D_X1; 	
   ODE_out_di_manual_mode = di_port_MANUAL_MODE_ON; 	
   ODE_out_di_SPD_group = di_port_SPD_GRP_OK;      	
   ODE_out_di_sc_ext = di_port_MAIN_SC_OK;       

   //PDO Communication
   // PDO ¨¦?????
   if(CO_NMToperatingState == NMT_OPERATIONAL){

      LastStateOperationalGrade ++;

      //verify operating state of monitored nodes
      // ?????¡è????????¡ì?????????????????????
      #if CO_NO_CONS_HEARTBEAT > 0
      if(CO_HBcons_AllMonitoredOperational == NMT_OPERATIONAL){
      #endif

         LastStateOperationalGrade ++;


      //    //Read RPDOs -------------------------------------------------------------
      //    // ?????????PDO
      //    //Following code realizes Static RPDO Mapping
      //    // ???¨¦????????????????¡ã????????? PDO ¨¦?????????¡ã?
      //    #if CO_NO_RPDO > 0
      //       #ifdef CO_IO_DIGITAL_OUTPUTS
      //          if(CO_RPDO_New(0)){
      //             CO_DISABLE_CANRX_TMR();
      //             ODE_Write_Digital_Output.DWORD[0] = CO_RPDO(0).DWORD[0];
      //             CO_RPDO_New(0) = 0;
      //             CO_ENABLE_CANRX_TMR();
      //          }
      //       #endif
      //    #endif
      //    #if CO_NO_RPDO > 1
      //       #ifdef CO_IO_ANALOG_OUTPUTS
      //          if(CO_RPDO_New(1)){
      //             CO_DISABLE_CANRX_TMR();
      //             ODE_Write_Analog_Output[0] = CO_RPDO(1).WORD[0];
      //             ODE_Write_Analog_Output[1] = CO_RPDO(1).WORD[1];
      //             CO_RPDO_New(1) = 0;
      //             CO_ENABLE_CANRX_TMR();
      //          }
      //       #endif
      //    #endif
            if(CO_RPDO_New(0)){
                  CO_DISABLE_CANRX_TMR();

                  ODE_in_control_word.WORD[0]   = can_in_control_word;
                  ODE_in_target_position[0]     = can_in_target_position_1;
                  ODE_in_target_position[1]     = can_in_target_position_2;
                  ODE_in_target_position[2]     = can_in_target_position_3;

                  CO_RPDO_New(0) = 0;
                  CO_ENABLE_CANRX_TMR();
            }

            if(CO_RPDO_New(1)){
                  CO_DISABLE_CANRX_TMR();

                  ODE_in_speed[0]   = can_in_speed_1;
                  ODE_in_speed[1]   = can_in_speed_2;
                  ODE_in_speed[2]   = can_in_speed_3;

                  CO_RPDO_New(1) = 0;
                  CO_ENABLE_CANRX_TMR();
            }

      #if CO_NO_CONS_HEARTBEAT > 0
      }// end if(CO_HBcons_AllMonitoredOperational == NMT_OPERATIONAL)
      #endif

      // //Write TPDOs ------------------------------------------------------------
      // //Following code realizes Static TPDO Mapping
      // //??????¨¦??PDO
      // //???¨¦????????????????¡ã??????¨¦??PDO ???¨¦?????????¡ã?
      // //Transmission is Synchronous or Change of State, depends on Transmission_type.
      // //Inhibit timer and Periodic Event Timer can be used.
      // //???¨¦??????????????????????????????????????¨¦??????????????Transmission_type.
      // //???????????¡§?????????¨¦???????¡§?????¡ì??????????????????
      // #if CO_NO_TPDO > 0
      //    #ifdef CO_IO_DIGITAL_INPUTS
      //       if(CO_TPDO_InhibitTimer[0] == 0 &&
      //          CO_TPDO(0).DWORD[0] != ODE_Read_Digital_Input.DWORD[0]){
      
      //          CO_TPDO(0).DWORD[0] = ODE_Read_Digital_Input.DWORD[0];
      //          if(ODE_TPDO_Parameter[0].Transmission_type >= 254)
      //             CO_TPDOsend(0);
      //       }
      //    #endif
      // #endif
      // #if CO_NO_TPDO > 1
      //    #ifdef CO_IO_ANALOG_INPUTS
      //       if(CO_TPDO_InhibitTimer[1] == 0 && (
      //          CO_TPDO(1).WORD[0] != ODE_Read_Analog_Input[0] ||
      //          CO_TPDO(1).WORD[1] != ODE_Read_Analog_Input[1] ||
      //          CO_TPDO(1).WORD[2] != ODE_Read_Analog_Input[2] ||
      //          CO_TPDO(1).WORD[3] != ODE_Read_Analog_Input[3])){
   
      //          CO_TPDO(1).WORD[0] = ODE_Read_Analog_Input[0];
      //          CO_TPDO(1).WORD[1] = ODE_Read_Analog_Input[1];
      //          CO_TPDO(1).WORD[2] = ODE_Read_Analog_Input[2];
      //          CO_TPDO(1).WORD[3] = ODE_Read_Analog_Input[3];
      //          if(ODE_TPDO_Parameter[1].Transmission_type >= 254)
      //             CO_TPDOsend(1);
      //       }
      //    #endif
      // #endif
      // #if CO_NO_TPDO > 2
      //    #ifdef CO_IO_ANALOG_INPUTS
      //       if(CO_TPDO_InhibitTimer[2] == 0 && (
      //          CO_TPDO(2).WORD[0] != ODE_Read_Analog_Input[4] ||
      //          CO_TPDO(2).WORD[1] != ODE_Read_Analog_Input[5] ||
      //          CO_TPDO(2).WORD[2] != ODE_Read_Analog_Input[6] ||
      //          CO_TPDO(2).WORD[3] != ODE_Read_Analog_Input[7])){
   
      //          CO_TPDO(2).WORD[0] = ODE_Read_Analog_Input[4];
      //          CO_TPDO(2).WORD[1] = ODE_Read_Analog_Input[5];
      //          CO_TPDO(2).WORD[2] = ODE_Read_Analog_Input[6];
      //          CO_TPDO(2).WORD[3] = ODE_Read_Analog_Input[7];
      //          if(ODE_TPDO_Parameter[2].Transmission_type >= 254)
      //             CO_TPDOsend(2);
      //       }
      //    #endif
      // #endif

      if(CO_TPDO_InhibitTimer[0] == 0 && (
         can_out_system_state != ODE_out_system_state.WORD[0] ||
         can_out_system_error != ODE_out_system_error.WORD[0] ||
         can_out_DI_status    != ODE_out_DI_status.WORD[0]    ||
         can_out_DO_status    != ODE_out_DO_status.WORD[0])){

            can_out_system_state = ODE_out_system_state.WORD[0];
            can_out_system_error = ODE_out_system_error.WORD[0];
            can_out_DI_status    = ODE_out_DI_status.WORD[0];
            can_out_DO_status    = ODE_out_DO_status.WORD[0];

            if(ODE_TPDO_Parameter[0].Transmission_type >= 254)
                  CO_TPDOsend(0);
      }

      if(CO_TPDO_InhibitTimer[1] == 0 && (
         can_out_actual_position_1 != ODE_out_actual_position[0] ||
         can_out_actual_position_2 != ODE_out_actual_position[1] ||
         can_out_actual_position_3 != ODE_out_actual_position[2] ||
         can_out_LVDT_out_1        != ODE_out_LVDT_out[0])){

            can_out_actual_position_1 = ODE_out_actual_position[0];
            can_out_actual_position_2 = ODE_out_actual_position[1];
            can_out_actual_position_3 = ODE_out_actual_position[2];
            can_out_LVDT_out_1        = ODE_out_LVDT_out[0];

            if(ODE_TPDO_Parameter[1].Transmission_type >= 254)
                  CO_TPDOsend(1);
      }

      if(CO_TPDO_InhibitTimer[2] == 0 && (
         can_out_LVDT_out_2        != ODE_out_LVDT_out[1]        ||
         can_out_LVDT_out_3        != ODE_out_LVDT_out[2]        ||
         can_out_actual_pressure_1 != ODE_out_actual_pressure[0] ||
         can_out_actual_pressure_2 != ODE_out_actual_pressure[1] )){

            can_out_LVDT_out_2        = ODE_out_LVDT_out[1];       
            can_out_LVDT_out_3        = ODE_out_LVDT_out[2];       
            can_out_actual_pressure_1 = ODE_out_actual_pressure[0];
            can_out_actual_pressure_2 = ODE_out_actual_pressure[1];

            if(ODE_TPDO_Parameter[2].Transmission_type >= 254)
                  CO_TPDOsend(2);
      }

      if(CO_TPDO_InhibitTimer[3] == 0 && (
         can_out_actual_pressure_3 != ODE_out_actual_pressure[2] ||
         can_out_cab_temperature   != ODE_out_temperature[0]     ||
         can_out_hub_temperature   != ODE_out_temperature[1]    )){

            can_out_actual_pressure_3 = ODE_out_actual_pressure[2];
            can_out_cab_temperature   = ODE_out_temperature[0]    ;
            can_out_hub_temperature   = ODE_out_temperature[1]    ;

            if(ODE_TPDO_Parameter[3].Transmission_type >= 254)
                  CO_TPDOsend(3);
      }


   } //end if(CO_NMToperatingState == NMT_OPERATIONAL)

   
   // ???????????????????????¡ì?????????NMT_OPERATIONAL ????¡è???????????????????
   if(LastStateOperationalGrade < LastStateOperationalGradePrev){ //NMT_OPERATIONAL (this or monitored nodes) was just lost
      SwitchOffNode();
   }
   LastStateOperationalGradePrev = LastStateOperationalGrade;


   // Write to Hardware --------------------------------------------------------
   // ?????????
   //CHANGE THIS LINE -> port_xxx = ODE_Write_Digital_Output.BYTE[0];
   //CHANGE THIS LINE -> port_xxx = ODE_Write_Digital_Output.BYTE[1];
   //CHANGE THIS LINE -> port_xxx = ODE_Write_Digital_Output.BYTE[2];
   //CHANGE THIS LINE -> port_xxx = ODE_Write_Digital_Output.BYTE[3];
   //CHANGE THIS LINE -> AN_OUT_port_xxx = ODE_Write_Analog_Output[0];
   //CHANGE THIS LINE -> AN_OUT_port_xxx = ODE_Write_Analog_Output[1];
//    do_port_BOX_HEAT = (ODE_in_cab_heat) ? 1 : 0 ;
//   do_port_PITCH_LUB = (ODE_in_lubrication_PI) ? 1 : 0 ;
//    do_port_ROTOR_LUB = (ODE_in_lubrication_MB) ? 1 : 0 ;


}

