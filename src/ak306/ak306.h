//******************************************************************************
//
// File Name : ak306.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _AK306_H

#define _AK306_H

#define AK306_INPUT_MODE_UNCONNECTED				0x00
#define AK306_INPUT_MODE_DISCRETE				0x01
#define AK306_INPUT_MODE_FREQUENCY				0x02
#define AK306_INPUT_MODE_INJECTOR				0x04
#define AK306_INPUT_MODE_RS232					0x08

#define AK306_INPUT_FLAG_DIAL					0x01
#define AK306_INPUT_FLAG_NOIGNITION				0x02
#define AK306_INPUT_FLAG_FILTER					0x04
                                                        
#define AK306_ADC_MODE_UNCONNECTED				0x00
#define AK306_ADC_MODE_FUELSENSOR				0x01
#define AK306_ADC_MODE_ANALOGSENSOR				0x02
#define AK306_ADC_MODE_DISCRETE					0x04
#define AK306_ADC_MODE_POWER					0x08
#define AK306_ADC_MODE_RS232_LLS255				0x09
#define AK306_ADC_MODE_FREQ						0x0A

#define AK306_IGNITION_ALWAYS_ON				0x00
#define AK306_IGNITION_BY_INPUT1                0x01
#define AK306_IGNITION_BY_INPUT2                0x02
#define AK306_IGNITION_BY_INPUT3                0x04
#define AK306_IGNITION_BY_INPUT4                0x08
#define AK306_IGNITION_BY_ADC1                  0x10
#define AK306_IGNITION_BY_ADC2                  0x20
#define AK306_IGNITION_BY_CAN					0x40

#define AK306_ENGINE_BY_IGNITION				0x00
#define AK306_ENGINE_BY_INPUT1					0x01
#define AK306_ENGINE_BY_INPUT2					0x02
#define AK306_ENGINE_BY_INPUT3					0x04
#define AK306_ENGINE_BY_INPUT4					0x08
#define AK306_ENGINE_BY_ADC1					0x10
#define AK306_ENGINE_BY_ADC2					0x20
#define AK306_ENGINE_BY_CAN						0x40

#define AK306_MOVE_BY_IGNITION					0x00
#define AK306_MOVE_BY_ODOMETER1					0x01
#define AK306_MOVE_BY_ODOMETER2					0x02
#define AK306_MOVE_BY_ODOMETER3					0x03
#define AK306_MOVE_BY_ODOMETER4					0x04
#define AK306_MOVE_BY_CAN						0x05

#define	AK306_MODE_TYPE_DRIVE					0x01
#define	AK306_MODE_TYPE_SLEEP					0x02
#define	AK306_MODE_TYPE_SLEEP_DEEP				0x04

#define AK306_MODE_FLAG_TRACK_SMOOTH_OFF		0x01
#define AK306_MODE_FLAG_STATIC_NAVIGATION_OFF	0x02

#define AK306_POINT_FLAG_NAVVALID				0x0001
#define AK306_POINT_FLAG_MOVING					0x0002
#define AK306_POINT_FLAG_IGNITION				0x0004
#define AK306_POINT_FLAG_ENGINE					0x0008
#define AK306_POINT_FLAG_DI1					0x0010
#define AK306_POINT_FLAG_DI2					0x0020
#define AK306_POINT_FLAG_DI3					0x0040
#define AK306_POINT_FLAG_DI4					0x0080
#define AK306_POINT_FLAG_O1						0x0100
#define AK306_POINT_FLAG_ACCEELL				0x0200
#define AK306_POINT_FLAG_AI1					0x0400
#define AK306_POINT_FLAG_AI2					0x0800
#define AK306_POINT_FLAG_DRIVE					0x1000
#define AK306_POINT_FLAG_SLEEP					0x2000
#define AK306_POINT_FLAG_SLEEPDEEP				0x4000
#define AK306_POINT_FLAG_STATUS					0x8000

#define AK306_CMD_INFO  						0x01
#define AK306_CMD_CONFIG       					0x02
#define AK306_CMD_SETTIME						0x03
#define AK306_CMD_POSITION     					0x04
#define AK306_CMD_KEEPALIVE    					0x05
#define AK306_CMD_FIRMWARE						0x06
#define AK306_CMD_SERVER						0x07
#define AK306_CMD_ACK							0x08
#define AK306_CMD_NACK							0x09

#define AK306_POLYNOM							0xC003
#define AK306_MAX_ZONES							0x04

#define AK306_MASK_ALTITUDE						0x00000001
#define AK306_MASK_ADC1							0x00000002
#define AK306_MASK_ADC2							0x00000004
#define AK306_MASK_INJECTOR						0x00000008
#define AK306_MASK_INPUT1COUNTER				0x00000010
#define AK306_MASK_INPUT1FREQUENCY				0x00000020
#define AK306_MASK_INPUT1DUTYCYCLE				0x00000040
#define AK306_MASK_INPUT2COUNTER				0x00000080
#define AK306_MASK_INPUT2FREQUENCY				0x00000100
#define AK306_MASK_INPUT2DUTYCYCLE				0x00000200
#define AK306_MASK_INPUT3COUNTER				0x00000400
#define AK306_MASK_INPUT3FREQUENCY				0x00000800
#define AK306_MASK_INPUT3DUTYCYCLE				0x00001000
#define AK306_MASK_INPUT4COUNTER				0x00002000
#define AK306_MASK_INPUT4FREQUENCY				0x00004000
#define AK306_MASK_INPUT4DUTYCYCLE				0x00008000
#define AK306_MASK_CAN_RPM						0x00010000
#define AK306_MASK_CAN_SPEED					0x00020000
#define AK306_MASK_CAN_FUELCONSUME				0x00040000
#define AK306_MASK_CAN_FUELLEVEL				0x00080000
#define AK306_MASK_CAN_MILAGE					0x00100000
#define AK306_MASK_CAN_SUSPENSION				0x00200000
#define AK306_MASK_COG							0x00400000

#define AK306_SUPPORTED_MESSAGES_MASK			0x07FFFFFF

#define AK306_EVENTMASK_ADC1					0x00000001
#define AK306_EVENTMASK_ADC2					0x00000002
#define AK306_EVENTMASK_INPUT1					0x00000004
#define AK306_EVENTMASK_INPUT2					0x00000008
#define AK306_EVENTMASK_INPUT3					0x00000010
#define AK306_EVENTMASK_INPUT4					0x00000020

#define AK306_DIGITAL_PORT_UNCONNECTED			0x00
#define AK306_DIGITAL_PORT_CAN_AUTO				0x01
#define AK306_DIGITAL_PORT_CAN_J1939SILENT		0x02
#define AK306_DIGITAL_PORT_CAN_J1939NORMAL		0x03
#define AK306_DIGITAL_PORT_RS485				0x80

#pragma pack (1) 

typedef struct ak306_point_header{
	unsigned int	mask;
	unsigned int 	t;
	unsigned int 	latitude;
	unsigned int 	longitude;
	unsigned short 	speed;
	unsigned short	flags;
} AK306POINTHEADER;

typedef struct AK306_INFO_Payload {
	unsigned short	nVer;
	unsigned int	tProfile;
	unsigned char		nReason;
	unsigned char		szBalance[234];
} AK306_INFO_PAYLOAD;

typedef struct AK306_SETTIME_Payload {
	unsigned int	t;
} AK306_SETTIME_PAYLOAD;

typedef struct AK306_SERVER_Payload {
	char		szServerAddress[64];
	unsigned short	nDPort;
	unsigned short	nUPort;
} AK306_SERVER_PAYLOAD;

typedef struct AK306_CONFIG_Payload 
{	
	unsigned int	MessageMask1;
	unsigned int	MessageMask2;

	unsigned short	EventMask;

	unsigned int	ProfileTimestamp;
	unsigned int	CurrentTime;

	char 		Phone[32];
	
	unsigned char		IgnitionDetectMethod;
	unsigned char		EngineDetectMethod;
	unsigned char		MoveDetectMethod;
	unsigned short	OdometerMoveTimeout;
	unsigned short	OdometerTicksCount;
		
	unsigned short	DriveInterval;
	unsigned char		DrivePPP;
	unsigned char		DriveFlags;

	unsigned short	SleepDelayInterval;
	unsigned short	SleepWakeupInterval;
	unsigned char		SleepFlags;

	unsigned int	SleepDeepDelayInterval;
	unsigned short	SleepDeepMaxGPSSearchTime;
	unsigned short	SleepDeepMaxGSMSearchTime;
	unsigned short	SleepDeepWakeupInterval;
	unsigned char		SleepDeepFlags;

	unsigned char		Input1Mode;
	unsigned int	Input1ActiveInterval;
	unsigned int	Input1InactiveInterval;
	char 		Input1SMSOnActive[16];
	char 		Input1SMSOnInactive[16];

	unsigned char		Input2Mode;
	unsigned int	Input2ActiveInterval;
	unsigned int	Input2InactiveInterval;
	char 		Input2SMSOnActive[16];
	char 		Input2SMSOnInactive[16];

	unsigned char		Input3Mode;
	unsigned int	Input3ActiveInterval;
	unsigned int	Input3InactiveInterval;
	char 		Input3SMSOnActive[16];
	char 		Input3SMSOnInactive[16];

	unsigned char		Input4Mode;
	unsigned int	Input4ActiveInterval;
	unsigned int	Input4InactiveInterval;
	char 		Input4SMSOnActive[16];
	char 		Input4SMSOnInactive[16];
	unsigned char		Input4InjectorEDSCount;

	unsigned char		ADC1Mode;
	float		ADC1_Threshold;
	float		ADC1_Min;
	char		ADC1_Min_SMS[16];
	float		ADC1_Max;
	char		ADC1_Max_SMS[16];

	unsigned char		ADC2Mode;
	float		ADC2_Threshold;
	float		ADC2_Min;
	char		ADC2_Min_SMS[16];
	float		ADC2_Max;
	char		ADC2_Max_SMS[16];
	
	char		sms[4][16];
	char		script[4][32];

	unsigned int	ZoneId[AK306_MAX_ZONES];
	
	char		Input1Flags;
	char		Input2Flags;
	char		Input3Flags;
	char		Input4Flags;
	char		Input5Flags;
	char		Input6Flags;

	char		dummy[122];
	float		ZoneLat1[AK306_MAX_ZONES];
	float		ZoneLon1[AK306_MAX_ZONES];
	float		ZoneLat2[AK306_MAX_ZONES];
	float		ZoneLon2[AK306_MAX_ZONES];
	unsigned char		ZoneFlags[AK306_MAX_ZONES];

	unsigned short	nMaxSpeed1;
	unsigned short	nMaxSpeed2;
	unsigned short	nMaxSpeed3;
	
	unsigned char		DigitalPortType;
	unsigned int	nRS485Speed;
	unsigned int	nRS232Speed;

	unsigned char		nCMIC;
	unsigned char		nCLVL;
	unsigned char		nES;
	unsigned char		nSES;
	unsigned char		bAutoAnswer;

} AK306_CONFIG_PAYLOAD;

typedef struct AK306_ACK_Payload {
	unsigned short	seqNo;
} AK306_ACK_PAYLOAD;

typedef struct AK306_NACK_Payload {
	unsigned short	seqNo;
} AK306_NACK_PAYLOAD;

typedef union AK306_command_payload {
	AK306_CONFIG_PAYLOAD  	CmdConfig;	
	AK306_SETTIME_PAYLOAD	CmdSetTime;
	AK306_SERVER_PAYLOAD	CmdServer;
	AK306_ACK_PAYLOAD		CmdAck;
	AK306_NACK_PAYLOAD		CmdNack;
	AK306_INFO_PAYLOAD		CmdInfo;
	unsigned char			buffer[1024];
} AK306_COMMAND_PAYLOAD;

typedef struct AK306_command_header{
	unsigned short	size;
	unsigned char		imei[8];
	unsigned char		command_id;
	unsigned short	seqNo;
	unsigned short	crc;
} AK306_COMMAND_HEADER;

typedef struct AK306_command_tx { 
	AK306_COMMAND_HEADER		header;
	AK306_COMMAND_PAYLOAD	payload;
} AK306COMMAND;

typedef struct cmd_ack {
	unsigned short	size;
	unsigned char	imei[8];
	unsigned char	command_id;
	unsigned short	seqNo;
	unsigned short	crc;
	unsigned short	nSeqNo;
} CMD_ACK;

#pragma pack () 

#endif

// End


