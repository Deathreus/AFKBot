#pragma once


class Vector2D;
template<class T> class CList;
class INavMeshArea;

class INavMeshGrid
{
public:
	virtual void Destroy() = 0;

	virtual Vector2D GetExtentLow() = 0;

	virtual Vector2D GetExtentHigh() = 0;

	virtual int GetGridSizeX() = 0;
	virtual int GetGridSizeY() = 0;

	virtual CList<CList<INavMeshArea*>> &GetGridLists() = 0;
	virtual CList<INavMeshArea*> GetGridAreas(int index) = 0;
};
