//-----------------------------------------------------------------------                                                  
// T I T L E : lin_slave_example.c                                                                                         
//-----------------------------------------------------------------------                                                  
																														   
//----- I N C L U D E S    
#include <avr/io.h>
#include "config.h"                                                                                                       
#include "lin_drv.h"                                                                                                       
																														   
//----- D E C L A R A T I O N S     
#define LIN_ID_ENUM_START		0x00
#define LIN_ID_ENUM_CONTINUE	0x01  
#define LIN_ID_ENUM_CHECK		0x10 //low 4 bits are slave address                                                                                
#define LIN_ID_GET_CELLS		0x20 //low 4 bits are slave address
#define LIN_ID_SET_DRAIN		0x30 
																									   
#define LEN_ENUM_START			1                                                                 
#define LEN_ENUM_CONTINUE		1
#define LEN_ENUM_CHECK			1
#define LEN_GET_CELLS			4    
#define LEN_SET_DRAIN			8 

#define LIN_ENUM_CHECK_RESPONSE 0xCC

unsigned char lin_id;
unsigned char lin_enum_check_response[LEN_ENUM_CHECK] = {LIN_ENUM_CHECK_RESPONSE};  
unsigned char lin_cell_voltage[LEN_GET_CELLS];
unsigned char lin_drain_state[LEN_SET_DRAIN];                                                                              
								
volatile unsigned char lin_addr;  																						   
volatile unsigned char lin_slave_err_nb = 0;                                                                               

//----- F U N C T I O N S -----------------------------------------------                                                                                                  
																														   
//.......................................................................                                                                        
// lin_id_task function of "lin_slave_example.c"                                                                           
//                                                                                                                         
//      The LIN ID received must be checked and compared to                                                                     
//          the ones that the node must manage.                                                                        
//.......................................................................                                                                        
void lin_id_task (void) {
	lin_id = Lin_get_id()

	lin_id_enum_check = LIN_ID_ENUM_CHECK | lin_addr;
	lin_id_get_cells = LIN_ID_ENUM_CHECK | lin_addr;


	switch (lin_id) {    
		case LIN_ID_ENUM_START:  
			//we are starting enumeration                                                                                   
			lin_addr = 0;
			if (canEnumerate()) {
				lin_rx_response(LIN_2X, LEN_ENUM_START);
			}
			break;

		case LIN_ID_ENUM_CONTINUE:                                                                                     
			//if our ENUMI line is high, set our address
			if (canEnumerate()) {
				lin_rx_response(LIN_2X, LEN_ENUM_START);
			}
			break;   

		case lin_id_enum_check:   
			lin_tx_response(LIN_2X, lin_enum_check_response, LEN_ENUM_CHECK);
			break;  

		case lin_id_get_cells:                                                                                     
			lin_tx_response(LIN_2x, lin_cell_voltage, LEN_GET_CELLS)										   
			break;

		case LEN_SET_DRAIN:                                                                                     
			lin_rx_response(LIN_2X, LEN_SET_DRAIN);										   
			break;                                                                                  
																			   
		default:                                                                                           
			// ID: absent/refused                                                                  
			break;                                                                                 
	} 
}                                                                                                                          
																														   
																														   
//.......................................................................                                                                        
// lin_rx_task function of "lin_slave_example.c"                                                                           
//                                                                                                                         
//      - Save response data                                                                                       
//      - Update application signals.                                                                              
//.......................................................................                                                                        
void lin_rx_task (void) {                                                                                                  
																														   
unsigned char l_temp;                                                                                                      
	
	char * lin_receive_ptr;

	switch (lin_id) {
		case LIN_ID_ENUM_START
			lin_receive_ptr = lin_id;
			break;

		case LIN_ID_ENUM_CONTINUE
			lin_receive_ptr = lin_id;
			break;

		case LEN_SET_DRAIN
			lin_receive_ptr = lin_drain_state;
			break;
	}			

	lin_get_response (lin_receive_ptr);                                                                           
																												   
	                                                                                                           
}                                                                                                                          
																														   
																														   
//.......................................................................                                                                        
// lin_tx_task function of "lin_slave_example.c"                                                                           
//                                                                                                                         
//      - Update buffer array with application signals                                                             
//          to be ready for the next transmission                                                                  
//.......................................................................                                                                        
void lin_tx_task (void) {
	//pick the correct 4 bits of the drain status
	drainStatus =  lin_drain_state[lin_addr >> 1] >> 4*(lin_addr & 0x01) & 0x0F
	set_drains(drainStatus);
	//blocking. only update one at a time to make sure we don't miss any LIN frames
	get_cell_voltages(lin_cell_voltage, cell_number);
}                                                                                                                          
																														   
																														   
//.......................................................................                                                                        
// lin_err_task function of "lin_slave_example.c"                                                                          
//                                                                                                                         
//      - If LIN error, increment the node error number                                                            
//.......................................................................                                                                        
void lin_err_task (void) {                                                                                                 
																														   
	// Global variable update                                                                                      
	lin_slave_err_nb++;                                                                                            
}                                                                                                                          
																														   
																														   
//.......................................................................                                                                        
//. . . M A I N ( )                                                                         
//                                                                                                                         
// main function of "lin_slave_example.c"                                                                                  
//                                                                                                                         
//      In a 'no end' loop, do:                                                                                    
//          - if LIN_ID_0 (Rx response 1 Byte )                                                                    
//              . PORT-B.0 = DC motor command -> clockwise                                                 
//              . PORT-B.1 = DC motor command -> counterclockwise                                          
//          - if LIN_ID_1 (Tx response 2 bytes)                                                                    
//              . Byte[0] = motor status                                                                   
//              . Byte[1] = node error number                                                              
//.......................................................................                                                                        
int main (void) {                                                                                                          
																														   
	// Port B setting for motor control
	DDRB |= 1<<PORTB0; DDRB |= 1<<PORTB1;
	PORTB &= ~(1<<PORTB0); PORTB &= ~(1<<PORTB1);

	// LIN Initialization
	lin_init((unsigned char)LIN_2X, ((unsigned short)CONF_LINBRR));
	
	// No End Loop
	while (1) {
		switch (Lin_get_it()) {
			case LIN_IDOK:
				lin_id_task();
				Lin_clear_idok_it();
				break;
			case LIN_RXOK:
				lin_rx_task();
				Lin_clear_rxok_it();
				break;
			case LIN_TXOK:
				lin_tx_task();
				Lin_clear_txok_it();
				break;
			case LIN_ERROR:
				lin_err_task();
				Lin_clear_err_it();
				break;
			default:
				break;
		}
	}
	return 0;
}
