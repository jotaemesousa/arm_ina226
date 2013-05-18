/*
 * INA226.cpp
 *
 *  Created on: May 13, 2013
 *      Author: joao
 */

#include "INA226.h"
#include <inc/lm3s2776.h>
#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <driverlib/debug.h>
#include <driverlib/sysctl.h>
#include <driverlib/systick.h>
#include <driverlib/gpio.h>
#include <driverlib/rom.h>
#include "driverlib/interrupt.h"
#include "driverlib/i2c.h"
#include "utils/cmdline.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include <stdint.h>
#include <math.h>

extern "C"
{
#include "libraries/I2Cdev/i2cutil.h"
}

// CAL 2560
// max current 0.000152587890625 LSB

INA226::INA226()
{
	addr_ = 0;
	conf_reg_high_ = 0x41;
	conf_reg_low_ = 0x27;
}

INA226::INA226(char addr)
{
	addr_ = addr;
	conf_reg_high_ = 0x41;
	conf_reg_low_ = 0x27;
}

INA226::~INA226()
{
	// TODO Auto-generated destructor stub
}

uint16_t INA226::read_register(uint8_t reg)
{
	if(addr_)
	{
		unsigned char data[2];
		data[0] = reg;

		i2cWrite(addr_, data, 1);
		i2cRead(addr_, data, 2);

		conf_reg_high_ = data[0];
		conf_reg_low_ = data[1];

		return (conf_reg_high_ << 8) | conf_reg_low_;
	}
}

void INA226::write_register(uint8_t reg, uint16_t value)
{
	if(addr_)
	{
		unsigned char data[3];
		data[0] = reg;
		data[1] = (value & 0xFF00) >> 8;
		data[2] = value & 0xFF;
		i2cWrite(addr_, data, 3);
	}
}

void INA226::set_sample_average(int avg)
{
	uint8_t new_conf_reg_high = conf_reg_high_ & 0xF1;

	switch (avg)
	{
	case 4:
		new_conf_reg_high |= 0b00000010;
		break;
	case 16:
		new_conf_reg_high |= 0b00000100;
		break;
	case 64:
		new_conf_reg_high |= 0b00000110;
		break;
	case 128:
		new_conf_reg_high |= 0b00001000;
		break;
	case 256:
		new_conf_reg_high |= 0b00001010;
		break;
	case 512:
		new_conf_reg_high |= 0b00001100;
		break;
	case 1024:
		new_conf_reg_high |= 0b00001110;
		break;
	}
	write_register(REG_CONFGURATION, (new_conf_reg_high << 8) | conf_reg_low_);
	conf_reg_high_ = new_conf_reg_high;
}

int INA226::get_bus_voltage(bool return_raw_data)
{
	unsigned int read_data = read_register(REG_BUS_VOLTAGE);

	if(return_raw_data)
	{
		read_data *= 125;
		read_data /= 100;
		return read_data;
	}
	else
	{
		return read_data;
	}
}

int INA226::get_bus_current(void)
{
	unsigned int read_data = read_register(REG_CURRENT);

	read_data = TwoComplement2ModSig_16bit(read_data);
}

int INA226::get_shunt_voltage(bool return_raw_data)
{
	unsigned int read_data = read_register(REG_CURRENT);

	read_data = TwoComplement2ModSig_16bit(read_data);

	if(return_raw_data)
	{
		read_data *= 25;
		read_data /= 10;
		return read_data;
	}
	else
	{
		return read_data;
	}
}

void INA226::set_vbus_conv_timer(uint16_t ct)
{
	uint8_t new_conf_reg_high = conf_reg_high_ & 0xFE;
	uint16_t new_conf_reg_low = conf_reg_low_ & 0x3F;
	if(ct < 8)
	{
		new_conf_reg_high |= ((ct & 0b00000111) >> 2);
		new_conf_reg_low |= (((ct & 0b00000111) << 6) && 0xFF);
	}
	new_conf_reg_low |= (new_conf_reg_high << 8);
	write_register(REG_CONFGURATION, new_conf_reg_low);
	conf_reg_low_ = new_conf_reg_low && 0xFF;
	conf_reg_high_ = new_conf_reg_high;
}

void INA226::set_vshunt_conv_timer(uint16_t ct)
{
	uint16_t new_conf_reg_low = conf_reg_low_ & 0x38;

	if(ct < 8)
	{
		new_conf_reg_low |= ((ct & 0b00000111) << 3);
	}
	new_conf_reg_low |= (conf_reg_high_ << 8);
	write_register(REG_CONFGURATION, new_conf_reg_low);
	conf_reg_low_ = new_conf_reg_low && 0xFF;
}

void INA226::set_operating_mode(uint8_t mode)
{
	uint16_t new_conf_reg_low = conf_reg_low_ & 0x07;

	if(mode < 8)
	{
		new_conf_reg_low |= (mode & 0b00000111);
	}
	new_conf_reg_low |= (conf_reg_high_ << 8);
	write_register(REG_CONFGURATION, new_conf_reg_low);
	conf_reg_low_ = new_conf_reg_low && 0xFF;
}

void INA226::set_calibration_value(uint16_t calib)
{
	write_register(REG_CALIBRATION, calib & 0xFFFF);
}

uint16_t INA226::get_calibration_value(void)
{
	return read_register(REG_CALIBRATION);
}


int TwoComplement2ModSig_16bit(uint16_t a)
{
	int mask = 0;
	mask = 0x8000;
	if((a & mask) == mask)
	{
		a = ~a;
		a = a + 1;
		return -a;
	}
	else
	{
		return a;
	}
}
