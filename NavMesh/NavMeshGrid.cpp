#include "NavMeshGrid.h"


// SourceMod
struct sort_infoADT
{
	cell_t *array_base;
	cell_t array_bsize;
	ICellArray *array_hndl;
};

sort_infoADT g_SortInfoADT;

int sort_array_custom(const void *elem1, const void *elem2)
{
	int index1 = (int)(((cell_t)((cell_t *)elem1 - g_SortInfoADT.array_base)) / g_SortInfoADT.array_bsize);
	int index2 = (int)(((cell_t)((cell_t *)elem2 - g_SortInfoADT.array_base)) / g_SortInfoADT.array_bsize);

	int iGridIndex1 = GetArrayCell(g_SortInfoADT.array_hndl, index1, NavMeshGridList_Owner);
	int iGridIndex2 = GetArrayCell(g_SortInfoADT.array_hndl, index2, NavMeshGridList_Owner);

	if (iGridIndex1 < iGridIndex2) return -1;
	else if (iGridIndex1 > iGridIndex2) return 1;
	return 0;
}

void SortADTArray(ICellArray *array)
{
	size_t arraysize = array->size();
	size_t blocksize = array->blocksize();
	cell_t *base = array->base();

	sort_infoADT oldinfo = g_SortInfoADT;

	g_SortInfoADT.array_base = base;
	g_SortInfoADT.array_bsize = (cell_t)blocksize;
	g_SortInfoADT.array_hndl = array;

	qsort(array, arraysize, blocksize * sizeof(cell_t), sort_array_custom);

	g_SortInfoADT = oldinfo;
}

CNavMeshGrid::CNavMeshGrid(float x1, float y1, float x2, float y2, int x, int y, ICellArray *areas, ICellArray *list)
{
	this->extentLoX = x1;
	this->extentLoY = y1;
	this->extentHiX = x2;
	this->extentHiY = y2;
	this->gridSizeX = x;
	this->gridSizeY = y;
	this->gridareas = areas;
	this->gridlist = list;

	SortADTArray(this->gridlist);

	bool bAllIn = true;
	int iErrorAreaIndex = -1;

	for (int iGridIndex = 0, iSize = this->gridareas->size(); iGridIndex < iSize; iGridIndex++)
	{
		int iStartIndex = -1;
		int iEndIndex = -1;
		GetGridListBounds(iGridIndex, &iStartIndex, &iEndIndex);
		SetArrayCell(this->gridareas, iGridIndex, iStartIndex, NavMeshGrid_ListStartIndex);
		SetArrayCell(this->gridareas, iGridIndex, iEndIndex, NavMeshGrid_ListEndIndex);

		if (iStartIndex != -1)
		{
			for (int iListIndex = iStartIndex; iListIndex <= iEndIndex; iListIndex++)
			{
				int iAreaIndex = GetArrayCell(this->gridlist, iListIndex);
				if (iAreaIndex == -1)
				{
					if (iErrorAreaIndex == -1)
						iErrorAreaIndex = iAreaIndex;
					bAllIn = false;
					smutils->LogError(myself, "Warning! Invalid nav area found in list of grid index %d!", iGridIndex);
				}
			}
		}
	}

	if (bAllIn)
	{
		smutils->LogMessage(myself, "All areas parsed into the grid!");
	}
	else
	{
		smutils->LogError(myself, "Warning! Not all nav areas were parsed into the grid! Please check your nav mesh!");
		smutils->LogError(myself, "First encountered nav area ID %d not in the grid!", iErrorAreaIndex);
	}
}

CNavMeshGrid::~CNavMeshGrid()
{
	delete this->gridareas;
	delete this->gridlist;
}

float CNavMeshGrid::GetExtentLowX() { return this->extentLoX; }

float CNavMeshGrid::GetExtentLowY() { return this->extentLoY; }

float CNavMeshGrid::GetExtentHighX() { return this->extentHiX; }

float CNavMeshGrid::GetExtentHighY() { return this->extentHiY; }

int CNavMeshGrid::GetGridSizeX() { return this->gridSizeX; }

int CNavMeshGrid::GetGridSizeY() { return this->gridSizeY; }

ICellArray *CNavMeshGrid::GetGridAreas() { return this->gridareas; }

ICellArray *CNavMeshGrid::GetGridList() { return this->gridlist; }

void CNavMeshGrid::GetGridListBounds(int iGridIndex, int *iStartIndex, int *iEndIndex)
{
	*iStartIndex = -1;
	*iEndIndex = -1;

	for (int i = 0, iSize = this->gridlist->size(); i < iSize; i++)
	{
		if (GetArrayCell(this->gridlist, i, NavMeshGridList_Owner) == iGridIndex)
		{
			if (*iStartIndex == -1) *iStartIndex = i;
			*iEndIndex = i;
		}
	}
}
