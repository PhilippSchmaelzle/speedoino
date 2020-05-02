#include "global.h"


timing 			Millis;
uart 			Serial;
ILI9325 		TFT;
sensing 		Sensors;
akting      	Aktors;
debugging 		Debug;
timer       	Timer;
speedo  		Speedo;
filemanager_v2 	Filemanager_v2;
sd				SD;
configuration	Config;
menu         	Menu;        // pins aktivieren, sonst nix

sprint      	Sprint;
LapTime  		LapTimer;
speedcams   	SpeedCams;
#ifdef DEMO_MODE
speedo_demo Demo;
#endif



void init_speedo(void){
	Millis.init();
	Serial.init(USART1,115200);
	Serial.init(USART2,115200);
	Serial.init(USART3,115200);



	Serial.puts_ln(USART1,"=== Speedoino ===");
	Serial.puts(USART1,GIT_REV);                // print Software release
	Serial.puts(USART1," HW:");
	Serial.puts_ln(USART1,Config.get_hw_version());    // print Hardware release

	// first, set all variables to a zero value
	Sensors.init();
	Speedo.clear_vars();        // refresh cycle
	// read configuration file from sd card
	SD.init();                 // try open SD Card
	Config.read(CONFIG_FOLDER,"BASE.TXT",READ_MODE_CONFIGFILE,"");     	// load base config
	Config.read(CONFIG_FOLDER,"SPEEDO.TXT",READ_MODE_CONFIGFILE,"");    // speedovalues, avg,max,time
	Config.read(CONFIG_FOLDER,"GANG.TXT",READ_MODE_CONFIGFILE,"");   	// gang
	Config.read(CONFIG_FOLDER,"TEMPER.TXT",READ_MODE_CONFIGFILE,"");    // Temperatur
	Config.read_skin();        // skinning
	//	check if read SD read was okay, if not: load your default backup values
	Aktors.check_vars();        // check if color of outer LED are OK
	Sensors.check_vars();        // check if config read was successful
	Speedo.check_vars();        // rettet das Skinning wenn SD_failed von den sensoren auf true gesetzt wird
	Sensors.single_read();    // read all sensor values once to ensure we are ready to show them
	Aktors.init();            // Start outer LEDs // ausschlag des zeigers // Motorausschlag und block bis motor voll ausgeschlagen, solange das letzte intro bild halten
	TFT.init();
	//	pOLED.init_speedo();         // Start Screen //execute this AFTER Config->init(), init() will load  phase,config,startup. PopUp will be shown if sd access fails
	Menu.init();                 // Start butons // adds the connection between pins and vars
	Menu.display();             // execute this AFTER pOLED.init_speedo!! this will show the menu and, if state==11, draws speedosymbols
	Speedo.reset_bak();         // reset all storages, to force the redraw of the speedo
	Config.ram_info();
	Serial.puts_ln(USART1,"=== Setup finished ===");

}


int main(void) {
	init_speedo();


//	const uint8_t Emo2_Jpg_Table[] = {
//	0xFF,0xD8,0xFF,0xE0,0x00,0x10,0x4A,0x46,0x49,0x46,0x00,0x01,0x01,0x01,0x00,0x60,
//	0x00,0x60,0x00,0x00,0xFF,0xDB,0x00,0x43,0x00,0x08,0x06,0x06,0x07,0x06,0x05,0x08,
//	0x07,0x07,0x07,0x09,0x09,0x08,0x0A,0x0C,0x14,0x0D,0x0C,0x0B,0x0B,0x0C,0x19,0x12,
//	0x13,0x0F,0x14,0x1D,0x1A,0x1F,0x1E,0x1D,0x1A,0x1C,0x1C,0x20,0x24,0x2E,0x27,0x20,
//	0x22,0x2C,0x23,0x1C,0x1C,0x28,0x37,0x29,0x2C,0x30,0x31,0x34,0x34,0x34,0x1F,0x27,
//	0x39,0x3D,0x38,0x32,0x3C,0x2E,0x33,0x34,0x32,0xFF,0xDB,0x00,0x43,0x01,0x09,0x09,
//	0x09,0x0C,0x0B,0x0C,0x18,0x0D,0x0D,0x18,0x32,0x21,0x1C,0x21,0x32,0x32,0x32,0x32,
//	0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,
//	0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,
//	0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0xFF,0xC0,
//	0x00,0x11,0x08,0x00,0x78,0x00,0xA0,0x03,0x01,0x22,0x00,0x02,0x11,0x01,0x03,0x11,
//	0x01,0xFF,0xC4,0x00,0x1F,0x00,0x00,0x01,0x05,0x01,0x01,0x01,0x01,0x01,0x01,0x00,
//	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
//	0x0A,0x0B,0xFF,0xC4,0x00,0xB5,0x10,0x00,0x02,0x01,0x03,0x03,0x02,0x04,0x03,0x05,
//	0x05,0x04,0x04,0x00,0x00,0x01,0x7D,0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,0x21,
//	0x31,0x41,0x06,0x13,0x51,0x61,0x07,0x22,0x71,0x14,0x32,0x81,0x91,0xA1,0x08,0x23,
//	0x42,0xB1,0xC1,0x15,0x52,0xD1,0xF0,0x24,0x33,0x62,0x72,0x82,0x09,0x0A,0x16,0x17,
//	0x18,0x19,0x1A,0x25,0x26,0x27,0x28,0x29,0x2A,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,
//	0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,
//	0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,
//	0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,
//	0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,
//	0xB8,0xB9,0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,
//	0xD6,0xD7,0xD8,0xD9,0xDA,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF1,
//	0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFF,0xC4,0x00,0x1F,0x01,0x00,0x03,
//	0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
//	0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0xFF,0xC4,0x00,0xB5,0x11,0x00,
//	0x02,0x01,0x02,0x04,0x04,0x03,0x04,0x07,0x05,0x04,0x04,0x00,0x01,0x02,0x77,0x00,
//	0x01,0x02,0x03,0x11,0x04,0x05,0x21,0x31,0x06,0x12,0x41,0x51,0x07,0x61,0x71,0x13,
//	0x22,0x32,0x81,0x08,0x14,0x42,0x91,0xA1,0xB1,0xC1,0x09,0x23,0x33,0x52,0xF0,0x15,
//	0x62,0x72,0xD1,0x0A,0x16,0x24,0x34,0xE1,0x25,0xF1,0x17,0x18,0x19,0x1A,0x26,0x27,
//	0x28,0x29,0x2A,0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,
//	0x4A,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,
//	0x6A,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x82,0x83,0x84,0x85,0x86,0x87,0x88,
//	0x89,0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,
//	0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xC2,0xC3,0xC4,
//	0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xE2,
//	0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,
//	0xFA,0xFF,0xDA,0x00,0x0C,0x03,0x01,0x00,0x02,0x11,0x03,0x11,0x00,0x3F,0x00,0xEF,
//	0x2E,0x2F,0x14,0x93,0xC8,0xAC,0xBB,0x8B,0xA1,0x9E,0x2B,0x1A,0x5D,0x4C,0x67,0xEF,
//	0x55,0x49,0x35,0x2C,0xF7,0x15,0xE5,0x39,0x5C,0xF6,0xD5,0x33,0x52,0x69,0x04,0xA8,
//	0x43,0x0C,0x8A,0xB1,0xA0,0x3C,0x36,0x71,0x3C,0x06,0x47,0x6D,0xCE,0x58,0x33,0x9C,
//	0xF5,0xED,0x5C,0xE1,0xD4,0x07,0x4C,0xD5,0xCB,0x19,0x7C,0xD6,0xC8,0x35,0x12,0x4E,
//	0x51,0xE5,0x31,0xAD,0x87,0x8C,0x95,0xD9,0xD0,0xF8,0xA6,0x0B,0xBB,0xBF,0x0B,0x5F,
//	0x5B,0xE9,0xF0,0x79,0xF7,0x13,0xC7,0xE5,0xAA,0x06,0x03,0x39,0x38,0x3C,0x9F,0x6C,
//	0xD7,0x9F,0xC1,0xE1,0x99,0x3C,0x29,0xE0,0x8B,0xB8,0xAF,0x24,0x8A,0x3B,0xCB,0xD0,
//	0x77,0x27,0xDE,0xDC,0x7A,0x6D,0x03,0xB8,0x1C,0x73,0xEF,0xEF,0x5D,0xF5,0x9E,0xA2,
//	0xD6,0xC7,0x64,0xA7,0x31,0x9F,0xD2,0xB9,0x7D,0x6A,0xEA,0x3D,0x53,0xC7,0xB0,0xA4,
//	0x99,0x7B,0x2D,0x39,0x1A,0x46,0x45,0x3D,0x76,0xA0,0x71,0x8F,0xAB,0x3A,0x7D,0x76,
//	0xD6,0xF8,0x19,0xCA,0x0D,0xC1,0xFC,0x2B,0x57,0xF2,0x3C,0x7C,0x4D,0x39,0x43,0x4E,
//	0xE6,0x15,0xA7,0x87,0x7C,0x61,0xE2,0x6D,0x09,0x6C,0xA2,0xBB,0x5B,0x4B,0x28,0x51,
//	0x54,0x45,0x21,0x31,0x83,0xC6,0x46,0x70,0x09,0x27,0x04,0x1C,0x1F,0x5A,0xE6,0xB5,
//	0xEF,0x05,0x78,0xC7,0x46,0xB3,0x2B,0x33,0x4D,0x77,0x66,0x9D,0xAD,0xA6,0x77,0x55,
//	0xEF,0xCA,0x1C,0x11,0xD3,0xAE,0x31,0x5F,0x41,0xE9,0xF6,0xE7,0x4E,0xD2,0xE3,0x8D,
//	0xC7,0x9B,0x3E,0x0B,0xC9,0xB7,0xF8,0xE4,0x62,0x59,0xB1,0xE9,0x96,0x27,0x1D,0x80,
//	0xF6,0xAA,0xFA,0x84,0x96,0x96,0xB6,0xB2,0x4D,0xA8,0xDE,0xC7,0x1E,0x41,0xC9,0x2C,
//	0x14,0x01,0xD7,0x68,0xF5,0xAD,0x3F,0xB5,0x2A,0x3A,0x9E,0xEA,0x56,0xEC,0x44,0x61,
//	0x65,0xA9,0xF2,0xAC,0x13,0xCD,0xB8,0xA2,0xBB,0x80,0x7E,0xF0,0x07,0x83,0xDF,0x91,
//	0x5D,0x25,0xB9,0x59,0x61,0x45,0x6C,0x21,0x8D,0x79,0x2D,0x92,0x78,0x3C,0x75,0xE4,
//	0x7D,0x38,0xAD,0x7F,0x16,0x6A,0x9E,0x1E,0x9B,0x51,0x6B,0xCD,0x1C,0x4B,0xF6,0xB6,
//	0xFB,0xEE,0xB1,0x05,0x47,0x1D,0xCF,0x63,0x9E,0xBC,0xE3,0x9F,0xD6,0xB9,0xC1,0x3B,
//	0xDC,0x72,0xEE,0xAC,0xC4,0x11,0x82,0xD8,0xFD,0x4F,0xD6,0xBE,0x8B,0x0D,0x2E,0x78,
//	0x29,0x35,0x66,0x73,0x57,0xBC,0x9E,0x83,0xEE,0xEE,0xE1,0x44,0xDB,0x1B,0x6F,0xC8,
//	0x3E,0xF5,0x90,0xD2,0x3D,0xC4,0xB9,0xE7,0x04,0xD5,0xF5,0xD3,0x80,0x42,0xC1,0xB1,
//	0xCE,0x0E,0x79,0x14,0xAB,0x6A,0x14,0x6E,0x49,0x94,0x8E,0xDC,0x81,0x5D,0x0D,0x36,
//	0xC9,0x83,0x84,0x16,0x83,0x22,0x80,0xA3,0x00,0x3C,0xB4,0x6C,0x67,0x2C,0x33,0x9A,
//	0xB3,0xF6,0x68,0xC0,0x0D,0x21,0xB8,0x77,0xEA,0x70,0x84,0x2E,0x3F,0x2A,0x74,0x51,
//	0x93,0xB5,0x7C,0xC2,0x9F,0x37,0x62,0x78,0x1F,0x95,0x5C,0x16,0x71,0xAB,0x14,0xF3,
//	0x92,0x53,0xD4,0x71,0x9F,0xCC,0xE6,0xB5,0x8D,0x3B,0xEC,0x65,0x3A,0x96,0xDC,0x8E,
//	0x3F,0xB3,0x44,0x06,0x01,0x50,0x39,0x01,0x94,0x06,0xF7,0xC9,0xC5,0x4B,0xE6,0x99,
//	0x22,0x21,0x21,0x91,0xD0,0xE0,0x93,0x92,0x71,0xFA,0x54,0x91,0xE9,0xF0,0x64,0xE2,
//	0x74,0x52,0x3A,0x80,0x06,0x73,0xF8,0xF1,0x52,0x1D,0x3A,0x05,0xF9,0xBC,0xF6,0x23,
//	0xA0,0xFD,0xDA,0x9F,0xD4,0x1A,0xD1,0x46,0xDD,0x0E,0x69,0x54,0x87,0x56,0x57,0xDE,
//	0xBC,0x6E,0x1B,0x73,0xC8,0x01,0xF1,0xFD,0x69,0x8B,0xE5,0x29,0xDB,0xD7,0x3C,0xFD,
//	0xFE,0x6A,0x73,0x02,0xC4,0x32,0x67,0x90,0x03,0xD9,0x46,0x2A,0xBC,0x98,0x6C,0xE6,
//	0x52,0xDD,0x87,0xA8,0xA5,0x2F,0x32,0x62,0xD3,0xD8,0xAF,0x34,0x6B,0xB7,0xFB,0xA7,
//	0xF1,0x35,0x9F,0x23,0xBA,0xFF,0x00,0x10,0x3E,0x99,0xAB,0x57,0x2A,0xA8,0x72,0x33,
//	0xC7,0xA1,0xC5,0x67,0x48,0xD8,0x07,0x39,0xFC,0xB3,0x58,0xCD,0x9D,0xB4,0x63,0xA1,
//	0xE8,0xD2,0x6A,0x2D,0x9E,0xB5,0x01,0xD4,0x5B,0x3D,0x6A,0x8C,0xD9,0x5A,0xAB,0xB9,
//	0x99,0x80,0xE6,0xBE,0x1D,0x23,0xED,0xEE,0x6B,0x0B,0xFC,0x9E,0x4D,0x6D,0x68,0xFA,
//	0xAC,0x92,0xFF,0x00,0xC7,0xB4,0x61,0x90,0x75,0x96,0x46,0xDA,0x9F,0x87,0x76,0xFD,
//	0x07,0xBD,0x72,0x0C,0x86,0x59,0x04,0x44,0x7C,0x83,0x97,0xF7,0xF6,0xAD,0x5B,0x7B,
//	0xB7,0x8E,0x26,0x89,0x0E,0x0B,0x29,0x15,0xA2,0x49,0x19,0x3F,0x78,0xEA,0x63,0xB8,
//	0xD5,0xF5,0x16,0x30,0xC3,0x24,0x38,0x1F,0xF2,0xD0,0xA9,0x40,0xDF,0x45,0xC9,0x3E,
//	0xBD,0xEB,0x9B,0xB7,0xBF,0xBC,0xD0,0x3C,0x72,0x1A,0xED,0x05,0xC1,0x91,0x01,0x7F,
//	0x2D,0x4F,0x29,0x95,0xC9,0x03,0xE8,0x95,0xD2,0xE8,0xF7,0xF1,0x44,0xDF,0x7F,0x19,
//	0x51,0xD0,0xF7,0xC0,0xAC,0xBF,0x11,0x6E,0xB3,0xD5,0x74,0x9D,0x7A,0x15,0xDF,0xE4,
//	0xC9,0xE4,0x48,0x07,0x4C,0x1E,0x06,0x7F,0x5A,0xE8,0xA0,0xD5,0xDA,0x7D,0x55,0x8E,
//	0x2C,0x5D,0x25,0x28,0xFA,0x1D,0x47,0x88,0xBC,0x74,0xB6,0xD1,0x2A,0x69,0x88,0x93,
//	0xBB,0x2E,0x44,0xD2,0x9D,0xB1,0xA7,0x19,0x24,0xF7,0x24,0x02,0x38,0x07,0xBE,0x3D,
//	0x05,0x78,0x9E,0xBD,0xAF,0x6A,0x9A,0xE5,0xDB,0x3D,0xD5,0xEC,0xB3,0xAB,0x1E,0x17,
//	0xEE,0xAB,0x7B,0xED,0x18,0x03,0xA5,0x6B,0x5F,0x47,0x71,0xAE,0xEA,0xA2,0xD6,0xCE,
//	0x09,0x66,0x9A,0x77,0xF9,0x62,0x41,0xF3,0x1F,0x40,0x7B,0x73,0xD4,0x9E,0xC3,0x3D,
//	0x85,0x7A,0xEF,0x81,0xBE,0x14,0xD9,0xE8,0xA9,0x0E,0xA1,0xAD,0x24,0x77,0x5A,0x90,
//	0xC1,0x09,0xD6,0x38,0x4F,0x60,0x07,0x72,0x3D,0x7F,0x2A,0xEF,0xA5,0x4A,0x8E,0x16,
//	0x09,0xDB,0x5F,0xC4,0xF2,0x69,0x53,0x9D,0x59,0x3E,0xC7,0x05,0xE0,0x6F,0x85,0x3A,
//	0x86,0xB4,0xB1,0xDE,0x6A,0xDB,0xAD,0x74,0xF6,0x3B,0x84,0x5B,0x76,0xC9,0x30,0xF5,
//	0xFF,0x00,0x64,0x7B,0x9A,0xF5,0x7B,0x3F,0x00,0xF8,0x7A,0x2F,0xB9,0xA2,0xD8,0xB2,
//	0xC6,0xA0,0x20,0x92,0x30,0xDF,0x89,0x27,0x26,0xBB,0x20,0xAA,0xA9,0x80,0x00,0xE2,
//	0xA2,0x23,0x62,0x1A,0xC2,0xA5,0x79,0x4D,0xEA,0xCF,0x4A,0x9D,0x28,0xC3,0x6D,0xCE,
//	0x0B,0x57,0xF0,0xC7,0x86,0xA4,0x3E,0x5D,0xCE,0x8F,0x05,0xAB,0x60,0x85,0x68,0xD7,
//	0x60,0x3F,0x8A,0xE2,0xBC,0x73,0xC5,0xDA,0x4E,0x93,0xA4,0xDF,0x24,0x7A,0x64,0xD2,
//	0x3C,0xD9,0x39,0x4D,0xFB,0xD4,0xFB,0x67,0xB1,0x1F,0xE7,0xD6,0xBE,0x8F,0x9E,0x28,
//	0xEE,0x11,0xD6,0x58,0x96,0x45,0xFE,0xEB,0x0C,0xD7,0x97,0x6B,0xFA,0x5E,0x89,0x63,
//	0xAD,0x8B,0xAB,0xED,0x26,0x17,0x82,0x0B,0x5B,0xA7,0x86,0x24,0xF9,0x7C,0xD9,0x14,
//	0x2B,0x00,0xC4,0x0E,0x98,0x0F,0xEB,0xD3,0xF3,0xAA,0x35,0x1D,0xF5,0x66,0x93,0x8C,
//	0x54,0x1B,0xB6,0xA7,0x9A,0x69,0xFA,0xAD,0xE6,0xB5,0x73,0x1D,0xA0,0x82,0x33,0x30,
//	0x24,0x47,0xB6,0x20,0x4F,0xFB,0xB8,0x3C,0x1E,0xE7,0x9F,0x4E,0x3A,0x52,0xDE,0x78,
//	0x62,0xF6,0x06,0xCC,0xAA,0xF1,0x02,0x70,0x77,0x74,0x27,0xBE,0x08,0xE8,0x7D,0x8D,
//	0x55,0xB6,0xD5,0x45,0x8F,0x8A,0xD6,0xF2,0x38,0x16,0x2C,0xB7,0xDC,0x45,0xDA,0xA0,
//	0xF6,0xDB,0xD7,0x8E,0x9D,0x49,0xEF,0x5E,0x9F,0xE3,0x8B,0x08,0xEF,0x34,0x4D,0x3A,
//	0xF6,0x3F,0x96,0x46,0xB9,0x40,0x4F,0x6E,0x55,0xB1,0xFF,0x00,0xEB,0xF7,0xAE,0x88,
//	0x57,0xF6,0x73,0x5D,0x99,0xCD,0x52,0x93,0xAB,0x45,0xCB,0x69,0x23,0xCE,0xCE,0x89,
//	0x75,0x61,0x1A,0x3B,0x34,0x73,0x44,0xE4,0x61,0x87,0x3C,0x77,0xC7,0xAF,0x7E,0x38,
//	0x35,0x34,0x53,0x59,0xCC,0x3C,0xA6,0x0F,0x04,0x99,0xC1,0xDB,0x8E,0xBF,0x46,0xE9,
//	0xF8,0x1A,0xE8,0x6F,0xEC,0x6F,0x2D,0xA3,0x2A,0xE8,0xCB,0xBB,0x01,0xE1,0x62,0x70,
//	0x4F,0x4C,0x8F,0x5E,0x06,0x2B,0x02,0xEC,0xAC,0x40,0x89,0xE0,0x75,0x7C,0x77,0x5C,
//	0x1E,0x99,0xE0,0xF7,0xE7,0x8A,0xF7,0x63,0xC9,0x38,0xA9,0x41,0xE8,0xCF,0x99,0x97,
//	0x3B,0x93,0x85,0x4D,0xD7,0x6F,0xF2,0x2E,0xC1,0x65,0xA7,0x85,0xFD,0xF4,0xF2,0x13,
//	0xE8,0x78,0x3F,0x50,0x0D,0x55,0xBD,0xB7,0xB3,0x8D,0x5B,0x6C,0x9C,0x37,0x20,0x15,
//	0xF9,0x87,0xD7,0x15,0x9C,0x43,0xAA,0x33,0x5B,0xBB,0x10,0x09,0x1B,0x73,0x83,0x8E,
//	0x78,0xF4,0x3D,0x2A,0x9B,0xDE,0xE3,0x2A,0xEA,0x55,0xF9,0xCA,0xFD,0xC3,0xFF,0x00,
//	0xD7,0xA9,0x69,0x20,0x85,0x09,0xB7,0xA4,0xAE,0x36,0xE5,0x17,0x24,0xAB,0x60,0x76,
//	0xE7,0x15,0x99,0x27,0x27,0x89,0x0F,0xE7,0xFF,0x00,0xD7,0xA9,0xE5,0x98,0x3A,0xF4,
//	0x23,0xDF,0x03,0xF9,0x8A,0xAE,0xE7,0xB1,0x1F,0xCE,0xB9,0xA7,0xA9,0xEA,0x52,0x8B,
//	0x4B,0x53,0xD2,0x2E,0x6D,0x72,0xDD,0x2A,0xB2,0x59,0x10,0x4B,0x62,0xBA,0x29,0xAD,
//	0xC0,0x3D,0x32,0x6A,0x9C,0x90,0x09,0x15,0xA3,0x3F,0x74,0x8D,0xA7,0x1F,0xAD,0x7C,
//	0x8A,0x47,0xD4,0xB9,0x19,0x10,0xDA,0xB9,0xB7,0x47,0x60,0xC0,0xBF,0xCE,0x73,0xDB,
//	0x3C,0xE3,0xF0,0xE9,0xF8,0x55,0x8B,0x7B,0x47,0x6F,0x98,0x0C,0x62,0xB5,0x5E,0x01,
//	0xE4,0xE0,0xFA,0x55,0x88,0x12,0x2B,0x78,0xBC,0xC9,0x58,0x2C,0x60,0x8E,0x4F,0x15,
//	0x76,0xBB,0x26,0xF6,0x43,0x74,0x4D,0x22,0x6B,0xB5,0x0C,0x0E,0x02,0x9C,0x67,0xE9,
//	0x5D,0x5D,0xFF,0x00,0x87,0xE1,0x97,0x4E,0x5B,0x56,0x02,0x57,0xB9,0x65,0x51,0x1F,
//	0xF1,0x67,0x83,0x9C,0x7E,0x15,0xCB,0x5D,0x78,0x9C,0x68,0x7A,0x56,0x20,0x41,0xE7,
//	0x4C,0x4F,0x92,0xA4,0x65,0xD8,0xF3,0xD1,0x7B,0x01,0xEA,0x71,0xD6,0xBB,0x7F,0x87,
//	0x96,0x17,0xD2,0x69,0xC3,0x5C,0xD6,0x19,0x9E,0xEE,0xE4,0x7E,0xE5,0x1C,0xE7,0xCA,
//	0x8F,0xDB,0xEB,0xED,0xC6,0x31,0x8C,0x64,0xD6,0x90,0x83,0x5E,0xF1,0xCF,0x52,0xA5,
//	0xDF,0x29,0xA9,0xE0,0x9F,0x04,0x5A,0x78,0x53,0x4E,0x42,0xDB,0x66,0xD4,0x5D,0x00,
//	0x9A,0xE0,0x8E,0x73,0xE8,0xBE,0x83,0xF9,0xF7,0xF6,0xEA,0x8B,0x00,0x2A,0x31,0x30,
//	0xC7,0x3C,0x52,0xC8,0x85,0x97,0x20,0xD6,0xB2,0x9B,0x93,0xBB,0x31,0x8C,0x14,0x55,
//	0x90,0x86,0x41,0xEB,0x51,0xBB,0xE5,0x71,0x50,0x49,0x1C,0xC3,0xA1,0xCD,0x55,0x32,
//	0xCE,0x1C,0xA9,0x43,0xF5,0xAC,0xF9,0xAC,0x6A,0xA2,0x8B,0x3B,0x09,0xCD,0x71,0xBE,
//	0x31,0xD1,0x65,0xBD,0xB5,0x12,0xDB,0x81,0xF6,0x88,0x5B,0xCC,0x8C,0x30,0xE0,0x9E,
//	0xE0,0xFB,0x11,0x90,0x7D,0x8D,0x77,0x16,0xC0,0x95,0xC9,0x18,0xA8,0xEE,0x2D,0xC4,
//	0x84,0x8C,0x64,0x55,0x27,0x70,0xBE,0xB6,0x3E,0x74,0x9B,0xC3,0xE9,0xAA,0x5D,0x45,
//	0xF6,0x5B,0x07,0x81,0xD9,0x8E,0x03,0x92,0x40,0xE7,0x1F,0x90,0x39,0xFC,0xAB,0xBA,
//	0xF1,0x0D,0x94,0xB7,0x16,0xF6,0x36,0x09,0x19,0x68,0xA1,0x2A,0xFD,0x0F,0x51,0xD3,
//	0xF9,0x63,0xFE,0x04,0x6B,0xD0,0xA1,0xD0,0xAD,0x23,0x2D,0xE5,0x5B,0xAA,0x33,0x36,
//	0xE6,0x3E,0xB5,0x05,0xFE,0x9B,0x6E,0x09,0x25,0x41,0x61,0x43,0x52,0xB1,0xA7,0x34,
//	0x52,0xB2,0x47,0x27,0xE2,0x3B,0x25,0x9F,0x45,0x6B,0x8F,0x9B,0x74,0x51,0x93,0xB8,
//	0x1E,0x7F,0xFD,0x75,0xE7,0x73,0xB2,0x5F,0x5B,0x6F,0x65,0x25,0x88,0x39,0x60,0x76,
//	0x90,0x73,0x8F,0xE7,0x9E,0xB5,0xEB,0x5A,0x84,0x71,0xCB,0xA7,0xCB,0x01,0x03,0x0C,
//	0x84,0x11,0x5E,0x2E,0x66,0x6D,0x3F,0x59,0x96,0xDA,0x47,0x64,0x47,0x6C,0x6E,0xCE,
//	0xDF,0x9B,0xD4,0xFB,0x11,0x8F,0xE7,0x5E,0xA6,0x0A,0xBC,0x95,0x37,0x15,0xAB,0x5A,
//	0xD8,0xF1,0xB3,0x0C,0x24,0x2A,0x55,0x8C,0xDE,0x97,0xD2,0xFE,0x65,0x09,0x60,0xF2,
//	0xC9,0xC3,0x9C,0xF6,0x65,0x1C,0x7A,0xFE,0x06,0xB2,0xEE,0xCC,0xD1,0x92,0xB2,0x22,
//	0xB0,0xED,0xB8,0x67,0x15,0xBB,0xA8,0xC2,0x55,0xCB,0xA1,0xD8,0x41,0xC1,0x42,0x30,
//	0x7F,0x0A,0xC6,0x96,0x6C,0xA9,0x04,0x63,0x3D,0x8F,0x23,0x15,0xDD,0x4F,0x15,0x0A,
//	0x88,0xE1,0x78,0x6A,0xB4,0x65,0xEF,0x2B,0x99,0x12,0x6D,0xCF,0x00,0xAF,0xD0,0xE4,
//	0x54,0x47,0x3F,0x5A,0xB7,0x2A,0x03,0xC8,0xE3,0xE9,0x55,0xD9,0x31,0xFF,0x00,0xD6,
//	0xA2,0x4A,0xFB,0x1D,0x31,0x67,0xB3,0x5C,0xCD,0xB7,0x3D,0xAA,0xAC,0x4D,0x90,0x0B,
//	0x1C,0x13,0x5E,0xE6,0xD6,0x90,0xC8,0xB8,0x2A,0xA7,0xEB,0x59,0xB7,0x5E,0x1F,0xD3,
//	0x9B,0x73,0x3D,0x85,0xA9,0xE0,0xFC,0xE6,0x21,0xC5,0x7C,0xCF,0xB3,0x67,0xBC,0xAB,
//	0x27,0xA1,0xE4,0x13,0x4E,0x91,0xA8,0x7C,0x16,0xE7,0x0A,0xA0,0x64,0x93,0xE8,0x2A,
//	0x93,0xCE,0x55,0x7E,0xD5,0x72,0xC3,0x72,0x9C,0x46,0xBD,0x40,0x3E,0x8A,0x3F,0x89,
//	0xB3,0xED,0xF9,0x0A,0xED,0x35,0x2F,0x0C,0xD9,0xDD,0x5C,0x4A,0x90,0x1F,0xB3,0x48,
//	0x32,0x11,0x86,0x59,0x7F,0x2C,0xFF,0x00,0x2C,0x56,0x08,0xD2,0x4E,0x84,0x5E,0x5B,
//	0xA8,0xC4,0x8E,0xAB,0xB5,0x66,0x61,0x9C,0x93,0xD4,0x8F,0x4F,0x40,0x3E,0xBD,0x68,
//	0x8D,0xBA,0x84,0x9B,0xE8,0x62,0xE8,0xFA,0x55,0xFE,0xB1,0xAB,0xD8,0x79,0x9F,0x29,
//	0xD4,0x65,0xDB,0x9E,0xAE,0x90,0x27,0x27,0x1D,0x80,0xC7,0xE2,0x49,0xFA,0x57,0xD0,
//	0x48,0x89,0x0C,0x29,0x12,0x28,0x54,0x40,0x15,0x54,0x0E,0x00,0x15,0xE4,0x5F,0x08,
//	0x94,0x5E,0xEA,0x17,0x37,0xCE,0x77,0x25,0xA5,0xB4,0x76,0xF0,0xE7,0xB0,0x6C,0xB9,
//	0xFE,0x82,0xBD,0x56,0xE6,0xE0,0x43,0x19,0x7E,0xC3,0x9A,0xDA,0x7A,0x68,0x73,0xD3,
//	0x57,0xD4,0xC6,0xF1,0x96,0xAE,0xBA,0x56,0x8D,0x33,0xC6,0x4F,0x9D,0x8C,0xA8,0x56,
//	0xC1,0x18,0xE6,0xAD,0xF8,0x5B,0xC5,0x76,0x9E,0x21,0xD2,0x92,0x58,0x81,0x59,0x36,
//	0x06,0x64,0x27,0x90,0x0F,0xF9,0xC5,0x70,0x9E,0x26,0xF1,0x1D,0xAD,0xD6,0x8D,0x2D,
//	0xB3,0x24,0x8D,0x72,0xD2,0x30,0xDC,0x48,0xDA,0x07,0xB7,0x73,0xDF,0xF3,0xAA,0x1E,
//	0x14,0xBF,0xB5,0xD3,0x26,0xD3,0xA4,0x87,0x2A,0xE4,0x34,0x53,0x45,0x83,0xCE,0x0F,
//	0x53,0xF5,0x04,0x74,0xFE,0xEF,0xAD,0x37,0xCA,0xA5,0x64,0xCF,0x6D,0xE5,0xFF,0x00,
//	0xB9,0x49,0xAD,0x75,0xFE,0xBD,0x19,0xEC,0xAC,0x58,0xA9,0x2A,0x32,0x7E,0x94,0xDB,
//	0x63,0xE7,0xC1,0x92,0x85,0x58,0x12,0x08,0x23,0xD2,0xB1,0x2C,0xBC,0x46,0x65,0x84,
//	0x9F,0x2C,0x8F,0x4F,0x7A,0x92,0x3F,0x10,0xDB,0xF9,0x6C,0xDE,0x68,0x07,0xF8,0x81,
//	0x3C,0x8A,0xC9,0x4A,0x2D,0x9E,0x4B,0xA7,0x25,0xA1,0xA9,0x24,0xEB,0x11,0xC1,0x20,
//	0x55,0x39,0xF5,0x88,0x60,0x52,0x4B,0x28,0x1E,0xA6,0xB8,0xBD,0x7B,0xC5,0xF6,0xF6,
//	0xDB,0x8A,0xCA,0x09,0xF4,0x06,0xBC,0xCB,0x5B,0xF1,0xBD,0xCD,0xC9,0x75,0x8A,0x42,
//	0x07,0x4C,0x8A,0x5C,0xD7,0x76,0x89,0xB2,0xA2,0x92,0xBC,0x8F,0x5C,0xD5,0x3C,0x7F,
//	0x69,0x66,0x0A,0x99,0x94,0x37,0xA0,0x35,0x85,0x07,0x8E,0x13,0x50,0x94,0x88,0xD8,
//	0x1F,0xA9,0xAF,0x0C,0xB9,0xBF,0x9A,0xE1,0xCF,0x98,0xCD,0x9C,0x13,0x90,0x7F,0x9D,
//	0x49,0x61,0x72,0xC8,0xCA,0xF1,0x5C,0x85,0x60,0x40,0x0B,0x21,0x2B,0xBB,0xF1,0xE8,
//	0x3F,0x12,0x2B,0x65,0x87,0xA8,0xD5,0xEE,0x60,0xF1,0x14,0xA0,0xF9,0x6C,0x7D,0x00,
//	0xF7,0x6F,0x71,0x16,0x77,0x00,0x71,0xC1,0x1D,0x2B,0xCB,0xFC,0x55,0x02,0x9B,0xEF,
//	0x30,0x8C,0x48,0xA3,0x0E,0x3D,0x47,0xA8,0xF5,0xEF,0x53,0x69,0xDE,0x28,0xB9,0x47,
//	0x58,0x25,0xCA,0x39,0x19,0x1B,0x87,0xCA,0xE3,0xD4,0x1F,0xEB,0x4D,0xD6,0x6E,0x96,
//	0xE5,0x77,0xB2,0x03,0x22,0xFD,0xE5,0x04,0x12,0x3F,0xFA,0xD5,0xBE,0x1E,0x9D,0x5A,
//	0x6D,0x4E,0x2A,0xE6,0x15,0xEA,0xD1,0xA9,0x17,0x4E,0x4E,0xD7,0xD8,0xCB,0x4B,0x91,
//	0x77,0x02,0xDB,0x5C,0x38,0xF3,0x82,0xFE,0xE6,0x6E,0xD2,0x8F,0x43,0xEF,0xFE,0x7A,
//	0xF2,0x70,0xEF,0x17,0xC9,0x90,0xEE,0x51,0xF5,0x15,0x72,0xE2,0xDB,0x74,0x7B,0xAD,
//	0xDB,0xEF,0x60,0xF9,0x67,0xA6,0x7D,0xBD,0x2B,0x2E,0x5B,0xC6,0x61,0xB2,0x65,0xCB,
//	0x0F,0x5E,0xB5,0xD7,0x1A,0x51,0x9B,0xE7,0xA6,0xFE,0x47,0x2B,0x9C,0xA2,0xB9,0x26,
//	0xBE,0x64,0x66,0x65,0x23,0x90,0x0F,0xE1,0x55,0x24,0x6C,0xB6,0x46,0x05,0x3C,0x95,
//	0x27,0x8C,0x7E,0x74,0xD2,0xB8,0x03,0x24,0x7D,0x05,0x6F,0x1A,0x6C,0xC5,0xB4,0x7D,
//	0x72,0x62,0xD6,0xD7,0xFE,0x7D,0x0F,0xD2,0x46,0xFF,0x00,0xE2,0x6B,0x2A,0xFA,0xFF,
//	0x00,0x5D,0x84,0x38,0x93,0x4C,0x91,0xE3,0x1F,0xC7,0x1B,0xAB,0x03,0xF8,0x67,0x3F,
//	0xA5,0x6F,0x36,0xA0,0x08,0xE5,0xB1,0x55,0xEE,0x2F,0x11,0x6D,0xDD,0x99,0xD7,0x1B,
//	0x7B,0x9F,0xAF,0xFF,0x00,0x5A,0xBE,0x7D,0xA5,0xDC,0xF6,0x63,0x37,0xD5,0x23,0x81,
//	0x9B,0xC5,0x10,0xDA,0xDE,0x05,0xBA,0x86,0x68,0x64,0x6C,0xE0,0x4A,0x85,0x0F,0xEB,
//	0x56,0x3F,0xB6,0x6D,0xEF,0x73,0xE5,0xB2,0x36,0x7A,0x82,0x78,0x23,0xDE,0xB9,0x5F,
//	0x12,0xEB,0x49,0x77,0xAA,0x4D,0x03,0x2A,0xCB,0x13,0x3E,0x36,0x30,0xC8,0xFF,0x00,
//	0xF5,0xD7,0x2F,0x77,0x78,0x34,0xC9,0xB7,0x40,0xEC,0xF0,0x1E,0xA8,0x4F,0xCC,0xBF,
//	0x43,0xDC,0x7F,0x9E,0x6B,0x24,0x9B,0xD1,0x1D,0x2D,0x45,0x6A,0xD1,0xE8,0x9A,0x2E,
//	0xAF,0xA5,0xF8,0x6A,0xFA,0xEB,0x62,0xAD,0xB5,0xAC,0xF8,0x2C,0x57,0xEE,0xA9,0xC9,
//	0xC1,0xC7,0x61,0xFA,0x56,0x9D,0xE6,0xBB,0x75,0xAA,0x16,0x8A,0x29,0x4D,0xAD,0x91,
//	0xC0,0xDE,0x5B,0x0D,0x2F,0xA1,0x18,0xC9,0x00,0xFB,0x0E,0x7D,0x47,0x4A,0xF2,0x8B,
//	0xD9,0x2E,0x6E,0x34,0xD6,0x99,0x15,0xD6,0x39,0xE2,0x22,0x36,0x91,0x70,0xAE,0x0E,
//	0x47,0x04,0xFE,0x3F,0x88,0xAA,0xFA,0x4E,0xAB,0xAA,0x78,0x62,0xC5,0x92,0xEE,0xD9,
//	0x96,0x19,0x58,0x79,0x6C,0xC0,0x67,0xA7,0xAF,0xA7,0x4C,0x76,0xEB,0x8A,0xD3,0xD9,
//	0x54,0xA9,0x07,0x67,0xA8,0xA3,0x5E,0x95,0x19,0xAB,0xC7,0x4E,0xFD,0x0F,0x54,0x7B,
//	0x9D,0x0F,0x46,0x56,0xF3,0xDF,0x6C,0x80,0x12,0x54,0x62,0x32,0xE3,0xD4,0x63,0x2C,
//	0xDF,0xF7,0xD5,0x70,0x5A,0xEF,0x8A,0x5A,0xE2,0xF6,0x25,0xD3,0x63,0xD9,0x0C,0x25,
//	0x9B,0x25,0x79,0x73,0xEB,0xC9,0x27,0xA6,0x78,0x26,0xB2,0xB5,0x2F,0x13,0xDB,0xEA,
//	0x56,0xCA,0xC5,0xDD,0x2E,0x23,0x39,0x53,0xF5,0xEB,0x58,0xA3,0x51,0x73,0x32,0xC9,
//	0x91,0xBD,0x4E,0x41,0xA5,0x4B,0x09,0x28,0xBE,0x66,0x8E,0xBF,0xAF,0xD2,0x94,0x74,
//	0x95,0xA4,0x7A,0x2D,0xB7,0xC4,0x59,0xE1,0xD0,0x8D,0xBA,0xC2,0xBF,0x69,0x6E,0x3C,
//	0xDF,0x55,0xAE,0x72,0x3D,0x43,0x56,0x92,0x46,0xB8,0x2F,0x21,0x8D,0xCF,0x20,0x55,
//	0x16,0xB7,0xB9,0x9B,0xFD,0x32,0x58,0x4A,0x24,0xA7,0x7E,0x40,0xE3,0x9A,0xF4,0xBF,
//	0x0A,0xD9,0xDA,0xC9,0xA5,0xAC,0x52,0x22,0xB7,0xC8,0x0F,0x3E,0xB4,0x49,0x24,0x63,
//	0x52,0x6A,0x52,0x72,0x89,0xE6,0x97,0xD2,0xCB,0x33,0x9F,0x9C,0x9C,0xF5,0x2D,0x54,
//	0xC4,0x05,0xCE,0xD5,0x52,0xCC,0x6B,0xB5,0xF1,0x76,0x8B,0x1D,0xA5,0xE1,0xB8,0xB7,
//	0x8C,0x2C,0x52,0x75,0x55,0x1F,0x74,0xD2,0x68,0x5A,0x52,0x5B,0xC7,0xF6,0x9B,0x8C,
//	0x79,0x8D,0xF7,0x54,0xF6,0xA2,0x33,0xB2,0xB2,0x13,0x8F,0x36,0xAC,0xE5,0xB4,0xED,
//	0x05,0xAE,0x27,0x32,0x5C,0xC6,0xCB,0x10,0x3D,0x0F,0x1B,0xAB,0x36,0x4D,0x2E,0x79,
//	0xE5,0x78,0x2D,0x22,0x57,0xF2,0xC1,0x55,0xC7,0x46,0x19,0x3F,0x36,0x49,0xEB,0x5E,
//	0xAF,0x25,0x98,0xBD,0xB6,0x64,0x88,0xAA,0x3E,0x38,0x27,0xA6,0x6B,0x1A,0xDA,0xC9,
//	0x6C,0x43,0x2B,0xAA,0xEE,0xCF,0xCC,0xA4,0x0E,0x0D,0x39,0x62,0xE7,0x49,0x5C,0xD7,
//	0x0F,0x95,0xD2,0xC5,0xC9,0xA9,0x3B,0x5B,0xB1,0xC6,0xD8,0xE9,0xF7,0xD7,0xF0,0xC9,
//	0x6C,0x11,0xA2,0x9E,0x10,0x0F,0x96,0xE3,0x19,0xCF,0x19,0xC1,0xFA,0x56,0xFD,0x84,
//	0x17,0x69,0x62,0x90,0xDF,0x5B,0x92,0xCD,0x21,0xDC,0xC0,0x92,0x50,0xEE,0xC0,0xCE,
//	0x7D,0xBB,0x8E,0xD8,0xAD,0x79,0x27,0x54,0x01,0xC2,0x83,0xB3,0x3C,0x77,0xC1,0xEB,
//	0xFD,0x0F,0xE0,0x2A,0x1B,0x9B,0xDD,0xEB,0xB7,0xB8,0x1B,0x73,0x58,0x54,0xC7,0x54,
//	0x9E,0xDA,0x1E,0xAE,0x1B,0x24,0xA3,0x45,0xDD,0xEB,0xD1,0xDE,0xDA,0xEB,0xD8,0xC8,
//	0xD6,0x6C,0x6D,0xEC,0xAD,0x9A,0x6C,0x33,0x01,0x26,0xD2,0x15,0xB0,0x70,0x79,0x04,
//	0xFB,0x75,0x15,0xCA,0xDD,0xCA,0xB3,0x70,0x1C,0x36,0x07,0x1B,0x97,0x18,0xFC,0x6B,
//	0xB0,0xBF,0x7F,0xB4,0x5A,0x4B,0x19,0xE8,0xE9,0xCF,0xD7,0xA8,0xFE,0x55,0x4E,0x0F,
//	0x09,0xC7,0xE4,0xA3,0x33,0xAB,0xCA,0x46,0x4A,0xE0,0xE0,0x13,0xD8,0x60,0x8A,0xF4,
//	0xB0,0xD9,0xAC,0x15,0x2F,0xF6,0x85,0x77,0xDF,0xA9,0xE3,0x63,0xB8,0x76,0x6F,0x11,
//	0xFE,0xCB,0xA4,0x5A,0xDB,0xA5,0xCE,0x31,0x94,0x83,0xDB,0xF0,0xA6,0x73,0xEB,0x5A,
//	0xDA,0xC6,0x9E,0x96,0x13,0xED,0x4E,0x47,0x46,0xDB,0xC8,0xCF,0xB5,0x66,0x94,0x6D,
//	0xB9,0x23,0x03,0xDF,0x8A,0xF5,0xA9,0x54,0x85,0x68,0x29,0xC3,0x63,0xE7,0xF1,0x14,
//	0x27,0x87,0xA8,0xE9,0x54,0xDD,0x1F,0x49,0xDD,0xE8,0x9E,0x2A,0x40,0x5A,0x31,0x65,
//	0x27,0xB2,0xDC,0x91,0xFC,0xD2,0xB9,0xBD,0x4A,0xDF,0xC6,0x42,0x37,0x57,0xD2,0xC3,
//	0xA0,0x07,0x25,0x2E,0x54,0xFF,0x00,0x4A,0xE9,0xAE,0x7C,0x59,0x10,0x18,0xF3,0x31,
//	0xF8,0xD6,0x55,0xC7,0x8B,0xA2,0xFB,0x3C,0x80,0xC9,0x96,0x39,0xEA,0x3A,0xD7,0xCD,
//	0x7B,0x87,0xB5,0x17,0x3E,0xC8,0xF2,0xAB,0xCB,0xE4,0xB7,0xBB,0x68,0xAF,0x2D,0x66,
//	0x82,0x60,0x7E,0x6C,0xF3,0x8A,0xA1,0x71,0x3D,0xBC,0xEA,0x5E,0x39,0x73,0x8E,0x70,
//	0x6A,0xD7,0x88,0x2F,0x56,0xF7,0x54,0x9A,0xEA,0x79,0x11,0x4B,0x74,0x5C,0xF3,0xF9,
//	0x56,0x04,0x3A,0x84,0x96,0x57,0x0F,0x25,0xAB,0x95,0xDC,0xBB,0x18,0x82,0x46,0x47,
//	0xA5,0x76,0x50,0xA1,0x19,0xD9,0xEC,0x72,0xE2,0x31,0x0E,0x17,0x5B,0x9D,0x77,0x86,
//	0x7C,0x47,0x7D,0xE1,0xFD,0x39,0x84,0xF7,0x98,0xB5,0x99,0x5D,0x61,0xB7,0xDF,0x97,
//	0x50,0x08,0x6F,0xBB,0xD8,0x12,0x4E,0x33,0x81,0xF3,0x31,0xEE,0x73,0x85,0xAC,0xDF,
//	0xDF,0x6A,0x77,0x65,0xE6,0x53,0xF3,0xB9,0xD8,0xCC,0x40,0x04,0x1E,0x7D,0x71,0xFA,
//	0xD6,0x5F,0xDA,0x22,0x69,0x99,0xA4,0x52,0xEB,0xD8,0x13,0x80,0x3F,0x2A,0xB3,0x6B,
//	0x70,0x81,0x98,0x47,0x3F,0x97,0x91,0xFE,0xAE,0x41,0xB9,0x1B,0xDB,0xDA,0xBD,0x15,
//	0x18,0xC1,0xDD,0x1E,0x7A,0x5C,0xFF,0x00,0x10,0x8B,0x65,0x0C,0x64,0x2D,0xDA,0x4F,
//	0x16,0x4F,0xDF,0x04,0x15,0xFC,0xC0,0x35,0x64,0xE9,0x76,0xA9,0xD5,0xA6,0x1D,0xFA,
//	0x8E,0x7F,0x4A,0x16,0xEA,0x38,0xE4,0x31,0x3E,0x22,0x3D,0x0A,0x9F,0x9D,0x31,0xC7,
//	0x4E,0xE3,0xE9,0x52,0xC5,0x31,0xDD,0x1E,0xDC,0x34,0x6A,0x08,0xDA,0x8F,0x9E,0x0F,
//	0x3D,0x38,0x3C,0x50,0xE0,0xE5,0xD4,0xDA,0x0E,0x11,0x7B,0x5C,0xB2,0x97,0x92,0x47,
//	0x6C,0x2D,0xD6,0xEA,0xE0,0xC6,0x06,0x00,0xCA,0xE3,0x1F,0x95,0x5D,0xB4,0xD5,0xB5,
//	0x1B,0x52,0xAF,0x6F,0x7C,0x54,0x77,0x0C,0xB9,0xFF,0x00,0x0A,0xCD,0x56,0x86,0x42,
//	0x37,0x62,0x29,0x47,0x50,0x78,0x19,0xCF,0xBD,0x39,0xE0,0x9B,0x3C,0x61,0x87,0x4C,
//	0x83,0x5C,0xF2,0xC3,0xF9,0x1D,0x51,0xAB,0x16,0xAE,0x99,0xB9,0x7B,0xE2,0xAB,0xFB,
//	0x94,0x51,0x27,0x92,0xF8,0xEA,0x4A,0xF5,0xAA,0x23,0x5E,0x9A,0x4B,0x85,0x5B,0x99,
//	0xBC,0xB8,0xCF,0x19,0x8B,0x9C,0x7D,0x72,0x3F,0x95,0x52,0x6B,0x47,0x72,0x00,0x61,
//	0x9F,0x4C,0xD6,0x8D,0x85,0x82,0x46,0xA9,0x24,0xF6,0x3B,0x94,0x72,0x65,0x24,0x30,
//	0x23,0xFD,0xD3,0xDB,0xE8,0x2B,0x0A,0xF4,0xE9,0xD2,0x87,0x34,0x91,0xD9,0x82,0x55,
//	0x2B,0xD6,0x51,0x83,0x5F,0x34,0xDA,0xFC,0x0D,0xDB,0x3D,0xB7,0x50,0x97,0xB4,0xD5,
//	0x65,0x61,0xDF,0x6B,0x29,0xDB,0xF5,0x18,0xC8,0xFC,0x6A,0x29,0x5A,0x78,0x37,0x43,
//	0x3C,0xE6,0x66,0xEA,0x24,0x2B,0xB4,0x91,0xF9,0x9A,0x94,0x43,0x60,0x88,0x1D,0x6D,
//	0xED,0xCF,0x19,0x57,0x44,0x00,0xFE,0x04,0x73,0x55,0xAE,0x86,0xF1,0xF2,0x48,0xD9,
//	0x5E,0x40,0x73,0xBB,0xF5,0xEB,0xFC,0xEB,0xC5,0x94,0xF9,0xB4,0xBE,0x9E,0x67,0xD7,
//	0x61,0xF0,0xCA,0x9B,0xE7,0xE5,0x57,0xF2,0xFF,0x00,0x22,0x33,0x3E,0x01,0x04,0xD5,
//	0x69,0xE4,0x0A,0xB8,0x0C,0x4F,0xBE,0x31,0x50,0xBB,0x96,0x38,0x3C,0x1F,0x7A,0x72,
//	0x21,0x95,0xE3,0x88,0xB8,0x25,0x88,0xE0,0x67,0x8E,0xBD,0x78,0xFF,0x00,0x1E,0xB5,
//	0x71,0x82,0x46,0xF3,0x9A,0x5B,0x0C,0x96,0x5D,0xDB,0x54,0x7A,0x0C,0xD6,0x8B,0x5D,
//	0x9B,0x4B,0x65,0x24,0x93,0x23,0x28,0x0A,0xB5,0x40,0xC3,0x09,0x52,0x45,0xCA,0x09,
//	0x59,0xB0,0x91,0x82,0x4B,0x38,0xF5,0x00,0x0E,0x9F,0x5C,0x66,0x87,0x06,0xE6,0x4C,
//	0xBB,0x90,0xC7,0x0B,0x95,0xE4,0xFD,0x07,0xA5,0x37,0x04,0xED,0x7D,0x8C,0x7D,0xB5,
//	0xD4,0xB9,0x77,0x33,0xF5,0x88,0x81,0xD2,0xDE,0xE1,0x9B,0xF7,0x9B,0xBA,0xFA,0x9C,
//	0x80,0x7F,0xC3,0xF0,0xAE,0x6A,0x42,0xEA,0x70,0xE0,0x82,0x40,0x3C,0xFB,0xF3,0x5D,
//	0x7E,0xA3,0x65,0xF6,0xA4,0x8A,0x12,0x3C,0xBB,0x74,0xC7,0x03,0x92,0xD8,0xE0,0x2A,
//	0xFD,0x3B,0x9F,0x5A,0x62,0x78,0x72,0x19,0x5E,0x4B,0x9B,0xE6,0x75,0x27,0x02,0x38,
//	0x54,0x60,0x60,0x70,0x01,0x3D,0x7A,0x57,0xA7,0x86,0xC6,0xC6,0x8D,0x3E,0x59,0x33,
//	0xE6,0x33,0x0C,0xAE,0xBE,0x2E,0xB7,0x3D,0x38,0xF6,0x5F,0xE6,0xFF,0x00,0x43,0xB3,
//	0xD4,0x7C,0x20,0x21,0xC9,0xFF,0x00,0x84,0x82,0x43,0xFF,0x00,0x02,0x23,0xFA,0xD7,
//	0x1B,0xAC,0x68,0x5F,0x66,0x02,0x46,0xB9,0x13,0x67,0xB9,0x62,0x4F,0xEB,0x57,0x14,
//	0x78,0x93,0x55,0x98,0x2D,0xBD,0x96,0xD7,0x3D,0x1A,0x67,0xDA,0x3F,0x5C,0x0A,0xD2,
//	0x93,0xE1,0xBF,0x8A,0x75,0x18,0xFC,0xCB,0x9D,0x67,0x4A,0x44,0xC6,0x40,0x37,0x07,
//	0x8F,0xFC,0x76,0xB0,0x8A,0xE5,0x7E,0xF4,0x92,0x3C,0xD9,0xE2,0x60,0xD5,0x96,0xBF,
//	0x23,0xCF,0x2E,0xE3,0x8A,0x35,0xDA,0xB8,0x2D,0x9E,0xDD,0x6A,0x91,0x1C,0xF4,0xC5,
//	0x7A,0x3A,0xFC,0x1E,0xD7,0x24,0x24,0x45,0xAB,0x68,0x72,0x7B,0x0B,0xB3,0xFF,0x00,
//	0xC4,0xD4,0x57,0x1F,0x06,0xFC,0x5B,0x12,0xE5,0x23,0xB3,0x9F,0xDA,0x39,0xFF,0x00,
//	0xC4,0x0A,0xEF,0x86,0x26,0x8A,0x56,0xE6,0x47,0x9B,0x3B,0xC9,0xDE,0xC7,0x9E,0x53,
//	0xD5,0xCA,0x74,0x00,0x8F,0x71,0x5D,0xCC,0x3F,0x08,0xFC,0x54,0xE0,0x99,0xED,0x16,
//	0x1C,0x1C,0x63,0xCC,0x56,0x27,0xF2,0x35,0x72,0x2F,0x86,0x17,0x50,0x27,0xFA,0x4D,
//	0x9D,0xF3,0xBF,0xFB,0x31,0xFC,0xA7,0xD7,0xA7,0xF8,0xD3,0x78,0xBA,0x2B,0xA8,0xA3,
//	0x09,0x3D,0x8E,0x1E,0x3B,0xE1,0xE5,0x79,0x46,0x18,0xD8,0x1E,0xC5,0x07,0x34,0x18,
//	0x62,0x2A,0x18,0x06,0x52,0x4F,0xF0,0x9A,0xDF,0xD6,0x3C,0x0D,0xA8,0xDB,0x1F,0x36,
//	0xD2,0xCE,0x71,0x1F,0x42,0x92,0x21,0x53,0xF8,0x66,0xB9,0x86,0x12,0xDB,0x39,0x8E,
//	0x44,0x65,0x75,0xEA,0xAD,0x90,0x45,0x6B,0x0A,0x91,0x9A,0xBC,0x59,0x7C,0xD6,0xD2,
//	0x68,0xB2,0x1A,0xEE,0x3F,0x92,0x29,0x4B,0xA8,0x1D,0x08,0xE3,0xF2,0x3C,0x50,0x2F,
//	0x65,0x87,0x86,0x85,0x47,0x1F,0xC2,0x4A,0xFF,0x00,0x23,0x55,0xC5,0xD3,0x2F,0x4E,
//	0x29,0x56,0xE4,0xE7,0x9A,0xD5,0x49,0x89,0xB8,0xF4,0x66,0xE6,0x97,0x24,0x77,0x92,
//	0x6E,0x49,0x19,0x24,0x07,0xFD,0x51,0x6C,0xF1,0xEB,0xC8,0xFF,0x00,0x3E,0xB5,0xD3,
//	0xA3,0x47,0x1A,0x0F,0x3E,0x57,0xDC,0x7B,0x64,0x0A,0xF3,0xD3,0x78,0xCB,0x22,0x34,
//	0x79,0x0C,0xA7,0x20,0xD7,0x59,0xA6,0x4D,0x1D,0xC4,0x6B,0x2B,0x06,0x0C,0x7A,0xEE,
//	0xF5,0xFA,0xF7,0xAF,0x1F,0x31,0xA5,0x26,0xF9,0xDB,0xD0,0xFA,0xFE,0x1E,0xC5,0x52,
//	0x71,0x74,0x12,0xB4,0xBB,0xF7,0x36,0x31,0x10,0x43,0xE5,0x6F,0x01,0xBA,0xE7,0x35,
//	0x56,0x5E,0x3A,0x31,0xFC,0x6A,0xDA,0x15,0x91,0x31,0x1E,0xEF,0xAF,0x6A,0xA5,0x31,
//	0xD8,0xFB,0x4B,0x03,0x5E,0x54,0x37,0x3E,0xA2,0xE9,0x22,0x10,0x0E,0xE3,0x2F,0x97,
//	0xB9,0x14,0x7C,0xC7,0x3C,0x2F,0xA1,0x35,0x05,0xEC,0xD1,0xC3,0x00,0x62,0xF9,0x08,
//	0x99,0x6E,0x00,0x19,0x27,0xA6,0x71,0xC9,0xC6,0x32,0x7F,0xC3,0x15,0x23,0x06,0x2E,
//	0x11,0x47,0x2D,0xD0,0x2F,0x3F,0xA5,0x67,0xEB,0xCE,0xD2,0x58,0x9D,0xB8,0x52,0x36,
//	0xA1,0x50,0xA0,0x70,0x07,0x5C,0xE7,0xFF,0x00,0xD7,0x5D,0x94,0xA2,0xA5,0x34,0x99,
//	0xE4,0x66,0x35,0x25,0x4E,0x94,0xA5,0x1E,0x86,0x43,0x6B,0x53,0x19,0x09,0x08,0x3C,
//	0xBE,0x81,0x4F,0x20,0x56,0xFD,0x9D,0xC4,0x17,0x01,0xA4,0x81,0x83,0x10,0x02,0x96,
//	0xCE,0x06,0x73,0x83,0xF8,0x1E,0xB9,0x38,0xEE,0x2B,0x8D,0xAD,0x7F,0x0E,0xDD,0x1B,
//	0x7D,0x48,0x26,0xF0,0xA9,0x27,0x07,0x23,0x23,0xF1,0x1E,0x9D,0x6B,0xD2,0xC4,0x61,
//	0xE3,0xC9,0x78,0xAD,0x8F,0x93,0xCB,0xF3,0x0A,0xAA,0xBA,0x53,0x77,0x4C,0xEE,0xAD,
//	0x44,0x6D,0x18,0x78,0xC8,0x69,0xD8,0x7C,0xC5,0x8E,0x5B,0x3D,0xFE,0x94,0xD9,0x20,
//	0x72,0x49,0x7C,0x9A,0xA5,0x05,0xC4,0x76,0x8E,0xCC,0xD2,0x3A,0x92,0x3D,0x41,0x04,
//	0xF7,0x39,0x1C,0x1A,0x98,0xDF,0xC5,0x21,0x3F,0xBD,0xC7,0x7C,0x9E,0x05,0x78,0x52,
//	0x84,0x94,0xB4,0x3E,0xEE,0x9C,0xE2,0xE3,0x73,0xBA,0x28,0xC0,0x16,0x50,0x0E,0x4F,
//	0x53,0xCE,0x3F,0x11,0x4B,0x80,0xE3,0x05,0x89,0x1E,0xA7,0x07,0x8F,0x6F,0xFF,0x00,
//	0x55,0x14,0x57,0x49,0xF0,0x40,0x4E,0x5F,0x21,0x58,0xF1,0x81,0x8E,0x49,0x1F,0xAF,
//	0xF2,0x14,0x86,0x1C,0x80,0x54,0x10,0xD9,0xFB,0xA4,0x7A,0x7A,0x91,0xFF,0x00,0xD6,
//	0xA2,0x8A,0x60,0x3C,0x5C,0xDE,0x22,0x9F,0x2E,0xF6,0x64,0x53,0xCB,0x15,0x76,0xC2,
//	0x8F,0xCF,0x15,0x72,0x0D,0x77,0x53,0x88,0x8D,0xB7,0x52,0xB6,0x0F,0x05,0xBE,0x6C,
//	0x8F,0xC4,0x1C,0xD1,0x45,0x2E,0x54,0x4B,0x8A,0x3A,0x3D,0x17,0x5D,0xB8,0xBE,0xB7,
//	0x91,0xEE,0x22,0x42,0x52,0x42,0xA0,0x14,0xC7,0x61,0xFE,0x35,0x7E,0x6D,0x4A,0x14,
//	0x52,0x64,0xB3,0x8C,0xA8,0x19,0x27,0x66,0x68,0xA2,0xBB,0x29,0x53,0x8D,0x91,0xC5,
//	0x56,0x29,0x37,0x62,0xA0,0xB8,0xD0,0x6F,0x78,0x93,0x4F,0xB1,0x94,0xFA,0x34,0x20,
//	0x93,0xF8,0x11,0x49,0xFD,0x91,0xE1,0xDD,0xC4,0xFF,0x00,0xC2,0x3D,0x61,0x93,0xDF,
//	0xEC,0x6B,0xFF,0x00,0xC4,0xD1,0x45,0x6D,0x28,0x72,0xEC,0xCC,0x52,0x4C,0xAB,0x77,
//	0x64,0x63,0x0C,0xBA,0x6E,0x99,0xA6,0xC2,0xA5,0x40,0x1B,0x61,0x01,0xB3,0xDC,0xF4,
//	0xE9,0xED,0xD7,0xDC,0x56,0x0D,0xEE,0x8D,0x1B,0xC8,0x09,0xD3,0x74,0xED,0xDC,0x16,
//	0x32,0x41,0x92,0x4E,0x79,0x23,0xF0,0xA2,0x8A,0xCD,0xA3,0xD4,0xC1,0x54,0x71,0x4E,
//	0x29,0x0D,0x6D,0x35,0x16,0x19,0x02,0xE9,0xFA,0x68,0x5C,0x30,0x8C,0x79,0x23,0xF0,
//	0xCF,0xF5,0xA8,0x97,0x47,0x41,0x6E,0x47,0xF6,0x7E,0x95,0xE6,0x6E,0xE3,0x16,0xE3,
//	0x1B,0x71,0xFC,0xF3,0x45,0x14,0x94,0x11,0xE9,0x2A,0xF3,0xE6,0x1E,0x9A,0x35,0xBB,
//	0x15,0xDD,0xA5,0xE9,0xC7,0x00,0x17,0xC4,0x23,0x8E,0xBE,0xDF,0xEE,0xFE,0xBE,0xD5,
//	0x1D,0xDF,0x87,0xEC,0x5C,0x62,0x3D,0x33,0x4D,0x03,0x07,0x76,0xFB,0x55,0x6E,0x3F,
//	0x4C,0x7E,0xB4,0x51,0x54,0x95,0x99,0x94,0xEA,0x39,0x27,0x73,0x26,0x4F,0x0D,0x5B,
//	0xC7,0x7E,0x77,0x68,0xFA,0x4F,0xD8,0xBB,0x11,0x6C,0xA5,0xF1,0x8F,0xA6,0x3A,0xD4,
//	0xF6,0x9E,0x1D,0xB1,0xFE,0xD1,0x0C,0x34,0xCD,0x31,0x60,0x5E,0x70,0x2C,0xD7,0x77,
//	0x4F,0x5F,0xAF,0xB5,0x14,0x57,0x57,0x33,0x70,0x3C,0x68,0xC1,0x2A,0xC9,0x23,0x55,
//	0xB4,0x98,0x54,0x16,0xFE,0xCF,0xD3,0x5E,0x4C,0x0C,0x7E,0xE0,0x0E,0xFC,0x83,0xD7,
//	0xB7,0x19,0xED,0xE8,0x6A,0x64,0xD2,0x34,0xCF,0x9B,0x3A,0x75,0x94,0x67,0x3C,0x6C,
//	0x85,0x7A,0x7B,0xF1,0xD7,0xAD,0x14,0x57,0x27,0x2A,0x3D,0x95,0x52,0x56,0xB5,0xCF,
//	0xFF,0xD9,
//	};
//
//
//	//--------------------------------------------------------------
//	// Picture-Struktur
//	//--------------------------------------------------------------
//	UB_Picture Emo2_Jpg = {
//	  Emo2_Jpg_Table, // Picture-Daten
//	  5714,         // Anzahl der Bytes
//	};
//
//	uint32_t start=Millis.get();
//	TFT.UB_Graphic_DrawJpg(&Emo2_Jpg,60,10);
//	Serial.puts(USART1,"Time:");
//	Serial.puts(USART1,(int)(Millis.get()-start));
//	while(1);

//	TFT.draw_bmp(0,0,(uint8_t*)"/kojla.bmp");
//	_delay_ms(5000);
	TFT.clear_screen();
	TFT.draw_bmp(0,100,(uint8_t*)"/logo.bmp");
	_delay_ms(1000);
	TFT.clear_screen();

	//	TFT.draw_bmp(0,0,"test.bmp");

	/******************** setup procedure ********************************************
	 * all initialisations must been made before the main loop, before THIS
	 ******************** setup procedure ********************************************/
	unsigned long   previousMillis = 0;
#ifdef LOAD_CALC
	unsigned long load_calc=0;
	unsigned long lasttime_calc=0;
#endif

	for (;;) {
		Sensors.mCAN.check_message();
		//////////////////////////////////////////////////
		//		Sensors.m_reset->set_deactive(false,false);
		//		Serial3.end();
		//		Serial3.begin(115200);
		//		while(true){
		//			while(Serial3.available()>0){
		//				Serial.print(Serial3.read(),BYTE);
		//			}
		//			while(Serial.available()>0){
		//				Serial3.print(Serial.read(),BYTE);
		//			}
		//		}
		//////////////////////////////////////////////////
		Sensors.mReset.toggle(); 		// toggle pin, if we don't toggle it, the ATmega8 will reset us, kind of watchdog<
		Debug.speedo_loop(21,1,0," "); 	// intensive debug= EVERY loop access reports the Menustate
		Sensors.mGPS.check_flag();    	// check if a GPS sentence is ready
		//		pAktors.check_flag(); 				// updated expander
		Sensors.pull_values();			// very important, updates all the sensor values

		/************************* timer *********************/
		Timer.every_sec();		// 1000 ms
		Timer.every_qsec();			// 250  ms
		Timer.every_custom();  		// one custom timer, redrawing the speedo, time is defined by "refresh_cycle" in the base.txt
		/************************* push buttons *********************
		 * using true as argument, this will activate bluetooth input as well
		 ************************* push buttons*********************/
		//Menu.button_test(true,false);     // important!! if we have a pushed button we will draw something, depending on the menustate
		if(Menu.button_test(true,false)){ // important!! if we have a pushed button we will draw something, depending on the menustate
			Serial.puts(USART1,"Menustate:");
			Serial.puts_ln(USART1,Menu.state);
		}
		/************************ every deamon activity is clear, now draw speedo ********************
		 * we are round about 0000[1]1 - 0000[1]9
		 ************************ every deamon activity is clear, now draw speedo ********************/
		Sensors.mCAN.check_message();

		if((Menu.state/10)==1 || Menu.state==7311111)  {
			Speedo.loop(previousMillis);
		}
		//////////////////// Sprint Speedo ///////////////////
		else if( Menu.state==MENU_SPRINT*10+1 ) {
			Sprint.loop();
		}
		////////////////// Clock Mode ////////////////////////
		else if(Menu.state==291){
			Sensors.mClock.loop();
		}
		////////////////// Speed Cam Check - Mode ////////////////////////
		else if(Menu.state==BMP(0,0,0,0,M_TOUR_ASSISTS,SM_TOUR_ASSISTS_SPEEDCAM_STATUS,1)){
			//			SpeedCams.calc();
			//			SpeedCams.interface();
		}
		////////////////// race mode ////////////////////
		else if(Menu.state==M_LAP_T*100+11){
			LapTimer.waiting_on_speed_up();
		}
		else if(Menu.state==M_LAP_T*1000+111){
			LapTimer.race_loop();
		}
		////////////////// set gps point ////////////////////
		else if(Menu.state==M_LAP_T*10000L+3111){
			LapTimer.gps_capture_loop();
		}
		//////////////////// voltage mode ///////////////////
		else if(Menu.state==531){
			Sensors.addinfo_show_loop();
		}
		//////////////////// stepper mode ///////////////////
		else if(Menu.state==541){
			//			Aktors.mStepper.loop();
		}
		//////////////////// gps scan ///////////////////
		else if(Menu.state==511){
			Sensors.mGPS.loop();
		}
		//////////////////// outline scan ///////////////////
		else if(Menu.state==721){
			Sensors.mSpeed.check_umfang();
		}
		////////////////// gear scan ///////////////
		else if(floor(Menu.state/100)==71){
			Sensors.mGear.calibrate();
		}

#ifdef LOAD_CALC
		load_calc++;
		if(millis()-lasttime_calc>1000){
			Serial.print(load_calc);
			Serial.println(" cps"); // 182 w/o interrupts, 175 w/ interrupts, 172 w/ much interrupts
			load_calc=0;
			lasttime_calc=millis();
		}
#endif
	} // end for
}