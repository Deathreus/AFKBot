#ifndef __war3source_navmeshcornerlightintensity_h__
#define __war3source_navmeshcornerlightintensity_h__

#include "public\INavMeshCornerLightIntensity.h"


class CNavMeshCornerLightIntensity : public INavMeshCornerLightIntensity
{
public:
	CNavMeshCornerLightIntensity(eNavCorner cornerType, float lightIntensity);
	~CNavMeshCornerLightIntensity();

	eNavCorner GetCornerType();
	float GetLightIntensity();

private:
	eNavCorner cornerType;
	float lightIntensity;
};

#endif