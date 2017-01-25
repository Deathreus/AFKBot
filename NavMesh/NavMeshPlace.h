#ifndef __war3source_navmeshplace_h__
#define __war3source_navmeshplace_h__

#include "public\INavMeshPlace.h"


class CNavMeshPlace : public INavMeshPlace
{
public:
	CNavMeshPlace(unsigned int id, const char *name);
	~CNavMeshPlace();

	const char *GetName();
	unsigned int GetID();

private:
	unsigned int id;
	char name[256];
};

#endif