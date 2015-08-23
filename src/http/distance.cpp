#include <omp.h>
#include <math.h>
#include <stdio.h>
#include <xmmintrin.h>
#include <pmmintrin.h>
#include "sse_mathfun.h"
#include "../core/aligned.h"

_ALIGNED(const float) fc_A	= 6378137;
_ALIGNED(const double) c_A	= 6378137;
_ALIGNED(const double) c_a	= 1/298.257223563;
_ALIGNED(const float) c_e2	= (float)(2*c_a - c_a*c_a);

_ALIGNED(const float) fOne	= 1.0f;
_ALIGNED(const float) fTwo	= 2.0f;
_ALIGNED(const float) fPI	= 3.14159265358979f;
_ALIGNED(const float) f180	= 180.0f;
_ALIGNED(const float) fX	= 1 - c_e2;

_ALIGNED(float) fResult[4];

float GetDistanceMT(float *pLat, float *pLon, size_t nPointsCount)
{
	int iPoint = 0;
	int nPointsAfterEnd;
	char bFirstPoint = 1;

	__m128 prev_x, prev_y, prev_z;

	__m128 distance = _mm_set1_ps(0);

	nPointsAfterEnd = 4 - nPointsCount & 0x03;

	_mm_prefetch((char *)&pLon[iPoint], _MM_HINT_NTA);

	for (iPoint = nPointsCount; iPoint < nPointsCount + nPointsAfterEnd; iPoint++) {
		pLat[iPoint] = pLat[nPointsCount - 1];
		pLon[iPoint] = pLon[nPointsCount - 1];
	}

	#pragma omp parallel for private(prev_x,prev_y,prev_z) reduction(+:distance) firstprivate(bFirstPoint)
	for (iPoint = 0; iPoint < nPointsCount; iPoint += 4) {

		float fResult[4];
		__m128 latval;
		__m128 lonval;
				
		__m128 sseval;
		__m128 tmpval;
		__m128 sinlat;
		__m128 coslat;
		__m128 sinlon;
		__m128 coslon;

		__m128 nval;
		__m128 xval;
		__m128 yval;
		__m128 zval;
		__m128 xsub, ysub, zsub;

		sseval = _mm_load_ps(&pLat[iPoint]);
		tmpval = _mm_mul_ps(sseval, _mm_load_ps1(&fPI));
		latval = _mm_div_ps(tmpval, _mm_load_ps1(&f180));

		sincos_ps(latval, &sinlat, &coslat);

		sseval = _mm_load_ps(&pLon[iPoint]);
		tmpval = _mm_mul_ps(sseval, _mm_load_ps1(&fPI));
		lonval = _mm_div_ps(tmpval, _mm_load_ps1(&f180));

		_mm_prefetch((char *)&pLat[iPoint + 4], _MM_HINT_NTA);
		_mm_prefetch((char *)&pLon[iPoint + 4], _MM_HINT_NTA);

		sincos_ps(lonval, &sinlon, &coslon);

	 	tmpval = _mm_mul_ps(sinlat, sinlat);
		tmpval = _mm_mul_ps(tmpval, _mm_load_ps1(&c_e2));
		tmpval = _mm_sub_ps(_mm_load_ps1(&fOne), tmpval);
		sseval = _mm_sqrt_ps(tmpval);
		nval = _mm_div_ps(_mm_load_ps1(&fc_A), sseval);

		xval = _mm_mul_ps(nval, _mm_mul_ps(coslat, coslon));

		if (bFirstPoint == 1) {

			if (iPoint > 0) {

				__m128 ll = _mm_set_ps(0, 0, pLon[iPoint - 1]*fPI/180, pLat[iPoint - 1]*fPI/180);
				__m128 s, c;

				sincos_ps(ll, &s, &c);

				_mm_store_ps(fResult, s);
				const float fSinB1 = fResult[0];
				const float fSinL1 = fResult[1];

				_mm_store_ps(fResult, c);
				const float fCosB1 = fResult[0];
				const float fCosL1 = fResult[1];

				ll = _mm_set_ps(0, 0, 0, 1 - c_e2*fSinB1*fSinB1);
				ll = _mm_sqrt_ps(ll);
				_mm_store_ps(fResult, ll);

				const float N = fc_A/fResult[0];

				const float X = N*fCosB1*fCosL1;
				const float Y = N*fCosB1*fSinL1;
				const float Z = fX*N*fSinB1;

				prev_x = _mm_set1_ps(X);
				prev_y = _mm_set1_ps(Y);
				prev_z = _mm_set1_ps(Z);

				xsub = _mm_shuffle_ps(_mm_move_ss(prev_x, xval), xval, _MM_SHUFFLE(2, 1, 0, 3));

				bFirstPoint = 0;
			}
			else {
				xsub = _mm_shuffle_ps(xval, xval, _MM_SHUFFLE(2, 1, 0, 0));
			}
		}
		else
			xsub = _mm_shuffle_ps(_mm_move_ss(prev_x, xval), xval, _MM_SHUFFLE(2, 1, 0, 3));

		prev_x = xval;

		xval = _mm_sub_ps(xval, xsub);
		xval = _mm_mul_ps(xval, xval);

		yval = _mm_mul_ps(nval, _mm_mul_ps(coslat, sinlon));
		if (bFirstPoint == 1) {
			ysub = _mm_shuffle_ps(yval, yval, _MM_SHUFFLE(2, 1, 0, 0));
		}
		else
			ysub = _mm_shuffle_ps(_mm_move_ss(prev_y, yval), yval, _MM_SHUFFLE(2, 1, 0, 3));

		prev_y = yval;

		yval = _mm_sub_ps(yval, ysub);
		yval = _mm_mul_ps(yval, yval);

		zval = _mm_mul_ps(_mm_load_ps1(&fX), _mm_mul_ps(nval, sinlat));
		if (bFirstPoint == 1) {
			zsub = _mm_shuffle_ps(zval, zval, _MM_SHUFFLE(2, 1, 0, 0));
			bFirstPoint = 0;
		}
		else
			zsub = _mm_shuffle_ps(_mm_move_ss(prev_z, zval), zval, _MM_SHUFFLE(2, 1, 0, 3));

		prev_z = zval;

		zval = _mm_sub_ps(zval, zsub);
		zval = _mm_mul_ps(zval, zval);

		nval = _mm_mul_ps(nval, _mm_load_ps1(&fTwo));

		tmpval = _mm_mul_ps(asin_ps(_mm_div_ps(_mm_sqrt_ps(_mm_add_ps(xval, _mm_add_ps(yval, zval))), nval)), nval);

		_mm_store_ps(fResult, tmpval);

// To get down to SSE2 change to
//		tmpval = _mm_add_ps(tmpval, _mm_movehl_ps(tmpval, tmpval));
//		tmpval = _mm_add_ss(tmpval, _mm_shuffle_ps(tmpval, tmpval, 1));
		tmpval = _mm_hadd_ps(tmpval, tmpval);
		tmpval = _mm_hadd_ps(tmpval, tmpval);

 		distance = _mm_add_ps(distance, tmpval);
	}

	_mm_store_ps(fResult, distance);

	return fResult[0];
}

float GetDistanceST(float *pLat, float *pLon, size_t nPointsCount)
{
	unsigned int iPoint = 0;
	unsigned int nPointsAfterEnd;

	__m128 latval;
	__m128 lonval;
				
	__m128 sseval;
	__m128 tmpval;
	__m128 sinlat;
	__m128 coslat;
	__m128 sinlon;
	__m128 coslon;

	__m128 nval;
	__m128 xval;
	__m128 yval;
	__m128 zval;
	__m128 xsub, ysub, zsub;
	__m128 prev_x, prev_y, prev_z;

	__m128 distance = _mm_set1_ps(0);

	nPointsAfterEnd = 4 - nPointsCount & 0x03;

	_mm_prefetch((char *)&pLon[iPoint], _MM_HINT_NTA);

	for (iPoint = nPointsCount; iPoint < nPointsCount + nPointsAfterEnd; iPoint++) {
		pLat[iPoint] = pLat[nPointsCount - 1];
		pLon[iPoint] = pLon[nPointsCount - 1];
	}

	for (iPoint = 0; iPoint < nPointsCount; iPoint += 4) {

		sseval = _mm_load_ps(&pLat[iPoint]);
		tmpval = _mm_mul_ps(sseval, _mm_load_ps1(&fPI));
		latval = _mm_div_ps(tmpval, _mm_load_ps1(&f180));

		_mm_prefetch((char *)&pLat[iPoint + 4], _MM_HINT_NTA);
		_mm_prefetch((char *)&pLon[iPoint + 4], _MM_HINT_NTA);

		sincos_ps(latval, &sinlat, &coslat);

		sseval = _mm_load_ps(&pLon[iPoint]);
		tmpval = _mm_mul_ps(sseval, _mm_load_ps1(&fPI));
		lonval = _mm_div_ps(tmpval, _mm_load_ps1(&f180));

		sincos_ps(lonval, &sinlon, &coslon);

	 	tmpval = _mm_mul_ps(sinlat, sinlat);
		tmpval = _mm_mul_ps(tmpval, _mm_load_ps1(&c_e2));
		tmpval = _mm_sub_ps(_mm_load_ps1(&fOne), tmpval);
		sseval = _mm_sqrt_ps(tmpval);
		nval = _mm_div_ps(_mm_load_ps1(&fc_A), sseval);

		xval = _mm_mul_ps(nval, _mm_mul_ps(coslat, coslon));
		if (iPoint == 0)
			xsub = _mm_shuffle_ps(xval, xval, _MM_SHUFFLE(2, 1, 0, 0));
		else
			xsub = _mm_shuffle_ps(_mm_move_ss(prev_x, xval), xval, _MM_SHUFFLE(2, 1, 0, 3));

		prev_x = xval;

		xval = _mm_sub_ps(xval, xsub);
		xval = _mm_mul_ps(xval, xval);

		yval = _mm_mul_ps(nval, _mm_mul_ps(coslat, sinlon));
		if (iPoint == 0)
			ysub = _mm_shuffle_ps(yval, yval, _MM_SHUFFLE(2, 1, 0, 0));
		else
			ysub = _mm_shuffle_ps(_mm_move_ss(prev_y, yval), yval, _MM_SHUFFLE(2, 1, 0, 3));

		prev_y = yval;

		yval = _mm_sub_ps(yval, ysub);
		yval = _mm_mul_ps(yval, yval);

		zval = _mm_mul_ps(_mm_load_ps1(&fX), _mm_mul_ps(nval, sinlat));
		if (iPoint == 0)
			zsub = _mm_shuffle_ps(zval, zval, _MM_SHUFFLE(2, 1, 0, 0));
		else
			zsub = _mm_shuffle_ps(_mm_move_ss(prev_z, zval), zval, _MM_SHUFFLE(2, 1, 0, 3));

		prev_z = zval;

		zval = _mm_sub_ps(zval, zsub);
		zval = _mm_mul_ps(zval, zval);

		nval = _mm_mul_ps(nval, _mm_load_ps1(&fTwo));

		tmpval = _mm_mul_ps(asin_ps(_mm_div_ps(_mm_sqrt_ps(_mm_add_ps(xval, _mm_add_ps(yval, zval))), nval)), nval);

// To get down to SSE2 change to
//		tmpval = _mm_add_ps(tmpval, _mm_movehl_ps(tmpval, tmpval));
//		tmpval = _mm_add_ss(tmpval, _mm_shuffle_ps(tmpval, tmpval, 1));
		tmpval = _mm_hadd_ps(tmpval, tmpval);
		tmpval = _mm_hadd_ps(tmpval, tmpval);

		distance = _mm_add_ps(distance, tmpval);
	}

	_mm_store_ps(fResult, distance);

	return fResult[0];
}

float GetDistanceCPU(float *pLat, float *pLon, size_t nPointsCount)
{
	double mileage = 0;

	const double fSinL1 = sin (pLon[0]*fPI/180);
	const double fCosL1 = cos (pLon[0]*fPI/180);
	const double fSinB1 = sin (pLat[0]*fPI/180);
	const double fCosB1 = cos (pLat[0]*fPI/180);
					
	const double N1 = c_A/sqrt (1 - c_e2*fSinB1*fSinB1);

	double X1 = N1*fCosB1*fCosL1;
	double Y1 = N1*fCosB1*fSinL1;
	double Z1 = (1 - c_e2)*N1*fSinB1;

	for (size_t iPoint = 1; iPoint < nPointsCount; iPoint++) {

		const double fSinL2 = sin (pLon[iPoint]*fPI/180);
		const double fCosL2 = cos (pLon[iPoint]*fPI/180);
		const double fSinB2 = sin (pLat[iPoint]*fPI/180);
		const double fCosB2 = cos (pLat[iPoint]*fPI/180);
				
		const double N2 = c_A/sqrt (1 - c_e2*fSinB2*fSinB2);

		const double X2 = N2*fCosB2*fCosL2;
		const double Y2 = N2*fCosB2*fSinL2;
		const double Z2 = (1 - c_e2)*N2*fSinB2;
				
		const double D = sqrt ((X1 - X2)*(X1 - X2) + (Y1 - Y2)*(Y1 - Y2) + (Z1 - Z2)*(Z1 - Z2));

		const double R = N1;
		const double D2 = 2*R*asin (.5f*D/R);
			
		X1 = X2;
		Y1 = Y2;
		Z1 = Z2;

		mileage += D2;
	}

	return mileage;
}
