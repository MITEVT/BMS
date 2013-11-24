//-----------------------------------------------------------------------
// config.h for "main.c"
//-----------------------------------------------------------------------
                                                                                                                           
//----- I N C L U D E S
#include <avr/io.h>                 // Use AVR-GCC library

//----- D E C L A R A T I O N S
// Use an external crystal oscillator for "lin_master_example.c"
#define FOSC            8000        // in KHz
#define LIN_BAUDRATE    19200		// in bit/s
