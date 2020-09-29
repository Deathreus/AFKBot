#ifndef __war3source_navmeshhidingspot_h__
#define __war3source_navmeshhidingspot_h__

#include "public/INavMeshHidingSpot.h"
#include <vector.h>


class CNavMeshHidingSpot : public INavMeshHidingSpot
{
public:
	CNavMeshHidingSpot(unsigned int id, float x, float y, float z, unsigned char flags);
	~CNavMeshHidingSpot() {}

	inline void Destroy() { delete this; }

	unsigned int GetID();

	float GetX();
	float GetY();
	float GetZ();

	const Vector &GetOrigin();

	unsigned char GetFlags();

private:
	unsigned int id;
	
	Vector pos;

	unsigned char flags;
};

#endif