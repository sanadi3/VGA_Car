#include <stdint.h>
#include <stdio.h>
	
#define PIXEL_BUFFER 0xc8000000
#define CHAR_BUFFER 0xc9000000
#define PS_DATA 0xff200100
	
#define LEFT_MAKE 0x1C
#define RIGHT_MAKE 0x23

int left_lane;
int right_lane;
int score;
int car_x;
int car_y;
int speed;
int lane; // equal to 0 if left 1 if right

int increment = 1.05;

#define PRIV_TIMER 0xfffec600
volatile int *MPcore_private_timer_ptr = (int *)PRIV_TIMER;
int default_timer_count = 2000000;
double count = 2000000;

void init_timer(){
	// initialize
    *(MPcore_private_timer_ptr + 0) = default_timer_count; // write to load register
    *(MPcore_private_timer_ptr + 2) = 0b11; // set enable bit
	}

void update_timer(){
	count /= 1.20;
	*(MPcore_private_timer_ptr + 0) = (int)count;
    *(MPcore_private_timer_ptr + 2) = 0b11;            
}

void restore_timer() {
    count = default_timer_count;
    *(MPcore_private_timer_ptr + 0) = (int)count;
    *(MPcore_private_timer_ptr + 2) = 0b11;
}

int timer_expired(){
	// value at load

    if (*(MPcore_private_timer_ptr + 3)) { // read interrupt status reg
        *(MPcore_private_timer_ptr + 3) = 1; // clear interrupt bit
        return 1;
    }
    return 0;
}



void VGA_draw_point(int x, int y, short c) {
	// TODO
	// third argument c is color. only access pixel buffer memory
	// pixel colors are accessed at 0xc8000000 | y << 1024 | x << 1
	if(y<240){
	volatile short *pixel_ptr = (volatile short *)(PIXEL_BUFFER | (y << 10) | (x << 1));
	*pixel_ptr = c;
	}
	
	
}

void VGA_clear_pixelbuff() {
	// TODO
	// in the pixel buffer, call draw_point with a value of 0 for every possible pixel which is of size 320 and 240

	for(int y =0; y < 240; y++){
		for(int x = 0; x < 320; x++){
			VGA_draw_point(x, y, 0);
		}
	}
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
			VGA_write_char(j, i, 0);
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

//<------------ GAME INITIALIZATION ------------>
// to initialize game variable values


// hard code the road colors. this draws without the car there
void draw_road(){
	int ROAD_HEIGHT = 240;
	int ROAD_WIDTH = 100;
	
	short blue = 0x001F;
	unsigned int grey = (15 << 11) | (31 << 5) | 15;
	
	// draw blue portion
	for(int y =0; y< ROAD_HEIGHT; y++){
		for(int x =10; x<ROAD_WIDTH-10; x++){
			VGA_draw_point(x,y,blue);
		}
	}
	
	// draw borders
	for(int y =0; y < ROAD_HEIGHT; y++){
		for(int x =0; x < 10; x++){
			VGA_draw_point(x,y,(short)grey);
		}
	}
	
	// draw borders
	for(int y =0; y < ROAD_HEIGHT; y++){
		for(int x =90; x < 100; x++){
			VGA_draw_point(x,y,(short)grey);
		}
	}
}

// draw the car, coordinates are where the center should be
void draw_car(int x, int y){
	//int x_mid = 35;
	//int y_mid = 24;
	
	//int car_width = 30;
	//int car_height = 48;
	
	int x_start = x -15;
	int x_end = x + 15;
	
	int y_start = y - 24;
	int y_end = y +24;
	
	unsigned int red = (31 << 11);
	
	for (int j = y_start; j< y_end; j++){
		for(int i = x_start; i< x_end; i++){
			VGA_draw_point(i, j, (short)red);
		}
	}
}

void draw_car_traffic(int x, int y){
	//int x_mid = 35;
	//int y_mid = 24;
	
	//int car_width = 30;
	//int car_height = 48;
	
	int x_start = x -15;
	int x_end = x + 15;
	
	int y_start = y - 24;
	int y_end = y +24;
	
	unsigned short yellow = 0xFFE0;
	
	for (int j = y_start; j< y_end; j++){
		for(int i = x_start; i< x_end; i++){
			VGA_draw_point(i, j, yellow);
		}
	}
}

void draw_right_screen(){
	for(int y =0; y< 240; y++){
		for(int x =100; x<320; x++){
		VGA_draw_point(x,y, 0x0000);
		}
	}
	
}


void right_screen_initial_msg(){
	const char *start_msg = "Press S to start.";
	for(int i = 0; start_msg[i] != '\0'; i++){
		VGA_write_char(40+i, 15, start_msg[i]);
	}
}

void redraw_road(int x, int y){
	
	short blue = 0x001F;
	int x_left = x-15;
	int x_right = x+15;
	int y_top = y-24;
	int y_bottom = y+24;
	
	for(int j =y_top; j<y_bottom; j++){
		for(int i = x_left; i<x_right; i++){
			VGA_draw_point(i, j, blue);
		}
	}
}

// car object
typedef struct{
	int active;
	int id;
	int x;
	int y;
} Car;


// array of cars. 3 inactive
Car cars[3] = { {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} };


void init_game(){
	
	VGA_clear_charbuff();
	car_x = 35;
	car_y = 216;
	
	lane =0;
	
	score = 0;
	
	for (int i = 0; i < 3; i++) {
        cars[i].active = 0;
        cars[i].x = 0;
        cars[i].y = 0;
    }
	
	VGA_clear_pixelbuff();
	
	// draw road with dimensions 100 x 240
	draw_road();
	
	// configure initial speed
	speed = 50;
	
	// draw car with dimensions 30 x 48
	draw_car(car_x, car_y);
	
	draw_right_screen();
	right_screen_initial_msg();
	
}

// <--------------------- END GAME IALIZATION ------->

// <--------------- GAMEPLAY MECHANICS -------------->

// ----------------------- //
// Psuedo Number Generator //
// ----------------------- //

unsigned int seed = 12345;  // You can set this to any starting value

// Function to generate a pseudo-random number
unsigned int pseudo_random() {
    // LCG parameters (from Numerical Recipes)
    seed = (1103515245 * seed + 12345) & 0x7fffffff;
    return seed;
}

// Function to get a pseudo-random number within a specific range [min, max]
unsigned int random_in_range(int min, int max) {
    return ((pseudo_random() % (max - min + 1)) + min) % 2;
}


void spawn_object(){
	for(int i =0; i<3; i++){
		if(cars[i].active == 0){
			int pos = random_in_range(0,100);
			cars[i].active =1;
			cars[i].y = 0;
			if(pos == 0){ // left lane
			cars[i].x = 35;}
			else{
			cars[i].x = 65;}
			break;
		}
	}
}

int get_top_car_coordinate(){
	int min_y = 300; 
	for (int i = 0; i < 3; i++) {
		if (cars[i].active && cars[i].y < min_y) {
			min_y = cars[i].y;
		}
	}

	if (min_y == 300) {
		return 240; 
	}

	return min_y - 24;
}

void right_screen_end_msg(){
	const char *end_msg = "Game Over!";
	for(int i = 0; end_msg[i] != '\0'; i++){
		VGA_write_char(40+i, 5, end_msg[i]);
	}
	
	const char *ask_msg = "Press S to play again";
	for(int i = 0; ask_msg[i] != '\0'; i++){
		VGA_write_char(40+i, 30, ask_msg[i]);
	}
}


void draw_traffic(){
	for(int i =0; i< 3; i++){
		if(cars[i].active == 1){
			draw_car_traffic(cars[i].x, cars[i].y);
		}
	}
}

// replace the car with blue
void erase_car(int x, int y){
	int x_mid = 35;
	int y_mid = 24;
	
	int car_width = 30;
	int car_height = 48;
	
	int x_start = x -15;
	int x_end = x + 15;
	
	int y_start = y - 24;
	int y_end = y +24;
	
	
	short blue = 0x001F;
	
	for (int j = y_start; j< y_end; j++){
		for(int i = x_start; i<x_end; i++){
			VGA_draw_point(i,j, blue);
		}
	}
}

void update_score(){
	score++;
	update_timer();

}


void update_objects(){
	for (int i = 0; i < 3; i++) {
		if (cars[i].active) {
			if(cars[i].y-25>=0){
			redraw_road(cars[i].x, cars[i].y-25);}
			
			 cars[i].y++;  // move car down

			// set to not active so the function will stop drawing it
            if (cars[i].y - 24 >= 240) {
                cars[i].active = 0;
                update_score();
                continue;
            }

            draw_car_traffic(cars[i].x, cars[i].y);  // draw car at new y
   
		}
	}
}


int check_collision(){
	
	int player_left_boundary = car_x - 15;
	int player_right_boundary = car_x +15;
	int player_top_boundary = car_y - 24;
	int player_bottom_boundary = car_y + 24;
	
	for(int i =0; i<3; i++){
		if(cars[i].active == 1){
			int car_left_boundary = cars[i].x - 15;
			int car_right_boundary = cars[i].x +15;
			int car_top_boundary = cars[i].y - 24;
			int car_bottom_boundary = cars[i].y + 24;
			for(int x = (car_x-15); x<(car_x+15); x++){
				for(int y = (car_y-15); y<(car_y+15); y++){
					if (player_right_boundary > car_left_boundary &&
			    		player_left_boundary < car_right_boundary &&
			    		player_bottom_boundary > car_top_boundary &&   
			    		player_top_boundary < car_bottom_boundary) {return 1;}
			}
		}
	}
	}
	return 0;
}

void update_character_position(){
	char data;
    if (read_PS2_data(&data) != 0) {
        //if (data == 0x1C && lane != 0) {  // A
			if (data == 0x1C) {  // A
            erase_car(car_x, car_y);
            car_x = 35;
            draw_car(car_x, car_y);
			lane = 0;
        } //else if (data == 0x23 && lane != 1) {// D
			
		else if (data == 0x23) {
            erase_car(car_x, car_y);
            car_x = 65;
            draw_car(car_x, car_y);
			lane = 1;
        }
    }
	

}

void display_score_message(){
	const char *score_msg = "Score:";
	for(int i = 0; score_msg[i] != '\0'; i++){
		VGA_write_char(40+i, 15, score_msg[i]);
	}
}



void display_score(){
	char result[10];
    sprintf(result, "%d", score);
	
	for(int i =0; result[i] != '\0'; i++){
		VGA_write_char(60+i, 15, result[i]);
			}
}

int main() {
	init_timer();
	
	while(1){
	init_game();

	int ready_to_play =0;
    int frame_count = 0;

	int beginning =1;
	
	
	while(beginning){
		char data;
    	if (read_PS2_data(&data) != 0){
			if(data == 0x1B){
				beginning =0;
				ready_to_play =1;}
			}
		}
		
	VGA_clear_charbuff();
	display_score_message();
		
    while (ready_to_play) {
		
		display_score();
		
        update_character_position();

        if (get_top_car_coordinate() >= 96) {
            spawn_object();
		}

        if (timer_expired()) { 
            update_objects();  
			 if (check_collision()) {
        		ready_to_play = 0; 
    		}
        }

        draw_traffic();  // draw active cars
    }
	
	right_screen_end_msg();
	restore_timer();
	
	while(!ready_to_play){
		char data;
    	if (read_PS2_data(&data) != 0){
		if(data == 0x1B){
			break;
			}
		}
	}
	}
	return 0;
}