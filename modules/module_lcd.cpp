#include "module_lcd.h"
#include "../include/includes.h"
#include "../drivers/i2c.cpp"

//*******************************************************************
//Class for accessing GNUBLIN Module-LCD 4x20
//*******************************************************************

//-------------Konstruktor-------------
// set error flag=false

gnublin_module_lcd::gnublin_module_lcd() 
{
	version = (char *) "0.3";
	error_flag=false;
}


//-------------get Error Message-------------
// get the last ErrorMessage
// parameters:		NONE
// return:			[const char*]ErrorMessage	Error Message as c-string

const char *gnublin_module_lcd::getErrorMessage(){
	return ErrorMessage.c_str();
}

//-------------------------------Fail-------------------------------
//returns the error flag. if something went wrong, the flag is true
bool gnublin_module_lcd::fail(){
	return error_flag;
}

//-------------set Address-------------
// set the slave address
// parameters:		[int]Address	i2c slave Address
// return:			NONE

void gnublin_module_lcd::setAddress(int Address){
	pca.setAddress(Address);
}


//-------------------set devicefile----------------
// set the i2c device file. default is "/dev/i2c-1"
// parameters:		[string]filename	path to the dev file
// return:			NONE

void gnublin_module_lcd::setDevicefile(std::string filename){
	pca.setDevicefile(filename);
}

//-------------------lcd out----------------
// sends the given data and RS/RW pings to the pca9555
// parameters:		[unsigned char]rsrw		contains the RS/RW pins
//					[unsigned char]data		contains the data to send
// returns:			[int]  1		success
// 					[int] -1		failure
int gnublin_module_lcd::lcd_out(unsigned char rsrw, unsigned char data ){
	if(!pca.writePort(0, data)){			//send data on Port 0
		error_flag=true;
		ErrorMessage = pca.getErrorMessage();
		return -1;
	}
	if(!pca.writePort(1, rsrw)){			//send RS/RW bits on Port 1
		error_flag=true;
		ErrorMessage = pca.getErrorMessage();
		return -1;
	}
	if(!pca.writePort(1, (rsrw | LCD_EN))){	//enable on
		error_flag=true;
		ErrorMessage = pca.getErrorMessage();
		return -1;
	}
	usleep(LCD_ENABLE_US);
	if(!pca.writePort(1, rsrw)){			//enable off
		error_flag=true;
		ErrorMessage = pca.getErrorMessage();
		return -1;
	}
	return 1;
}

//-------------------lcd data----------------
// sends the given char to the display
// parameters:		[unsigned char]data		contains the char to send
// returns:			[int]  1		success
// 					[int] -1		failure
int gnublin_module_lcd::lcd_data(unsigned char data){
        if(!lcd_out(LCD_RS, (unsigned char) data)){
			return -1;
		}
        usleep(LCD_WRITEDATA_US);
        return 1;
}

//-------------------lcd command----------------
// sends the given command to the display
// parameters:		[unsigned char]data		contains the command to send
// returns:			[int]  1		success
// 					[int] -1		failure
int gnublin_module_lcd::lcd_command(unsigned char data){
        if(!lcd_out(0x00, (unsigned char) data)){
		    return -1;
		}
        usleep(LCD_COMMAND_US);
        return 1;
}

//-------------------clear command----------------
// sends the clear command to the display
// parameters:		none
// returns:			[int]  1		success
// 					[int] -1		failure
int gnublin_module_lcd::lcd_clear(){
        if(!lcd_command(LCD_CLEAR_DISPLAY)){
			return -1;
		}
        usleep(LCD_CLEAR_DISPLAY_MS);
        return 1;
}

//-------------------cursor home command----------------
// sends the home command to the display
// parameters:		none
// returns:			[int]  1		success
// 					[int] -1		failure
int gnublin_module_lcd::lcd_home(){
        if(!lcd_command(LCD_CURSOR_HOME)){
			return -1;
		}
        usleep(LCD_CURSOR_HOME_MS);
        return 1;
}

//-------------------set display command----------------
// sends the set display command to the display
// parameters:		[int]cursor		if = 1, the cursor is visible
//					[int]blink		if = 1, the cursor blinks
// returns:			[int]  1		success
// 					[int] -1		failure
int gnublin_module_lcd::lcd_setdisplay(int cursor, int blink){
        unsigned char set_display;
        //Display ON/OFF Control
        set_display = LCD_SET_DISPLAY + LCD_DISPLAY_ON;
        if(cursor) set_display = set_display + LCD_CURSOR_ON;
        if(blink) set_display = set_display + LCD_BLINKING_ON;
        if(!lcd_command(set_display))
        	return -1;
        return 1;
}

//-------------------set cursor command----------------
// sends the set cursor command to the display
// parameters:		[int]x			set the cursor to x - position
//					[int]y			set the cursor to y - position
// returns:			[int]  1		success
// 					[int] -1		failure
int gnublin_module_lcd::lcd_setcursor(unsigned char x, unsigned char y){
        unsigned char data;
        switch(x){
                case 1: //1. Zeile
                        data = LCD_SET_DDADR + LCD_DDADR_LINE1 + y;
                        break;

                case 2:    // 2. Zeile
                        data = LCD_SET_DDADR + LCD_DDADR_LINE2 + y;
                        break;

                case 3:    // 3. Zeile
                        data = LCD_SET_DDADR + LCD_DDADR_LINE3 + y;
                        break;

                case 4:    // 4. Zeile
                        data = LCD_SET_DDADR + LCD_DDADR_LINE4 + y;
                         break;

                default:
                		error_flag=true;
						ErrorMessage = "Wrong line/column";
                        return -1;
        }
        if(!lcd_command(data)){
        	return -1;
        }
        return 1;
}

//-------------------string----------------
// sends the sting to the display
// parameters:		[const char *]data	the string to send
// returns:			[int]  1		success
// 					[int] -1		failure
int gnublin_module_lcd::lcd_string(const char *data){
        while(*data != '\0'){
                if(!lcd_data( *data++)){
                	return -1;
                }
        }
        return 1;
}


//-------------------init----------------
// initializes the display
// parameters:		none
// returns:			[int]  1		success
// 					[int] -1		failure
int gnublin_module_lcd::lcd_init(){
	//Set Ports as output
	if(!pca.portMode(0, "out")){ 	//Port 0 as Output
		error_flag=true;
		ErrorMessage = pca.getErrorMessage();
		return -1;
	}
	if(!pca.portMode(1, "out")){ 	//Port 1 as Output
		error_flag=true;
		ErrorMessage = pca.getErrorMessage();
		return -1;
	}

	//// initial alle Ausgänge auf Null
	if(!pca.writePort(0, 0x00)){
		error_flag=true;
		ErrorMessage = pca.getErrorMessage();
		return -1;
	}
	if(!pca.writePort(1, 0x00)){
		error_flag=true;
		ErrorMessage = pca.getErrorMessage();
		return -1;
	}
	
	usleep(LCD_BOOTUP_MS);

	//function set
	if(!lcd_command(LCD_SET_FUNCTION |
                        LCD_FUNCTION_8BIT |
                        LCD_FUNCTION_2LINE |
                        LCD_FUNCTION_5X7)){
		return -1;
	}
	usleep(LCD_SET_8BITMODE_MS);

	//Display ON/OFF Control
	if(!lcd_setdisplay(0, 0)){
		return -1;
	}

	if(!lcd_clear()){
		return -1;
	}
	
	//entry mode set
	if(!lcd_command(LCD_SET_ENTRY |
						LCD_ENTRY_INCREASE |
						LCD_ENTRY_NOSHIFT)){
		return -1;
	}
	if(!lcd_setcursor(1, 1)){
		return -1;
	}
	if(!lcd_string("embedded-projects")){
		return -1;
	}
	if(!lcd_setcursor(2, 4)){
		return -1;
	}
	if(!lcd_string("GNUBLIN-LCD")){
		return -1;
	}
	if(!lcd_setcursor(3, 2)){
		return -1;
	}
	if(!lcd_string("www.gnublin.org")){
		return -1;
	}
	if(!lcd_setcursor(4, 4)){
		return -1;
	}
	if(!lcd_string("Version ")){
		return -1;
	}
	if(!lcd_string(version)){
		return -1;
	}
	return 1;
}