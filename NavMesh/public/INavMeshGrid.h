#ifndef __war3source_inavmeshgrid_h__
#define __war3source_inavmeshgrid_h__


class Vector2D;
template<class T> class CList;
class INavMeshArea;

class INavMeshGrid
{
public:
	virtual void Destroy() = 0;

	virtual const Vector2D &GetExtentLow() = 0;

	virtual const Vector2D &GetExtentHigh() = 0;

	virtual int GetGridSizeX() = 0;
	virtual int GetGridSizeY() = 0;

	virtual const CList<CList<INavMeshArea*>> *GetGridLists() = 0;
	virtual const CList<INavMeshArea*> *GetGridAreas(int index) = 0;
};

#endif