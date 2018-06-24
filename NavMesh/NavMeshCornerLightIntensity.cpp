#include "NavMeshCornerLightIntensity.h"


CNavMeshCornerLightIntensity::CNavMeshCornerLightIntensity(eNavCorner cornerType, float lightIntensity)
{
	this->cornerType = cornerType;
	this->lightIntensity = lightIntensity;
}

eNavCorner CNavMeshCornerLightIntensity::GetCornerType() { return this->cornerType; }

float CNavMeshCornerLightIntensity::GetLightIntensity() { return this->lightIntensity; }
