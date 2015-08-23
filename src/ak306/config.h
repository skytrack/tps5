//******************************************************************************
//
// File Name : config.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _CONFIG_H

#define _CONFIG_H

typedef struct blob_record
{	
	char 			Phone[32];
	char			fw[32];
	unsigned char	MoveDetectMethod;
	unsigned short	OdometerMoveTimeout;
	unsigned short	OdometerTicksCount;
		
	unsigned short	DriveInterval;
	unsigned char	DrivePPP;

	unsigned short	SleepDelayInterval;
	unsigned short	SleepWakeupInterval;

	unsigned int	SleepDeepDelayInterval;
	unsigned short	SleepDeepMaxGPSSearchTime;
	unsigned short	SleepDeepMaxGSMSearchTime;
	unsigned short	SleepDeepWakeupInterval;

	unsigned char	Input1Mode;
	unsigned int	Input1ActiveInterval;
	unsigned int	Input1InactiveInterval;
	char 			Input1SMSOnActive[16];
	char 			Input1SMSOnInactive[16];

	unsigned char	Input2Mode;
	unsigned int	Input2ActiveInterval;
	unsigned int	Input2InactiveInterval;
	char 			Input2SMSOnActive[16];
	char 			Input2SMSOnInactive[16];

	unsigned char	Input3Mode;
	unsigned int	Input3ActiveInterval;
	unsigned int	Input3InactiveInterval;
	char 			Input3SMSOnActive[16];
	char 			Input3SMSOnInactive[16];

	unsigned char	Input4Mode;
	unsigned int	Input4ActiveInterval;
	unsigned int	Input4InactiveInterval;
	char 			Input4SMSOnActive[16];
	char 			Input4SMSOnInactive[16];

	unsigned char	ADC1Mode;
	float			ADC1_Threshold;

	unsigned char	ADC2Mode;
	float			ADC2_Threshold;
	
	char			sms[4][16];
	char			script[4][32];

	unsigned char	nCMIC;
	unsigned char	nCLVL;
	unsigned char	nES;
	unsigned char	nSES;
	bool			bAutoAnswer;

	bool			altitude;
	bool			cog;
	bool			rpm;

	bool			wait_expanded;
	bool			sleep_expanded;
	bool			input1_expanded;
	bool			input2_expanded;
	bool			input3_expanded;
	bool			input4_expanded;
	bool			adc1_expanded;
	bool			adc2_expanded;
	bool			audio_expanded;

	time_t			timestamp;
	bool			need_profile;

	unsigned int	actual_fw_ver;
	unsigned int	requested_fw_ver;

	unsigned char	info[512];
} BLOB_RECORD;

extern BLOB_RECORD default_config;
extern std::string fw_dir;

int config_init();
int config_destroy();

int config_get_json(DB_OBJECT *object, unsigned char *s, size_t *len);
int config_put_json(DB_OBJECT *object, unsigned char *json, size_t len);
int config_get_device_caps(DB_OBJECT *object, unsigned long long *result);

#endif

// End
