#pragma once

//--------------------------------------------------------------------------------------------------
struct Vec4
{
	Vec4() {};
	~Vec4() {};
	explicit Vec4(float initialX, float initialY, float initialZ, float initialW);
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	float w = 0.f;

	const Vec4	operator-(Vec4 const& vecToSubtract) const;		// vec4 - vec4
	const Vec4	operator-() const;								// -vec4, i.e. "unary negation"
	void		operator*=(float const uniformScale);			// vec4 *= float
};