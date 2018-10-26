#ifndef __war3source_navmeshconnection_h__
#define __war3source_navmeshconnection_h__

#include "public/INavMeshConnection.h"


class CNavMeshConnection : public INavMeshConnection
{
public:
	CNavMeshConnection(unsigned int connectingAreaID, eNavDir direction);
	~CNavMeshConnection() {}

	inline void Destroy() { delete this; }

	unsigned int GetConnectingAreaID();
	eNavDir GetDirection();

private:
	unsigned int connectingAreaID;
	eNavDir direction;
};

#endif