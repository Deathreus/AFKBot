#ifndef __war3source_navmeshloader_h__
#define __war3source_navmeshloader_h__

#include "public/INavMesh.h"

#include "../extension.h"


extern IFileSystem *filesystem;

class CNavMeshLoader
{
public:
	static INavMesh *Load(char *, int);
	static bool Save(INavMesh *);
};

#endif