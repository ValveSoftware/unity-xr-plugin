#pragma once

#include "UnityXRTypes.h"
#include <limits>

#ifdef __linux__
#include <cmath>
#endif

#define EPSILON 0.00001F

using namespace std;

inline float FastestInvSqrt( float f )
{
	union
	{
		float f;
		int i;
	} u;
	float fhalf = 0.5f * f;
	u.f = f;
	int i = u.i;
	i = 0x5f3759df - ( i >> 1 );
	u.i = i;
	f = u.f;
	f = f * ( 1.5f - fhalf * f * f );
	// f = f*(1.5f - fhalf*f*f); // uncommenting this would be two iterations
	return f;
}

struct XRVector2 : public UnityXRVector2
{
	XRVector2( UnityXRVector2 v ) : UnityXRVector2( v ) {}

	XRVector2( float x = 0.0f, float y = 0.0f ) : UnityXRVector2 { x, y } { }
};


struct XRVector3 : public UnityXRVector3
{
	XRVector3( UnityXRVector3 v ) : UnityXRVector3( v ) {}

	XRVector3( float x = 0.0f, float y = 0.0f, float z = 0.0f ) : UnityXRVector3 { x, y, z } { }

	XRVector3 &operator+=( const XRVector3 &v );
	XRVector3 &operator-=( const XRVector3 &v );
	XRVector3 &operator*=( float scalar );

	static XRVector3 zero;
	static XRVector3 one;

	float SqrMagnitude() const { return Dot( *this, *this ); }

	static float Dot( const XRVector3 &lhs, const XRVector3 &rhs );
	static float Distance( const XRVector3 &lhs, const XRVector3 &rhs );
	static XRVector3 Lerp( XRVector3 &lhs, XRVector3 &rhs, float t );
};

inline XRVector3 operator-( const XRVector3 &v ) { return XRVector3( -v.x, -v.y, -v.z ); }
inline XRVector3 operator-( const XRVector3 &lhs, const XRVector3 &rhs ) { return XRVector3( lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z ); }
inline XRVector3 operator+( const XRVector3 &lhs, const XRVector3 &rhs ) { return XRVector3( lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z ); }
inline XRVector3 operator*( const XRVector3 &inV, float s ) { return XRVector3( inV.x * s, inV.y * s, inV.z * s ); }
inline XRVector3 operator*( float s, const XRVector3 &inV ) { return XRVector3( inV.x * s, inV.y * s, inV.z * s ); }


struct XRVector4 : UnityXRVector4
{
	XRVector4( UnityXRVector4 v ) : UnityXRVector4( v ) {}

	XRVector4( float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f ) : UnityXRVector4 { x, y, z, w } {}

	XRVector4 &operator+=( const XRVector4 &v );
	XRVector4 &operator-=( const XRVector4 &v );
	XRVector4 &operator*=( const float aScalar );

	XRVector4 &operator=( const XRVector4 rhs )
	{
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
		w = rhs.w;
		return *this;
	}

	static XRVector4 zero;
	static XRVector4 one;

	static float Dot( const XRVector4 &lhs, const XRVector4 &rhs )
	{
		return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
	}
	float SqrMagnitude() { return Dot( *this, *this ); }

	static float Distance( const XRVector4 &lhs, const XRVector4 &rhs );

	operator XRVector2() const { return XRVector2( x, y ); }
	operator XRVector3() const { return XRVector3( x, y, z ); }
};

inline XRVector4 operator-( const XRVector4 &lhs, const XRVector4 &rhs ) { return XRVector4( lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w ); }
inline XRVector4 operator*( const XRVector4 &inV, const float s ) { return XRVector4( inV.x * s, inV.y * s, inV.z * s, inV.w * s ); }
inline XRVector4 operator*( const float s, const XRVector4 &inV ) { return XRVector4( inV.x * s, inV.y * s, inV.z * s, inV.w * s ); }

struct XRQuaternion : public UnityXRVector4
{
	XRQuaternion( UnityXRVector4 v ) : UnityXRVector4 { v } {}

	XRQuaternion( float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f ) : UnityXRVector4 { x, y, z, w } {}

	friend float SqrMagnitude( const XRQuaternion &q );
	friend float Magnitude( const XRQuaternion &q );
	friend XRQuaternion Normalize( const XRQuaternion &q ) 
	{ 
		return 1.0f / Magnitude( q ) * q; 
	}

	XRQuaternion &operator+=( const XRQuaternion &aQuat );
	XRQuaternion &operator-=( const XRQuaternion &aQuat );
	XRQuaternion &operator*=( const float        aScalar );
	XRQuaternion &operator*=( const XRQuaternion &aQuat );

	friend XRQuaternion operator-( const XRQuaternion &q )
	{
		return XRQuaternion( -q.x, -q.y, -q.z, -q.w );
	}

	friend XRQuaternion operator+( const XRQuaternion &lhs, const XRQuaternion &rhs )
	{
		XRQuaternion q( lhs );
		return q += rhs;
	}

	friend XRQuaternion  operator-( const XRQuaternion &lhs, const XRQuaternion &rhs )
	{
		XRQuaternion t( lhs );
		return t -= rhs;
	}

	friend XRQuaternion  operator*( const float s, const XRQuaternion &q )
	{
		XRQuaternion t( q );
		return t *= s;
	}

	inline friend XRQuaternion operator*( const XRQuaternion &lhs, const XRQuaternion &rhs )
	{
		return XRQuaternion(
			lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y,
			lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z,
			lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x,
			lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z );
	}

	static XRQuaternion identity;

	static float Dot( const XRQuaternion &q1, const XRQuaternion &q2 )
	{
		return ( q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w );
	}

	static XRQuaternion Lerp( XRQuaternion &lhs, XRQuaternion &rhs, float t )
	{
		XRQuaternion result = ( 1 - t ) * lhs + t * rhs;
		return Normalize( result );
	}
};

struct XRMatrix3x3
{
	float m[3][3];

	XRMatrix3x3( float m00 = 0.0f, float m01 = 0.0f, float m02 = 0.0f,
		float m10 = 0.0f, float m11 = 0.0f, float m12 = 0.0f,
		float m20 = 0.0f, float m21 = 0.0f, float m22 = 0.0f );

	inline float Get( int row, int col ) const;

	static XRMatrix3x3 identity;
};

struct XRMatrix4x4
{
	float m[4][4];
	XRMatrix4x4( float m00 = 0.0f, float m01 = 0.0f, float m02 = 0.0f, float m03 = 0.0f,
		float m10 = 0.0f, float m11 = 0.0f, float m12 = 0.0f, float m13 = 0.0f,
		float m20 = 0.0f, float m21 = 0.0f, float m22 = 0.0f, float m23 = 0.0f,
		float m30 = 0.0f, float m31 = 0.0f, float m32 = 0.0f, float m33 = 0.0f );

	XRMatrix4x4 &operator*=( const XRMatrix4x4 &inM );

	void Set( int row, int col, float v );
	float Get( int row, int col ) const;
	void SetCol( int col, const XRVector4 &v );
	XRVector4 GetCol( int col ) const;
	void SetRow( int row, const XRVector4 &v );
	XRVector4 GetRow( int row ) const;
	XRVector3 TransformPoint( const XRVector3 &p ) const;
	XRVector3 TransformVector( const XRVector3 &v ) const;

	XRVector3 &Translation() { return reinterpret_cast< XRVector3 & >( m[3][0] ); }
	const XRVector3 &Translation() const { return reinterpret_cast< const XRVector3 & >( m[3][0] ); }

	operator const UnityXRMatrix4x4 &( ) const;

	static XRMatrix4x4 identity;

	static XRMatrix4x4 Transpose( const XRMatrix4x4 &m );
	static XRMatrix4x4 FastOrthonormalInverse( const XRMatrix4x4 &m );
};

XRMatrix4x4 operator*( const XRMatrix4x4 &lhs, const XRMatrix4x4 &rhs );

void MatrixToQuaternion( const XRMatrix4x4 &m, XRQuaternion &q );
void MatrixToQuaternion( const XRMatrix3x3 &kRot, XRQuaternion &q );
void MatrixToTranslationRotation( const XRMatrix4x4 &windowsMatrix, XRVector3 &unityTranslation, XRQuaternion &unityRotation );

void QuaternionToMatrix( const XRQuaternion &q, XRMatrix3x3 &m );
void QuaternionToMatrix( const XRQuaternion &q, XRMatrix4x4 &m );

inline bool CompareApproximately( const XRVector3 &inV0, const XRVector3 &inV1, const float inMaxDist )
{
	return ( inV1 - inV0 ).SqrMagnitude() <= inMaxDist * inMaxDist;
}

inline bool CompareApproximately( float f0, float f1, float epsilon = 0.000001F )
{
	float dist = ( f0 - f1 );
	dist = std::abs( dist );
	return dist <= epsilon;
}

inline XRVector3 &XRVector3::operator+=( const XRVector3 &v )
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

inline XRVector3 &XRVector3::operator-=( const XRVector3 &v )
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

inline XRVector3 &XRVector3::operator*=( const float scalar )
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}

inline float XRVector3::Dot( const XRVector3 &lhs, const XRVector3 &rhs )
{
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

inline float XRVector3::Distance( const XRVector3 &lhs, const XRVector3 &rhs )
{
	XRVector3 temp = rhs - lhs;
	return sqrtf( temp.SqrMagnitude() );
}

inline XRVector3 XRVector3::Lerp( XRVector3 &lhs, XRVector3 &rhs, float t )
{
	return ( 1 - t ) * lhs + t * rhs;
}

inline bool CloseTo( XRVector3 &lhs, XRVector3 &rhs, float e )
{
	float d = XRVector3::Dot( lhs, rhs );
	return d <= e;
}

inline bool CloseTo( UnityXRVector3 &lhs, UnityXRVector3 &rhs, float e )
{
	XRVector3 l( lhs );
	XRVector3 r( rhs );
	return CloseTo( l, r, e );
}

inline XRVector4 &XRVector4::operator+=( const XRVector4 &v )
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return *this;
}

inline XRVector4 &XRVector4::operator-=( const XRVector4 &v )
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
	return *this;
}

inline XRVector4 &XRVector4::operator*=( float scalar )
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	w *= scalar;
	return *this;
}

inline float XRVector4::Distance( const XRVector4 &lhs, const XRVector4 &rhs )
{
	XRVector4 temp = rhs - lhs;
	return sqrtf( temp.SqrMagnitude() );
}
