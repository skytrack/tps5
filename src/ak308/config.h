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
	char			fw[33];

	unsigned int	port;

	unsigned int	wakeup;
	unsigned int	wakeup_active_interval;
	unsigned int	wakeup_inactive_interval;
	char			wakeup_sms_on_active[16];
	char			wakeup_sms_on_inactive[16];

	bool			vcc_expanded;
	bool			vcc_as_di;
	float			vcc_threshold;
	unsigned int	vcc_active_time;
	unsigned int	vcc_inactive_time;
	char			vcc_sms_on_active[16];
	char			vcc_sms_on_inactive[16];
	bool			vcc_transmit;

	bool			accell_expanded;
	bool			accell_as_di;
	unsigned int	accell_sense;
	unsigned int	accell_active_time;
	unsigned int	accell_inactive_time;
	char			accell_sms_on_active[16];
	char			accell_sms_on_inactive[16];
	bool			accell_transmit;

	float			fuel_threshold;
	unsigned int	fuel_period;

	unsigned int	DriveInterval;
	unsigned int	DrivePPP;

	bool			wait_expanded;
	unsigned int	SleepDelayInterval;
	unsigned int	SleepWakeupInterval;
	bool			wait_after_fuel;
	bool			wait_to_active_fuel;

	bool			sleep_expanded;
	unsigned int	SleepDeepDelayInterval;
	unsigned int	SleepDeepWakeupInterval;
	unsigned int	SleepDeepMaxGPSSearchTime;
	unsigned int	SleepDeepMaxGSMSearchTime;
	bool			sleep_after_fuel;
	bool			sleep_to_active_fuel;

	bool			input1_expanded;
	unsigned int	Input1Mode;
	unsigned int	Input1ActiveInterval;
	unsigned int	Input1InactiveInterval;
	char			Input1SMSOnActive[16];
	char			Input1SMSOnInactive[16];

	bool			input2_expanded;
	unsigned int	Input2Mode;
	unsigned int	Input2ActiveInterval;
	unsigned int	Input2InactiveInterval;
	char			Input2SMSOnActive[16];
	char			Input2SMSOnInactive[16];

	unsigned char	Phone[33];

	bool			altitude;
	bool			cog;

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
