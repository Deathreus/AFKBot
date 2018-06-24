#ifndef __war3source_inavmeshconnection_h__
#define __war3source_inavmeshconnection_h__

#include "..\NavDirType.h"

class INavMeshConnection 
{
public:
	virtual void Destroy() = 0;

	virtual unsigned int GetConnectingAreaID() = 0;
	virtual eNavDir GetDirection() = 0;
};

#endif