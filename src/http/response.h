//******************************************************************************
//
// File Name : response.h
// Author  : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _RESPONSE_H

#define _RESPONSE_H

extern unsigned char response_success_ka[];
extern const size_t response_success_ka_length;
extern unsigned char response_success_close[];
extern const size_t response_success_close_length;
extern unsigned char response_fail_ka[];
extern const size_t response_fail_ka_length;
extern unsigned char response_fail_close[];
extern const size_t response_fail_close_length;
extern unsigned char response_fail_msg_ka[];
extern const size_t response_fail_msg_ka_length;
extern unsigned char response_fail_msg_close[];
extern const size_t response_fail_msg_close_length;
extern unsigned char response_fail_400_ka[];
extern const size_t response_fail_400_ka_length;
extern unsigned char response_fail_400_close[];
extern const size_t response_fail_400_close_length;
extern unsigned char response_fail_401_ka[];
extern const size_t response_fail_401_ka_length;
extern unsigned char response_fail_401_close[];
extern const size_t response_fail_401_close_length;
extern unsigned char response_fail_403_ka[];
extern const size_t response_fail_403_ka_length;
extern unsigned char response_fail_403_close[];
extern const size_t response_fail_403_close_length;
extern unsigned char response_fail_404_ka[];
extern const size_t response_fail_404_ka_length;
extern unsigned char response_fail_404_close[];
extern const size_t response_fail_404_close_length;
extern unsigned char response_fail_405_ka[];
extern const size_t response_fail_405_ka_length;
extern unsigned char response_fail_405_close[];
extern const size_t response_fail_405_close_length;
extern unsigned char response_fail_413_ka[];
extern const size_t response_fail_413_ka_length;
extern unsigned char response_fail_413_close[];
extern const size_t response_fail_413_close_length;
extern unsigned char response_fail_414_ka[];
extern const size_t response_fail_414_ka_length;
extern unsigned char response_fail_414_close[];
extern const size_t response_fail_414_close_length;
extern unsigned char response_fail_415_ka[];
extern const size_t response_fail_415_ka_length;
extern unsigned char response_fail_415_close[];
extern const size_t response_fail_415_close_length;
extern unsigned char response_fail_422_ka[];
extern const size_t response_fail_422_ka_length;
extern unsigned char response_fail_422_close[];
extern const size_t response_fail_422_close_length;
extern unsigned char response_fail_431_ka[];
extern const size_t response_fail_431_ka_length;
extern unsigned char response_fail_431_close[];
extern const size_t response_fail_431_close_length;
extern unsigned char response_fail_500_ka[];
extern const size_t response_fail_500_ka_length;
extern unsigned char response_fail_500_close[];
extern const size_t response_fail_500_close_length;

extern unsigned char response_success_empty_object_ka[];
extern const size_t response_success_empty_object_ka_length;
extern unsigned char response_success_empty_object_close[];
extern const size_t response_success_empty_object_close_length;

extern unsigned char response_success_empty_array_ka[];
extern const size_t response_success_empty_array_ka_length;
extern unsigned char response_success_empty_array_close[];
extern const size_t response_success_empty_array_close_length;

extern unsigned char response_redirect_ka[];
extern const size_t response_redirect_ka_length;
extern unsigned char response_redirect_close[];
extern const size_t response_redirect_close_length;

int response_fail_with_message(unsigned char *s, size_t *l, const unsigned char *message, const size_t msg_len, const bool keep_alive);
unsigned char *response_success_object(unsigned char *dst, size_t *dst_size, const bool keep_alive, unsigned char **content_length_ptr, size_t *initial_content_length);
int response_success_id(unsigned char *dst, size_t *dst_size, const bool keep_alive, unsigned int id);

#endif

// End