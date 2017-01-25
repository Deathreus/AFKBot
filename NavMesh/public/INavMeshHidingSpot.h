#ifndef __war3source_inavmeshhidingspot_h__
#define __war3source_inavmeshhidingspot_h__

class INavMeshHidingSpot
{
public:
	virtual unsigned int GetID() = 0;

	virtual float GetX() = 0;
	virtual float GetY() = 0;
	virtual float GetZ() = 0;

	virtual unsigned char GetFlags() = 0;
};

#endif