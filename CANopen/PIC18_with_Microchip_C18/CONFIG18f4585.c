
#ifndef __CONFIG__
   #define __CONFIG__
   /**
    * @brief configuration bits
    * 
    */
   #pragma config OSC       = HS         // Oscillator Selection
   #pragma config IESO      = OFF        // Osc. Switch Enable
   #pragma config PWRT      = ON         // Power Up Timer
   #pragma config BOREN     = SBORENCTRL        // Brown Out Reset
   #pragma config BORV      = 1          // Brown Out Voltage   (4.2V)
   #pragma config WDT       = ON        // Watchdog Timer
   #pragma config WDTPS     = 1024         // Watchdog Postscaler (1:64)
   #pragma config MCLRE     = ON         //<< MCLR Pin Enable bit
   #pragma config PBADEN    = OFF        //<< PORTB Pins Configured for A/D
   #pragma config STVREN    = ON         // Stack Overflow Reset
   #pragma config LVP       = OFF        // Low Voltage ICSP
   #pragma config DEBUG     = ON        // Background Debugger Enable
   #pragma config CP0       = OFF        // Code Protection Block 0
   #pragma config CP1       = OFF        // Code Protection Block 1
   #pragma config CP2       = OFF        // Code Protection Block 2
   #pragma config CPB       = OFF        // Boot Block Code Protection
   #pragma config CPD       = OFF        // Data EEPROM Code Protection
   #pragma config WRT0      = OFF        // Write Protection Block 0
   #pragma config WRT1      = OFF        // Write Protection Block 1
   #pragma config WRT2      = OFF        // Write Protection Block 2
   #pragma config WRTB      = OFF        // Boot Block Write Protection
   #pragma config WRTC      = OFF        // Configuration Register Write Protection
   #pragma config WRTD      = OFF        // Data EEPROM Write Protection
   #pragma config EBTR0     = OFF        // Table Read Protection Block 0
   #pragma config EBTR1     = OFF        // Table Read Protection Block 1
   #pragma config EBTR2     = OFF        // Table Read Protection Block 2
   #pragma config EBTRB     = OFF        // Boot Block Table Read Protection

#endif


