//******************************************************************************
//
// File Name : importer.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _IMPORTER_H

#define _IMPORTER_H

#define IMPORTER_FLAG_DI1			0x00000001
#define IMPORTER_FLAG_DI2			0x00000002
#define IMPORTER_FLAG_DI3			0x00000004
#define IMPORTER_FLAG_DI4			0x00000008
#define IMPORTER_FLAG_IGNITION		0x00000010
#define IMPORTER_FLAG_ENGINE		0x00000020
#define IMPORTER_FLAG_MOVE			0x00000040
#define IMPORTER_FLAG_NAV			0x00000080
#define IMPORTER_FLAG_ALT			0x00000100
#define IMPORTER_FLAG_COG			0x00000200
#define IMPORTER_FLAG_COUNTER1		0x00000400
#define IMPORTER_FLAG_COUNTER2		0x00000800
#define IMPORTER_FLAG_COUNTER3		0x00001000
#define IMPORTER_FLAG_COUNTER4		0x00002000
#define IMPORTER_FLAG_FREQUENCY1	0x00004000
#define IMPORTER_FLAG_FREQUENCY2	0x00008000
#define IMPORTER_FLAG_FREQUENCY3	0x00010000
#define IMPORTER_FLAG_FREQUENCY4	0x00020000
#define IMPORTER_FLAG_ANALOG1		0x00040000
#define IMPORTER_FLAG_ANALOG2		0x00080000
#define IMPORTER_FLAG_ANALOG3		0x00100000
#define IMPORTER_FLAG_INJECTOR		0x00200000
#define IMPORTER_FLAG_RS485_1		0x00400000
#define IMPORTER_FLAG_RS485_2		0x00800000
#define IMPORTER_FLAG_RS232_1		0x01000000
#define IMPORTER_FLAG_RS232_2		0x02000000
#define IMPORTER_FLAG_EVENT			0x04000000

#pragma pack(push, 1)

typedef struct tag_import_record
{
	unsigned char	size;
	unsigned char	dev_id[8];
	unsigned int	t;
	unsigned int	flags;
	unsigned char	var[255];
} IMPORT_RECORD;

typedef struct tag_import_pack
{
	unsigned int	size;
	unsigned int	id;
	IMPORT_RECORD	record;
} IMPORT_PACK;

#pragma pack(pop)

#endif

// End
