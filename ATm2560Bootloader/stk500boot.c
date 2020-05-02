
/*****************************************************************************
Title:     STK500v2 compatible bootloader
           Modified for Wiring board ATMega128-16MHz
Author:    Peter Fleury <pfleury@gmx.ch>   http://jump.to/fleury
File:      $Id: stk500boot.c,v 1.11 2006/06/25 12:39:17 peter Exp $
Compiler:  avr-gcc 3.4.5 or 4.1 / avr-libc 1.4.3
Hardware:  All AVRs with bootloader support, tested with ATmega8
License:   GNU General Public License

Modified:  Worapoht Kornkaewwattanakul <dev@avride.com>   http://www.avride.com
Date:      17 October 2007
Update:    1st, 29 Dec 2007 : Enable CMD_SPI_MULTI but ignore unused command by return 0x00 byte response..
Compiler:  WINAVR20060421
Description: add timeout feature like previous Wiring bootloader

DESCRIPTION:
    This program allows an AVR with bootloader capabilities to
    read/write its own Flash/EEprom. To enter Programming mode
    an input pin is checked. If this pin is pulled low, programming mode
    is entered. If not, normal execution is done from $0000
    "reset" vector in Application area.
    Size fits into a 1024 word bootloader section
	when compiled with avr-gcc 4.1
	(direct replace on Wiring Board without fuse setting changed)

USAGE:
    - Set AVR MCU type and clock-frequency (F_CPU) in the Makefile.
    - Set baud rate below (AVRISP only works with 115200 bps)
    - compile/link the bootloader with the supplied Makefile
    - program the "Boot Flash section size" (BOOTSZ fuses),
      for boot-size 1024 words:  program BOOTSZ01
    - enable the BOOT Reset Vector (program BOOTRST)
    - Upload the hex file to the AVR using any ISP programmer
    - Program Boot Lock Mode 3 (program BootLock 11 and BootLock 12 lock bits) // (leave them)
    - Reset your AVR while keeping PROG_PIN pulled low // (for enter bootloader by switch)
    - Start AVRISP Programmer (AVRStudio/Tools/Program AVR)
    - AVRISP will detect the bootloader
    - Program your application FLASH file and optional EEPROM file using AVRISP

Note:
    Erasing the device without flashing, through AVRISP GUI button "Erase Device"
    is not implemented, due to AVRStudio limitations.
    Flash is always erased before programming.

	AVRdude:
	Please uncomment #define REMOVE_CMD_SPI_MULTI when using AVRdude.
	Comment #define REMOVE_PROGRAM_LOCK_BIT_SUPPORT to reduce code size
	Read Fuse Bits and Read/Write Lock Bits is not supported

NOTES:
    Based on Atmel Application Note AVR109 - Self-programming
    Based on Atmel Application Note AVR068 - STK500v2 Protocol

LICENSE:
    Copyright (C) 2006 Peter Fleury

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

 *****************************************************************************/

//************************************************************************
//*	Edit History
//************************************************************************
//*	Jul  7,	2010	<MLS> = Mark Sproul msproul@skycharoit.com
//*	Jul  7,	2010	<MLS> Working on mega2560. No Auto-restart
//*	Jul  7,	2010	<MLS> Switched to 8K bytes (4K words) so that we have room for the monitor
//*	Jul  8,	2010	<MLS> Found older version of source that had auto restart, put that code back in
//*	Jul  8,	2010	<MLS> Adding monitor code
//*	Jul 11,	2010	<MLS> Added blinking LED while waiting for download to start
//*	Jul 11,	2010	<MLS> Added EEPROM test
//*	Jul 29,	2010	<MLS> Added recchar_timeout for timing out on bootloading
//*	Aug 23,	2010	<MLS> Added support for atmega2561
//*	Aug 26,	2010	<MLS> Removed support for BOOT_BY_SWITCH
//************************************************************************



#include	<inttypes.h>
#include	<avr/io.h>
#include	<avr/interrupt.h>
#include	<avr/boot.h>
#include	<avr/pgmspace.h>
#include	<util/delay.h>
#include	<avr/eeprom.h>
#include	<avr/common.h>
#include	<stdlib.h>
#include	"command.h"
#include	"display.h"
#include   "logo.h"


#ifndef EEWE
#define EEWE 1
#endif
#ifndef EEMWE
#define EEMWE 2
#endif

//#define	_DEBUG_SERIAL_

/*
 * Uncomment the following lines to save code space
 */
//#define	REMOVE_PROGRAM_LOCK_BIT_SUPPORT		// disable program lock bits
//

/*
 * define CPU frequency in Mhz here if not defined in Makefile
 */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/*
 * UART Baudrate, AVRStudio AVRISP only accepts 115200 bps
 */

#ifndef BAUDRATE
#define BAUDRATE 115200
#endif

/*
 *  Enable (1) or disable (0) USART double speed operation
 */
#define UART_BAUDRATE_DOUBLE_SPEED 1
/*
 * HW and SW version, reported to AVRISP, must match version of AVRStudio
 */
#define CONFIG_PARAM_BUILD_NUMBER_LOW	0
#define CONFIG_PARAM_BUILD_NUMBER_HIGH	0
#define CONFIG_PARAM_HW_VER				0x0F
#define CONFIG_PARAM_SW_MAJOR			2
#define CONFIG_PARAM_SW_MINOR			0x0A

/*
 * Calculate the address where the bootloader starts from FLASHEND and BOOTSIZE
 * (adjust BOOTSIZE below and BOOTLOADER_ADDRESS in Makefile if you want to change the size of the bootloader)
 */
//#define BOOTSIZE 1024
#if FLASHEND > 0x0F000
#define BOOTSIZE 8192
#else
#define BOOTSIZE 2048
#endif

#define APP_END  (FLASHEND -(2*BOOTSIZE) + 1)

/*
 * Signature bytes are not available in avr-gcc io_xxx.h
 */
#define SIGNATURE_BYTES 0x1E9801


/* ATMega with two USART, use UART0 */
#define	UART_BAUD_RATE_LOW			UBRR0L
#define	UART_STATUS_REG				UCSR0A
#define	UART_CONTROL_REG			UCSR0B
#define	UART_ENABLE_TRANSMITTER		TXEN0
#define	UART_ENABLE_RECEIVER		RXEN0
#define	UART_TRANSMIT_COMPLETE		TXC0
#define	UART_RECEIVE_COMPLETE		RXC0
#define	UART_DATA_REG				UDR0
#define	UART_DOUBLE_SPEED			U2X0



/*
 * Macro to calculate UBBR from XTAL and baudrate
 */
#define UART_BAUD_SELECT(baudRate,xtalCpu) (((float)(xtalCpu))/(((float)(baudRate))*8.0)-1.0+0.5)


/*
 * States used in the receive state machine
 */
#define	ST_START		0
#define	ST_GET_SEQ_NUM	1
#define ST_MSG_SIZE_1	2
#define ST_MSG_SIZE_2	3
#define ST_GET_TOKEN	4
#define ST_GET_DATA		5
#define	ST_GET_CHECK	6
#define	ST_PROCESS		7

/*
 * use 16bit address variable for ATmegas with <= 64K flash
 */
#if defined(RAMPZ)
typedef uint32_t address_t;
#else
typedef uint16_t address_t;
#endif

/*
 * function prototypes
 */
static void sendchar(char c);

/*
 * since this bootloader is not linked against the avr-gcc crt1 functions,
 * to reduce the code size, we need to provide our own initialization
 */
void __jumpMain	(void) __attribute__ ((naked)) __attribute__ ((section (".init9")));
#include <avr/sfr_defs.h>

//#define	SPH_REG	0x3E
//#define	SPL_REG	0x3D

//*****************************************************************************
void __jumpMain(void)
{
	//*	July 17, 2010	<MLS> Added stack pointer initialzation
	//*	the first line did not do the job on the ATmega128

	asm volatile ( ".set __stack, %0" :: "i" (RAMEND) );

	asm volatile ( "ldi	16, %0" :: "i" (RAMEND >> 8) );
	asm volatile ( "out %0,16" :: "i" (AVR_STACK_POINTER_HI_ADDR) );
	asm volatile ( "ldi	16, %0" :: "i" (RAMEND & 0x0ff) );
	asm volatile ( "out %0,16" :: "i" (AVR_STACK_POINTER_LO_ADDR) );
	asm volatile ( "clr __zero_reg__" );									// GCC depends on register r1 set to 0
	asm volatile ( "out %0, __zero_reg__" :: "I" (_SFR_IO_ADDR(SREG)) );	// set SREG to 0
	asm volatile ( "jmp main");												// jump to main()
}


//*****************************************************************************
void delay_ms(unsigned int timedelay)
{
	unsigned int i;
	for (i=0;i<timedelay;i++)
	{
		_delay_ms(0.5);
	}
}

//*****************************************************************************
/*
 * send single byte to USART, wait until transmission is completed
 */
static void sendchar(char c)
{
	UART_DATA_REG	=	c;										// prepare transmission
	while (!(UART_STATUS_REG & (1 << UART_TRANSMIT_COMPLETE)));	// wait until byte sent
	UART_STATUS_REG |= (1 << UART_TRANSMIT_COMPLETE);			// delete TXCflag
}


//************************************************************************
static int	Serial_Available(void)
{
	return(UART_STATUS_REG & (1 << UART_RECEIVE_COMPLETE));	// wait for data
}

//************************************************************************
// basic display functions
static void senden_spi(unsigned char zeichen){
	unsigned char a=0, b=0b10000000;
	fastWriteLow(SPI_CS); // cs auf low => aktivieren
	fastWriteHigh(SPI_CLK); // clock auf high, zu fallender flanke werden daten �bernommen

	for(a=0;a<8;a++){ /* a = 0..7, wegen 8 pixel zeichen breite */
		if(zeichen&b)	
			fastWriteHigh(SPI_DATA); /* wenn zeichen&b dann porta bit 0 auf high serial data line */
		else 
			fastWriteLow(SPI_DATA); /* PORTA bit 0 auf low zwingen serial data line */
		fastWriteLow(SPI_CLK); // clock auf high, zu fallender flanke werden daten �bernommen
		fastWriteHigh(SPI_CLK); // clock auf high, zu fallender flanke werden daten �bernommen
		b=b>>1;	/* bitmaske einen nach recht schieben => msb first ?! */
	}
	fastWriteHigh(SPI_CS); // cs auf high => deaktivieren
}

/////////////////////////////// command funktion ///////////////////////////////
static void send_command(unsigned char theCommand){
	fastWriteLow(SPI_CD); //turn to command mode
	senden_spi(theCommand);
	fastWriteHigh(SPI_CD); //turn to data mode
}

/////////////////////////////// data funktion ///////////////////////////////
static void send_char(unsigned char zeichen){
	fastWriteHigh(SPI_CD); //turn to data mode
	senden_spi(zeichen);
}
//************************************************************************	

//************************************************************************
// init display
static void init_display(void){
	DDRC = 0x00 | (1<<PC7) | (1<<PC5) | (1<<PC3) | (1<<PC2) | (1<<PC1);

	_delay_ms(1); // TODO: Change to 1 ?

	fastWriteHigh(SPI_CS);
	fastWriteLow(SPI_CD);
	fastWriteLow(SPI_DATA);
	fastWriteLow(SPI_CLK);

	// reset
	fastWriteLow(SPI_RESET);
	_delay_ms(1);
	fastWriteHigh(SPI_RESET);
	//_delay_ms(1);

	// init sequenze
	/////////////////////////////
	// XXXXXX | Normale | 180� //
	// ----------------------- //
	// Re-Map | 0x41 | 0x52 //
	// Offset | 0x44 | 0x4C //
	/////////////////////////////

	send_command(0x15);	send_command(0x00);	send_command(0x3F);	// Column Address
	send_command(0x75);	send_command(0x00);	send_command(0x3F);
	send_command(0x81);	send_command(0x66);	// Contrast Control
	send_command(0x86);// Current Range
	send_command(0xA0);	send_command(0x52);// Re-map
	send_command(0xA1);	send_command(0x00);// Display Start Line
	send_command(0xA2);	send_command(0x4C);// Display Offset
	send_command(0xA4);// Display Mode
	send_command(0xA8);	send_command(0x3F);// Multiplex Ratio
	send_command(0xB1);	send_command(0xA8);// set prechange // Phase Length
	send_command(0xB2);	send_command(0x46);// Row Period
	send_command(0xB3);	send_command(0xF1); // war f1 // Display Clock Divide
	send_command(0xBF);	send_command(0x0D);// VSL
	send_command(0xBE);	send_command(0x02);	send_command(0xBC);	send_command(0x38); // VCOMH
	send_command(0xB8);	send_command(0x01);	send_command(0x11);	send_command(0x22);	send_command(0x32);	 // Gamma
	send_command(0x43);	send_command(0x54);	send_command(0x65);	send_command(0x76);
	send_command(0xAD); /* Set DC-DC */
	send_command(0x02); /* 03=ON, 02=Off */
	send_command(0xAF); // Display ON/OFF
	/* AF=ON, AE=Sleep Mode */
};

// display filled rect
static void clear_screen(void){
	send_command(0x15);
	send_command(0x00);
	send_command(0x7F);
	send_command(0x75);
	send_command(0x00);
	send_command(0x3F);
	int a;
	for (a=0;a<(128*32);a++){
		send_char(0x00);
	}
}

// display horizontal line
static void draw_line(unsigned char x,unsigned char y,unsigned char width){
	send_command(0x15);
	send_command(x/2);
	send_command((width-1+x)/2);
	send_command(0x75);
	send_command(y);
	send_command(y);
	int a;
	for(a=0;a<(width/2);a++){
		send_char(0xff);
	};
}

static void show_logo(void){
	unsigned char a,zeile,buchstabe;
	unsigned char x=15;
	unsigned char y=47;

	unsigned char data,send_me;
	for(a=0;a<9;a++){
		if(a==0){ // S
			buchstabe=5;
		} else if(a==1){ // P
			buchstabe=6;
		} else if(a==2 || a==3){ // E
			buchstabe=0;
		} else if(a==4){ // D
			buchstabe=1;
		} else if(a==5 || a==8){ // O
			buchstabe=4;
		} else if(a==6){ // I
			buchstabe=2;
		} else if(a==7){ // N
			buchstabe=3;
		}
		// SPEEDOINO
		send_command(0x15);
		send_command(x);
		send_command(x+2); // buchstaben sind 5px breit + spacer
		send_command(0x75);
		send_command(y);
		send_command(y+5); // und 5 px hoch

		buchstabe=buchstabe*6;
		for(zeile=0;zeile<6;zeile++){
			data=speedoino_data[buchstabe];
			buchstabe++;

			send_me=0x00;
			if(data&0b10000000)	send_me|=0xf0;
			if(data&0b01000000)	send_me|=0x0f;
			send_char(send_me);

			send_me=0x00;
			if(data&0b00100000)	send_me|=0xf0;
			if(data&0b00010000)	send_me|=0x0f;
			send_char(send_me);

			send_me=0x00;
			if(data&0b00001000)	send_me|=0xf0;
			if(data&0b00000100)	send_me|=0x0f;
			send_char(send_me);
		};

		x+=3;
	};
}

//************************************************************************



#define	 MAX_TIME_COUNT	(F_CPU >> 1)
//*****************************************************************************
static unsigned char recchar_timeout(void)
{
	uint32_t count = 0;

	while (!(UART_STATUS_REG & (1 << UART_RECEIVE_COMPLETE)))
	{
		// wait for data
		count++;
		if (count > MAX_TIME_COUNT)
		{
			unsigned int	data;
#if (FLASHEND > 0x0FFFF)
			data	=	pgm_read_word_far(0);	//*	get the first word of the user program
#else
			data	=	pgm_read_word_near(0);	//*	get the first word of the user program
#endif
			if (data != 0xffff)					//*	make sure its valid before jumping to it.
			{
				asm volatile(
						"clr	r30		\n\t"
						"clr	r31		\n\t"
						"ijmp	\n\t"
				);
			}
			count	=	0;
		}
	}
	return UART_DATA_REG;
}



//*****************************************************************************
int main(void)
{
	address_t		address			=	0;
	address_t		eraseAddress	=	0;
	unsigned char	msgParseState;
	unsigned int	ii				=	0;
	unsigned char	checksum		=	0;
	unsigned char	seqNum			=	1;
	unsigned int	msgLength		=	0;
	unsigned char	msgBuffer[285];
	unsigned char	c, *p;
	unsigned char   isLeave = 0;

	unsigned long	boot_timeout;
	unsigned long	boot_timer;
	unsigned int	boot_state;

	/////////Kolja von hier/////////
	/* enabled in ph3 to read if its a bluetooth reset*/
	DDRH  &= ~(1<<PH3); // one input
	PORTH =  0x00 | (1<<PH3); // with pullup

	/* enabled pull up on CAN CS connection, to deactivate can bus interface (interface is low active)*/
	PORTK |= (1<<PK4);
	/////////Kolja bis hier/////////


	boot_timer	=	0;
	boot_state	=	0;
	boot_timeout	=	250000; // 7 seconds , approx 2us per step when optimize "s"

	/*
	 * Branch to bootloader or application code ?
	 */

	/*
	 * Init UART
	 * set baudrate and enable USART receiver and transmiter without interrupts
	 */
#if UART_BAUDRATE_DOUBLE_SPEED
	UART_STATUS_REG		|=	(1 <<UART_DOUBLE_SPEED);
#endif
	UART_BAUD_RATE_LOW	=	UART_BAUD_SELECT(BAUDRATE,F_CPU);
	UART_CONTROL_REG	=	(1 << UART_ENABLE_RECEIVER) | (1 << UART_ENABLE_TRANSMITTER);

	asm volatile ("nop");			// wait until port has changed

	/////////Kolja von hier/////////
	init_display(); // this is the moment when we can init the display
	clear_screen();
	//_delay_ms(2);
	msgLength		=	11;
	msgBuffer[0] 	=	CMD_SIGN_ON;
	msgBuffer[1] 	=	STATUS_CMD_OK;
	msgBuffer[2] 	=	8;
	msgBuffer[3] 	=	'A';
	msgBuffer[4] 	=	'V';
	msgBuffer[5] 	=	'R';
	msgBuffer[6] 	=	'I';
	msgBuffer[7] 	=	'S';
	msgBuffer[8] 	=	'P';
	msgBuffer[9] 	=	'_';
	msgBuffer[10]	=	'2';
	sendchar(MESSAGE_START);
	checksum	=	MESSAGE_START^0;

	sendchar(seqNum);
	checksum	^=	seqNum;

	c			=	((msgLength>>8)&0xFF);
	sendchar(c);
	checksum	^=	c;

	c			=	msgLength&0x00FF;
	sendchar(c);
	checksum ^= c;

	sendchar(TOKEN);
	checksum ^= TOKEN;

	p	=	msgBuffer;
	while ( msgLength )
	{
		c	=	*p++;
		sendchar(c);
		checksum ^=c;
		msgLength--;
	}
	sendchar(checksum);
	seqNum++;


	//filled_rect(0,0,128,20,0xff);
	/////////Kolja bis hier /////////

	while (boot_state==0)
	{
		while ((!(Serial_Available())) && (boot_state == 0))		// wait for data
		{
			_delay_ms(0.001);
			/////////Kolja von hier /////////
			// nur wenn das bit gesetzt ist
			if(bit_is_set(PINH,3)){
				/////////Kolja bis hier/////////
				boot_timer++;
				/////////Kolja von hier /////////
			}
			/////////Kolja bis hier /////////

			if (boot_timer > boot_timeout)
			{
				boot_state	=	1; // (after ++ -> boot_state=2 bootloader timeout, jump to main 0x00000 )
			}
		}
		boot_state++; // ( if boot_state=1 bootloader received byte from UART, enter bootloader mode)
	}


	if (boot_state==1)
	{
		//*	main loop
		while (!isLeave)
		{
			/*
			 * Collect received bytes to a complete message
			 */
			msgParseState	=	ST_START;
			while ( msgParseState != ST_PROCESS )
			{
				if (boot_state==1)
				{
					boot_state	=	0;
					c			=	UART_DATA_REG;
				}
				else
				{
					c	=	recchar_timeout();
				}



				switch (msgParseState){
				case ST_START:
					if ( c == MESSAGE_START )
					{
						msgParseState	=	ST_GET_SEQ_NUM;
						checksum		=	MESSAGE_START^0;
					}
					break;

				case ST_GET_SEQ_NUM:
					if ( (c == 1) || (c == seqNum) )
					{
						seqNum			=	c;
						msgParseState	=	ST_MSG_SIZE_1;
						checksum		^=	c;
					}
					else
					{
						msgParseState	=	ST_START;
					}
					break;

				case ST_MSG_SIZE_1:
					msgLength		=	c<<8;
					msgParseState	=	ST_MSG_SIZE_2;
					checksum		^=	c;
					break;

				case ST_MSG_SIZE_2:
					msgLength		|=	c;
					msgParseState	=	ST_GET_TOKEN;
					checksum		^=	c;
					break;

				case ST_GET_TOKEN:
					if ( c == TOKEN )
					{
						msgParseState	=	ST_GET_DATA;
						checksum		^=	c;
						ii				=	0;
					}
					else
					{
						msgParseState	=	ST_START;
					}
					break;

				case ST_GET_DATA:
					msgBuffer[ii++]	=	c;
					checksum		^=	c;
					if (ii == msgLength )
					{
						msgParseState	=	ST_GET_CHECK;
					}
					break;

				case ST_GET_CHECK:
					if ( c == checksum )
					{
						msgParseState	=	ST_PROCESS;
					}
					else
					{
						msgParseState	=	ST_START;
					}
					break;
				}	//	switch
			}	//	while(msgParseState)

			/*
			 * Now process the STK500 commands, see Atmel Appnote AVR068
			 */

			switch (msgBuffer[0]){
			case CMD_SPI_MULTI:
			{
				unsigned char answerByte;
				unsigned char flag=0;

				if ( msgBuffer[4]== 0x30 )
				{
					unsigned char signatureIndex = msgBuffer[6];

					if ( signatureIndex == 0 ){
						draw_line(0,LOADING_Y,128);
						draw_line(0,LOADING_Y+4,128);
						// top optimize it written by hand:
						send_command(0x15);
						send_command(0);
						send_command(0);
						send_command(0x75);
						send_command(LOADING_Y+1);
						send_command(LOADING_Y+3);
						send_char(0xf0);
						send_char(0xf0);
						send_char(0xf0);

						send_command(0x15);
						send_command(63);
						send_command(64);
						send_command(0x75);
						send_command(LOADING_Y+1);
						send_command(LOADING_Y+3);
						send_char(0x0f);
						send_char(0x0f);
						send_char(0x0f);

						show_logo();

						answerByte = (SIGNATURE_BYTES >>16) & 0x000000FF;
					} else if ( signatureIndex == 1 ) {
						answerByte = (SIGNATURE_BYTES >> 8) & 0x000000FF;
					} else {
						answerByte = SIGNATURE_BYTES & 0x000000FF;
					};
				}
				else if ( msgBuffer[4] & 0x50 )
				{
					answerByte = 0; //read fuse/lock bits not implemented, return dummy value
				}
				else
				{
					answerByte = 0; // for all others command are not implemented, return dummy value for AVRDUDE happy <Worapoht>
					// flag = 1; // Remark this line for AVRDUDE <Worapoht>
				}
				if ( !flag )
				{
					msgLength = 7;
					msgBuffer[1] = STATUS_CMD_OK;
					msgBuffer[2] = 0;
					msgBuffer[3] = msgBuffer[4];
					msgBuffer[4] = 0;
					msgBuffer[5] = answerByte;
					msgBuffer[6] = STATUS_CMD_OK;
				}
				// darf nicht in dem init kommen, sonst malen wir das jedes mal - bei jedem Startup, auch wenn wir das gar nicht wollen

			}
			break;

			case CMD_SIGN_ON:
				msgLength		=	11;
				msgBuffer[1] 	=	STATUS_CMD_OK;
				msgBuffer[2] 	=	8;
				msgBuffer[3] 	=	'A';
				msgBuffer[4] 	=	'V';
				msgBuffer[5] 	=	'R';
				msgBuffer[6] 	=	'I';
				msgBuffer[7] 	=	'S';
				msgBuffer[8] 	=	'P';
				msgBuffer[9] 	=	'_';
				msgBuffer[10]	=	'2';
				break;

			case CMD_GET_PARAMETER:
			{
				unsigned char value;

				switch(msgBuffer[1])
				{
				case PARAM_BUILD_NUMBER_LOW:
					value	=	CONFIG_PARAM_BUILD_NUMBER_LOW;
					break;
				case PARAM_BUILD_NUMBER_HIGH:
					value	=	CONFIG_PARAM_BUILD_NUMBER_HIGH;
					break;
				case PARAM_HW_VER:
					value	=	CONFIG_PARAM_HW_VER;
					break;
				case PARAM_SW_MAJOR:
					value	=	CONFIG_PARAM_SW_MAJOR;
					break;
				case PARAM_SW_MINOR:
					value	=	CONFIG_PARAM_SW_MINOR;
					break;
				default:
					value	=	0;
					break;
				}
				msgLength		=	3;
				msgBuffer[1]	=	STATUS_CMD_OK;
				msgBuffer[2]	=	value;
			}
			break;

			case CMD_LEAVE_PROGMODE_ISP:
				isLeave			=	1;
				msgLength		=	2;
				msgBuffer[1]	=	STATUS_CMD_OK;
				//*	fall thru
				break;
			case CMD_SET_PARAMETER: // same as enter progmode
			case CMD_ENTER_PROGMODE_ISP:
				msgLength		=	2;
				msgBuffer[1]	=	STATUS_CMD_OK;
				break;

			case CMD_READ_SIGNATURE_ISP:
			{
				unsigned char signatureIndex	=	msgBuffer[4];
				unsigned char signature;

				if ( signatureIndex == 0 )
					signature	=	(SIGNATURE_BYTES >>16) & 0x000000FF;
				else if ( signatureIndex == 1 )
					signature	=	(SIGNATURE_BYTES >> 8) & 0x000000FF;
				else
					signature	=	SIGNATURE_BYTES & 0x000000FF;

				msgLength		=	4;
				msgBuffer[1]	=	STATUS_CMD_OK;
				msgBuffer[2]	=	signature;
				msgBuffer[3]	=	STATUS_CMD_OK;


			}
			break;

			case CMD_READ_LOCK_ISP:
				msgLength		=	4;
				msgBuffer[1]	=	STATUS_CMD_OK;
				msgBuffer[2]	=	boot_lock_fuse_bits_get( GET_LOCK_BITS );
				msgBuffer[3]	=	STATUS_CMD_OK;
				break;

			case CMD_READ_FUSE_ISP:
			{
				unsigned char fuseBits;

				if ( msgBuffer[2] == 0x50 )
				{
					if ( msgBuffer[3] == 0x08 )
						fuseBits	=	boot_lock_fuse_bits_get( GET_EXTENDED_FUSE_BITS );
					else
						fuseBits	=	boot_lock_fuse_bits_get( GET_LOW_FUSE_BITS );
				}
				else
				{
					fuseBits	=	boot_lock_fuse_bits_get( GET_HIGH_FUSE_BITS );
				}
				msgLength		=	4;
				msgBuffer[1]	=	STATUS_CMD_OK;
				msgBuffer[2]	=	fuseBits;
				msgBuffer[3]	=	STATUS_CMD_OK;
			}
			break;

#ifndef REMOVE_PROGRAM_LOCK_BIT_SUPPORT
			case CMD_PROGRAM_LOCK_ISP:
			{
				unsigned char lockBits	=	msgBuffer[4];

				lockBits	=	(~lockBits) & 0x3C;	// mask BLBxx bits
				boot_lock_bits_set(lockBits);		// and program it
				boot_spm_busy_wait();

				msgLength		=	3;
				msgBuffer[1]	=	STATUS_CMD_OK;
				msgBuffer[2]	=	STATUS_CMD_OK;
			}
			break;
#endif
			case CMD_CHIP_ERASE_ISP:
				eraseAddress	=	0;
				msgLength		=	2;
				msgBuffer[1]	=	STATUS_CMD_OK;
				break;

			case CMD_LOAD_ADDRESS:
#if defined(RAMPZ)
				address	=	( ((address_t)(msgBuffer[1])<<24)|((address_t)(msgBuffer[2])<<16)|((address_t)(msgBuffer[3])<<8)|(msgBuffer[4]) )<<1;
#else
				address	=	( ((msgBuffer[3])<<8)|(msgBuffer[4]) )<<1;		//convert word to byte address
#endif
				msgLength		=	2;
				msgBuffer[1]	=	STATUS_CMD_OK;
				break;

			case CMD_PROGRAM_FLASH_ISP:
			case CMD_PROGRAM_EEPROM_ISP:
			{
				unsigned int	size	=	((msgBuffer[1])<<8) | msgBuffer[2]; // 80
				unsigned char	*p	=	msgBuffer+10;
				unsigned int	data;
				unsigned char	highByte, lowByte;
				address_t		tempaddress	=	address;


				if ( msgBuffer[0] == CMD_PROGRAM_FLASH_ISP )
				{
					// erase only main section (bootloader protection)
					if (eraseAddress < APP_END )
					{
						boot_page_erase(eraseAddress);	// Perform page erase
						boot_spm_busy_wait();		// Wait until the memory is erased.
						eraseAddress += SPM_PAGESIZE;	// point to next page to be erase
					}

					/* Write FLASH */
					do {
						lowByte		=	*p++;
						highByte 	=	*p++;

						data		=	(highByte << 8) | lowByte;
						boot_page_fill(address,data);

						address	=	address + 2;	// Select next word in memory
						size	-=	2;				// Reduce number of bytes to write by two
					} while (size);					// Loop until all bytes written

					boot_page_write(tempaddress);

					//show progress
					send_command(0x15);
					send_command(124*tempaddress/APP_END/2+1);
					send_command(124*tempaddress/APP_END/2+1);
					send_command(0x75);
					send_command(LOADING_Y+2);
					send_command(LOADING_Y+2);
					send_char(0xff);
					//show progress
					boot_spm_busy_wait();
					boot_rww_enable();				// Re-enable the RWW section
				}
				else
				{
					/* write EEPROM */
					do {
						EEARL	=	address;			// Setup EEPROM address
						EEARH	=	(address >> 8);
						address++;						// Select next EEPROM byte

						EEDR	=	*p++;				// get byte from buffer
						EECR	|=	(1<<EEMWE);			// Write data into EEPROM
						EECR	|=	(1<<EEWE);

						while (EECR & (1<<EEWE));	// Wait for write operation to finish
						size--;						// Decrease number of bytes to write
					} while (size);					// Loop until all bytes written
				}
				msgLength	=	2;
				msgBuffer[1]	=	STATUS_CMD_OK;
			}
			break;

			case CMD_READ_FLASH_ISP:
			case CMD_READ_EEPROM_ISP:
			{
				unsigned int	size	=	((msgBuffer[1])<<8) | msgBuffer[2];
				unsigned char	*p		=	msgBuffer+1;
				msgLength				=	size+3;

				*p++	=	STATUS_CMD_OK;
				if (msgBuffer[0] == CMD_READ_FLASH_ISP )
				{
					unsigned int data;

					// Read FLASH
					do {
#if defined(RAMPZ)
						data	=	pgm_read_word_far(address);
#else
						data	=	pgm_read_word_near(address);
#endif
						*p++	=	(unsigned char)data;		//LSB
						*p++	=	(unsigned char)(data >> 8);	//MSB
						address	+=	2;							// Select next word in memory
						size	-=	2;
					}while (size);
				}
				else
				{
					/* Read EEPROM */
					do {
						EEARL	=	address;			// Setup EEPROM address
						EEARH	=	((address >> 8));
						address++;					// Select next EEPROM byte
						EECR	|=	(1<<EERE);			// Read EEPROM
						*p++	=	EEDR;				// Send EEPROM data
						size--;
					} while (size);
				}
				*p++	=	STATUS_CMD_OK;
			}
			break;

			default:
				msgLength		=	2;
				msgBuffer[1]	=	STATUS_CMD_FAILED;
				break;
			}

			/*
			 * Now send answer message back
			 */
			sendchar(MESSAGE_START);
			checksum	=	MESSAGE_START^0;

			sendchar(seqNum);
			checksum	^=	seqNum;

			c			=	((msgLength>>8)&0xFF);
			sendchar(c);
			checksum	^=	c;

			c			=	msgLength&0x00FF;
			sendchar(c);
			checksum ^= c;

			sendchar(TOKEN);
			checksum ^= TOKEN;

			p	=	msgBuffer;
			while ( msgLength )
			{
				c	=	*p++;
				sendchar(c);
				checksum ^=c;
				msgLength--;
			}
			sendchar(checksum);
			seqNum++;


		}
	}



#ifdef _DEBUG_SERIAL_
	sendchar('j');
	//	sendchar('u');
	//	sendchar('m');
	//	sendchar('p');
	//	sendchar(' ');
	//	sendchar('u');
	//	sendchar('s');
	//	sendchar('r');
	sendchar(0x0d);
	sendchar(0x0a);

	delay_ms(100);
#endif



	asm volatile ("nop");			// wait until port has changed

	/*
	 * Now leave bootloader
	 */

	UART_STATUS_REG	&=	0xfd;
	boot_rww_enable();				// enable application section


	asm volatile(
			"clr	r30		\n\t"
			"clr	r31		\n\t"
			"ijmp	\n\t"
	);
	//	asm volatile ( "push r1" "\n\t"		// Jump to Reset vector in Application Section
	//					"push r1" "\n\t"
	//					"ret"	 "\n\t"
	//					::);

	/*
	 * Never return to stop GCC to generate exit return code
	 * Actually we will never reach this point, but the compiler doesn't
	 * understand this
	 */
	for(;;);
}
