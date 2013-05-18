extern "C" {
#include <math.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_i2c.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "driverlib/i2c.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "stdint.h"

#define SYSTICKS_PER_SECOND     1000

static unsigned long milliSec = 0;

void SysTickHandler()
{
	milliSec++;
}

uint32_t millis(){
	return milliSec;
}





void InitConsole(void)
{
	//
	// Enable GPIO port A which is used for UART0 pins.
	// TODO: change this to whichever GPIO port you are using.
	//
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	//
	// Select the alternate (UART) function for these pins.
	// TODO: change this to select the port/pin you are using.
	//
	MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	//
	// Initialize the UART for console I/O.
	//
	UARTStdioInit(0);
}


}

#include "INA226.h"
static unsigned long ulClockMS=0;


int main(void)
{
	MAP_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |SYSCTL_XTAL_12MHZ); //50MHZ

	//
	// Enable peripherals to operate when CPU is in sleep.
	//
	MAP_SysCtlPeripheralClockGating(true);

	//
	// Configure SysTick to occur 1000 times per second, to use as a time
	// reference.  Enable SysTick to generate interrupts.
	//
	MAP_SysTickPeriodSet(MAP_SysCtlClockGet() / SYSTICKS_PER_SECOND);
	MAP_SysTickIntEnable();
	MAP_SysTickEnable();

	//
	// Get the current processor clock frequency.
	//
	ulClockMS = MAP_SysCtlClockGet() / (3 * 1000);

	InitConsole();

	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// Initialize the UART as a console for text I/O.
#ifdef DEBUG
	UARTprintf("Setting up UART ... \n");
#endif
	UARTStdioInitExpClk(0,115200);

#ifdef DEBUG
    UARTprintf("SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0)\n");
#endif

    //I2C
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

#ifdef DEBUG
    UARTprintf("GPIOPinTypeI2C(GPIO_PORTB_BASE,GPIO_PIN_2 | GPIO_PIN_3);\n");
#endif

    GPIOPinTypeI2C(GPIO_PORTB_BASE,GPIO_PIN_2 | GPIO_PIN_3);

#ifdef DEBUG
    UARTprintf("I2CMasterInitExpClk(I2C0_MASTER_BASE,SysCtlClockGet(),false);\n");
#endif

    I2CMasterInitExpClk(I2C0_MASTER_BASE,SysCtlClockGet(),false);  //false = 100khz , true = 400khz
    I2CMasterTimeoutSet(I2C0_MASTER_BASE, 1000);



	INA226 power_meter = INA226(0x45);
	uint16_t read_ina;
	read_ina = power_meter.read_register(REG_CONFGURATION);

	UARTprintf("Conf reg = %X\n", read_ina);

	power_meter.set_sample_average(4);

	read_ina = power_meter.read_register(REG_CONFGURATION);

	power_meter.set_sample_average(16);

	read_ina = power_meter.read_register(REG_CONFGURATION);

	UARTprintf("Conf reg = %X", read_ina);

	while (1)
	{
		read_ina = power_meter.get_bus_voltage();
		UARTprintf("Bus Voltage = %d mV\n", read_ina);
//		read_ina = power_meter.read_register(REG_CURRENT);
//		UARTprintf("Current = %X\n", read_ina);
		SysCtlDelay(200*ulClockMS);
	}

}

