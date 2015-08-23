//******************************************************************************
//
// File Name : config.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _CONFIG_305_H

#define _CONFIG_305_H

typedef struct blob_record
{	
	char 			Phone[32];
	char			fw[32];
		
	unsigned short	DriveInterval;
	unsigned char	DrivePPP;

	unsigned short	ParkInterval;
	unsigned char	ParkPPP;

	bool			bDisableStatic;

	unsigned char	Input1Mode;
	char 			Input1SMSOnActive[16];

	unsigned char	Input2Mode;
	char 			Input2SMSOnActive[16];

	unsigned char	Input3Mode;
	char 			Input3SMSOnActive[16];

	unsigned char	Input4Mode;
	char 			Input4SMSOnActive[16];

	char			sms_a1[16];		
	char			sms_a2[16];		
	char			sms_a3[16];		
	char			sms_a4[16];		
	char			sms_d1[16];		
	char			sms_d2[16];		
	char			sms_d3[16];		
	char			sms_d4[16];		

	unsigned char	nCMIC;
	unsigned char	nCLVL;
	unsigned short 	EchoModel;			
	unsigned short 	EchoLevel;			
	unsigned short 	EchoPatterns;
	bool			bAutoAnswer;

	bool			input1_expanded;
	bool			input2_expanded;
	bool			input3_expanded;
	bool			input4_expanded;
	bool			audio_expanded;

	time_t			timestamp;
	bool			need_profile;

	unsigned int	actual_fw_ver;
	unsigned int	requested_fw_ver;

	unsigned char	info[512];
} BLOB_RECORD_305;

extern BLOB_RECORD_305 default_config;
extern std::string fw_dir;

int config_init();
int config_destroy();

int config_get_json(DB_OBJECT *object, unsigned char *s, size_t *len);
int config_put_json(DB_OBJECT *object, unsigned char *json, size_t len);
int config_get_device_caps(DB_OBJECT *object, unsigned long long *result);

#endif

// End
