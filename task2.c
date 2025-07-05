#include <stdint.h>
	
#define PIXEL_BUFFER 0xc8000000
#define CHAR_BUFFER 0xc9000000
#define PS_DATA 0xff200100

void VGA_clear_pixelbuff() {
	// TODO
	// in the pixel buffer, call draw_point with a value of 0 for every possible pixel which is of size 320 and 240

	for(int y =0; y < 240; y++){
		for(int x = 0; x < 320; x++){
			VGA_draw_point(x, y, 0);
		}
	}
}

void VGA_draw_point(int x, int y, short c) {
	// TODO
	// third argument c is color. only access pixel buffer memory
	// pixel colors are accessed at 0xc8000000 | y << 1024 | x << 1
	volatile short *pixel_ptr = (volatile short *)(PIXEL_BUFFER | (y << 10) | (x << 1));
	*pixel_ptr = c;
	
	
}

void VGA_write_char(int x, int y, char c) {
// TODO
// ascii code is passed in the third argument. calculate coordinate using
// volatile uint16_t *char_ptr = (volatile uint16_t *)(CHAR_BUFFER | (y << 7) || (x))
if(!((x>=0 && x<80) && (y>=0 && y<60))){
	return;}
volatile char *char_ptr = (volatile char *)(CHAR_BUFFER |(y << 7) | (x));
*char_ptr = c;

}

void VGA_clear_charbuff() {
	// TODO
	// clear all ascii characters. loop like in clear pixel
	for(int i = 0; i<60; i++){
		for(int j = 0; j<80; j++){
			VGA_write_char(j,i,0);
		}
	}
}


int read_PS2_data(char *data) {
	// TODO
	
	int RVALID = ((*(volatile int *)0xff200100) >> 15) & 0x1;
	if (RVALID == 1){
		volatile unsigned char *c =  (volatile unsigned char*) PS_DATA;
		*data = *c;
		return 1;
	}
	return 0;
}

void write_hex_digit(unsigned int x,unsigned int y, char c) {
    if (c > 9) {
        c += 55;
    } else {
        c += 48;
    }
    c &= 255;
    VGA_write_char(x,y,c);
}
void write_byte_kbrd(unsigned int x,unsigned int y, unsigned int c) {
   char lower=c>>4 &0x0F;
   write_hex_digit(x,y,lower);
   char upper=c&0x0F;
   write_hex_digit(x+1,y,upper);
   return;
}

void input_loop_fun() {
    unsigned int x = 0;
    unsigned int y = 0;
	VGA_clear_pixelbuff();
    VGA_clear_charbuff();

    while (y<=59) {
    
			char data;
            char r2 = read_PS2_data(&data);

            if (r2 != 0) {  // Check if data is available

				write_byte_kbrd(x,y,data); 
                x += 3;
                if (x > 79) {
                    y++;
                    x = 0;
                }

                if (y > 59) {  // Check if loop should exit
                    return;  // End of input loop
                }
            }
    }
}


int main() {
	input_loop_fun();
	return 0;
}
