#include "XRMath.h"

#include <limits>
#include <cassert>
#include <cmath>

XRVector3 XRVector3::zero( 0.0f, 0.0f, 0.0f );
XRVector3 XRVector3::one( 1.0f, 1.0f, 1.0f );

XRVector4 XRVector4::zero( 0.0f, 0.0f, 0.0f, 0.0f );
XRVector4 XRVector4::one( 1.0f, 1.0f, 1.0f, 1.0f );

XRQuaternion XRQuaternion::identity( 0.0F, 0.0F, 0.0F, 1.0F );

float SqrMagnitude( const XRQuaternion &q ) { return XRQuaternion::Dot( q, q ); }
float Magnitude( const XRQuaternion &q ) { return std::sqrt( SqrMagnitude( q ) ); }

XRQuaternion &XRQuaternion::operator+=( const XRQuaternion &aQuat )
{
	x += aQuat.x;
	y += aQuat.y;
	z += aQuat.z;
	w += aQuat.w;
	return *this;
}

XRQuaternion &XRQuaternion::operator-=( const XRQuaternion &aQuat )
{
	x -= aQuat.x;
	y -= aQuat.y;
	z -= aQuat.z;
	w -= aQuat.w;
	return *this;
}

XRQuaternion &XRQuaternion::operator*=( const float        aScalar )
{
	x *= aScalar;
	y *= aScalar;
	z *= aScalar;
	w *= aScalar;
	return *this;
}

XRQuaternion &XRQuaternion::operator*=( const XRQuaternion &rhs )
{
	float tempx = w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y;
	float tempy = w * rhs.y + y * rhs.w + z * rhs.x - x * rhs.z;
	float tempz = w * rhs.z + z * rhs.w + x * rhs.y - y * rhs.x;
	float tempw = w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z;
	x = tempx; y = tempy; z = tempz; w = tempw;
	return *this;

}

static XRMatrix3x3 identity( 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f );

XRMatrix3x3::XRMatrix3x3( float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22 )
{
	m[0][0] = m00;
	m[0][1] = m01;
	m[0][2] = m02;
	m[1][0] = m10;
	m[1][1] = m11;
	m[1][2] = m12;
	m[2][0] = m20;
	m[2][1] = m21;
	m[2][2] = m22;
}

float XRMatrix3x3::Get( int row, int col ) const
{
	assert( 0 <= row && row <= 2 && 0 <= col && col <= 2 );
	return m[row][col];
}

XRMatrix4x4::XRMatrix4x4( float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33 )
{
	m[0][0] = m00;
	m[0][1] = m01;
	m[0][2] = m02;
	m[0][3] = m03;
	m[1][0] = m10;
	m[1][1] = m11;
	m[1][2] = m12;
	m[1][3] = m13;
	m[2][0] = m20;
	m[2][1] = m21;
	m[2][2] = m22;
	m[2][3] = m23;
	m[3][0] = m30;
	m[3][1] = m31;
	m[3][2] = m32;
	m[3][3] = m33;
}

XRMatrix4x4 XRMatrix4x4::identity(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f );

static void MultiplyMatrices4x4( const XRMatrix4x4 &inA, const XRMatrix4x4 &inB, XRMatrix4x4 &out )
{
	// TODO: Non-Performant. Look to replace with SIMD?
	for ( int col = 0; col < 4; ++col )
	{
		for ( int row = 0; row < 4; ++row )
		{
			out.m[row][col] = XRVector4::Dot( inA.GetRow( row ), inB.GetCol( col ) );
		}
	}
}

XRMatrix4x4 &XRMatrix4x4::operator*=( const XRMatrix4x4 &inM )
{
	assert( &inM != this );
	XRMatrix4x4 tmp;
	MultiplyMatrices4x4( *this, inM, tmp );
	*this = tmp;
	return *this;
}

XRMatrix4x4 operator*( const XRMatrix4x4 &lhs, const XRMatrix4x4 &rhs )
{
	XRMatrix4x4 result;
	MultiplyMatrices4x4( lhs, rhs, result );
	return result;
}

void XRMatrix4x4::Set( int row, int col, float v )
{
	assert( 0 <= row && row <= 3 && 0 <= col && col <= 3 );
	this->m[row][col] = v;
}

float XRMatrix4x4::Get( int row, int col ) const
{
	assert( 0 <= row && row <= 3 && 0 <= col && col <= 3 );
	return this->m[row][col];
}


void XRMatrix4x4::SetCol( int col, const XRVector4 &v )
{
	assert( 0 <= col && col <= 3 );
	m[0][col] = v.x;
	m[1][col] = v.y;
	m[2][col] = v.z;
	m[3][col] = v.w;
}

XRVector4 XRMatrix4x4::GetCol( int col ) const
{
	assert( 0 <= col && col <= 3 );
	return XRVector4( m[0][col], m[1][col], m[2][col], m[3][col] );
}

void XRMatrix4x4::SetRow( int row, const XRVector4 &v )
{
	assert( 0 <= row && row <= 3 );
	m[row][0] = v.x;
	m[row][1] = v.y;
	m[row][2] = v.z;
	m[row][3] = v.w;
}

XRVector4 XRMatrix4x4::GetRow( int row ) const
{
	assert( 0 <= row && row <= 3 );
	return XRVector4( m[row][0], m[row][1], m[row][2], m[row][3] );
}

XRVector3 XRMatrix4x4::TransformPoint( const XRVector3 &p ) const
{
	XRVector4 t( p.x, p.y, p.z, 1.0f );
	return XRVector3(
		XRVector4::Dot( t, GetCol( 0 ) ),
		XRVector4::Dot( t, GetCol( 1 ) ),
		XRVector4::Dot( t, GetCol( 2 ) ) );
}

XRVector3 XRMatrix4x4::TransformVector( const XRVector3 &v ) const
{
	XRVector4 t( v.x, v.y, v.z, 0.0f );
	return XRVector3(
		XRVector4::Dot( t, GetCol( 0 ) ),
		XRVector4::Dot( t, GetCol( 1 ) ),
		XRVector4::Dot( t, GetCol( 2 ) ) );
}

XRMatrix4x4::operator const UnityXRMatrix4x4 &( ) const
{
	return *reinterpret_cast< const UnityXRMatrix4x4 * >( &m[0][0] );
}

void MatrixToQuaternion( const XRMatrix4x4 &m, XRQuaternion &q )
{
	XRMatrix3x3 mat(
		m.m[0][0], m.m[0][1], m.m[0][2],
		m.m[1][0], m.m[1][1], m.m[1][2],
		m.m[2][0], m.m[2][1], m.m[2][2] );

	MatrixToQuaternion( mat, q );
}

void MatrixToQuaternion( const XRMatrix3x3 &kRot, XRQuaternion &q )
{
	// Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
	// article "Quaternion Calculus and Fast Animation".
#if DEBUGMODE
	float det = kRot.GetDeterminant();
	Assert( CompareApproximately( det, 1.0F, .005f ) );
#endif
	float fTrace = kRot.Get( 0, 0 ) + kRot.Get( 1, 1 ) + kRot.Get( 2, 2 );
	float fRoot;

	if ( fTrace > 0.0f )
	{
		// |w| > 1/2, may as well choose w > 1/2
		fRoot = std::sqrt( fTrace + 1.0f );   // 2w
		q.w = 0.5f * fRoot;
		fRoot = 0.5f / fRoot;  // 1/(4w)
		q.x = ( kRot.Get( 1, 2 ) - kRot.Get( 2, 1 ) ) * fRoot;
		q.y = ( kRot.Get( 2, 0 ) - kRot.Get( 0, 2 ) ) * fRoot;
		q.z = ( kRot.Get( 0, 1 ) - kRot.Get( 1, 0 ) ) * fRoot;
	}
	else
	{
		// |w| <= 1/2
		int s_iNext[3] = { 1, 2, 0 };
		int i = 0;
		if ( kRot.Get( 1, 1 ) > kRot.Get( 0, 0 ) )
			i = 1;
		if ( kRot.Get( 2, 2 ) > kRot.Get( i, i ) )
			i = 2;
		int j = s_iNext[i];
		int k = s_iNext[j];

		fRoot = std::sqrt( kRot.Get( i, i ) - kRot.Get( j, j ) - kRot.Get( k, k ) + 1.0f );
		float *apkQuat[3] = { &q.x, &q.y, &q.z };
		assert( fRoot >= EPSILON );
		*apkQuat[i] = 0.5f * fRoot;
		fRoot = 0.5f / fRoot;
		q.w = ( kRot.Get( j, k ) - kRot.Get( k, j ) ) * fRoot;
		*apkQuat[j] = ( kRot.Get( i, j ) + kRot.Get( j, i ) ) * fRoot;
		*apkQuat[k] = ( kRot.Get( i, k ) + kRot.Get( k, i ) ) * fRoot;
	}
	q = Normalize( q );
}

/*static*/ XRMatrix4x4 XRMatrix4x4::Transpose( const XRMatrix4x4 &m )
{
	XRMatrix4x4 res;
	for ( int i = 0; i < 4; ++i )
	{
		for ( int j = 0; j < 4; ++j )
		{
			res.m[i][j] = m.m[j][i];
		}
	}
	return res;
}

XRMatrix4x4 XRMatrix4x4::FastOrthonormalInverse( const XRMatrix4x4 &m )
{
	XRMatrix4x4 res = identity;

	// transpose upper 3x3 basis
	for ( int i = 0; i < 3; ++i )
	{
		for ( int j = 0; j < 3; ++j )
		{
			res.m[i][j] = m.m[j][i];
		}
	}

	// invert the translation
	res.Translation() = res.TransformVector( -m.Translation() );

	return res;
}

void MatrixToTranslationRotation( const XRMatrix4x4 &matrix, XRVector3 &unityTranslation, XRQuaternion &unityRotation )
{
	unityTranslation.x = matrix.Get( 3, 0 );
	unityTranslation.y = matrix.Get( 3, 1 );
	unityTranslation.z = matrix.Get( 3, 2 );

	MatrixToQuaternion( matrix, unityRotation );
}
