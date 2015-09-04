/*******************************************************************************
* This file provides the functions for the DS18B20 Sample code

* Author: ANG JIA YI @ Cytron Technologies Sdn. Bhd.
*******************************************************************************/
// This file is written for PIC16F877A, if other model of PIC is uses, please modify accordingly


#include <htc.h>		//include hitech C header file
#include "system.h"		//include system header file
#include "lcd.h"		//include lcd header file
/*******************************************************************************
* DEVICE CONFIGURATION WORDS                                                   *
*******************************************************************************/
// This configuration bits are for PIC16F877A using 20MHz Crystal or HS for OSC
#if defined (HITECH_V9_80)	//if Hi-Tech V9.80 compiler is being used
__CONFIG(HS &			// High Speed Crystal.
		 WDTDIS &		// Disable Watchdog Timer.
		 PWRTEN &		// Enable Power Up Timer.
		 BORDIS &		// Disable Brown Out Reset.
		 LVPDIS);		// Disable Low Voltage Programming.

#elif defined (HITECH_V9_82)		//else if Hi-Tech V9.82 compiler is being used
// If Hi-TECH C Compiler v9.82 is use, this configuration bit should be used
__CONFIG(FOSC_HS &			// High Speed Crystal.
		 WDTE_OFF &		// Disable Watchdog Timer.
		 PWRTE_ON &		// Enable Power Up Timer.
		 BOREN_OFF &		// Disable Brown Out Reset.
		 LVP_OFF);		// Disable Low Voltage Programming.
#endif	//end of pre-processor if condition

/*******************************************************************************
* Defination of label		                                                  *
*******************************************************************************/

#define Skip_ROM 		0xCC
#define Convert_T 		0x44
#define Read_scratchpad 0xBE

#define Port_18B20 	RB0
#define Tx_18B20 	TRISB0 = 0
#define Rx_18B20 	TRISB0 = 1


/*******************************************************************************
* PRIVATE FUNCTION PROTOTYPES                                                  *
*******************************************************************************/
void delay_ms(unsigned int ui_value);
void beep(unsigned char uc_count);
unsigned char reset();
void write(char WRT);
unsigned char read();

/*******************************************************************************
* Global Variables                                                             *
*******************************************************************************/

/*******************************************************************************
* MAIN FUNCTION                                                                *
*******************************************************************************/
int main(void)
{
	unsigned temp;
	unsigned short tempL, tempH, fraction;
	unsigned int i = 0;	// declare a variable to store 
	// ensure all the hardware port in zero initially
	PORTA = 0;
	PORTB = 0;
	PORTC = 0;
	PORTD = 0;
	PORTE = 0;

	// Initialize the I/O port direction, this must be configured according to circuit
	// TRISX control pin direction, output pin must be configure as '0'
	// while input must be configure as '1'
	TRISA = 0;
	TRISB = 0b00001110;
	TRISC = 0;
	TRISD = 0;
	TRISE = 0;

	//Initialize LCD to be use
	lcd_initialize();
	
	lcd_putstr("Cytron Tech");  //LCD display for the starting part
	lcd_goto(0xc0);
	lcd_putstr("DS18B20 sw1=rdy");
	delay_ms(1000);
	lcd_clear();
	lcd_putstr("DS18B20");  //LCD display for the starting part
	lcd_goto(0xc6);
	lcd_putstr(":Degree C");
	
	while(1) 	// create an infinite loop
	{

			if(!reset())
			{
			write(Skip_ROM);		
			write(Convert_T);		
			delay_ms(750);
			
			reset();
			write(Skip_ROM);		
			write(Read_scratchpad);	
			
			tempL = read();			//read low temp value
			tempH = read();			//read high temp value
			i=((unsigned int)tempH << 8 ) + (unsigned int)tempL;		//put both value in one variable
			i = (float)i * 6.25 ;		//calcuation from the table provided 
			lcd_goto(0xc0);				
			lcd_bcd(5,i);				//display the temperation in LCD
			}
	}	
	while(1) continue;	// infinite loop to prevent PIC from reset if there is no more program	
}

/*******************************************************************************
* PRIVATE FUNCTION: delay_ms
*
* PARAMETERS:
* ~ ui_value	- The period for the delay in miliseconds.
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Delay in miliseconds.
*
*******************************************************************************/
void delay_ms(unsigned int ui_value)
{
	while (ui_value-- > 0) {
		__delay_ms(1);		// macro from HI-TECH compiler which will generate 1ms delay base on value of _XTAL_FREQ in system.h
	}	
}

/*******************************************************************************
* PRIVATE FUNCTION: beep
*
* PARAMETERS:
* ~ uc_count	- How many times we want to beep.
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Beep for the specified number of times.
*
*******************************************************************************/
void beep(unsigned char uc_count)
{
	while (uc_count-- > 0) {
		BUZZER = 1;	//buzzer is share at RC2 pin
		delay_ms(80);
		BUZZER = 0;
		delay_ms(100);
	}
}

unsigned char reset()
{
	Tx_18B20; // Tris = 0 (output)
	Port_18B20 = 0; // set pin# to low (0)
	__delay_us(480); // 1 wire require time delay
	Rx_18B20; // Tris = 1 (input)
	__delay_us(60); // 1 wire require time delay
	
		if (Port_18B20 == 0) // if there is a presence pluse
		{ 
		__delay_us(480);
		return 0; // return 0 ( 1-wire is presence)
		} 
		else 
		{
		__delay_us(480);
		return 1; // return 1 ( 1-wire is NOT presence)
		}
}

void write(char WRT)
{
	char i,Cmd;
	Cmd=WRT;
	Rx_18B20; // set pin# to input (1)
	
		for(i = 0; i < 8; i++)
		{
			if((Cmd & (1<<i))!= 0) 
			{
			// write 1
			Tx_18B20; // set pin# to output (0)
			Port_18B20 = 0; // set pin# to low (0)
			__delay_us(1); // 1 wire require time delay
			Rx_18B20; // set pin# to input (release the bus)
			__delay_us(60); // 1 wire require time delay
			} 
			else 
			{
			//write 0
			Tx_18B20; // set pin# to output (0)
			Port_18B20 = 0; // set pin# to low (0)
			__delay_us(60); // 1 wire require time delay
			Rx_18B20; // set pin# to input (release the bus)
			}
		}
}

unsigned char read()
{
	char i,result = 0;
	Rx_18B20; // TRIS is input(1)
		for(i = 0; i < 8; i++)
		{
		Tx_18B20; // TRIS is output(0)
		Port_18B20 = 0; // genarate low pluse for 2us
		__delay_us(2);
		Rx_18B20; // TRIS is input(1) release the bus
		if(Port_18B20 != 0) 
		result |= 1<<i;
		__delay_us(60); // wait for recovery time
		}
	return result;
}
