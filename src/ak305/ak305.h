//******************************************************************************
//
// File Name	: ak305.h
// Author	: Skytrack ltd - Copyright (C) 2011
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _AK305_H

#define _AK305_H

#define AK300_DEFAULT_KEY		"AAAAAAAAAAAAAAAA"
#define AK300_AUTH_TIMEOUT    		60
#define AK300_ACK_TIMEOUT		30
#define AK300_TIMEOUT			300

#define TERMINAL_EVENT_ONLINE		0x4001
#define TERMINAL_EVENT_OFFLINE		0x4002
#define TERMINAL_EVENT_REASON		0x4003
#define TERMINAL_UPDATE_START           0x4020
#define TERMINAL_UPDATE_RESTRICTED      0x4021
#define TERMINAL_UPDATE_FAILED          0x4022
#define TERMINAL_UPDATE_DONE            0x4023

#define AK300_PROTOCOL			1
#define AK300_POLYNOM			0xC003

#define AK300_CMD_NEGOTIATE    		1
#define AK300_CMD_OPTION       		3
#define AK300_CMD_RESTART      		4
#define AK300_CMD_FAILED       		5
#define AK300_CMD_KEEPALIVE    		6
#define AK300_CMD_ALIVECONFIRM 		7
#define AK300_CMD_AT           		8
#define AK300_CMD_VERSION  		11
#define AK300_CMD_ACK			13
#define AK300_CMD_SETTIME		14
#define AK300_CMD_PROFILE		15
#define AK300_CMD_FIRMWARE		16
#define AK300_CMD_SERVER		17
#define AK300_CMD_EEPROM		18
#define AK300_CMD_REASON		19
#define AK300_CMD_POSITION     		21
#define AK305_CMD_SENDSMS     		22
#define AK305_CMD_ACKSMS     		23

#define AK300_OPTION_TRANSACTION	5
#define AK300_OPTION_AK300COMBO		6
#define AK300_OPTION_AK300INPUTS	7
#define AK300_OPTION_AK300OUTPUTS	8
#define AK305_OPTION_ZONE		9

#define AK300_INVALID_COMMAND		1
#define AK300_INVALID_DB_USER		2
#define AK300_DBSERVER_ERROR		3
#define AK300_UNKNOWN_TERMINAL		4
#define AK300_UNKNOWN_COMMAND		5
#define AK300_INVALID_PROTOCOL		6

#pragma pack (1) 

typedef struct ak300_position {
	unsigned int	t;
	unsigned short	event_id;
	unsigned int 	latitude;
	unsigned int 	longitude;
	unsigned int 	altitude;
	unsigned short 	speed;
	unsigned int 	di1;
	unsigned int	di2;
	unsigned int	di3;
	unsigned int	di4;
	unsigned short 	ai1;
	unsigned short 	ai2;
	signed   short 	ai3;
	unsigned short 	vbat;
	unsigned short	iant;		
} AK300POSITION;

typedef struct AK300_POSITION_Payload {
	unsigned char	pos_count;
	AK300POSITION	Positions[4];
} AK300_POSITION_PAYLOAD;

typedef struct AK300_FAILED_Payload {
	unsigned int	code;
} AK300_FAILED_PAYLOAD;

typedef struct AK300_AT_Payload {
	char		buf[244];
} AK300_AT_PAYLOAD;

typedef struct AK300_VERSION_Payload {
	unsigned short	nVer;
} AK300_VERSION_PAYLOAD;

typedef struct AK300_SETTIME_Payload {
	unsigned int	t;
} AK300_SETTIME_PAYLOAD;

typedef struct AK300_PROFILE_Payload {
	unsigned int	t;
} AK300_PROFILE_PAYLOAD;

typedef struct AK300_SERVER_Payload {
	unsigned int	nServer;
	unsigned short	nPort;
	unsigned int	nUServer;
	unsigned short	nUPort;
} AK300_SERVER_PAYLOAD;

typedef struct AK300_SENDSMS_Payload {
	unsigned int	rule_id;
	char		phone[34];
	char		msg[160];
} AK300_SENDSMS_PAYLOAD;

typedef struct AK300_ACKSMS_Payload {
	unsigned int	rule_id;
} AK300_ACKSMS_PAYLOAD;

typedef struct AK300_OPTION_Payload {
	unsigned char 	code;
	union {
		struct {
			char code;
			int t;
		} Transaction;

		struct {
			unsigned short 	DriveInterval;
			unsigned char  	DrivePPP;
			unsigned short 	ParkInterval;
			unsigned char  	ParkPPP;
			unsigned short 	SpeakerLevel;			
			unsigned short 	MicLevel;			
			unsigned short 	EchoModel;			
			unsigned short 	EchoLevel;			
			unsigned short 	EchoPatterns;
			char 		Phonebook[5][34];		
			unsigned char	AutoAnswer;
			unsigned char	StopMethod;
			unsigned int 	nMinStopTimeToDetect;
			unsigned int 	nMinDriveTicksToDetect;
			unsigned char	UART_Choice;
			unsigned char	bStaticNavDisabled;
		} ak300_combo;

		struct {
			char Choice[4];
			char DialNumber[4][34];		
			char SMSText[4][16];		
		} ak300_inputs;

		struct {
			char Choice[4];
			char SMSA[4][16];		
			char SMSD[4][16];		
		} ak300_outputs;

		struct {					
			unsigned int zone_id;
			unsigned char mode_id;
			unsigned int tFrom;
			unsigned int tTo;
			unsigned int lat1;
			unsigned int long1;
			unsigned int lat2;
			unsigned int long2;
			char Phone[33];
			char Title[41];
		} ak305_zone;

	} Option; 
} AK300_OPTION_PAYLOAD;

typedef struct ak300_ALIVECONFIRM_Payload {
	unsigned char	dummy;
} AK300_ALIVECONFIRM_PAYLOAD;

typedef struct ak300_REASON_Payload {
	unsigned char	nReason;
	unsigned char	nTask;
} AK300_REASON_PAYLOAD;

typedef union ak300_command_payload {
	AK300_POSITION_PAYLOAD 		CmdPosition;	
	AK300_OPTION_PAYLOAD  		CmdOption;	
	AK300_FAILED_PAYLOAD		CmdFailed;
	AK300_AT_PAYLOAD		CmdAT;
	AK300_VERSION_PAYLOAD		CmdVersion;
	AK300_SETTIME_PAYLOAD		CmdSetTime;
	AK300_ALIVECONFIRM_PAYLOAD	CmdAliveConfirm;
	AK300_PROFILE_PAYLOAD		CmdProfile;
	AK300_SERVER_PAYLOAD		CmdServer;
	AK300_REASON_PAYLOAD		CmdReason;
	AK300_SENDSMS_PAYLOAD		CmdSendSMS;
	AK300_ACKSMS_PAYLOAD		CmdAckSMS;
	unsigned char			CmdNegotiate[16];
	unsigned char			CmdFirmware[240];
	unsigned char			RawData[240];
} AK300_COMMAND_PAYLOAD;

typedef struct ak300_command_header{
	unsigned short 		size;
	unsigned char 		protocol_id;
	unsigned char 		command_id;
	unsigned int		seqNo;
	unsigned char		reserved1;
	unsigned char		reserved2;
	unsigned char		reserved3;
	unsigned char		reserved4;
	unsigned short 		crc;
} AK300_COMMAND_HEADER;

typedef struct ak300_command { 
	AK300_COMMAND_HEADER	header;
	AK300_COMMAND_PAYLOAD	payload;
	unsigned char		Padding[8];
} AK300COMMAND;

int AK305_Start(
			const char *lpszDBAddress,
			const char *lpszDBName,
			const char *lpszDBLogin,
			const char *lpszDBPassword,
			const char *lpsz305Address,
			const unsigned short _nDataPort,
			const unsigned short _nUpdatePort,			
			const char *lpszFWPath,
			const char *lpszDefaultFW
		);

int AK305_Cleanup();
int AK305_Stop();

#pragma pack () 

#endif

// End
