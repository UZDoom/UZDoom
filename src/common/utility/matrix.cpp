/*
** matrix.cpp
**
** Simplified version of VSMatrix that has been adjusted for GZDoom's needs.
**
**---------------------------------------------------------------------------
**
** Copyright 2014-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** This file was originally derived from Very Simple Math Library,
** which was licensed under the GNU Lesser General Public License 3.
**
** Copyright 2013 Lighthouse3D
**
** All subsequent changes and the integrated work as a whole are licensed
** under the GNU General Public License as stated above.
**
**---------------------------------------------------------------------------
**
** Full documentation at
** http://www.lighthouse3d.com/very-simple-libs
**
** This class aims at easing geometric transforms, camera
** placement and projection definition for programmers
** working with OpenGL core versions.
*/

#include <algorithm>
#include <math.h>
#include "matrix.h"

#if defined(__x86_64__) || defined(_M_X64)
	#include <smmintrin.h>
	#include <emmintrin.h>
	#include <pmmintrin.h>
	#include <tmmintrin.h>
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4244)     // truncate from double to float
#endif

static inline float
DegToRad(float degrees)
{
	return (float)(degrees * (pi::pif() / 180.0f));
};

// sets the square matrix mat to the identity matrix,
// size refers to the number of rows (or columns)
void
VSMatrix::setIdentityMatrix( float *mat, int size) {

	// fill matrix with 0s
	for (int i = 0; i < size * size; ++i)
			mat[i] = 0.0f;

	// fill diagonal with 1s
	for (int i = 0; i < size; ++i)
		mat[i + i * size] = 1.0f;
}



// gl LoadIdentity implementation
void
VSMatrix::loadIdentity()
{
	// fill matrix with 0s
	for (int i = 0; i < 16; ++i)
			mMatrix[i] = 0.0f;

	// fill diagonal with 1s
	for (int i = 0; i < 4; ++i)
		mMatrix[i + i * 4] = 1.0f;
}


// gl MultMatrix implementation
void VSMatrix::multMatrix(const float *aMatrix)
{
	alignas(16) float res[16];

	#if defined(__x86_64__) || defined(_M_X64)
	for(int i = 0; i < 4; i++)
	{
		__m128 a = _mm_setr_ps(mMatrix[i], mMatrix[i + 4], mMatrix[i + 8], mMatrix[i + 12]); // [0] = m[i, 0]
																							 // [1] = m[i, 1]
																							 // [2] = m[i, 2]
																							 // [3] = m[i, 3]

		for(int j = 0; j < 4; j++)
		{
			const int jj = j << 2; // j * 4

			__m128 b = _mm_load_ps(aMatrix + jj); // [0] = b[0, j]
												  // [1] = b[1, j]
												  // [2] = b[2, j]
												  // [3] = b[3, j]

			__m128 c = _mm_mul_ps(a, b); // c[k] = a[i,k] * b[k,j] with k=0..3
										 // ----------------------
										 // c[0] = a[i,0] * b[0,j]
										 // c[1] = a[i,1] * b[1,j]
										 // c[2] = a[i,2] * b[2,j]
										 // c[3] = a[i,3] * b[3,j]

			c = _mm_hadd_ps(c, c); // c'2[0] = c[0] + c[1]
								   // c'2[1] = c[2] + c[3]
								   // --------------------------------------------
								   // c'2[0] = (a[i,0] * b[0,j]) + (a[i,1] * b[1,j])
								   // c'2[1] = (a[i,2] * b[2,j]) + (a[i,3] * b[3,j])

			c = _mm_hadd_ps(c, c); // c'3[0] = c'2[0] + c'2[1]
								   // ---------------------------------
								   // c'3[0] = c[0] + c[1] + c[2] + c[3]
								   // --------------------------------------------------------------------------------------
								   // c'3[0] = (a[i,0] * b[0,j]) + (a[i,1] * b[1,j]) + (a[i,2] * b[2,j]) + (a[i,3] * b[3,j])

			_mm_store_ss(res + (i + jj), c); // res[i, j] = sum of (a[i,k] * b[k,j]) with k=0..3
		}
	}
	#else
	for (int i = 0; i < 4; ++i) 
	{
		for (int j = 0; j < 4; ++j)
		{
			res[j*4 + i] = 0.0f;
			for (int k = 0; k < 4; ++k)
			{
				res[j*4 + i] += mMatrix[k*4 + i] * aMatrix[j*4 + k];
			}
		}
	}
	#endif
	memcpy(mMatrix, res, 16 * sizeof(float));
}

#ifdef USE_DOUBLE
// gl MultMatrix implementation
void
VSMatrix::multMatrix(const float *aMatrix)
{

	float res[16];

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			res[j * 4 + i] = 0.0f;
			for (int k = 0; k < 4; ++k)
			{
				res[j*4 + i] += mMatrix[k*4 + i] * aMatrix[j*4 + k];
			}
		}
	}
	memcpy(mMatrix, res, 16 * sizeof(float));
}
#endif

void VSMatrix::multQuaternion(const TVector4<float>& q)
{
	alignas(16) float m[16] = { float(0.0) };
	m[0 * 4 + 0] = float(1.0) - float(2.0) * q.Y * q.Y - float(2.0) * q.Z * q.Z;
	m[1 * 4 + 0] = float(2.0) * q.X * q.Y - float(2.0) * q.W * q.Z;
	m[2 * 4 + 0] = float(2.0) * q.X * q.Z + float(2.0) * q.W * q.Y;
	m[0 * 4 + 1] = float(2.0) * q.X * q.Y + float(2.0) * q.W * q.Z;
	m[1 * 4 + 1] = float(1.0) - float(2.0) * q.X * q.X - float(2.0) * q.Z * q.Z;
	m[2 * 4 + 1] = float(2.0) * q.Y * q.Z - float(2.0) * q.W * q.X;
	m[0 * 4 + 2] = float(2.0) * q.X * q.Z - float(2.0) * q.W * q.Y;
	m[1 * 4 + 2] = float(2.0) * q.Y * q.Z + float(2.0) * q.W * q.X;
	m[2 * 4 + 2] = float(1.0) - float(2.0) * q.X * q.X - float(2.0) * q.Y * q.Y;
	m[3 * 4 + 3] = float(1.0);
	multMatrix(m);
}

void VSMatrix::multQuaternion(const TQuaternion<float>& q)
{
	alignas(16) float m[16] = { float(0.0) };
	m[0 * 4 + 0] = float(1.0) - float(2.0) * q.Y * q.Y - float(2.0) * q.Z * q.Z;
	m[1 * 4 + 0] = float(2.0) * q.X * q.Y - float(2.0) * q.W * q.Z;
	m[2 * 4 + 0] = float(2.0) * q.X * q.Z + float(2.0) * q.W * q.Y;
	m[0 * 4 + 1] = float(2.0) * q.X * q.Y + float(2.0) * q.W * q.Z;
	m[1 * 4 + 1] = float(1.0) - float(2.0) * q.X * q.X - float(2.0) * q.Z * q.Z;
	m[2 * 4 + 1] = float(2.0) * q.Y * q.Z - float(2.0) * q.W * q.X;
	m[0 * 4 + 2] = float(2.0) * q.X * q.Z - float(2.0) * q.W * q.Y;
	m[1 * 4 + 2] = float(2.0) * q.Y * q.Z + float(2.0) * q.W * q.X;
	m[2 * 4 + 2] = float(1.0) - float(2.0) * q.X * q.X - float(2.0) * q.Y * q.Y;
	m[3 * 4 + 3] = float(1.0);
	multMatrix(m);
}


// gl LoadMatrix implementation
void
VSMatrix::loadMatrix(const float *aMatrix)
{
	memcpy(mMatrix, aMatrix, 16 * sizeof(float));
}

#ifdef USE_DOUBLE
// gl LoadMatrix implementation
void
VSMatrix::loadMatrix(const float *aMatrix)
{
	for (int i = 0; i < 16; ++i)
	{
		mMatrix[i] = aMatrix[i];
	}
}
#endif


// gl Translate implementation
void
VSMatrix::translate(float x, float y, float z)
{
	mMatrix[12] = mMatrix[0] * x + mMatrix[4] * y + mMatrix[8] * z + mMatrix[12];
	mMatrix[13] = mMatrix[1] * x + mMatrix[5] * y + mMatrix[9] * z + mMatrix[13];
	mMatrix[14] = mMatrix[2] * x + mMatrix[6] * y + mMatrix[10] * z + mMatrix[14];
}

void VSMatrix::transpose()
{
	float original[16];
	for (int cnt = 0; cnt < 16; cnt++)
		original[cnt] = mMatrix[cnt];

	mMatrix[0] = original[0];
	mMatrix[1] = original[4];
	mMatrix[2] = original[8];
	mMatrix[3] = original[12];
	mMatrix[4] = original[1];
	mMatrix[5] = original[5];
	mMatrix[6] = original[9];
	mMatrix[7] = original[13];
	mMatrix[8] = original[2];
	mMatrix[9] = original[6];
	mMatrix[10] = original[10];
	mMatrix[11] = original[14];
	mMatrix[12] = original[3];
	mMatrix[13] = original[7];
	mMatrix[14] = original[11];
	mMatrix[15] = original[15];
}

// gl Scale implementation
void
VSMatrix::scale(float x, float y, float z)
{
	mMatrix[0] *= x;   mMatrix[1] *= x;   mMatrix[2] *= x;   mMatrix[3] *= x;
	mMatrix[4] *= y;   mMatrix[5] *= y;   mMatrix[6] *= y;   mMatrix[7] *= y;
	mMatrix[8] *= z;   mMatrix[9] *= z;   mMatrix[10] *= z;   mMatrix[11] *= z;
}


// gl Rotate implementation
void
VSMatrix::rotate(float angle, float x, float y, float z)
{
	alignas(16) float mat[16];
	float v[3];

	v[0] = x;
	v[1] = y;
	v[2] = z;

	float radAngle = DegToRad(angle);
	float co = cos(radAngle);
	float si = sin(radAngle);
	normalize(v);
	float x2 = v[0]*v[0];
	float y2 = v[1]*v[1];
	float z2 = v[2]*v[2];

//	mat[0] = x2 + (y2 + z2) * co;
	mat[0] = co + x2 * (1 - co);// + (y2 + z2) * co;
	mat[4] = v[0] * v[1] * (1 - co) - v[2] * si;
	mat[8] = v[0] * v[2] * (1 - co) + v[1] * si;
	mat[12]= 0.0f;

	mat[1] = v[0] * v[1] * (1 - co) + v[2] * si;
//	mat[5] = y2 + (x2 + z2) * co;
	mat[5] = co + y2 * (1 - co);
	mat[9] = v[1] * v[2] * (1 - co) - v[0] * si;
	mat[13]= 0.0f;

	mat[2] = v[0] * v[2] * (1 - co) - v[1] * si;
	mat[6] = v[1] * v[2] * (1 - co) + v[0] * si;
//	mat[10]= z2 + (x2 + y2) * co;
	mat[10]= co + z2 * (1 - co);
	mat[14]= 0.0f;

	mat[3] = 0.0f;
	mat[7] = 0.0f;
	mat[11]= 0.0f;
	mat[15]= 1.0f;

	multMatrix(mat);
}


// gluLookAt implementation
void
VSMatrix::lookAt(float xPos, float yPos, float zPos,
					float xLook, float yLook, float zLook,
					float xUp, float yUp, float zUp)
{
	float dir[3], right[3], up[3];

	up[0] = xUp;	up[1] = yUp;	up[2] = zUp;

	dir[0] =  (xLook - xPos);
	dir[1] =  (yLook - yPos);
	dir[2] =  (zLook - zPos);
	normalize(dir);

	crossProduct(dir,up,right);
	normalize(right);

	crossProduct(right,dir,up);
	normalize(up);

	alignas(16) float m1[16],m2[16];

	m1[0]  = right[0];
	m1[4]  = right[1];
	m1[8]  = right[2];
	m1[12] = 0.0f;

	m1[1]  = up[0];
	m1[5]  = up[1];
	m1[9]  = up[2];
	m1[13] = 0.0f;

	m1[2]  = -dir[0];
	m1[6]  = -dir[1];
	m1[10] = -dir[2];
	m1[14] =  0.0f;

	m1[3]  = 0.0f;
	m1[7]  = 0.0f;
	m1[11] = 0.0f;
	m1[15] = 1.0f;

	setIdentityMatrix(m2,4);
	m2[12] = -xPos;
	m2[13] = -yPos;
	m2[14] = -zPos;

	multMatrix(m1);
	multMatrix(m2);
}


// gluPerspective implementation
void
VSMatrix::perspective(float fov, float ratio, float nearp, float farp)
{
	float f = 1.0f / tan (fov * (pi::pif() / 360.0f));

	loadIdentity();
	mMatrix[0] = f / ratio;
	mMatrix[1 * 4 + 1] = f;
	mMatrix[2 * 4 + 2] = (farp + nearp) / (nearp - farp);
	mMatrix[3 * 4 + 2] = (2.0f * farp * nearp) / (nearp - farp);
	mMatrix[2 * 4 + 3] = -1.0f;
	mMatrix[3 * 4 + 3] = 0.0f;
}


// gl Ortho implementation
void
VSMatrix::ortho(float left, float right,
			float bottom, float top,
			float nearp, float farp)
{
	loadIdentity();

	mMatrix[0 * 4 + 0] = 2 / (right - left);
	mMatrix[1 * 4 + 1] = 2 / (top - bottom);
	mMatrix[2 * 4 + 2] = -2 / (farp - nearp);
	mMatrix[3 * 4 + 0] = -(right + left) / (right - left);
	mMatrix[3 * 4 + 1] = -(top + bottom) / (top - bottom);
	mMatrix[3 * 4 + 2] = -(farp + nearp) / (farp - nearp);
}


// gl Frustum implementation
void
VSMatrix::frustum(float left, float right,
			float bottom, float top,
			float nearp, float farp)
{
	alignas(16) float m[16];

	setIdentityMatrix(m,4);

	m[0 * 4 + 0] = 2 * nearp / (right-left);
	m[1 * 4 + 1] = 2 * nearp / (top - bottom);
	m[2 * 4 + 0] = (right + left) / (right - left);
	m[2 * 4 + 1] = (top + bottom) / (top - bottom);
	m[2 * 4 + 2] = - (farp + nearp) / (farp - nearp);
	m[2 * 4 + 3] = -1.0f;
	m[3 * 4 + 2] = - 2 * farp * nearp / (farp-nearp);
	m[3 * 4 + 3] = 0.0f;

	multMatrix(m);
}


/*
// returns a pointer to the requested matrix
float *
VSMatrix::get(MatrixTypes aType)
{
	return mMatrix[aType];
}
*/


/* -----------------------------------------------------
			 SEND MATRICES TO OPENGL
------------------------------------------------------*/

// -----------------------------------------------------
//                      AUX functions
// -----------------------------------------------------


// Compute res = M * point
void VSMatrix::multMatrixPoint(const float *point, float *res) 
{
	#if defined(__x86_64__) || defined(_M_X64)
	__m128 p = _mm_setr_ps(point[0], point[1], point[2], point[3]);

	for (int i = 0; i < 4; ++i)
	{
		__m128 m = _mm_setr_ps(mMatrix[i], mMatrix[i + 4], mMatrix[i + 8], mMatrix[i + 12]); // [0] = m[i, 0]
																							 // [1] = m[i, 1]
																							 // [2] = m[i, 2]
																							 // [3] = m[i, 3]

		__m128 c = _mm_mul_ps(p, m); // c[j] = p[j] * m[i, j] with j=0..3
									 // ---------------------
									 // c[0] = p[0] * m[i, 0]
									 // c[1] = p[1] * m[i, 1]
									 // c[2] = p[2] * m[i, 2]
									 // c[3] = p[3] * m[i, 3]

		c = _mm_hadd_ps(c, c); // c'2[0] = c[0] + c[1]
							   // c'2[1] = c[2] + c[3]
							   // --------------------------------------------
							   // c'2[0] = (p[0] * m[i, 0]) + (p[1] * m[i, 1])
							   // c'2[1] = (p[2] * m[i, 2]) + (p[3] * m[i, 3])

		c = _mm_hadd_ps(c, c); // c'3[0] = c'2[0] + c'2[1]
							   // --------------------------------------------
							   // c'3[0] = (p[0] * m[i, 0]) + (p[1] * m[i, 1]) + (p[2] * m[i, 2]) + (p[3] * m[i, 3])

		_mm_store_ss(res + i, c); // res[i] = sum of (p[j] * m[i, j]) with j=0..3
	}
	#else
	for (int i = 0; i < 4; ++i) 
	{
		res[i] = (point[0] * mMatrix[i]) + (point[1] * mMatrix[i + 4]) + (point[2] * mMatrix[i + 8]) + (point[3] * mMatrix[i + 12]);

		/*
		res[i] = 0.0f;
		
		for (int j = 0; j < 4; j++) {

			res[i] += point[j] * mMatrix[j*4 + i];
		} 
		*/
	}
	#endif
}

// res = a cross b;
void
VSMatrix::crossProduct(const float *a, const float *b, float *res) {

	res[0] = a[1] * b[2]  -  b[1] * a[2];
	res[1] = a[2] * b[0]  -  b[2] * a[0];
	res[2] = a[0] * b[1]  -  b[0] * a[1];
}


// returns a . b
float
VSMatrix::dotProduct(const float *a, const float *b) {

	float res = a[0] * b[0]  +  a[1] * b[1]  +  a[2] * b[2];

	return res;
}


// Normalize a vec3
void
VSMatrix::normalize(float *a) {

	float mag = sqrt(a[0] * a[0]  +  a[1] * a[1]  +  a[2] * a[2]);

	a[0] /= mag;
	a[1] /= mag;
	a[2] /= mag;
}


// res = b - a
void
VSMatrix::subtract(const float *a, const float *b, float *res) {

	res[0] = b[0] - a[0];
	res[1] = b[1] - a[1];
	res[2] = b[2] - a[2];
}


// res = a + b
void
VSMatrix::add(const float *a, const float *b, float *res) {

	res[0] = b[0] + a[0];
	res[1] = b[1] + a[1];
	res[2] = b[2] + a[2];
}


// returns |a|
float
VSMatrix::length(const float *a) {

	return(sqrt(a[0] * a[0]  +  a[1] * a[1]  +  a[2] * a[2]));

}



// computes the derived normal matrix for the view matrix
void
VSMatrix::computeNormalMatrix(const float *aMatrix)
{

	double mMat3x3[9];

	mMat3x3[0] = aMatrix[0];
	mMat3x3[1] = aMatrix[1];
	mMat3x3[2] = aMatrix[2];

	mMat3x3[3] = aMatrix[4];
	mMat3x3[4] = aMatrix[5];
	mMat3x3[5] = aMatrix[6];

	mMat3x3[6] = aMatrix[8];
	mMat3x3[7] = aMatrix[9];
	mMat3x3[8] = aMatrix[10];

	double det, invDet;

	det = mMat3x3[0] * (mMat3x3[4] * mMat3x3[8] - mMat3x3[5] * mMat3x3[7]) +
		  mMat3x3[1] * (mMat3x3[5] * mMat3x3[6] - mMat3x3[8] * mMat3x3[3]) +
		  mMat3x3[2] * (mMat3x3[3] * mMat3x3[7] - mMat3x3[4] * mMat3x3[6]);

	invDet = 1.0/det;

	mMatrix[0] = (mMat3x3[4] * mMat3x3[8] - mMat3x3[5] * mMat3x3[7]) * invDet;
	mMatrix[1] = (mMat3x3[5] * mMat3x3[6] - mMat3x3[8] * mMat3x3[3]) * invDet;
	mMatrix[2] = (mMat3x3[3] * mMat3x3[7] - mMat3x3[4] * mMat3x3[6]) * invDet;
	mMatrix[3] = 0.0f;
	mMatrix[4] = (mMat3x3[2] * mMat3x3[7] - mMat3x3[1] * mMat3x3[8]) * invDet;
	mMatrix[5] = (mMat3x3[0] * mMat3x3[8] - mMat3x3[2] * mMat3x3[6]) * invDet;
	mMatrix[6] = (mMat3x3[1] * mMat3x3[6] - mMat3x3[7] * mMat3x3[0]) * invDet;
	mMatrix[7] = 0.0f;
	mMatrix[8] = (mMat3x3[1] * mMat3x3[5] - mMat3x3[4] * mMat3x3[2]) * invDet;
	mMatrix[9] = (mMat3x3[2] * mMat3x3[3] - mMat3x3[0] * mMat3x3[5]) * invDet;
	mMatrix[10] =(mMat3x3[0] * mMat3x3[4] - mMat3x3[3] * mMat3x3[1]) * invDet;
	mMatrix[11] = 0.0;
	mMatrix[12] = 0.0;
	mMatrix[13] = 0.0;
	mMatrix[14] = 0.0;
	mMatrix[15] = 1.0;

}


// aux function resMat = resMat * aMatrix
void
VSMatrix::multMatrix(float *resMat, const float *aMatrix)
{

	float res[16];

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			res[j*4 + i] = 0.0f;
			for (int k = 0; k < 4; ++k)
			{
				res[j*4 + i] += resMat[k*4 + i] * aMatrix[j*4 + k];
			}
		}
	}
	memcpy(resMat, res, 16 * sizeof(float));
}

static double mat3Determinant(const float *mMat3x3)
{
	return mMat3x3[0] * (mMat3x3[4] * mMat3x3[8] - mMat3x3[5] * mMat3x3[7]) +
		mMat3x3[1] * (mMat3x3[5] * mMat3x3[6] - mMat3x3[8] * mMat3x3[3]) +
		mMat3x3[2] * (mMat3x3[3] * mMat3x3[7] - mMat3x3[4] * mMat3x3[6]);
}

static double mat4Determinant(const float *matrix)
{
	float mMat3x3_a[9] =
	{
		matrix[1 * 4 + 1], matrix[2 * 4 + 1], matrix[3 * 4 + 1],
		matrix[1 * 4 + 2], matrix[2 * 4 + 2], matrix[3 * 4 + 2],
		matrix[1 * 4 + 3], matrix[2 * 4 + 3], matrix[3 * 4 + 3]
	};

	float mMat3x3_b[9] =
	{
		matrix[1 * 4 + 0], matrix[2 * 4 + 0], matrix[3 * 4 + 0],
		matrix[1 * 4 + 2], matrix[2 * 4 + 2], matrix[3 * 4 + 2],
		matrix[1 * 4 + 3], matrix[2 * 4 + 3], matrix[3 * 4 + 3]
	};

	float mMat3x3_c[9] =
	{
		matrix[1 * 4 + 0], matrix[2 * 4 + 0], matrix[3 * 4 + 0],
		matrix[1 * 4 + 1], matrix[2 * 4 + 1], matrix[3 * 4 + 1],
		matrix[1 * 4 + 3], matrix[2 * 4 + 3], matrix[3 * 4 + 3]
	};

	float mMat3x3_d[9] =
	{
		matrix[1 * 4 + 0], matrix[2 * 4 + 0], matrix[3 * 4 + 0],
		matrix[1 * 4 + 1], matrix[2 * 4 + 1], matrix[3 * 4 + 1],
		matrix[1 * 4 + 2], matrix[2 * 4 + 2], matrix[3 * 4 + 2]
	};

	float a, b, c, d;
	float value;

	a = mat3Determinant(mMat3x3_a);
	b = mat3Determinant(mMat3x3_b);
	c = mat3Determinant(mMat3x3_c);
	d = mat3Determinant(mMat3x3_d);

	value = matrix[0 * 4 + 0] * a;
	value -= matrix[0 * 4 + 1] * b;
	value += matrix[0 * 4 + 2] * c;
	value -= matrix[0 * 4 + 3] * d;

	return value;
}

static void mat4Adjoint(const float *matrix, float *result)
{
	float mMat3x3_a[9] =
	{
		matrix[1 * 4 + 1], matrix[2 * 4 + 1], matrix[3 * 4 + 1],
		matrix[1 * 4 + 2], matrix[2 * 4 + 2], matrix[3 * 4 + 2],
		matrix[1 * 4 + 3], matrix[2 * 4 + 3], matrix[3 * 4 + 3]
	};

	float mMat3x3_b[9] =
	{
		matrix[1 * 4 + 0], matrix[2 * 4 + 0], matrix[3 * 4 + 0],
		matrix[1 * 4 + 2], matrix[2 * 4 + 2], matrix[3 * 4 + 2],
		matrix[1 * 4 + 3], matrix[2 * 4 + 3], matrix[3 * 4 + 3]
	};

	float mMat3x3_c[9] =
	{
		matrix[1 * 4 + 0], matrix[2 * 4 + 0], matrix[3 * 4 + 0],
		matrix[1 * 4 + 1], matrix[2 * 4 + 1], matrix[3 * 4 + 1],
		matrix[1 * 4 + 3], matrix[2 * 4 + 3], matrix[3 * 4 + 3]
	};

	float mMat3x3_d[9] =
	{
		matrix[1 * 4 + 0], matrix[2 * 4 + 0], matrix[3 * 4 + 0],
		matrix[1 * 4 + 1], matrix[2 * 4 + 1], matrix[3 * 4 + 1],
		matrix[1 * 4 + 2], matrix[2 * 4 + 2], matrix[3 * 4 + 2]
	};

	float mMat3x3_e[9] =
	{
		matrix[0 * 4 + 1], matrix[2 * 4 + 1], matrix[3 * 4 + 1],
		matrix[0 * 4 + 2], matrix[2 * 4 + 2], matrix[3 * 4 + 2],
		matrix[0 * 4 + 3], matrix[2 * 4 + 3], matrix[3 * 4 + 3]
	};

	float mMat3x3_f[9] =
	{
		matrix[0 * 4 + 0], matrix[2 * 4 + 0], matrix[3 * 4 + 0],
		matrix[0 * 4 + 2], matrix[2 * 4 + 2], matrix[3 * 4 + 2],
		matrix[0 * 4 + 3], matrix[2 * 4 + 3], matrix[3 * 4 + 3]
	};

	float mMat3x3_g[9] =
	{
		matrix[0 * 4 + 0], matrix[2 * 4 + 0], matrix[3 * 4 + 0],
		matrix[0 * 4 + 1], matrix[2 * 4 + 1], matrix[3 * 4 + 1],
		matrix[0 * 4 + 3], matrix[2 * 4 + 3], matrix[3 * 4 + 3]
	};

	float mMat3x3_h[9] =
	{
		matrix[0 * 4 + 0], matrix[2 * 4 + 0], matrix[3 * 4 + 0],
		matrix[0 * 4 + 1], matrix[2 * 4 + 1], matrix[3 * 4 + 1],
		matrix[0 * 4 + 2], matrix[2 * 4 + 2], matrix[3 * 4 + 2]
	};

	float mMat3x3_i[9] =
	{
		matrix[0 * 4 + 1], matrix[1 * 4 + 1], matrix[3 * 4 + 1],
		matrix[0 * 4 + 2], matrix[1 * 4 + 2], matrix[3 * 4 + 2],
		matrix[0 * 4 + 3], matrix[1 * 4 + 3], matrix[3 * 4 + 3]
	};

	float mMat3x3_j[9] =
	{
		matrix[0 * 4 + 0], matrix[1 * 4 + 0], matrix[3 * 4 + 0],
		matrix[0 * 4 + 2], matrix[1 * 4 + 2], matrix[3 * 4 + 2],
		matrix[0 * 4 + 3], matrix[1 * 4 + 3], matrix[3 * 4 + 3]
	};

	float mMat3x3_k[9] =
	{
		matrix[0 * 4 + 0], matrix[1 * 4 + 0], matrix[3 * 4 + 0],
		matrix[0 * 4 + 1], matrix[1 * 4 + 1], matrix[3 * 4 + 1],
		matrix[0 * 4 + 3], matrix[1 * 4 + 3], matrix[3 * 4 + 3]
	};

	float mMat3x3_l[9] =
	{
		matrix[0 * 4 + 0], matrix[1 * 4 + 0], matrix[3 * 4 + 0],
		matrix[0 * 4 + 1], matrix[1 * 4 + 1], matrix[3 * 4 + 1],
		matrix[0 * 4 + 2], matrix[1 * 4 + 2], matrix[3 * 4 + 2]
	};

	float mMat3x3_m[9] =
	{
		matrix[0 * 4 + 1], matrix[1 * 4 + 1], matrix[2 * 4 + 1],
		matrix[0 * 4 + 2], matrix[1 * 4 + 2], matrix[2 * 4 + 2],
		matrix[0 * 4 + 3], matrix[1 * 4 + 3], matrix[2 * 4 + 3]
	};

	float mMat3x3_n[9] =
	{
		matrix[0 * 4 + 0], matrix[1 * 4 + 0], matrix[2 * 4 + 0],
		matrix[0 * 4 + 2], matrix[1 * 4 + 2], matrix[2 * 4 + 2],
		matrix[0 * 4 + 3], matrix[1 * 4 + 3], matrix[2 * 4 + 3]
	};

	float mMat3x3_o[9] =
	{
		matrix[0 * 4 + 0], matrix[1 * 4 + 0], matrix[2 * 4 + 0],
		matrix[0 * 4 + 1], matrix[1 * 4 + 1], matrix[2 * 4 + 1],
		matrix[0 * 4 + 3], matrix[1 * 4 + 3], matrix[2 * 4 + 3]
	};

	float mMat3x3_p[9] =
	{
		matrix[0 * 4 + 0], matrix[1 * 4 + 0], matrix[2 * 4 + 0],
		matrix[0 * 4 + 1], matrix[1 * 4 + 1], matrix[2 * 4 + 1],
		matrix[0 * 4 + 2], matrix[1 * 4 + 2], matrix[2 * 4 + 2]
	};

	result[0 * 4 + 0] = mat3Determinant(mMat3x3_a);
	result[1 * 4 + 0] = -mat3Determinant(mMat3x3_b);
	result[2 * 4 + 0] = mat3Determinant(mMat3x3_c);
	result[3 * 4 + 0] = -mat3Determinant(mMat3x3_d);
	result[0 * 4 + 1] = -mat3Determinant(mMat3x3_e);
	result[1 * 4 + 1] = mat3Determinant(mMat3x3_f);
	result[2 * 4 + 1] = -mat3Determinant(mMat3x3_g);
	result[3 * 4 + 1] = mat3Determinant(mMat3x3_h);
	result[0 * 4 + 2] = mat3Determinant(mMat3x3_i);
	result[1 * 4 + 2] = -mat3Determinant(mMat3x3_j);
	result[2 * 4 + 2] = mat3Determinant(mMat3x3_k);
	result[3 * 4 + 2] = -mat3Determinant(mMat3x3_l);
	result[0 * 4 + 3] = -mat3Determinant(mMat3x3_m);
	result[1 * 4 + 3] = mat3Determinant(mMat3x3_n);
	result[2 * 4 + 3] = -mat3Determinant(mMat3x3_o);
	result[3 * 4 + 3] = mat3Determinant(mMat3x3_p);
}

bool VSMatrix::inverseMatrix(VSMatrix &result)
{
	// Calculate mat4 determinant
	float det = mat4Determinant(mMatrix);

	// Inverse unknown when determinant is close to zero
	if (fabs(det) < 1e-15)
	{
		for (int i = 0; i < 16; i++)
			result.mMatrix[i] = float(0.0);
		return false;
	}
	else
	{
		mat4Adjoint(mMatrix, result.mMatrix);

		float invDet = float(1.0) / det;
		for (int i = 0; i < 16; i++)
		{
			result.mMatrix[i] = result.mMatrix[i] * invDet;
		}
	}
	return true;
}
