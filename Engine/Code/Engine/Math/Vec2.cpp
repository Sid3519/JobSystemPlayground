#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include<math.h>



//-----------------------------------------------------------------------------------------------
Vec2::Vec2( const Vec2& copy )
	: x( copy.x )
	, y( copy.y )
{
}


//-----------------------------------------------------------------------------------------------
Vec2::Vec2( float initialX, float initialY )
	: x( initialX )
	, y( initialY )
{
}

Vec2 const Vec2::MakeFromPolarRadians(float orientationRadians, float length)
{
	return Vec2(length * cosf(orientationRadians), length * sinf(orientationRadians));
}

Vec2 const Vec2::MakeFromPolarDegrees(float orientationDegrees, float length)
{
	float orientationRadians = orientationDegrees * (PI * INVERSE_ONE_EIGHTY);
	return Vec2::MakeFromPolarRadians(orientationRadians, length);
}

float Vec2::GetLength() const
{
	return sqrtf((x * x) + (y * y));
}

float Vec2::GetLengthSquared() const
{
	return ((x * x) + (y * y));
}

float Vec2::GetOrientationRadians() const
{
	return (atan2f(y, x));
}

float Vec2::GetOrientationDegrees() const
{
	float orientationInRadians = GetOrientationRadians();

	return (orientationInRadians * 180 * INVERSE_PI);
}

Vec2 const Vec2::GetRotated90Degrees() const
{	
	return Vec2(-y, x);
}

Vec2 const Vec2::GetRotatedMinus90Degrees() const
{
	return Vec2(y, -x);
}

Vec2 const Vec2::GetRotatedRadians(float deltaRadians) const
{
	float dist = GetLength();
	float radians = atan2f(y, x);
	radians += deltaRadians;
	
	// float rotatedX = dist * cosf(radians);
	// float rotatedY = dist * sinf(radians);

	return MakeFromPolarRadians(radians, dist);
}

Vec2 const Vec2::GetRotatedDegrees(float deltaDegrees) const
{
	float deltaRadians = deltaDegrees * PI * INVERSE_ONE_EIGHTY;
	Vec2 rotatedRadians = GetRotatedRadians(deltaRadians);
	return rotatedRadians;
}

Vec2 const Vec2::GetClamped(float maxLength) const
{
	float currentLengthSquared = GetLengthSquared();
	if (currentLengthSquared > (maxLength * maxLength))
	{
		Vec2 normalizedVec = GetNormalized();
		// TODO(sid): Clean this up...
		Vec2 clampedVec = normalizedVec * maxLength;

		return clampedVec;
	}

	return Vec2(x, y);
}

Vec2 const Vec2::GetNormalized() const
{
	float magnitude = GetLength();
	
	float inverseMagMultiplier = 0.f;
	if (magnitude != 0.f)
	{
		inverseMagMultiplier = 1 / magnitude;
	}
	Vec2 normalizedVec = Vec2(x * inverseMagMultiplier, y * inverseMagMultiplier);
	return normalizedVec;
}

Vec2 const Vec2::GetReflected(Vec2 const& bounceSurfaceNormal) const
{
	Vec2 surfacePlane = Vec2(-bounceSurfaceNormal.y, bounceSurfaceNormal.x);

	float projectionOfVectorOnSurfaceNormal = DotProduct2D(Vec2(x, y), bounceSurfaceNormal);
	float projectionOfVectorOnSurfacePlane = DotProduct2D(Vec2(x, y), surfacePlane);

	Vec2 projectedVectorAlongSurfaceNormal = bounceSurfaceNormal * projectionOfVectorOnSurfaceNormal;
	Vec2 projectedVectorAlongSurfacePlane = surfacePlane * projectionOfVectorOnSurfacePlane;

	Vec2 reflectedVector = projectedVectorAlongSurfacePlane - projectedVectorAlongSurfaceNormal;

	return reflectedVector;
}

void Vec2::SetOrientationRadians(float newOrientationRadians)
{
	float displacement = GetLength();

	// x = distance * cosf(newOrientationRadians);
	// y = distance * sinf(newOrientationRadians);
	SetPolarRadians(newOrientationRadians, displacement);
}

void Vec2::SetOrientationDegrees(float newOrientationDegrees)
{
	float displacement = GetLength();
	float orientationInRadians = newOrientationDegrees * PI * INVERSE_ONE_EIGHTY;

	// x = distance * cosf(orientationInRadians);
	// y = distance * sinf(orientationInRadians);
	SetPolarRadians(orientationInRadians, displacement);
}

void Vec2::SetPolarRadians(float newOrientationRadians, float newLength)
{
	x = newLength * cosf(newOrientationRadians);
	y = newLength * sinf(newOrientationRadians);
}

void Vec2::SetPolarDegrees(float newOrientationDegrees, float newLength)
{
	float orientationInRadians = newOrientationDegrees * (PI * INVERSE_ONE_EIGHTY);

	// x = newLength * cosf(orientationInRadians);
	// y = newLength * sinf(orientationInRadians);
	SetPolarRadians(orientationInRadians, newLength);
}

void Vec2::Rotate90Degrees()
{
	float originalX = x;
	x = -y;
	y = originalX;
}

void Vec2::RotateMinus90Degrees()
{
	float originalX = x;
	x = y;
	y = -originalX;
}

void Vec2::RotateRadians(float deltaRadians)
{
	float distance = GetLength();
	float angleInRadians = atan2f(y, x);
	angleInRadians += deltaRadians;

	SetPolarRadians(angleInRadians, distance);
}

void Vec2::RotateDegrees(float deltaDegrees)
{
	float deltaRadians = deltaDegrees * PI * INVERSE_ONE_EIGHTY;
	RotateRadians(deltaRadians);
}

void Vec2::SetLength(float newLength)
{
	Normalize();

	x *= newLength;
	y *= newLength;
}

void Vec2::ClampLength(float newLength)
{
	float currentLengthSquared = GetLengthSquared();
	if (currentLengthSquared > (newLength * newLength))
	{
		Normalize();

		x *= newLength;
		y *= newLength;
	}
}

void Vec2::Normalize()
{
	float magnitude = GetLength();
	if (magnitude == 0.f)
	{
		x *= 0.f;
		y *= 0.f;
		return;
	}
	float inverseMagMultiplier = 1.f / magnitude;

	x *= inverseMagMultiplier;
	y *= inverseMagMultiplier;
}

void Vec2::Reflect(Vec2 bounceSurfaceNormal)
{
	Vec2 surfacePlane = Vec2(-bounceSurfaceNormal.y, bounceSurfaceNormal.x);

	float projectionOfVectorOnSurfaceNormal = DotProduct2D(Vec2(x, y), bounceSurfaceNormal);
	float projectionOfVectorOnSurfacePlane = DotProduct2D(Vec2(x, y), surfacePlane);

	Vec2 projectedVectorAlongSurfaceNormal = bounceSurfaceNormal * projectionOfVectorOnSurfaceNormal;
	Vec2 projectedVectorAlongSurfacePlane = surfacePlane * projectionOfVectorOnSurfacePlane;

	Vec2 reflectedVector = projectedVectorAlongSurfacePlane - projectedVectorAlongSurfaceNormal;

	x = reflectedVector.x;
	y = reflectedVector.y;
}

float Vec2::NormalizeAndGetPreviousLength()
{
	float currentLength = GetLength();
	Normalize();

	return currentLength;
}

void Vec2::SetFromText(char const* text)
{
	Strings delimitedText = SplitStringOnDelimiter(text, ',');
	x = float(atof(delimitedText[0].data()));
	y = float(atof(delimitedText[1].data()));
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator + ( const Vec2& vecToAdd ) const
{
	return Vec2( x + vecToAdd.x, y + vecToAdd.y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator - ( const Vec2& vecToSubtract ) const
{
	return Vec2( x - vecToSubtract.x, y - vecToSubtract.y );
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-() const
{
	return Vec2( -x, -y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( float uniformScale ) const
{
	return Vec2( x * uniformScale, y * uniformScale);
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( const Vec2& vecToMultiply ) const
{
	return Vec2( x * vecToMultiply.x, y * vecToMultiply.y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator/( float inverseScale ) const
{
	float multiplier = 1 / inverseScale;
	return Vec2( x * multiplier, y * multiplier );
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator+=( const Vec2& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator-=( const Vec2& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator/=( const float uniformDivisor )
{
	float multiplier = 1 / uniformDivisor;

	x *= multiplier;
	y *= multiplier;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator=( const Vec2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
const Vec2 operator*( float uniformScale, const Vec2& vecToScale )
{
	return Vec2( vecToScale.x * uniformScale, vecToScale.y * uniformScale );
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator==( const Vec2& compare ) const
{
	return ((x == compare.x) && (y == compare.y));
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator!=( const Vec2& compare ) const
{
	return ((x != compare.x) || (y != compare.y));
}




