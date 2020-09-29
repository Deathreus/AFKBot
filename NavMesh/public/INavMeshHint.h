#ifndef __war3source_inavmeshhint_h__
#define __war3source_inavmeshhint_h__

class Vector;

class INavMeshHint
{
public:
	virtual void Destroy() = 0;

	virtual unsigned int GetID() = 0;

	virtual float GetX() = 0;
	virtual float GetY() = 0;
	virtual float GetZ() = 0;

	virtual const Vector &GetOrigin() = 0;
	
	virtual float GetYaw() = 0;

	virtual unsigned char GetFlags() = 0;
};

#endif