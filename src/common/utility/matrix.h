/*
** matrix.h
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

#ifndef __VSMatrix__
#define __VSMatrix__

#include <stdlib.h>
#include "vectors.h"
#include "quaternion.h"

#ifndef NO_SSE
#include <emmintrin.h>
#endif

class alignas(16) VSMatrix
{

	public:

		VSMatrix() = default;

		VSMatrix(int)
		{
			loadIdentity();
		}

		void translate(float x, float y, float z);
		void scale(float x, float y, float z);
		void rotate(float angle, float x, float y, float z);
		void loadIdentity();
		void multVector(float *aVector);
		
		void multMatrix(const float *aMatrix); // aMatrix **MUST** be 16-byte aligned

		void multMatrix(const VSMatrix &aMatrix)
		{
			multMatrix(aMatrix.mMatrix);
		}
		void multQuaternion(const TVector4<float>& q);
		void multQuaternion(const TQuaternion<float>& q);
		void loadMatrix(const float *aMatrix);
		void lookAt(float xPos, float yPos, float zPos, float xLook, float yLook, float zLook, float xUp, float yUp, float zUp);
		void perspective(float fov, float ratio, float nearp, float farp);
		void ortho(float left, float right, float bottom, float top, float nearp=-1.0f, float farp=1.0f);
		void frustum(float left, float right, float bottom, float top, float nearp, float farp);
		void copy(float * pDest)
		{
			memcpy(pDest, mMatrix, 16 * sizeof(float));
		}

		const float *get() const
		{
			return mMatrix;
		}

		void multMatrixPoint(const float *point, float *res);

#ifdef USE_DOUBLE
		void computeNormalMatrix(const float *aMatrix);
#endif
		void computeNormalMatrix(const float *aMatrix);
		void computeNormalMatrix(const VSMatrix &aMatrix)
		{
			computeNormalMatrix(aMatrix.mMatrix);
		}
		bool inverseMatrix(VSMatrix &result);
		void transpose();

	protected:
		static void crossProduct(const float *a, const float *b, float *res);
		static float dotProduct(const float *a, const float * b);
		static void normalize(float *a);
		static void subtract(const float *a, const float *b, float *res);
		static void add(const float *a, const float *b, float *res);
		static float length(const float *a);
		static void multMatrix(float *resMatrix, const float *aMatrix);

		static void setIdentityMatrix(float *mat, int size = 4);
	public:
		/// The storage for matrices
		float mMatrix[16];

};


class Matrix3x4	// used like a 4x4 matrix with the last row always being (0,0,0,1)
{
	float m[3][4];

public:

	void MakeIdentity()
	{
		memset(m, 0, sizeof(m));
		m[0][0] = m[1][1] = m[2][2] = 1.f;
	}

	void Translate(float x, float y, float z)
	{
		m[0][3] = m[0][0]*x + m[0][1]*y + m[0][2]*z + m[0][3];
		m[1][3] = m[1][0]*x + m[1][1]*y + m[1][2]*z + m[1][3];
		m[2][3] = m[2][0]*x + m[2][1]*y + m[2][2]*z + m[2][3];
	}

	void Scale(float x, float y, float z)
	{
		m[0][0] *=x;
		m[1][0] *=x;
		m[2][0] *=x;

		m[0][1] *=y;
		m[1][1] *=y;
		m[2][1] *=y;

		m[0][2] *=z;
		m[1][2] *=z;
		m[2][2] *=z;
	}

	void Rotate(float ax, float ay, float az, float angle)
	{
		Matrix3x4 m1;

		FVector3 axis(ax, ay, az);
		axis.MakeUnit();
		double c = cos(angle * pi::pi()/180.), s = sin(angle * pi::pi()/180.), t = 1 - c;
		double sx = s*axis.X, sy = s*axis.Y, sz = s*axis.Z;
		double tx, ty, txx, tyy, u, v;

		tx = t*axis.X;
		m1.m[0][0] = float( (txx=tx*axis.X) + c );
		m1.m[0][1] = float(   (u=tx*axis.Y) - sz);
		m1.m[0][2] = float(   (v=tx*axis.Z) + sy);

		ty = t*axis.Y;
		m1.m[1][0] = float(              u    + sz);
		m1.m[1][1] = float( (tyy=ty*axis.Y) + c );
		m1.m[1][2] = float(   (u=ty*axis.Z) - sx);

		m1.m[2][0] = float(              v  - sy);
		m1.m[2][1] = float(              u  + sx);
		m1.m[2][2] = float(     (t-txx-tyy) + c );

		m1.m[0][3] = 0.f;
		m1.m[1][3] = 0.f;
		m1.m[2][3] = 0.f;

		*this = (*this) * m1;
	}

	Matrix3x4 operator *(const Matrix3x4 &other)
	{
		Matrix3x4 result;

		result.m[0][0] = m[0][0]*other.m[0][0] + m[0][1]*other.m[1][0] + m[0][2]*other.m[2][0];
		result.m[0][1] = m[0][0]*other.m[0][1] + m[0][1]*other.m[1][1] + m[0][2]*other.m[2][1];
		result.m[0][2] = m[0][0]*other.m[0][2] + m[0][1]*other.m[1][2] + m[0][2]*other.m[2][2];
		result.m[0][3] = m[0][0]*other.m[0][3] + m[0][1]*other.m[1][3] + m[0][2]*other.m[2][3] + m[0][3];

		result.m[1][0] = m[1][0]*other.m[0][0] + m[1][1]*other.m[1][0] + m[1][2]*other.m[2][0];
		result.m[1][1] = m[1][0]*other.m[0][1] + m[1][1]*other.m[1][1] + m[1][2]*other.m[2][1];
		result.m[1][2] = m[1][0]*other.m[0][2] + m[1][1]*other.m[1][2] + m[1][2]*other.m[2][2];
		result.m[1][3] = m[1][0]*other.m[0][3] + m[1][1]*other.m[1][3] + m[1][2]*other.m[2][3] + m[1][3];

		result.m[2][0] = m[2][0]*other.m[0][0] + m[2][1]*other.m[1][0] + m[2][2]*other.m[2][0];
		result.m[2][1] = m[2][0]*other.m[0][1] + m[2][1]*other.m[1][1] + m[2][2]*other.m[2][1];
		result.m[2][2] = m[2][0]*other.m[0][2] + m[2][1]*other.m[1][2] + m[2][2]*other.m[2][2];
		result.m[2][3] = m[2][0]*other.m[0][3] + m[2][1]*other.m[1][3] + m[2][2]*other.m[2][3] + m[2][3];

		return result;
	}

	FVector3 operator *(const FVector3 &vec)
	{
		FVector3 result;

		result.X = vec.X*m[0][0] + vec.Y*m[0][1] + vec.Z*m[0][2] + m[0][3];
		result.Y = vec.X*m[1][0] + vec.Y*m[1][1] + vec.Z*m[1][2] + m[1][3];
		result.Z = vec.X*m[2][0] + vec.Y*m[2][1] + vec.Z*m[2][2] + m[2][3];
		return result;
	}
};

#endif
