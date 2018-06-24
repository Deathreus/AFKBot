#ifndef __war3source_navmeshvisiblearea_h__
#define __war3source_navmeshvisiblearea_h__

#include "public\INavMeshVisibleArea.h"


class CNavMeshVisibleArea : public INavMeshVisibleArea
{
public:
	CNavMeshVisibleArea(unsigned int visibleAreaID, unsigned char attributes);
	~CNavMeshVisibleArea() {}

	inline void Destroy() { delete this; }

	unsigned int GetVisibleAreaID();
	unsigned char GetAttributes();

private:
	unsigned int visibleAreaID;
	unsigned char attributes;
};

#endif