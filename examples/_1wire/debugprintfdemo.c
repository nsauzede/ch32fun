/* Small example showing how to use the SWIO programming pin to 
   do printf through the debug interface */

#include "ch32fun.h"
#include "ch32v003_GPIO_branchless.h"
#include <stdio.h>

#include "1wire.c"

uint32_t count;

int last = 0;
void handle_debug_input( int numbytes, uint8_t * data )
{
	last = data[0];
	count += numbytes;
}

int main()
{
	SystemInit();

	while( !DebugPrintfBufferFree() );

	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC;

	funPinMode( PD0, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP );
	funPinMode( PC0, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP );

	// GPIO C4 Push-Pull
	GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_I_pullUp, GPIO_Speed_In);

	while(1)
	{
		funDigitalWrite( PD0, 1 );
		funDigitalWrite( PC0, 1 );

		int16_t temp = ds18b20_read_temp() / 16;
//		int16_t temp = ds18b20_read_temp();     // 0x0550
//		printf( "+%lu temp=%x\n", count++, temp );
		printf( "+%lu temp=%d\n", count++, temp );
//		printf( "+%lu\n", count++ );
		Delay_Ms(100);
		int i;
		for( i = 0; i < 10000; i++ )
			poll_input();

		funDigitalWrite( PD0, 0 );
		funDigitalWrite( PC0, 0 );

		printf( "-%lu[%c]\n", count++, last );
		Delay_Ms(100);
	}
}

