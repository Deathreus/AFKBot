#ifndef __war3source_inavmeshladderconnection_h__
#define __war3source_inavmeshladderconnection_h__

#include "../NavDirType.h"

class INavMeshLadderConnection
{
public:
	virtual void Destroy() = 0;

	virtual unsigned int GetConnectingLadderID() = 0;
	virtual eNavLadderDir GetDirection() = 0;
};

#endif