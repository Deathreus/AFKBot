#ifndef __war3source_navmeshgrid_h__
#define __war3source_navmeshgrid_h__

#include "public/INavMeshGrid.h"


class CNavMeshGrid : public INavMeshGrid
{
public:
	CNavMeshGrid(float x1, float y1, float x2, float y2, int x, int y, ICellArray *areas, ICellArray *list);
	~CNavMeshGrid();

	float GetExtentLowX();
	float GetExtentLowY();

	float GetExtentHighX();
	float GetExtentHighY();

	int GetGridSizeX();
	int GetGridSizeY();

	ICellArray *GetGridAreas();
	ICellArray *GetGridList();

	void GetGridListBounds(int iGridIndex, int *iStartIndex, int *iEndIndex);

private:
	float extentLoX;
	float extentLoY;
	float extentHiX;
	float extentHiY;

	int gridSizeX;
	int gridSizeY;

	ICellArray *gridareas;
	ICellArray *gridlist;
};

// SourceMod
inline void SetArrayCell(ICellArray *array, int index, cell_t value, int block = 0)
{
	cell_t *blk = array->at(index);
	blk[block] = value;
}

inline cell_t GetArrayCell(ICellArray *array, int index, int block = 0)
{
	cell_t *blk = array->at(index);
	return blk[block];
}

inline int PushArrayCell(ICellArray *array, cell_t value)
{
	cell_t *blk = array->push();
	*blk = value;
	return array->size() - 1;
}

#endif