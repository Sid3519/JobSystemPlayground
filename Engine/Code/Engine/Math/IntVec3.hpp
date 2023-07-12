#pragma once

//--------------------------------------------------------------------------------------------------
struct IntVec3
{
	IntVec3() {};
	~IntVec3() {};

	IntVec3(IntVec3 const& copyFrom);
	explicit IntVec3(int initialX, int initialY, int initialZ);

	IntVec3	const operator+(IntVec3 const& vecToAdd) const;
	IntVec3	const operator-(IntVec3 const& vecToSubtract) const;

	bool		operator==(IntVec3 const& compare) const;
	bool		operator!=(IntVec3 const& compare) const;

	void		operator=	(IntVec3 const& copyFrom);
	void		operator+=	(IntVec3 const& vecToAdd);
	void		operator-=	(IntVec3 const& vecToSubtract);
	void		operator*=	(int uniformScale);

	int x = 0;
	int y = 0;
	int z = 0;
};