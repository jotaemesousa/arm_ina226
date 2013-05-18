/*
 * INA226.h
 *
 *  Created on: May 13, 2013
 *      Author: joao
 */

#ifndef INA226_H_
#define INA226_H_

#include "stdint.h"

// Registers
#define REG_CONFGURATION					0x00
#define REG_SHUNT_VOLTAGE					0x01
#define REG_BUS_VOLTAGE						0x02
#define REG_POWER							0x03
#define REG_CURRENT							0x04
#define REG_CALIBRATION						0x05
#define REG_MASK_ENABLE						0x06
#define REG_ALERT_LIMIT						0x07

// Operating Mode Settings
#define MODE_POWER_DOWN						0b00000000
#define MODE_SHUNT_VOLTAGE_TRIGGERED		0b00000001
#define MODE_BUS_VOLTAGE_TRIGGERED			0b00000010
#define MODE_SHUNT_BUS_VOLTAGE_TRIGGERED	0b00000011
#define MODE_SHUNT_VOLTAGE_CONTINUOUS		0b00000101
#define MODE_BUS_VOLTAGE_CONTINUOUS			0b00000110
#define MODE_SHUNT_BUS_VOLTAGE_CONTINUOUS	0b00000111

class INA226
{

	char addr_;
	uint16_t conf_reg_high_,conf_reg_low_;



public:
	INA226();
	INA226(char addr);
	~INA226();
//private
	uint16_t read_register(uint8_t reg);
	void write_register(uint8_t reg, uint16_t value);


	void set_i2c_addr(char addr){ addr_ = addr;}
	void set_sample_average(int avg);
	void set_vbus_conv_timer(uint16_t ct);
	void set_vshunt_conv_timer(uint16_t ct);
	void set_operating_mode(uint8_t mode);


	int get_bus_voltage(bool return_raw_data = true);
	int get_bus_current(void);
	int get_shunt_voltage(bool return_raw_data = true);

	void set_calibration_value(uint16_t calib);
	uint16_t get_calibration_value(void);


};

int TwoComplement2ModSig_16bit(uint16_t a);

#endif /* INA226_H_ */
