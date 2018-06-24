#ifndef __war3source_inavmeshplace_h__
#define __war3source_inavmeshplace_h__

class INavMeshPlace 
{
public:
	virtual void Destroy() = 0;

	virtual const char *GetName() = 0;
	virtual unsigned int GetID() = 0;
};

#endif