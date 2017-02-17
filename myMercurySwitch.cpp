// g++ -o myMercurySwitch myMercurySwitch.cpp
// RC Mains socket control program by Geoff Johnson.
// (c) 2012 Geoff Johnson.
// Based on the example GPIO in C program by Dom and Gert.
// Modified 2016 by Ian Parsons for Mercury switches
// Modified 2017 by Magnus Hanninen for Mercury switches

// Access from ARM Running Linux

#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

int  mem_fd;
unsigned char *gpio_mem, *gpio_map;
char *spi0_mem, *spi0_map;

// I/O access
volatile unsigned *gpio;

// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

// function prototypes.
void setup_io();
void SendCode(char* szCode);

int main(int argc, char **argv)
{ 
	int g,rep;

	if (argc != 3)
	{
		printf("RC Socket Control Program. Written by Geoff Johnson. (Mod by ITP 2016 and MH 2017)\n usage: switch 1-5 on/off\n");
	}
	else
	{
		// Set up gpi pointer for direct register access
		setup_io();

		// Switch GPIO 7 to output mode
		INP_GPIO(17); // must use INP_GPIO before we can use OUT_GPIO
		OUT_GPIO(17);

		char szOn[500] = {0};
		char szOff[500] = {0};

		if (strcmp (argv[1],"1") == 0)
		{
                        // my 1 sniffer on         0   1   0   0   0   1   0   1   0   1   0   1   0   1   0   1   0   0   1   1   0   0   1   1
			strcpy(szOn, "0000000000001000111010001000100011101000111010001110100011101000111010001110100010001110111010001000111011101000000000000000000000");
			
			// my 1 sniffer off        0   1   0   0   0   1   0   1   0   1   0   1   0   1   0   1   0   0   1   1   1   1   0   0   
			strcpy(szOn, "0000000000001000111010001000100011101000111010001110100011101000111010001110100010001110111011101110100010001000000000000000000000");
		}
		else if  (strcmp (argv[1],"2") == 0)
		{
			// my 2 sniffer on         0   1   0   0   0   1   0   1   0   1   0   1   0   1   0   1   1   1   0   0   0   0   1   1
			strcpy(szOn, "0000000000001000111010001000100011101000111010001110100011101000111010001110111011101000100010001000111011101000000000000000000000");
			
			// my 2 sniffer off        0   1   0   0   0   1   0   1   0   1   0   1   0   1   0   1   1   1   0   0   1   1   0   0
			strcpy(szOn, "0000000000001000111010001000100011101000111010001110100011101000111010001110111011101000100011101110100010001000000000000000000000");
		}
		else if  (strcmp (argv[1],"3") == 0)
		{
			// my 3 sniffer on         0   1   0   0   0   1   0   1   0   1   0   1   0   1   1   1   0   0   0   0   0   0   1   1
			strcpy(szOn, "0000000000001000111010001000100011101000111010001110100011101000111011101110100010001000100010001000111011101000000000000000000000");
			
			// my 3 sniffer off        0   1   0   0   0   1   0   1   0   1   0   1   0   1   1   1   0   0   0   0   1   1   0   0
			strcpy(szOn, "0000000000001000111010001000100011101000111010001110100011101000111011101110100010001000100011101110100010001000000000000000000000");
		}
		else if  (strcmp (argv[1],"4") == 0)
		{
			// my 4 sniffer on         0   1   0   0   0   1   0   1   0   1   0   1   1   1   0   1   0   0   0   0   0   0   1   1
			strcpy(szOn, "0000000000001000111010001000100011101000111010001110100011101110111010001110100010001000100010001000111011101000000000000000000000");
			
			// my 4 sniffer off        0   1   0   0   0   1   0   1   0   1   0   1   1   1   0   1   0   0   0   0   1   1   0   0
			strcpy(szOn, "0000000000001000111010001000100011101000111010001110100011101110111010001110100010001000100011101110100010001000000000000000000000");
		}
		else if  (strcmp (argv[1],"5") == 0)
		{
			// my 5 sniffer on         0   1   0   0   0   1   0   1   0   1   1   1   0   1   0   1   0   0   0   0   0   0   1   1
			strcpy(szOn, "0000000000001000111010001000100011101000111010001110111011101000111010001110100010001000100010001000111011101000000000000000000000");
			
			// my 5 sniffer off        0   1   0   0   0   1   0   1   0   1   1   1   0   1   0   1   0   0   0   0   1   1   0   0
			strcpy(szOn, "0000000000001000111010001000100011101000111010001110111011101000111010001110100010001000100011101110100010001000000000000000000000");
		}
		else
		{
			printf("Invalid Channel, it should be 1 to 5.\n");
		}

		if (strlen(szOn))
		{
			if (strcmp(argv[2],"on") == 0)
			{
				SendCode(szOn);
			}
			else if (strcmp(argv[2],"off") == 0)
			{
				SendCode(szOff);
			}
			else
			{
				printf("Last argument should be on or off.\n");
			}
		}
	}
	return 0;
} // main

//
// Set up a memory regions to access GPIO
//
void setup_io()
{
	/* open /dev/mem */
	if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) 
	{
		printf("Can't open /dev/mem \n");
		exit (-1);
	}

	/* mmap GPIO */
	// Allocate MAP block
	if ((gpio_mem = (unsigned char*)malloc(BLOCK_SIZE + (PAGE_SIZE-1))) == NULL) 
	{
		printf("Allocation error \n");
		exit (-1);
	}

	// Make sure pointer is on 4K boundary
	if ((unsigned long)gpio_mem % PAGE_SIZE)
	{
		gpio_mem += PAGE_SIZE - ((unsigned long)gpio_mem % PAGE_SIZE);
	}

	// Now map it
	gpio_map = (unsigned char *)mmap(
		(caddr_t)gpio_mem,
		BLOCK_SIZE,
		PROT_READ|PROT_WRITE,
		MAP_SHARED|MAP_FIXED,
		mem_fd,
		GPIO_BASE
		);

	if ((long)gpio_map < 0)
	{
		printf("mmap error %d\n", (int)gpio_map);
		exit (-1);
	}

	// Always use volatile pointer!
	gpio = (volatile unsigned *)gpio_map;
} // setup_io

// Function to send the output code to the RF transmitter connected to GPIO 7.
void SendCode(char* szCode)
{
	timespec sleeptime;
	timespec remtime;

	for (int iSend = 0 ; iSend < 20 ; iSend++) // ITP mod. send 20 times (should be plenty!)
	{
		sleeptime.tv_sec = 0;
		
		// next, set sleep time for shortest pulse width, for both "0" and "1" pulses
		sleeptime.tv_nsec = 90000; // ITP mod for Mercury. Value obtained by trial and error to match hand-held controller

		for (int i = 0 ; i < strlen(szCode) ; i++)
		{
			if (szCode[i] == '1')
			{
				GPIO_SET = 1<<7;
			}
			else
			{
				GPIO_CLR = 1<<7;
			}
			nanosleep(&sleeptime,&remtime);
		}
		// set time between blocks of pulses
		sleeptime.tv_nsec = 5000000; //5ms (ITP mod for Mercury)
		nanosleep(&sleeptime,&remtime);
	}
}