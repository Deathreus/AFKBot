#ifndef __war3source_inavmeshcornerlightintensity_h__
#define __war3source_inavmeshcornerlightintensity_h__

#include "../NavCornerType.h"

class INavMeshCornerLightIntensity 
{
public:
	virtual void Destroy() = 0;

	virtual eNavCorner GetCornerType() = 0;
	virtual float GetLightIntensity() = 0;
};

#endif