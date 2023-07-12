#pragma once

//--------------------------------------------------------------------------------------------------
struct IntVec2
{
public:
	int x = 0;
	int y = 0;

public:
	IntVec2() {};
	~IntVec2() {};

	IntVec2(IntVec2 const& copyFrom);
	explicit IntVec2(int initialX, int initialY);

	float GetLength() const;
	int GetTaxicabLength() const;
	int GetLengthSquared() const;
	float GetOrientationRadians() const;
	float GetOrientationDegrees() const;
	IntVec2 const GetRotated90Degrees() const;
	IntVec2 const GetRotatedMinus90Degrees() const;


	void Rotate90Degrees();
	void RotateMinus90Degrees();
	void SetFromText(char const* text);

	IntVec2	const operator+(IntVec2 const& vecToAdd) const;
	IntVec2	const operator-(IntVec2 const& vecToSubtract) const;

	void operator=(IntVec2 const& copyFrom);
	bool		operator==(const IntVec2& compare) const;
	bool		operator!=(const IntVec2& compare) const;
};