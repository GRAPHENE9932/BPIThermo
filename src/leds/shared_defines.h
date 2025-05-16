// WARNING: This header is supposed to be included into the leds source files
// and is not intended for interfacing with the leds's functions.

// Define pins for the TLC6C598 shift register's inputs.
#define PORT_ADDR_SER_IN_SRCK 0x05
#define DDR_ADDR_SER_IN_SRCK 0x04
#define BIT_SER_IN 0
#define BIT_SRCK 1
#define PORT_ADDR_RCK_CLR_G 0x0B
#define DDR_ADDR_RCK_CLR_G 0x0A
#define BIT_RCK 7
#define BIT_CLR 6
#define BIT_G 5

// Define the max duration of the LED's being on.
#define LED_DUTY_US 100
