//******************************************************************************
//
// File Name : module.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <map>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "module.h"
#include "median7.h"
#include "pla.h"
#include "../core/cross.h"
#include "../core/aligned.h"
#include "immintrin.h"

typedef struct fcb_data
{
	unsigned char *ptr;
	size_t bytes_left;
	int t_from;
	int t_to;
#ifdef DEBUG
	std::map<int, float> data;
#endif
} FCB_DATA;

void RealFuelCallback(int nTime, float fFuelLevel, void *pData)
{
	FCB_DATA *fcb = (FCB_DATA *)pData;
#ifdef DEBUG
	fcb->data[nTime] = fFuelLevel;
#endif
}

void FillCallback(int nTimeFrom, int nTimeTo, float fFuelFrom, float fFuelTo, void *pData)
{
	FCB_DATA *fcb = (FCB_DATA *)pData;

	if ((nTimeFrom >= fcb->t_from)&&(nTimeFrom < fcb->t_to)) {
	
		if (fcb->bytes_left > 100) {
	
			size_t len = sprintf((char *)fcb->ptr, "{\"start\":%u,\"end\":%u,\"fuel\":%f},", nTimeFrom, nTimeTo, fFuelTo - fFuelFrom);

			fcb->bytes_left -= len;
			fcb->ptr += len;
		}
	}	
}

void DrainCallback(int nTimeFrom, int nTimeTo, float fFuelFrom, float fFuelTo, void *pData)
{
	FCB_DATA *fcb = (FCB_DATA *)pData;

	if ((nTimeFrom >= fcb->t_from)&&(nTimeFrom < fcb->t_to)) {
	
		if (fcb->bytes_left > 100) {
	
			size_t len = sprintf((char *)fcb->ptr, "{\"start\":%u,\"end\":%u,\"fuel\":%f},", nTimeFrom, nTimeTo, fFuelTo - fFuelFrom);

			fcb->bytes_left -= len;
			fcb->ptr += len;
		}
	}	
}

size_t fuel_process2(int *time, float *lat, float *lng, unsigned short *speed, float *fuel, size_t data_length, 
					int t_from, int t_to,
					float fill_threshold, float drain_threshold, size_t filter_length,
					unsigned char *buffer, size_t bytes_left, unsigned int flags)
{
	FCB_DATA fcb;

	Median7_Init();
	Median7(fuel, data_length);

	bool rely_on_ignition = (flags & 1) > 0;
	bool rely_on_moving = (flags & 2) > 0;

	unsigned char *ptr = buffer;

	if (bytes_left < 1)
		return 0;

	*ptr++ = '[';
	bytes_left--;

	PLA pla = PLA_Create();
		
	PLA_SetBufferSize(pla, 1000);

	fcb.ptr = ptr;
	fcb.bytes_left = bytes_left;
	fcb.t_from = t_from;
	fcb.t_to = t_to;

	PLA_SetFuelCallback(pla, &RealFuelCallback, &fcb);
	PLA_SetFillCallback(pla, &FillCallback, &fcb);
	PLA_SetDrainCallback(pla, &DrainCallback, &fcb);
	PLA_SetFillThreshold(pla, (int)fill_threshold);
	PLA_SetDrainThreshold(pla, (int)drain_threshold);
	PLA_SetMaxFuelRate(pla, 50);

	for (size_t i = 0; i < data_length; i++) {
		PLA_ProcessFuel(pla, time[i], fuel[i], 0);
	}

	PLA_Destroy(pla);

	ptr = fcb.ptr;
	bytes_left = fcb.bytes_left;

/*
	// Прохожу по графику окном в 512 точек в поисках разнцы уровня топлива выше порога заправки
	for (size_t i = 0; i < data_length; i++) {

		// Начало заправки на скорости выше 10 км/ч невозможно
		if ((speed[i] & 0x7FF) > 100)
			continue;

		for (size_t k = i + 1; (k < data_length) && (k < i + 512); k++) {
			
			float delta = fuel[k] - fuel[i];
			
			if (delta > fill_threshold) {

				printf("%u %u %f\r\n", time[i], time[k], fuel[i]);

				// В этом месте между точками i и k однозначно есть заправка
				// Но по факту i может лежать за долго до фактического начала заправки
				
				// Вторым шагом ищу точку, начиная с которой все точки будут выше начальной, это не означает непрерывный рост от точки к точке, но все они будут выше начальной.
				bool start_point = false;
				for (size_t j = i; j < k; j++) {
					for (size_t n = j + 1; n < k; n++) {
						if (time[n] - time[j] > 60) {
							if (fuel[n] - fuel[j] > 5) {
	
								for (size_t min_i = j; min_i < k; min_i++) {
					
									start_point = true;

									for (size_t m = j + 1; m < k; m++) {
										if (fuel[m] <= fuel[j]) {
											start_point = false;
											break;
										}
									}

									if (start_point) {
										i = j;
										j = k;
										break;
									}
								}
							}

							break;
						}
					}
				}
				
				if (!start_point)
					continue;

				size_t min_i = i;
				float min_fuel = fuel[i];

				size_t j = k + 1;
				while ((j < data_length)&&(fuel[j] > fuel[j - 1]))
					j++;
				j--;

				for (size_t ii = j; ii < data_length; ii++) {

					if (fuel[ii] > fuel[j])
						j = ii;

					for (size_t kk = ii + 1; (kk < data_length) && (kk < ii + 512); kk++) {
						float deltadelta = fuel[kk] - fuel[ii];
						if (deltadelta > fill_threshold) {
							break;
						}
					}
				}

				if ((time[min_i] >= t_from)&&(time[min_i] < t_to)) {
					size_t len = sprintf((char *)ptr, "{\"start\":%u,\"end\":%u,\"fuel\":%f,\"lat\":%d,\"lng\":%d},", time[min_i], time[j], fuel[j] - min_fuel, (int)(lat[min_i] * 10000000), (int)(lng[min_i] * 10000000));
					ptr += len;
					bytes_left -= len;
				}

				i = j - 1;

				break;
			}
		}
	}

	if (rely_on_ignition) {

		size_t i = 0;

		bool prev_ignition;

		while ((i < data_length)&&((prev_ignition = (speed[i] & 0x8000) > 0) == false))
			i++;

		int i_stop;

		for (; i < data_length; i++) {

			bool ignition = (speed[i] & 0x8000) > 0;

			if (ignition != prev_ignition) {

				if (ignition) {

					// Уровень изменения топлива за период выключенного зажигания
					float fDelta = fuel[i] - fuel[i_stop];

					// Если пробит порог заправки
					if (fDelta >= fill_threshold) {
					
						char bEarly = 0;

						// С момента включения зажигания топливо может только падать

						// Если уровень поднимется выше - это не реальный конец заправки, т.е. зажигание включили в момент заправки
						float fMax = fuel[i] + 1;

						// Предполагаю что после этой заправки следующая должна быть не скоро, по крайней мере должны потратить 20% от залитого
						float fMin = (float)(fuel[i] - fDelta * 0.2);

						// Анализирую дальнейшее поведение
						// Если потратили 20% от заправки и ни разу не поднялись выше порога - это реальный конец заправки
						for (size_t i_test = i + 1; i_test < data_length; i_test++) {
						
							if (fuel[i_test] >= fMax) {
								bEarly = 1;
								break;
							}
							if (fuel[i_test] < fMin)
								break;
						}

						// Аналогично в обратную сторону чтобы убедиться что зажигание было выключено не в момент заправки
						fMin = fuel[i_stop] - 1;
						fMax = (float)(fuel[i_stop] + fDelta * 0.2);

						for (int i_test = i - 1; i_test >= 0; i_test--) {
						
							if (fuel[i_test] <= fMin) {
								bEarly = 1;
								//printf("late ignition off\r\n");
								break;
							}
							if (fuel[i_test] > fMax)
								break;
						}

						if (bEarly == 0) {
							if (time[i_stop] >= t_from) {
								size_t len = sprintf((char *)ptr, "{\"start\":%u,\"end\":%u,\"fuel\":%f},", time[i_stop], time[i], fDelta);
								ptr += len;
								bytes_left -= len;
							}
						}
					}
				}
				else {
					i_stop = i;
				}

				prev_ignition = ignition;
			}
		}
	}
*/
	if (*(ptr - 1) == ',') {
		ptr--;
		bytes_left++;
	}

	if (bytes_left < 1)
		return 0;

	*ptr++ = ']';

#ifdef DEBUG
	*ptr++ = ',';
	*ptr++ = 'c';
	*ptr++ = 'h';
	*ptr++ = 'a';
	*ptr++ = 'r';
	*ptr++ = 't';
	*ptr++ = ':';
	*ptr++ = '[';
	for (std::map<int, float>::iterator it = fcb.data.begin(); it != fcb.data.end(); ++it) {
		ptr += sprintf((char *)ptr, "{\"t\":%u,\"data\":%f},", it->first, it->second);
	}
	if (*(ptr - 1) == ',')
		ptr--;
	*ptr++ = ']';
#endif

	return ptr - buffer;
}

typedef struct tagSEGMENT
{
	int first_point;
	int last_point;
	float a;
	float b;
} SEGMENT;

#define _mm256_set_m128(va, vb) \
        _mm256_insertf128_ps(_mm256_castps128_ps256(vb), va, 1)

static int compare_float(const void *a, const void *b)
{
	if (*(float *)a < *(float *)b) return -1;
	if (*(float *)a > *(float *)b) return 1;
	return 0;
}

float median_point(float *input, size_t filter_length, size_t points_before, size_t points_after)
{
	_ALIGNED(float) sort_data[256];

	if ((points_before == 0) || (points_after == 0))
		return *input;

	size_t half_filter = filter_length >> 1;

	if (points_before < half_filter) {
		filter_length = points_before * 2 + 1;
		half_filter = points_before;
	}

	if (points_after < filter_length) {
		filter_length = points_after * 2 + 1;
		half_filter = points_after;
	}

	memcpy(sort_data, input - half_filter, filter_length * sizeof(float));
	qsort(sort_data, filter_length, sizeof(float), compare_float);

	return sort_data[half_filter];
}
/*
void median(float *input, float *output, size_t length, size_t filter_length, size_t points_before, size_t points_after)
{
	_ALIGNED(float) sort_data[256];
				
	size_t half_filter = filter_length >> 1;

	if (points_before < half_filter) {
		filter_length = points_before * 2;
		half_filter = filter_length
	}

	memcpy(sort_data, input, filter_length * sizeof(float));
	qsort(sort_data, filter_length, sizeof(float), compare_float);

	if (points_before < half_filter) {
		for (size_t j = 0; j < half_filter; j++) {
			output[j] = sort_data[half_filter];
		}
	}
	else {
		for (size_t j = 0; j < half_filter; j++) {
			memcpy(sort_data, input - half_filter + j, filter_length * sizeof(float));
			qsort(sort_data, filter_length, sizeof(float), compare_float);
			output[j] = sort_data[half_filter];
		}
	}

	for (size_t j = half_filter; j < (length - half_filter); j++) {

		memcpy(sort_data, input + j - half_filter, filter_length * sizeof(float));
		qsort(sort_data, filter_length, sizeof(float), compare_float);
		output[j] = sort_data[half_filter];
	}

	if (points_after < half_filter) {
		for (size_t j = length - half_filter; j < length; j++) {
			output[j] = sort_data[half_filter];
		}
	}
	else {
		for (size_t j = length - half_filter; j < length; j++) {
			memcpy(sort_data, input + j - half_filter, filter_length * sizeof(float));
			qsort(sort_data, filter_length, sizeof(float), compare_float);
			output[j] = sort_data[half_filter];
		}
	}
}
*/
size_t fuel_process(int *time, float *lat, float *lng, unsigned short *speed, float *fuel, size_t data_length, 
					int t_from, int t_to,
					float fill_threshold, float drain_threshold, float max_rate, size_t filter_length,
					unsigned char *buffer, size_t bytes_left, unsigned int flags)
{
	Median7_Init();

	if (filter_length > 0) {

		if (filter_length == 1)
			filter_length = 7;
		else
		if (filter_length == 2)
			filter_length = 11;
		else
			filter_length = 15;

		size_t segment_begin = 0;

		// Нахожу на графике ровные участки и фильтрую участки МЕЖДУ ними не трогая ровные учатски
		// Если фильтровать все подряд - ровные учатски затираются или искажаются, а они являются хорошими разделителями
		// В массив lat для каждой точки пишу 1 если из нее стартует ровный сегмент

		memcpy(lng, fuel, data_length * sizeof(float));

		for (size_t i = 0; i < data_length - 1; i++) {

			lat[i] = 0;

			size_t k = i + 1;

			if (i == 0x9b) {
				int a =1;
			}
			while ((k < data_length)&&(fuel[k] == fuel[i]))
				k++;

			if (time[k - 1] - time[i] >= 60) {

				lat[i] = 1;

				for (size_t j = segment_begin; j < i; j++)
					fuel[j] = median_point(&lng[j], filter_length, j, data_length - j - 1);
//				median(lng + segment_begin, fuel + segment_begin, i - segment_begin, filter_length, segment_begin, data_length - segment_begin);

				segment_begin = k;
				i = k - 1;
			}
		}

		if (segment_begin != data_length) {

			for (size_t j = segment_begin; j < data_length; j++)
				fuel[j] = median_point(&lng[j], filter_length, j, data_length - j - 1);
//			median(lng + segment_begin, fuel + segment_begin, data_length - segment_begin, filter_length, segment_begin, data_length - segment_begin);
		}
	}

	// Фильтрация завершена

	unsigned char *ptr = buffer;

	if (bytes_left < 1)
		return 0;

	*ptr++ = '[';
	bytes_left--;

	std::vector<SEGMENT> arr_segments;

	float fSumX = 0;
	float fSumY = 0;
	float fSumXY = 0;
	float fSumX2 = 0;

	float a = 0;
	float b = 0;

	SEGMENT *segments = (SEGMENT *)malloc(data_length * sizeof(SEGMENT));

	if (segments == NULL)
		return 0;

	for (size_t i = 0; i < data_length - 1; i++) {

		size_t j;

		if (lat[i] == 1) {

			for (j = i + 1; j < data_length; j++)
				if (fuel[j] != fuel[i])
					break;

			segments[i].first_point = i;
			segments[i].last_point = j - 1;
			segments[i].a = 0;
			segments[i].b = fuel[i];

			i = j - 1;
		}

		int segment_begin = time[i];

		// Нахожу самую дальнюю точку куда можно зайти от текущей в рамках критерия
		float fSumX		= 0;
		float fSumY		= fuel[i];
		float fSumXY	= 0;
		float fSumX2	= 0;

		int segment_length = 1;
		int farest_point = i;

		__m128i x_begin = _mm_set1_epi32(segment_begin);

		bool plain_segment = true;

		for (j = i + 1; j < data_length; j++) {

			// Если до текущей точки сегмент шел ровно, а в текущей - отклонился, сегмент завершается и в массив кладется ровный сегмент.
			if (plain_segment) {

				if (fuel[j] != fuel[i]) {
					
					plain_segment = false;

					if (time[j - 1] - segment_begin >= 60) {

						segments[i].first_point = i;
						segments[i].last_point = j - 1;
						segments[i].a = 0;
						segments[i].b = fuel[i];

						j = data_length + 1;

						break;
					}
				}
			}

			// Если текущая точка меняет сегмент настолько, что нарушается критерий, сегмент завершается и в массив кладется сегмент каким он был до текущей точки.
			float dT = (float)(time[j] - segment_begin);

			fSumX += dT;
			fSumY += fuel[j];
			fSumXY += dT * fuel[j];
			fSumX2 += dT * dT;

			segment_length++;

			float prev_a = a;
			float prev_b = b;

			a = (segment_length * fSumXY - fSumX * fSumY) / (segment_length * fSumX2 - fSumX * fSumX);
			b = (fSumY - a * fSumX) / segment_length;

			const __m256 imm_a = _mm256_set1_ps(a);
			const __m256 imm_b = _mm256_set1_ps(b);
			const __m256 imm_limit = _mm256_set1_ps(10.0f);

			for (size_t k = i; k <= j; k += 8) {

				const __m128i time_lo = _mm_loadu_si128((__m128i *)(time + k));
				const __m128i time_hi = _mm_loadu_si128((__m128i *)(time + k + 4));

				const __m128i real_time_lo = _mm_sub_epi32(time_lo, x_begin);
				const __m128i real_time_hi = _mm_sub_epi32(time_hi, x_begin);

				const __m256 t = _mm256_set_m128( _mm_cvtepi32_ps(real_time_hi), _mm_cvtepi32_ps(real_time_lo));

				const __m256 segment_fuel = _mm256_add_ps(_mm256_mul_ps(imm_a, t), imm_b);
				const __m256 sensor_fuel = _mm256_loadu_ps(fuel + k);

				const __m256 delta = _mm256_andnot_ps(_mm256_set1_ps(-0.f), _mm256_sub_ps(segment_fuel, sensor_fuel));

				const __m256 ymm_cmp = _mm256_cmp_ps(delta, imm_limit, _CMP_GT_OQ);

				unsigned long mask = _mm256_movemask_ps(ymm_cmp);

				if (mask != 0) {
#ifdef _MSC_VER
					_BitScanForward(&mask, mask);
#else
					mask = __builtin_ctz(mask);
#endif
					if (k + mask <= j) {

						segments[i].first_point = i;
						segments[i].last_point = j - 1;
						segments[i].a = prev_a;
						segments[i].b = prev_b;

						j = data_length + 1;

						break;
					}
				}
			}

			// Если в текушей точке стартует ровный сегмент - сегмент завершается текущей точкой включительно

			if (lat[j] == 1) {

				segments[i].first_point = i;
				segments[i].last_point = j;
				segments[i].a = a;
				segments[i].b = b;

				j = data_length + 1;

				break;
			}

		}

		if (j == data_length) {
			
			segments[i].first_point = i;
			segments[i].last_point = data_length - 1;
			segments[i].a = a;
			segments[i].b = b;
		}

#ifdef DEBUG
		struct tm tms1;
		struct tm tms2;

		time_t t1 = time[segments[i].first_point];
		time_t t2 = time[segments[i].last_point];

		localtime_r(&t1, &tms1);
		localtime_r(&t2, &tms2);

		printf("%u %f %f %f %02u.%02u.%04u %02u:%02u:%02u - %02u.%02u.%04u %02u:%02u:%02u\r\n", 
			i, segments[i].a, segments[i].b, fabs(fuel[segments[i].first_point] - segments[i].b), 
			tms1.tm_mday, tms1.tm_mon + 1, tms1.tm_year + 1900, tms1.tm_hour, tms1.tm_min, tms1.tm_sec,
			tms2.tm_mday, tms2.tm_mon + 1, tms2.tm_year + 1900, tms2.tm_hour, tms2.tm_min, tms2.tm_sec);
#endif
	}

	for (size_t i = 0; i < data_length - 1; i++) {
	
		float best_angle = 0;
		int best_segment = i;

		if (segments[i].a == 0) {
			best_segment = segments[i].last_point;
		}
		else {
			for (int j = i + 1; j <= segments[i].last_point; j++) {

				if (segments[j].a == 0) {
					best_segment = j;
					break;
				}

				float angle_delta = fabs(segments[j].a - segments[i].a);

				if (angle_delta >= best_angle) {
					best_angle = angle_delta;
					best_segment = j;
				}
			}
		}

		SEGMENT segment;

		segment.a = segments[i].a;
		segment.b = segments[i].b;

		segment.first_point = segments[i].first_point;

		if (best_segment == data_length - 1) {
			segment.last_point = segments[i].last_point;
		}
		else {
			segment.last_point = segments[best_segment].first_point;
		}

		arr_segments.push_back(segment);

		i = best_segment - 1;
	}

	bool fill_in_progress = false;
	int first_up;
	int last_up;

	bool drain_in_progress = false;
	int first_down;
	int last_down;
	int max_up;

	for (size_t i = 0; i < arr_segments.size(); i++) {

		time_t t1 = time[arr_segments[i].first_point];
		time_t t2 = time[arr_segments[i].last_point];

		float s1 = fuel[arr_segments[i].first_point];
		float s2 = fuel[arr_segments[i].last_point];

		float rate = ((s2 - s1) / (t2 - t1)) * 3600;

		if (s2 > s1) {

			if (fill_in_progress == false) {

				// Заправку стартует либо короткий сегмент, либо мощный сегмент
				if ((t2 - t1 < 3600)||(s2 - s1 >= fill_threshold)) {
				
					fill_in_progress = true;

					first_up = i;
					max_up = i;
					if (arr_segments[i].first_point == 1041) {
						int a = 1;
					}
				}
			}
			
			if (fill_in_progress == true) {

				last_up = i;

				if (fuel[arr_segments[i].last_point] > fuel[arr_segments[max_up].last_point])
					max_up = i;
			}
		}
		else {
			if (fill_in_progress == true) {

				float fill_fuel_begin = fuel[arr_segments[first_up].first_point];
				float fill_fuel_end = fuel[arr_segments[last_up].last_point];
				float fill_size = fill_fuel_end - fill_fuel_begin;

				if ((t2 - t1 > 600)||(fuel[arr_segments[last_up].last_point] - s1 > 10)||(s2 < (fill_fuel_begin + fill_size * 0.9))) {
				
					fill_in_progress = false;


					if (fill_threshold <= fill_size) {

						if ((time[arr_segments[first_up].first_point] >= t_from)&&(time[arr_segments[first_up].first_point] < t_to)) {
	
							if (bytes_left > 100) {
	
								size_t len = sprintf((char *)ptr, "{\"start\":%u,\"end\":%u,\"fuel\":%f},", time[arr_segments[first_up].first_point], time[arr_segments[max_up].last_point], fuel[arr_segments[max_up].last_point] - fill_fuel_begin);

								bytes_left -= len;
								ptr += len;
							}
							else
								return 0;
						}	
					}
				}
			}
		}

		if ((s2 < s1)&&(rate < -max_rate))  {

			if (drain_in_progress == false) {

				drain_in_progress = true;

				first_down = i;
			}

			last_down = i;
		}
		else {
			if (drain_in_progress == true) {
				
				drain_in_progress = false;

				float fill_fuel_begin = fuel[arr_segments[first_down].first_point];
				float fill_fuel_end = fuel[arr_segments[last_down].last_point];

				if (drain_threshold <= fill_fuel_begin - fill_fuel_end) {

					if ((time[arr_segments[first_down].first_point] >= t_from)&&(time[arr_segments[first_down].first_point] < t_to)) {
	
						if (bytes_left > 100) {
	
							size_t len = sprintf((char *)ptr, "{\"start\":%u,\"end\":%u,\"fuel\":%f},", time[arr_segments[first_down].first_point], time[arr_segments[last_down].last_point], fill_fuel_end - fill_fuel_begin);

							bytes_left -= len;
							ptr += len;
						}
						else 
							return 0;
					}	
				}
			}
		}

#ifdef DEBUG

		struct tm tms1;
		struct tm tms2;

		localtime_r(&t1, &tms1);
		localtime_r(&t2, &tms2);

		float f1 = arr_segments[i].b;
		float f2 = arr_segments[i].a * (time[arr_segments[i].last_point] - time[arr_segments[i].first_point]) + arr_segments[i].b;

		printf("%u %f %f (%f) %f %f (%f) %f %02u.%02u.%04u %02u:%02u:%02u - %02u.%02u.%04u %02u:%02u:%02u\r\n", 
			arr_segments[i].first_point, f1, f2, f2 - f1, s1, s2, s2 - s1, rate,
			tms1.tm_mday, tms1.tm_mon + 1, tms1.tm_year + 1900, tms1.tm_hour, tms1.tm_min, tms1.tm_sec,
			tms2.tm_mday, tms2.tm_mon + 1, tms2.tm_year + 1900, tms2.tm_hour, tms2.tm_min, tms2.tm_sec);
#endif

	}

	if (fill_in_progress == true) {

		float fill_fuel_begin = fuel[arr_segments[first_up].first_point];
		float fill_fuel_end = fuel[arr_segments[last_up].last_point];

		if (fill_threshold <= fill_fuel_end - fill_fuel_begin) {

			if ((time[arr_segments[first_up].first_point] >= t_from)&&(time[arr_segments[first_up].first_point] < t_to)) {
	
				if (bytes_left > 100) {
	
					size_t len = sprintf((char *)ptr, "{\"start\":%u,\"end\":%u,\"fuel\":%f},", time[arr_segments[first_up].first_point], time[arr_segments[max_up].last_point], fuel[arr_segments[max_up].last_point] - fill_fuel_begin);

					bytes_left -= len;
					ptr += len;
				}
				else
					return 0;
			}	
		}
	}

	if (drain_in_progress == true) {
				
		drain_in_progress = false;

		float fill_fuel_begin = fuel[arr_segments[first_down].first_point];
		float fill_fuel_end = fuel[arr_segments[last_down].last_point];

		if (drain_threshold <= fill_fuel_begin - fill_fuel_end) {

			if ((time[arr_segments[first_down].first_point] >= t_from)&&(time[arr_segments[first_down].first_point] < t_to)) {
	
				if (bytes_left > 100) {
	
					size_t len = sprintf((char *)ptr, "{\"start\":%u,\"end\":%u,\"fuel\":%f},", time[arr_segments[first_down].first_point], time[arr_segments[last_down].last_point], fill_fuel_end - fill_fuel_begin);

					bytes_left -= len;
					ptr += len;
				}
				else
					return 0;
			}	
		}
	}

	free(segments);

	if (*(ptr - 1) == ',') {
		ptr--;
		bytes_left++;
	}

	if (bytes_left < 1)
		return 0;

	*ptr++ = ']';

#ifdef DEBUG
	ptr += sprintf((char *)ptr, ", chart:[");

	for(size_t i = 0; i < data_length; i++) {
		ptr += sprintf((char *)ptr, "{t:%u,data:%f},", time[i], fuel[i]);
	}

	ptr--;
	*ptr++ = ']';
#endif

#ifdef DEBUG
	fflush(stdout);
#endif

	return ptr - buffer;
}

// End
