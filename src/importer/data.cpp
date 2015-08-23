//******************************************************************************
//
// File Name : data.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "api.h"
#include "common.h"
#include "data.h"
#include "../core/record.h"

#define SESSION_STATE_PACK_HEADER	0
#define SESSION_STATE_PACK_DATA		1
#define SESSION_STATE_RECORD_SIZE	2
#define SESSION_STATE_RECORD_DATA	3

static unsigned char *bit1 = record_data + 0;
static unsigned char *bit2 = record_data + 1;
static unsigned char *bit3 = record_data + 2;
static unsigned char *bit4 = record_data + 3;
static unsigned char *bit5 = record_data + 4;

DATA_SESSION *data_session_open()
{
	DATA_SESSION *session = (DATA_SESSION *)malloc(sizeof(DATA_SESSION));							

	session->pack_state				= SESSION_STATE_PACK_HEADER;
	session->pack_bytes_received	= 0;
	session->ptr					= (unsigned char *)&session->pack;

	return session;
}

void data_session_close(DATA_SESSION *session)
{
	api_log_printf("[IMPORTER] Closing session 0x%08X\r\n", session);
	free(session);
}

int data_session_timer(DATA_SESSION *session, char **p, size_t *l)
{
	data_session_close(session);

	*l = 0;

	return SESSION_COMPLETE;
}

int data_session_data(DATA_SESSION *session, unsigned char **p, size_t *l)
{
	unsigned char *data = *p;
	unsigned char *last_byte = data + *l;

	while (data != last_byte) {

		switch (session->pack_state) {

		default:
		case SESSION_STATE_PACK_HEADER:

			while ((data != last_byte)&&(session->pack_bytes_received != 8)) {
				session->pack_bytes_received++;
				*session->ptr++ = *data++;
			}

			if (session->pack_bytes_received != 8) {
				*l = 0;
				return 30;
			}

		
			api_log_printf("[Importer] Header id: %u, size: %u\r\n", session->pack.id, session->pack.size);

			session->pack_state		= SESSION_STATE_PACK_DATA;
			session->record_state	= SESSION_STATE_RECORD_SIZE;
			session->counter		= 0;
		
		case SESSION_STATE_PACK_DATA:
		
			while ((data != last_byte)&&(session->pack_bytes_received != session->pack.size)) {
				session->pack_bytes_received++;

				unsigned char ch = *data++;

				switch (session->record_state) {
				case SESSION_STATE_RECORD_SIZE:

					session->ptr = (unsigned char *)&session->pack.record;

					*session->ptr++ = ch;

					session->record_bytes_received = 1;
					session->record_state = SESSION_STATE_RECORD_DATA;					

					break;

				case SESSION_STATE_RECORD_DATA:

					*session->ptr++ = ch;
					session->record_bytes_received++;

					if (session->record_bytes_received == session->pack.record.size) {
						
						session->record_state = SESSION_STATE_RECORD_SIZE;

						IMPORT_RECORD *r = &session->pack.record;
						
						size_t i;
						for (i = 0; i < object_id.size(); i++) {

							if (memcmp(dev_id + i * 8, r->dev_id, 8) == 0) {

								void *stream = api_storage_get_stream_by_id(object_id[i]);

								if (stream != NULL) {

									record->t = r->t;

									unsigned char *src = r->var;
									unsigned short	*event_id;
									int				*lat;
									int				*lng;
									unsigned short	*speed;
									short			*alt;
									unsigned short	*cog;
									unsigned short	*counter1;
									unsigned short	*counter2;
									unsigned short	*counter3;
									unsigned short	*counter4;
									unsigned short	*frequency1;
									unsigned short	*frequency2;
									unsigned short	*frequency3;
									unsigned short	*frequency4;
									unsigned short	*analog1;
									unsigned short	*analog2;
									unsigned short	*analog3;
									unsigned int	*injector;
									unsigned short	*rs485_1;
									unsigned short	*rs485_2;
									unsigned short	*rs232_1;
									unsigned short	*rs232_2;
								
									*bit1 = RECORD_BIT1_FLAGS;
									*bit2 = 0;
									*bit3 = 0;
									*bit4 = 0;
									*bit5 = 0;

									if (r->flags & IMPORTER_FLAG_NAV) {

										lat = (int *)src;
										src += 4;
										lng = (int *)src;
										src += 4;
										speed = (unsigned short *)src;
										src += 2;

										*bit1 |= RECORD_BIT1_NAV;
									}

									if (r->flags & IMPORTER_FLAG_ALT) {
										alt = (short *)src;
										src += 2;

										*bit1 |= RECORD_BIT1_ALT;
									}

									if (r->flags & IMPORTER_FLAG_COG) {
										cog = (unsigned short *)src;
										src += 2;

										*bit1 |= RECORD_BIT1_COG;
									}

									if (r->flags & IMPORTER_FLAG_COUNTER1) {
										counter1 = (unsigned short *)src;
										src += 2;

										*bit3 |= RECORD_BIT3_COUNTER1;
									}

									if (r->flags & IMPORTER_FLAG_COUNTER2) {
										counter2 = (unsigned short *)src;
										src += 2;

										*bit3 |= RECORD_BIT3_COUNTER2;
									}

									if (r->flags & IMPORTER_FLAG_COUNTER3) {
										counter3 = (unsigned short *)src;
										src += 2;

										*bit3 |= RECORD_BIT3_COUNTER3;
									}
									
									if (r->flags & IMPORTER_FLAG_COUNTER4) {
										counter4 = (unsigned short *)src;
										src += 2;

										*bit3 |= RECORD_BIT3_COUNTER4;
									}

									if (r->flags & IMPORTER_FLAG_FREQUENCY1) {
										frequency1 = (unsigned short *)src;
										src += 2;

										*bit2 |= RECORD_BIT2_FREQUENCY1;
									}

									if (r->flags & IMPORTER_FLAG_FREQUENCY2) {
										frequency2 = (unsigned short *)src;
										src += 2;

										*bit2 |= RECORD_BIT2_FREQUENCY2;
									}

									if (r->flags & IMPORTER_FLAG_FREQUENCY3) {
										frequency3 = (unsigned short *)src;
										src += 2;

										*bit2 |= RECORD_BIT2_FREQUENCY3;
									}

									if (r->flags & IMPORTER_FLAG_FREQUENCY4) {
										frequency4 = (unsigned short *)src;
										src += 2;

										*bit2 |= RECORD_BIT2_FREQUENCY4;
									}

									if (r->flags & IMPORTER_FLAG_ANALOG1) {
										analog1 = (unsigned short *)src;
										src += 2;

										*bit2 |= RECORD_BIT2_ADC1;
									}

									if (r->flags & IMPORTER_FLAG_ANALOG2) {
										analog2 = (unsigned short *)src;
										src += 2;

										*bit2 |= RECORD_BIT2_ADC2;
									}

									if (r->flags & IMPORTER_FLAG_ANALOG3) {
										analog3 = (unsigned short *)src;
										src += 2;

										*bit2 |= RECORD_BIT2_ADC3;
									}

									if (r->flags & IMPORTER_FLAG_INJECTOR) {
										injector = (unsigned int *)src;
										src += 4;

										*bit5 |= RECORD_BIT5_INJECTOR;
									}

									if (r->flags & IMPORTER_FLAG_RS485_1) {
										rs485_1 = (unsigned short *)src;
										src += 2;

										*bit1 |= RECORD_BIT1_RS485_1;
									}

									if (r->flags & IMPORTER_FLAG_RS485_2) {
										rs485_2 = (unsigned short *)src;
										src += 2;

										*bit1 |= RECORD_BIT1_RS485_2;
									}

									if (r->flags & IMPORTER_FLAG_RS232_1) {
										rs232_1 = (unsigned short *)src;
										src += 2;

										*bit4 |= RECORD_BIT4_RS232_1;
									}

									if (r->flags & IMPORTER_FLAG_RS232_2) {
										rs232_2 = (unsigned short *)src;
										src += 2;

										*bit4 |= RECORD_BIT4_RS232_2;
									}

									if (r->flags & IMPORTER_FLAG_EVENT) {
										event_id = (unsigned short *)src;
										src += 2;

										*bit1 |= RECORD_BIT1_EVENT;
									}

									unsigned char *dst;

									if (*bit5 != 0) {
										*bit1 |= RECORD_BIT_MORE;
										*bit2 |= RECORD_BIT_MORE;
										*bit3 |= RECORD_BIT_MORE;
										*bit4 |= RECORD_BIT_MORE;
										dst = bit5 + 1;
									}
									else
									if (*bit4 != 0) {
										*bit1 |= RECORD_BIT_MORE;
										*bit2 |= RECORD_BIT_MORE;
										*bit3 |= RECORD_BIT_MORE;
										dst = bit4 + 1;
									}
									else
									if (*bit3 != 0) {
										*bit1 |= RECORD_BIT_MORE;
										*bit2 |= RECORD_BIT_MORE;
										dst = bit3 + 1;
									}
									else
									if (*bit2 != 0) {
										*bit1 |= RECORD_BIT_MORE;
										dst = bit2 + 1;
									}
									else {
										dst = bit1 + 1;
									}

									if (*bit1 & RECORD_BIT1_EVENT) {
										*(unsigned short *)dst = *event_id;
										dst += 2;
									}

									if (*bit4 & RECORD_BIT4_RS232_1) {
										int a = 1;
									}

									if (*bit1 & RECORD_BIT1_FLAGS) {

										unsigned char *flags1 = dst++;
										unsigned char *flags2 = dst;

										*flags1 = 0;
										*flags2 = 0;

										if (r->flags & IMPORTER_FLAG_DI1)
											*flags2 |= RECORD_FLAG2_DI1;

										if (r->flags & IMPORTER_FLAG_DI2)
											*flags2 |= RECORD_FLAG2_DI2;

										if (r->flags & IMPORTER_FLAG_DI3)
											*flags2 |= RECORD_FLAG2_DI3;

										if (r->flags & IMPORTER_FLAG_DI4)
											*flags2 |= RECORD_FLAG2_DI4;

										if (r->flags & IMPORTER_FLAG_IGNITION)
											*flags1 |= RECORD_FLAG1_IGNITION;

										if (r->flags & IMPORTER_FLAG_ENGINE)
											*flags1 |= RECORD_FLAG1_ENGINE;

										if (r->flags & IMPORTER_FLAG_MOVE)
											*flags1 |= RECORD_FLAG1_MOVE;

										if (*flags2 != 0) {
											*flags1 |= RECORD_FLAG1_MORE;
											dst++;
										}

										if (*bit1 & RECORD_BIT1_NAV) {
											*(int *)dst = *lat;
											dst += 4;
											*(int *)dst = *lng;
											dst += 4;
											*dst++ = *speed & 0xFF;

											if (*speed & 0x100)
												*flags1 |= RECORD_FLAG1_SPEED_9;
											if (*speed & 0x200)
												*flags1 |= RECORD_FLAG1_SPEED_10;
											if (*speed & 0x400)
												*flags1 |= RECORD_FLAG1_SPEED_11;

											if (*bit1 & RECORD_BIT1_ALT) {
												*(short *)dst = *alt;
												dst += 2;
											}

											if (*bit1 & RECORD_BIT1_COG) {
												*dst++ = *cog & 0xFF;
												if (*cog & 0x100)
													*flags1 |= RECORD_FLAG1_COG_9;
											}
										}
									}

									if (*bit1 & RECORD_BIT1_RS485_1) {
										*(unsigned short *)dst = *rs485_1;
										dst += 2;
									}

									if (*bit1 & RECORD_BIT1_RS485_2) {
										*(unsigned short *)dst = *rs485_2;
										dst += 2;
									}

									if (*bit1 & RECORD_BIT_MORE) {
									
										if (*bit2 & RECORD_BIT2_ADC1) {
											*(unsigned short *)dst = *analog1;
											dst += 2;
										}

										if (*bit2 & RECORD_BIT2_ADC2) {
											*(unsigned short *)dst = *analog2;
											dst += 2;
										}

										if (*bit2 & RECORD_BIT2_ADC3) {
											*(unsigned short *)dst = *analog3;
											dst += 2;
										}

										if (*bit2 & RECORD_BIT2_FREQUENCY1) {
											*(unsigned short *)dst = *frequency1;
											dst += 2;
										}
										if (*bit2 & RECORD_BIT2_FREQUENCY2) {
											*(unsigned short *)dst = *frequency2;
											dst += 2;
										}
										if (*bit2 & RECORD_BIT2_FREQUENCY3) {
											*(unsigned short *)dst = *frequency3;
											dst += 2;
										}
										if (*bit2 & RECORD_BIT2_FREQUENCY4) {
											*(unsigned short *)dst = *frequency4;
											dst += 2;
										}

										if (*bit2 & RECORD_BIT_MORE) {

											if (*bit3 & RECORD_BIT3_COUNTER1) {
												*(unsigned short *)dst = *counter1;
												dst += 2;
											}
											if (*bit3 & RECORD_BIT3_COUNTER2) {
												*(unsigned short *)dst = *counter2;
												dst += 2;
											}
											if (*bit3 & RECORD_BIT3_COUNTER3) {
												*(unsigned short *)dst = *counter3;
												dst += 2;
											}
											if (*bit3 & RECORD_BIT3_COUNTER4) {
												*(unsigned short *)dst = *counter4;
												dst += 2;
											}

											if (*bit3 & RECORD_BIT_MORE) {

												if (*bit4 & RECORD_BIT4_RS232_1) {
													*(unsigned short *)dst = *rs232_1;
													dst += 2;
												}
												if (*bit4 & RECORD_BIT4_RS232_2) {
													*(unsigned short *)dst = *rs232_2;
													dst += 2;
												}

												if (*bit4 & RECORD_BIT_MORE) {
													if (*bit5 & RECORD_BIT5_INJECTOR) {
														*(unsigned int *)dst = *injector;
														dst += 4;
													}
												}
											}
										}
									}

									record->size = dst - record_buffer;

									api_storage_add_record_to_stream(stream, record, record->size);

									session->counter++;
								}
								else {

									char imei[16];

									imei[0] = (r->dev_id[0] >> 4) + '0';
									imei[1] = (r->dev_id[0] & 0x0F) + '0';
									imei[2] = (r->dev_id[1] >> 4) + '0';
									imei[3] = (r->dev_id[1] & 0x0F) + '0';
									imei[4] = (r->dev_id[2] >> 4) + '0';
									imei[5] = (r->dev_id[2] & 0x0F) + '0';
									imei[6] = (r->dev_id[3] >> 4) + '0';
									imei[7] = (r->dev_id[3] & 0x0F) + '0';
									imei[8] = (r->dev_id[4] >> 4) + '0';
									imei[9] = (r->dev_id[4] & 0x0F) + '0';
									imei[10] = (r->dev_id[5] >> 4) + '0';
									imei[11] = (r->dev_id[5] & 0x0F) + '0';
									imei[12] = (r->dev_id[6] >> 4) + '0';
									imei[13] = (r->dev_id[6] & 0x0F) + '0';
									imei[14] = (r->dev_id[7] >> 4) + '0';
									imei[15] = '\0';

//									api_log_printf("[IMPORTER] No stream for device [%s]\r\n", imei);
								}

								break;
							}
						}

						if (i == object_id.size()) {

							char imei[16];

							imei[0] = (r->dev_id[0] >> 4) + '0';
							imei[1] = (r->dev_id[0] & 0x0F) + '0';
							imei[2] = (r->dev_id[1] >> 4) + '0';
							imei[3] = (r->dev_id[1] & 0x0F) + '0';
							imei[4] = (r->dev_id[2] >> 4) + '0';
							imei[5] = (r->dev_id[2] & 0x0F) + '0';
							imei[6] = (r->dev_id[3] >> 4) + '0';
							imei[7] = (r->dev_id[3] & 0x0F) + '0';
							imei[8] = (r->dev_id[4] >> 4) + '0';
							imei[9] = (r->dev_id[4] & 0x0F) + '0';
							imei[10] = (r->dev_id[5] >> 4) + '0';
							imei[11] = (r->dev_id[5] & 0x0F) + '0';
							imei[12] = (r->dev_id[6] >> 4) + '0';
							imei[13] = (r->dev_id[6] & 0x0F) + '0';
							imei[14] = (r->dev_id[7] >> 4) + '0';
							imei[15] = '\0';

							api_log_printf("[IMPORTER] Unknown device [%s]\r\n", imei);
						}

					}
				}
			}

			if (session->pack_bytes_received == session->pack.size) {

				api_log_printf("[Importer] Handled %u records\r\n", session->counter);

				session->pack_state				= SESSION_STATE_PACK_HEADER;
				session->pack_bytes_received	= 0;
				session->ptr					= (unsigned char *)&session->pack;

				*p = (unsigned char *)&session->pack.id;
				*l = sizeof(session->pack.id);

				return 300;
			}
			else {
				api_log_printf("[Importer] %u %u\r\n", session->pack_bytes_received, session->pack.size);
				*l = 0;
				return 30;
			}
		}
	}

	*l = 0;

	return 30;
}

