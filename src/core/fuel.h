//******************************************************************************
//
// File Name: fuel.h
// Author	: Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _FUEL_H

#define _FUEL_H

#define FUEL_SOURCE_F1	0x01
#define FUEL_SOURCE_F2	0x02
#define FUEL_SOURCE_F3	0x03
#define FUEL_SOURCE_F4	0x04
#define FUEL_SOURCE_F5	0x05
#define FUEL_SOURCE_F6	0x06
#define FUEL_SOURCE_F7	0x07
#define FUEL_SOURCE_F8	0x08

#define FUEL_SOURCE_A1	0x11
#define FUEL_SOURCE_A2	0x12
#define FUEL_SOURCE_A3	0x13
#define FUEL_SOURCE_A4	0x14
#define FUEL_SOURCE_A5	0x15
#define FUEL_SOURCE_A6	0x16
#define FUEL_SOURCE_A7	0x17
#define FUEL_SOURCE_A8	0x18

#define FUEL_SOURCE_RS485_1	0x100
#define FUEL_SOURCE_RS485_2	0x200

#define FUEL_SOURCE_RS232_1	0x300
#define FUEL_SOURCE_RS232_2	0x400

typedef struct ftr
{
	float sensor_value;
	float fuel_value;
	float d;
	bool operator<(const struct ftr &a) const { return sensor_value < a.sensor_value; }
} FUEL_TABLE_RECORD;

#endif

// End
