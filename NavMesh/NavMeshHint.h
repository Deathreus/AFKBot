#pragma once

#include "public/INavMeshHint.h"


class CNavMeshHint : public INavMeshHint
{
public:
	CNavMeshHint(unsigned int id, float x, float y, float z, float yaw, unsigned char flags);
	~CNavMeshHint() {}

	inline void Destroy() { delete this; }
	
	unsigned int GetID();
	
	float GetX();
	float GetY();
	float GetZ();

	Vector GetPos();
	
	float GetYaw();
	
	unsigned char GetFlags();
	
private:
	unsigned int id;
	
	Vector pos;
	
	float yaw;
	
	unsigned char flags;
};