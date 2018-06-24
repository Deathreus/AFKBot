#ifndef __war3source_navmeshladderconnection_h__
#define __war3source_navmeshladderconnection_h__

#include "public\INavMeshLadderConnection.h"


class CNavMeshLadderConnection : public INavMeshLadderConnection
{
public:
	CNavMeshLadderConnection(unsigned int connectingLadderID, eNavLadderDir direction);
	~CNavMeshLadderConnection() {}

	inline void Destroy() { delete this; }

	unsigned int GetConnectingLadderID();
	eNavLadderDir GetDirection();

private:
	unsigned int connectingLadderID;
	eNavLadderDir direction;
};

#endif