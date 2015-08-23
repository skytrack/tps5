//////////////////////////////////////////////////////////////////////////// 
//
//	Copyright (c) 2014 Skytrack Ltd
//
//	Terminal service data, header file
//
//	Original filename "AK308.h"
//
////////////////////////////////////////////////////////////////////////////


#ifndef _AK308_h

#define _AK308_h

#define AK308_IO_MODE_485						0x01
#define AK308_IO_MODE_SEPARATE					0x00

#define AK308_INPUT_MODE_UNCONNECTED			0x00
#define AK308_INPUT_MODE_DISCRETE				0x01
#define AK308_INPUT_MODE_FREQUENCY				0x02
#define AK308_INPUT_MODE_COUNTER				0x04

#define AK308_INPUT_FLAG_LLS					0x01

#define AK308_SOURCE_ACTIVE						0x00
#define AK308_SOURCE_VCC						0x01
#define AK308_SOURCE_ACCEL						0x02
#define AK308_SOURCE_INPUT1						0x03
#define AK308_SOURCE_INPUT2						0x04
#define AK308_INPUT_MODE_RS485_B				0x05

#define	AK308_MODE_TYPE_DRIVE					0x01
#define	AK308_MODE_TYPE_SLEEP					0x02
#define	AK308_MODE_TYPE_SLEEP_DEEP				0x04

#define	AK308_MODE_FLAG_WAIT_FOR_STABLE_LLS		0x01
#define	AK308_MODE_FLAG_SWITCH_TO_ACTIVE_ON_LLS	0x02

#define AK308_POINT_FLAG_NAVVALID				0x0001
#define AK308_POINT_FLAG_RESERVED				0x0002
#define AK308_POINT_FLAG_ACCEL					0x0004
#define AK308_POINT_FLAG_DI1					0x0010
#define AK308_POINT_FLAG_DI2					0x0020
#define AK308_POINT_FLAG_VCC					0x0040
#define AK308_POINT_FLAG_DRIVE					0x0080
#define AK308_POINT_FLAG_SLEEP					0x0100
#define AK308_POINT_FLAG_SLEEPDEEP				0x0200
#define AK308_POINT_FLAG_NEEDTOSEND				0x8000

#define AK308_CMD_INFO  						0x01
#define AK308_CMD_CONFIG_LEGACY					0x02
#define AK308_CMD_SETTIME						0x03
#define AK308_CMD_POSITION_LEGACY				0x04
#define AK308_CMD_KEEPALIVE    					0x05
#define AK308_CMD_FIRMWARE						0x06
#define AK308_CMD_SERVER						0x07
#define AK308_CMD_ACK							0x08
#define AK308_CMD_NACK							0x09
#define AK308_CMD_CONFIG       					0x10
#define AK308_CMD_POSITION     					0x11

#define AK308_POLYNOM							0xC003

#define AK308_MASK_ALTITUDE						0x0001
#define AK308_MASK_COG							0x0002
#define AK308_MASK_VCC							0x0004
#define AK308_MASK_INPUT1COUNTER				0x0008
#define AK308_MASK_INPUT1FREQUENCY				0x0010
#define AK308_MASK_INPUT2COUNTER				0x0020
#define AK308_MASK_INPUT2FREQUENCY				0x0040
#define AK308_MASK_LLS1							0x0080
#define AK308_MASK_LLS2							0x0100
#define AK308_MASK_LLS_DELTA					0x0200
#define AK308_MASK_ACCELL						0x0400

#define AK308_SUPPORTED_MESSAGES_MASK			0x03FF

#pragma pack (1) 

typedef struct ak308_point_header{
	unsigned short	mask;
	unsigned int 	t;
	unsigned int 	latitude;
	unsigned int 	longitude;
	unsigned short 	speed;
	unsigned short	flags;
} AK308POINTHEADER;

typedef struct AK308_INFO_Payload {
	unsigned short	nVer;
	unsigned int	tProfile;
	unsigned char	nReason;
	unsigned char	szBalance[234];
} AK308_INFO_PAYLOAD;

typedef struct AK308_SETTIME_Payload {
	unsigned int	t;
} AK308_SETTIME_PAYLOAD;

typedef struct AK308_SERVER_Payload {
	char			szServerAddress[64];
	unsigned short	nDPort;
	unsigned short	nUPort;
} AK308_SERVER_PAYLOAD;

typedef struct AK308_CONFIG_Payload 
{	
	unsigned int	ProfileTimestamp;
	unsigned int	CurrentTime;

	char			Phone[32];
	
	unsigned short	mask;
	
	unsigned int	wakeup;

	float			vcc_threshold;
	unsigned int	vcc_active_time;
	unsigned int	vcc_inactive_time;
	char			vcc_sms_on_active[16];
	char			vcc_sms_on_inactive[16];

	unsigned int	accell_sense;
	unsigned int	accell_active_time;
	unsigned int	accell_inactive_time;
	char			accell_sms_on_active[16];
	char			accell_sms_on_inactive[16];

	unsigned int	lls_period;
	float			lls_threshold;

	unsigned short	DriveInterval;
	unsigned char	DrivePPP;
	unsigned char	DriveFlags;

	unsigned short	SleepDelayInterval;
	unsigned short	SleepWakeupInterval;
	unsigned char	SleepFlags;

	unsigned int	SleepDeepDelayInterval;
	unsigned short	SleepDeepMaxGPSSearchTime;
	unsigned short	SleepDeepMaxGSMSearchTime;
	unsigned short	SleepDeepWakeupInterval;
	unsigned char	SleepDeepFlags;

	unsigned char	io_mode;
	
	unsigned char	Input1Mode;
	unsigned char	Input1Flags;
	unsigned int	Input1ActiveInterval;
	unsigned int	Input1InactiveInterval;
	char 			Input1SMSOnActive[16];
	char 			Input1SMSOnInactive[16];

	unsigned char	Input2Mode;
	unsigned char	Input2Flags;
	unsigned int	Input2ActiveInterval;
	unsigned int	Input2InactiveInterval;
	char 			Input2SMSOnActive[16];
	char 			Input2SMSOnInactive[16];

} AK308_CONFIG_PAYLOAD;


typedef struct AK308_ACK_Payload {
	unsigned short	seqNo;
} AK308_ACK_PAYLOAD;

typedef struct AK308_NACK_Payload {
	unsigned short	seqNo;
} AK308_NACK_PAYLOAD;

typedef union AK308_command_payload {
	AK308_INFO_PAYLOAD		CmdInfo;
	AK308_CONFIG_PAYLOAD  	CmdConfig;	
	AK308_SETTIME_PAYLOAD	CmdSetTime;
	AK308_SERVER_PAYLOAD	CmdServer;
	AK308_ACK_PAYLOAD		CmdAck;
	AK308_NACK_PAYLOAD		CmdNack;
	unsigned char			buffer[1024];	
} AK308_COMMAND_PAYLOAD;

typedef struct AK308_command_header{
	unsigned short	size;
	unsigned char	imei[8];
	unsigned char	command_id;
	unsigned short	seqNo;
	unsigned short	crc;
} AK308_COMMAND_HEADER;

typedef struct AK308_command { 
	AK308_COMMAND_HEADER	header;
	AK308_COMMAND_PAYLOAD	payload;
} AK308COMMAND;

typedef struct cmd_ack_308 {
	unsigned short	size;
	unsigned char	imei[8];
	unsigned char	command_id;
	unsigned short	seqNo;
	unsigned short	crc;
	unsigned short	nSeqNo;
} CMD_ACK308;

#pragma pack () 

#endif

// End
