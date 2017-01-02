#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <string.h>

int LCDAddr = 0x27;
int BLEN = 1;
int fd;
int isEmpty;

void write_word(int data){
	int temp = data;
	if ( BLEN == 1 )
		temp |= 0x08;
	else
		temp &= 0xF7;
	wiringPiI2CWrite(fd, temp);
}

void send_command(int comm){
	int buf;
	// Send bit7-4 firstly
	buf = comm & 0xF0;
	buf |= 0x04;			// RS = 0, RW = 0, EN = 1
	write_word(buf);
	delay(2);
	buf &= 0xFB;			// Make EN = 0
	write_word(buf);

	// Send bit3-0 secondly
	buf = (comm & 0x0F) << 4;
	buf |= 0x04;			// RS = 0, RW = 0, EN = 1
	write_word(buf);
	delay(2);
	buf &= 0xFB;			// Make EN = 0
	write_word(buf);
}

void send_data(int data){
	int buf;
	// Send bit7-4 firstly
	buf = data & 0xF0;
	buf |= 0x05;			// RS = 1, RW = 0, EN = 1
	write_word(buf);
	delay(2);
	buf &= 0xFB;			// Make EN = 0
	write_word(buf);

	// Send bit3-0 secondly
	buf = (data & 0x0F) << 4;
	buf |= 0x05;			// RS = 1, RW = 0, EN = 1
	write_word(buf);
	delay(2);
	buf &= 0xFB;			// Make EN = 0
	write_word(buf);
}

void init(){
	send_command(0x33);	// Must initialize to 8-line mode at first
	delay(5);
	send_command(0x32);	// Then initialize to 4-line mode
	delay(5);
	send_command(0x28);	// 2 Lines & 5*7 dots
	delay(5);
	send_command(0x0C);	// Enable display without cursor
	delay(5);
	send_command(0x01);	// Clear Screen
	wiringPiI2CWrite(fd, 0x08);
}

void clear(){
	send_command(0x01);	//clear Screen
}

void write(int x, int y, char data[]){
	int addr, i;
	int tmp;
	if (x < 0)  x = 0;
	if (x > 15) x = 15;
	if (y < 0)  y = 0;
	if (y > 1)  y = 1;

	// Move cursor
	addr = 0x80 + 0x40 * y + x;
	send_command(addr);
	
	tmp = strlen(data);
	for (i = 0; i < tmp; i++){
		send_data(data[i]);
	}
}
void trimData(char data[], char dataTrimmed[], int p){
	char t; int count=0;
	for(int i = p;; i++){
	    	if((t = data[i]) == '\n'){
			isEmpty = 1;
			return;
		}else{
			dataTrimmed[count] = t;
		        count++;
			if(count == 15){
				isEmpty = 0;
				return;
			}
		}
	}
	clear(); write(0,0, "Loop error");
}

void main(){
	fd = wiringPiI2CSetup(LCDAddr);
	init();
	char data[] = "Once upon a time, i was at New Years Eve!";
	char dataTrimmed[16];
	int row = 1;
	int index = 0; 
	isEmpty = 0;
	while( isEmpty == 0){
		trimData(data, dataTrimmed, index);
		index += 15;
		if(row == 1){
			write(0,0, dataTrimmed);
			delay(500);
			row = 2;
		}else if(row == 2){
			write(0,1, dataTrimmed);
			delay(3000);
			clear();
			row = 1;
		}
		
	}
}
