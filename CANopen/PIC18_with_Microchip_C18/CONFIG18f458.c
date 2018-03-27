//-------------------------------------------------------------------------------
//
// User functions
//
//-------------------------------------------------------------------------------
// NIC Sosef B.V.
//
// Sourcefile   : CONFIG18f458.c
//
// Datum aanvang: 30-08-2005
// Versie       : 1.02
// Coder        : Dennis Kroonen
//-------------------------------------------------------------------------------
#ifndef __CONFIG__
   #define __CONFIG__

   //----------------------------------------------------------------------
   // Definitie configuratie bits
   //----------------------------------------------------------------------
   #pragma config OSC  = HSPLL// Oscillator Selection
   #pragma config OSCS = OFF   // Osc. Switch Enable
   #pragma config PWRT = ON   // Power Up Timer
   #pragma config BOR  = ON   // Brown Out Reset
   #pragma config BORV = 42   // Brown Out Voltage   (4.2V)
   #pragma config WDT  = OFF   // Watchdog Timer
   #pragma config WDTPS= 64   // Watchdog Postscaler (1:64)
   #pragma config STVR = ON   // Stack Overflow Reset
   #pragma config LVP  = OFF   // Low Voltage ICSP
   #pragma config DEBUG= OFF   // Background Debugger Enable
   #pragma config CP0  = OFF   // Code Protection Block 0
   #pragma config CP1  = OFF   // Code Protection Block 1
   #pragma config CP2  = OFF   // Code Protection Block 2
   #pragma config CP3  = OFF   // Code Protection Block 3
   #pragma config CPB  = OFF   // Boot Block Code Protection
   #pragma config CPD  = OFF   // Data EEPROM Code Protection
   #pragma config WRT0 = OFF   // Write Protection Block 0
   #pragma config WRT1 = OFF   // Write Protection Block 1
   #pragma config WRT2 = OFF   // Write Protection Block 2
   #pragma config WRT3 = OFF   // Write Protection Block 3
   #pragma config WRTB = OFF   // Boot Block Write Protection
   #pragma config WRTC = OFF   // Configuration Register Write Protection
   #pragma config WRTD = OFF   // Data EEPROM Write Protection
   #pragma config EBTR0= OFF   // Table Read Protection Block 0
   #pragma config EBTR1= OFF   // Table Read Protection Block 1
   #pragma config EBTR2= OFF   // Table Read Protection Block 2
   #pragma config EBTR3= OFF   // Table Read Protection Block 3
   #pragma config EBTRB= OFF   // Boot Block Table Read Protection

#endif
// Einde van de headerfile CONFIG.H

