//******************************************************************************
//
// File Name: median7.cpp
// Author   : Skytrack ltd - Copyright (C) 2013
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdlib.h>
#include <stdio.h>
#include <immintrin.h>
#include <cfloat>
#include <string.h>
#include "../core/aligned.h"

#ifdef _MSC_VER
#include <intrin.h>
#endif

#define _mm256_shift_right(a) { a = _mm256_permute_ps(a, _MM_SHUFFLE(0, 3, 2, 1)); a = _mm256_blend_ps(a, _mm256_permute2f128_ps(a, a, 1), 0x08); }
#define _mm256_shift_left(a) { a = _mm256_permute_ps(a, _MM_SHUFFLE(2, 1, 0, 3)); a = _mm256_blend_ps(a, _mm256_permute2f128_ps(a, a, 1), 0x10); }

#define _mm256_shift_right2(b, a) { \
	__m256 tmp;	\
	b = _mm256_permute_ps(b, _MM_SHUFFLE(0, 3, 2, 1)); \
	b = _mm256_blend_ps(b, _mm256_permute2f128_ps(b, b, 1), 0x08); \
	a = _mm256_permute_ps(a, _MM_SHUFFLE(0, 3, 2, 1)); \
	tmp = _mm256_permute2f128_ps(a, a, 1); \
	b = _mm256_blend_ps(b, tmp, 0x80); \
	a = _mm256_blend_ps(a, tmp, 0x08); \
}

static int compare_float(const void *a, const void *b)
{
	if (*(float *)a < *(float *)b) return -1;
	if (*(float *)a > *(float *)b) return 1;
	return 0;
}

inline  __m256 _mm256_broadcast_lo_ss(__m256 a) {
	__m256 b = _mm256_permute_ps(a, _MM_SHUFFLE(0, 0, 0, 0)); \
	return _mm256_blend_ps(b, _mm256_permute2f128_ps(b, b, 1), 0xF0); \
}

inline  __m256 _mm256_broadcast_hi_ss(__m256 a) {
	__m256 b = _mm256_permute_ps(a, _MM_SHUFFLE(3, 3, 3, 3));
	return _mm256_blend_ps(b, _mm256_permute2f128_ps(b, b, 1), 0x0F);
}

inline  __m256 _mm256_broadcast_3_ss(__m256 a) {
	__m256 b = _mm256_permute_ps(a, _MM_SHUFFLE(3, 3, 3, 3));
	return _mm256_blend_ps(b, _mm256_permute2f128_ps(b, b, 1), 0xF0);
}

typedef struct _tagm7_control 
{
	__m256 remove;
	__m256 insert;
} Median7_CONTROL;

_ALIGNED(static Median7_CONTROL) Median7_control[8][8];

void Median7_Init()
{
	_ALIGNED(float) data[8];
	const unsigned maxU = ~0;
	const float qNan =  *((float*)&maxU);

	for (int remove = 0; remove < 8; remove++) {
		for (int insert = 0; insert < 8; insert++) {

			_mm256_store_ps(data, _mm256_setzero_ps());

			if (insert < remove) {
				for (int i = insert; i <= remove; i++)
					data[i] = qNan;
				data[7] = 0;
			}
			else
			if (insert > remove) {
				for (int i = remove; i < insert; i++)
					data[i] = qNan;
				data[7] = 0;
			}

			Median7_control[remove][insert].remove = _mm256_load_ps(data);

			_mm256_store_ps(data, _mm256_setzero_ps());

			if ((insert > remove)||(insert == 7))
				data[insert - 1] = qNan;
			else
				data[insert] = qNan;

			Median7_control[remove][insert].insert = _mm256_load_ps(data);
		}
	}
}

void Median7(float *pData, int len) 
{
	int items_left;
	int last_item_written;
	char bFirstPoint = 1;

	if (len >= 16) {
	
		pData += 3;

		int last_possible_item = len - 3 - 8;
		int last_possible_row = last_possible_item - last_possible_item % 8;
		last_item_written = last_possible_row + 3;

		__m256 ymm0, ymm_history0;

		#pragma omp parallel for private(ymm0, ymm_history0) firstprivate(bFirstPoint) num_threads(4)
		for (int iData = 0; iData <= last_possible_row; iData += 8) {

			_ALIGNED(float) initial_data[8];
			__m256 ymm1, ymm_next, ymm_oldest, ymm_store, ymm_history1;
			unsigned long remove_mask;
			unsigned long insert_mask;

			if (bFirstPoint == 1) {

				bFirstPoint = 0;

				ymm_history0 = _mm256_load_ps(&pData[iData - 3]);

				_mm256_store_ps(initial_data, ymm_history0);

				qsort(initial_data, 7, sizeof(float), compare_float);
				initial_data[7] = FLT_MAX;
			
				pData[iData] = initial_data[3];

				ymm0 = _mm256_load_ps(initial_data);
			}

			ymm_history1 = _mm256_load_ps(&pData[iData + 5]);			
			ymm_store = _mm256_setzero_ps();

//			_mm_prefetch((char *)&pData[iData + 13], _MM_HINT_NTA);

			for (int i = 0; i < 8; i++) {

				ymm_next = _mm256_broadcast_hi_ss(ymm_history0);
				ymm_oldest = _mm256_broadcast_lo_ss(ymm_history0);

				remove_mask = _mm256_movemask_ps(_mm256_cmp_ps(ymm0, ymm_oldest, _CMP_EQ_US));
#ifdef _MSC_VER
				_BitScanForward(&remove_mask, remove_mask);
#else
				remove_mask = __builtin_ctz(remove_mask);
#endif

				insert_mask = _mm256_movemask_ps(_mm256_cmp_ps(ymm0, ymm_next, _CMP_GE_OQ));
#ifdef _MSC_VER
				_BitScanForward(&insert_mask, insert_mask);
#else
				insert_mask = __builtin_ctz(insert_mask);
#endif

				if (insert_mask < remove_mask) {
					ymm1 = ymm0;
					_mm256_shift_left(ymm1);
					ymm0 = _mm256_blendv_ps(ymm0, ymm1, Median7_control[remove_mask][insert_mask].remove);
				}
				else
				if (insert_mask > remove_mask) {
					ymm1 = ymm0;
					_mm256_shift_right(ymm1);
					ymm0 = _mm256_blendv_ps(ymm0, ymm1, Median7_control[remove_mask][insert_mask].remove);
				}

				ymm0 = _mm256_blendv_ps(ymm0, ymm_next, Median7_control[remove_mask][insert_mask].insert);

				_mm256_shift_right(ymm_store);
				ymm_store = _mm256_blend_ps(ymm_store, _mm256_broadcast_3_ss(ymm0), 0x80);

				_mm256_shift_right2(ymm_history0, ymm_history1);
			}
			 
			_mm256_storeu_ps(&pData[iData + 1], ymm_store);
		}

		pData -= 3; 
	}
	else
		last_item_written = 6;

	items_left = len - last_item_written - 1 - 3;

	for (int i = last_item_written + 1; i <= last_item_written + items_left; i++) {
		_ALIGNED(float) sort_data[8];
		memcpy(sort_data, &pData[i - 3], 7 * sizeof(float));
		qsort(sort_data, 7, sizeof(float), compare_float);
		pData[i] = sort_data[3];
	}

	_mm256_zeroupper();
}

// End
