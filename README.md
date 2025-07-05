# VGA_Car

This project is a car dodging game implemented in C for an FPGA-based embedded system, using memory-mapped I/O to interface with VGA display and PS/2 keyboard peripherals. The game writes to the VGA pixel buffer at address 0xc8000000. Text is outputted through a buffer at 0xc9000000. Player input is captured by polling raw scancodes from a PS/2 keyboard interface at 0xff200100. A private ARM timer (0xfffec600) progresses the game and object redrawing, with the timer interval being updated to adjust speed of car movement.
