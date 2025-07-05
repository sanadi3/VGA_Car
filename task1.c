#include <stdint.h>
#define PIXEL_BUFFER 0xc8000000
#define CHAR_BUFFER 0xc9000000

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
		for(int j = 0; j<60; j++){
			for(int i = 0; i<80; i++){
				VGA_write_char(i, j, 0);
			}
		}
}

void draw_test_screen ()
{
    VGA_clear_pixelbuff();
    VGA_clear_charbuff();
	int SCREEN_HEIGHT=240;
	int SCREEN_WIDTH=320;
   for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            // Calculate color components to create a smooth gradient
            unsigned int red = (x * 31 / SCREEN_WIDTH) << 11;      // Horizontal red gradient
            unsigned int green = (y * 63 / SCREEN_HEIGHT) << 5;    // Vertical green gradient
            unsigned int blue = ((x + y) * 31 / (SCREEN_WIDTH + SCREEN_HEIGHT));  // Diagonal blue gradient

            // Combine red, green, and blue into a 16-bit color
            unsigned int color = red | green | blue;

            // Draw the pixel at (x, y) with the calculated color
            VGA_draw_point(x, y, color);
        }
    }


    const char *message = "Hello World!";
    int x = 20;
    int y = 5;
    
    for (int i = 0; message[i] != '\0'; i++) {
        VGA_write_char(x++, y, message[i]);
    }
}

int main() {
	draw_test_screen();

	return 0;
}
