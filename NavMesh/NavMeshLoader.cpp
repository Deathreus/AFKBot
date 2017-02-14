#include "smsdk_ext.h"
#include "List.h"
#include "NavMeshLoader.h"
#include "NavMesh.h"
#include "NavMeshPlace.h"
#include "NavMeshLadder.h"
#include "NavMeshConnection.h"
#include "NavMeshHidingSpot.h"
#include "NavMeshEncounterSpot.h"
#include "NavMeshEncounterPath.h"
#include "NavMeshLadderConnection.h"
#include "NavMeshCornerLightIntensity.h"
#include "NavMeshArea.h"
#include "NavMeshVisibleArea.h"
#include "NavMeshGrid.h"


CNavMeshLoader::CNavMeshLoader(const char *mapName)
{
	strcpy_s(this->mapName, sizeof(this->mapName), mapName);
	this->bytesRead = 0;
}

CNavMeshLoader::~CNavMeshLoader() {}

INavMesh *CNavMeshLoader::Load(char *error, int errorMaxlen)
{
	strcpy_s(error, errorMaxlen, "");
	int elementsRead = 0;

	char navPath[MAX_PATH];
	smutils->BuildPath(Path_Game, navPath, sizeof(navPath), "data\\afkbot\\nav\\%s.nav", this->mapName);

	CFileHandle *fileHandle = new CFileHandle(fopen(navPath, "rb"));
	if (fileHandle->FileError())
	{
		delete fileHandle;
		smutils->LogMessage(myself, "Failed to find nav file, attempting to find it within vpk's...");

		smutils->Format(navPath, sizeof(navPath), "maps\\%s.nav", this->mapName);
		fileHandle = new CFileHandle(filesystem->Open(navPath, "rb", "GAME"));
		if (fileHandle->HandleError())
		{
			delete fileHandle;
			snprintf(error, errorMaxlen, "Unable to find navigation mesh: %s", navPath);
			return NULL;
		}
		else
			smutils->LogMessage(myself, "Successfully found the nav file within a vpk!");
	}

	unsigned int magicNumber;
	elementsRead = fileHandle->Read(&magicNumber, 4, 1);
	this->bytesRead += sizeof(magicNumber);

	if (elementsRead < 1)
	{
		delete fileHandle;
		snprintf(error, errorMaxlen, "Error reading magic number value from navigation mesh: %s", navPath);
		return NULL;
	}

	if (magicNumber != 0xFEEDFACE)
	{
		delete fileHandle;
		snprintf(error, errorMaxlen, "Invalid magic number value from navigation mesh: %s [%p]", navPath, magicNumber);
		return NULL;
	}

	unsigned int version;
	elementsRead = fileHandle->Read(&version, 4, 1);

	if (elementsRead < 1)
	{
		delete fileHandle;
		snprintf(error, errorMaxlen, "Error reading version number from navigation mesh: %s", navPath);
		return NULL;
	}

	if (version < 6 || version > 16)
	{
		delete fileHandle;
		snprintf(error, errorMaxlen, "Invalid version number value from navigation mesh: %s [%d]", navPath, version);
		return NULL;
	}

	unsigned int navMeshSubVersion = 0;

	if (version >= 10)
	{
		fileHandle->Read(&navMeshSubVersion, 4, 1);
	}

	unsigned int saveBspSize;
	fileHandle->Read(&saveBspSize, 4, 1);

	unsigned char meshAnalyzed = 0;
	if (version >= 14)
	{
		fileHandle->Read(&meshAnalyzed, 1, 1);
	}

	bool isMeshAnalyzed = meshAnalyzed != 0;
	smutils->LogMessage(myself, "Is mesh analyzed: %s", isMeshAnalyzed ? "yes" : "no");

	unsigned short placeCount;
	fileHandle->Read(&placeCount, 2, 1);
	this->bytesRead += sizeof(placeCount);

	IList<INavMeshPlace*> *places = new CList<INavMeshPlace*>();

	for (unsigned int placeIndex = 0; placeIndex < placeCount; placeIndex++)
	{
		unsigned short placeSize;

		fileHandle->Read(&placeSize, 2, 1);

		char placeName[256];
		fileHandle->Read(placeName, 1, placeSize);

		places->Append(new CNavMeshPlace(placeIndex, placeName));
	}

	unsigned char unnamedAreas = 0;
	if (version > 11)
	{
		fileHandle->Read(&unnamedAreas, 1, 1);
	}

	bool hasUnnamedAreas = unnamedAreas != 0;
	smutils->LogMessage(myself, "Has unnamed areas: %s", hasUnnamedAreas ? "yes" : "no");

	IList<INavMeshArea*> *areas = new CList<INavMeshArea*>();

	unsigned int areaCount;
	fileHandle->Read(&areaCount, 4, 1);
	this->bytesRead += sizeof(areaCount);

	Vector2D vGridExtLow(0.0f, 0.0f);
	Vector2D vGridExtHi(0.0f, 0.0f);
	bool bExtLowX = false, bExtLowY = false;
	bool bExtHiX = false, bExtHiY = false;

	for (unsigned int areaIndex = 0; areaIndex < areaCount; areaIndex++)
	{
		unsigned int areaID;
		float x1, y1, z1, x2, y2, z2;
		unsigned int areaFlags = 0;
		IList<INavMeshConnection*> *connections = new CList<INavMeshConnection*>();
		IList<INavMeshHidingSpot*> *hidingSpots = new CList<INavMeshHidingSpot*>();
		IList<INavMeshEncounterPath*> *encounterPaths = new CList<INavMeshEncounterPath*>();
		IList<INavMeshLadderConnection*> *ladderConnections = new CList<INavMeshLadderConnection*>();
		IList<INavMeshCornerLightIntensity*> *cornerLightIntensities = new CList<INavMeshCornerLightIntensity*>();
		IList<INavMeshVisibleArea*> *visibleAreas = new CList<INavMeshVisibleArea*>();
		unsigned int inheritVisibilityFrom = 0;
		unsigned char hidingSpotCount = 0;
		unsigned int visibleAreaCount = 0;
		float earliestOccupyTimeFirstTeam = 0.0f;
		float earliestOccupyTimeSecondTeam = 0.0f;
		float northEastCornerZ;
		float southWestCornerZ;
		unsigned short placeID = 0;
		unsigned char unk01 = 0;

		fileHandle->Read(&areaID, 4, 1);

		if (version <= 8)
			fileHandle->Read(&areaFlags, 1, 1);
		else if (version < 13)
			fileHandle->Read(&areaFlags, 2, 1);
		else
			fileHandle->Read(&areaFlags, 4, 1);

		fileHandle->Read(&x1, 4, 1);
		fileHandle->Read(&y1, 4, 1);
		fileHandle->Read(&z1, 4, 1);
		fileHandle->Read(&x2, 4, 1);
		fileHandle->Read(&y2, 4, 1);
		fileHandle->Read(&z2, 4, 1);

		if (!bExtLowX || x1 < vGridExtLow.x)
		{
			bExtLowX = true;
			vGridExtLow.x = x1;
		}

		if (!bExtLowY || y1 < vGridExtLow.y)
		{
			bExtLowY = true;
			vGridExtLow.y = y1;
		}

		if (!bExtHiX || x2 > vGridExtHi.x)
		{
			bExtHiX = true;
			vGridExtHi.x = x2;
		}

		if (!bExtHiY || y2 > vGridExtHi.y)
		{
			bExtHiY = true;
			vGridExtHi.y = y2;
		}

		fileHandle->Read(&northEastCornerZ, 4, 1);
		fileHandle->Read(&southWestCornerZ, 4, 1);

		for (unsigned int direction = 0; direction < NAV_DIR_COUNT; direction++)
		{
			unsigned int connectionCount;
			fileHandle->Read(&connectionCount, 4, 1);

			for (unsigned int connectionIndex = 0; connectionIndex < connectionCount; connectionIndex++)
			{
				unsigned int connectingAreaID;
				fileHandle->Read(&connectingAreaID, 4, 1);

				INavMeshConnection *connection = new CNavMeshConnection(connectingAreaID, (eNavDir)direction);
				connections->Append(connection);
			}
		}

		fileHandle->Read(&hidingSpotCount, 1, 1);
		this->bytesRead += sizeof(hidingSpotCount);

		for (unsigned int hidingSpotIndex = 0; hidingSpotIndex < hidingSpotCount; hidingSpotIndex++)
		{
			unsigned int hidingSpotID;
			fileHandle->Read(&hidingSpotID, 4, 1);

			float hidingSpotX, hidingSpotY, hidingSpotZ;
			fileHandle->Read(&hidingSpotX, 4, 1);
			fileHandle->Read(&hidingSpotY, 4, 1);
			fileHandle->Read(&hidingSpotZ, 4, 1);

			unsigned char hidingSpotFlags;
			fileHandle->Read(&hidingSpotFlags, 1, 1);

			INavMeshHidingSpot *hidingSpot = new CNavMeshHidingSpot(hidingSpotID, hidingSpotX, hidingSpotY, hidingSpotZ, hidingSpotFlags);
			hidingSpots->Append(hidingSpot);
		}

		// These are old but we just need to read the data.
		if (version < 15)
		{
			unsigned char approachAreaCount;
			fileHandle->Read(&approachAreaCount, 1, 1);
			this->bytesRead += sizeof(approachAreaCount);

			for (unsigned int approachAreaIndex = 0; approachAreaIndex < approachAreaCount; approachAreaIndex++)
			{
				unsigned int approachHereID;
				fileHandle->Read(&approachHereID, 4, 1);

				unsigned int approachPrevID;
				fileHandle->Read(&approachPrevID, 4, 1);

				unsigned char approachType;
				fileHandle->Read(&approachType, 1, 1);

				unsigned int approachNextID;
				fileHandle->Read(&approachNextID, 4, 1);

				unsigned char approachHow;
				fileHandle->Read(&approachHow, 1, 1);
			}
		}

		unsigned int encounterPathCount;
		fileHandle->Read(&encounterPathCount, 4, 1);
		this->bytesRead += sizeof(encounterPathCount);

		for (unsigned int encounterPathIndex = 0; encounterPathIndex < encounterPathCount; encounterPathIndex++)
		{
			unsigned int encounterFromID;
			fileHandle->Read(&encounterFromID, 4, 1);

			unsigned char encounterFromDirection;
			fileHandle->Read(&encounterFromDirection, 1, 1);

			unsigned int encounterToID;
			fileHandle->Read(&encounterToID, 4, 1);

			unsigned char encounterToDirection;
			fileHandle->Read(&encounterToDirection, 1, 1);

			unsigned char encounterSpotCount;
			fileHandle->Read(&encounterSpotCount, 1, 1);

			IList<INavMeshEncounterSpot*> *encounterSpots = new CList<INavMeshEncounterSpot*>();

			for (int encounterSpotIndex = 0; encounterSpotIndex < encounterSpotCount; encounterSpotIndex++)
			{
				unsigned int encounterSpotOrderId;
				fileHandle->Read(&encounterSpotOrderId, 4, 1);

				unsigned char encounterSpotT;
				fileHandle->Read(&encounterSpotT, 1, 1);

				float encounterSpotParametricDistance = (float)encounterSpotT / 255.0f;

				INavMeshEncounterSpot *encounterSpot = new CNavMeshEncounterSpot(encounterSpotOrderId, encounterSpotParametricDistance);
				encounterSpots->Append(encounterSpot);
			}

			INavMeshEncounterPath *encounterPath = new CNavMeshEncounterPath(encounterFromID, (eNavDir)encounterFromDirection, encounterToID, (eNavDir)encounterToDirection, encounterSpots);
			encounterPaths->Append(encounterPath);
		}

		fileHandle->Read(&placeID, 2, 1);

		for (unsigned int ladderDirection = 0; ladderDirection < NAV_LADDER_DIR_COUNT; ladderDirection++)
		{
			unsigned int ladderConnectionCount;
			fileHandle->Read(&ladderConnectionCount, 4, 1);

			for (unsigned int ladderConnectionIndex = 0; ladderConnectionIndex < ladderConnectionCount; ladderConnectionIndex++)
			{
				unsigned int ladderConnectID;
				fileHandle->Read(&ladderConnectID, 4, 1);

				INavMeshLadderConnection *ladderConnection = new CNavMeshLadderConnection(ladderConnectID, (eNavLadderDir)ladderDirection);
				ladderConnections->Append(ladderConnection);
			}
		}

		fileHandle->Read(&earliestOccupyTimeFirstTeam, 4, 1);
		fileHandle->Read(&earliestOccupyTimeSecondTeam, 4, 1);

		if (version >= 11)
		{
			for (int navCornerIndex = 0; navCornerIndex < NAV_CORNER_COUNT; navCornerIndex++)
			{
				float navCornerLightIntensity;
				fileHandle->Read(&navCornerLightIntensity, 4, 1);

				INavMeshCornerLightIntensity *cornerLightIntensity = new CNavMeshCornerLightIntensity((eNavCorner)navCornerIndex, navCornerLightIntensity);
				cornerLightIntensities->Append(cornerLightIntensity);
			}

			if (version >= 16)
			{
				fileHandle->Read(&visibleAreaCount, 4, 1);
				this->bytesRead += sizeof(visibleAreaCount);

				for (unsigned int visibleAreaIndex = 0; visibleAreaIndex < visibleAreaCount; visibleAreaIndex++)
				{
					unsigned int visibleAreaID;
					fileHandle->Read(&visibleAreaID, 4, 1);

					unsigned char visibleAreaAttributes;
					fileHandle->Read(&visibleAreaAttributes, 1, 1);

					INavMeshVisibleArea *visibleArea = new CNavMeshVisibleArea(visibleAreaID, visibleAreaAttributes);
					visibleAreas->Append(visibleArea);
				}

				fileHandle->Read(&inheritVisibilityFrom, 4, 1);

				fileHandle->Read(&unk01, 4, 1);
			}
		}

		INavMeshArea *area = new CNavMeshArea(areaID, areaFlags, placeID, x1, y1, z1, x2, y2, z2,
			northEastCornerZ, southWestCornerZ, connections, hidingSpots, encounterPaths, ladderConnections,
			cornerLightIntensities, visibleAreas, inheritVisibilityFrom, earliestOccupyTimeFirstTeam, earliestOccupyTimeSecondTeam, unk01);

		areas->Append(area);
	}

	unsigned int ladderCount;
	fileHandle->Read(&ladderCount, 4, 1);
	this->bytesRead += sizeof(ladderCount);

	IList<INavMeshLadder*> *ladders = new CList<INavMeshLadder*>();

	for (unsigned int ladderIndex = 0; ladderIndex < ladderCount; ladderIndex++)
	{
		unsigned int ladderID;
		fileHandle->Read(&ladderID, 4, 1);

		float ladderWidth;
		fileHandle->Read(&ladderWidth, 4, 1);

		float ladderTopX, ladderTopY, ladderTopZ, ladderBottomX, ladderBottomY, ladderBottomZ;

		fileHandle->Read(&ladderTopX, 4, 1);
		fileHandle->Read(&ladderTopY, 4, 1);
		fileHandle->Read(&ladderTopZ, 4, 1);
		fileHandle->Read(&ladderBottomX, 4, 1);
		fileHandle->Read(&ladderBottomY, 4, 1);
		fileHandle->Read(&ladderBottomZ, 4, 1);

		float ladderLength;
		fileHandle->Read(&ladderLength, 4, 1);

		unsigned int ladderDirection;
		fileHandle->Read(&ladderDirection, 4, 1);

		unsigned int ladderTopForwardAreaID;
		fileHandle->Read(&ladderTopForwardAreaID, 4, 1);

		unsigned int ladderTopLeftAreaID;
		fileHandle->Read(&ladderTopLeftAreaID, 4, 1);

		unsigned int ladderTopRightAreaID;
		fileHandle->Read(&ladderTopRightAreaID, 4, 1);

		unsigned int ladderTopBehindAreaID;
		fileHandle->Read(&ladderTopBehindAreaID, 4, 1);

		unsigned int ladderBottomAreaID;
		fileHandle->Read(&ladderBottomAreaID, 4, 1);

		INavMeshLadder *ladder = new CNavMeshLadder(ladderID, ladderWidth, ladderLength, ladderTopX, ladderTopY, ladderTopZ,
			ladderBottomX, ladderBottomY, ladderBottomZ, (eNavDir)ladderDirection,
			ladderTopForwardAreaID, ladderTopLeftAreaID, ladderTopRightAreaID, ladderTopBehindAreaID, ladderBottomAreaID);

		ladders->Append(ladder);
	}

	// Grid work converted from Kit o' Rifty and the Source SDK

	int iGridSizeX = (int)((vGridExtLow.x - vGridExtHi.x) / 300.0f) + 1;
	int iGridSizeY = (int)((vGridExtLow.y - vGridExtHi.y) / 300.0f) + 1;

	ICellArray *gridareas = CellArray::New(NavMeshGrid_MaxStats);
	gridareas->resize(iGridSizeX * iGridSizeY);
	for (short int iGridIndex = 0, iSize = gridareas->size(); iGridIndex < iSize; iGridIndex++)
	{
		SetArrayCell(gridareas, iGridIndex, -1, NavMeshGrid_ListStartIndex);
		SetArrayCell(gridareas, iGridIndex, -1, NavMeshGrid_ListEndIndex);
	}

	ICellArray *gridlist = CellArray::New(NavMeshGridList_MaxStats);
	for (unsigned int areaIndex = 0; areaIndex < areaCount; areaIndex++)
	{
		INavMeshArea *area = areas->At(areaIndex);
		if (area)
		{
			Vector2D vExtLo, vExtHi;
			vExtLo.Init(area->GetNWExtentX(), area->GetNWExtentY());
			vExtHi.Init(area->GetSEExtentX(), area->GetSEExtentY());

			int loX = (int)((vExtLo.x - vGridExtLow.x) / 300.0f);
			clamp(loX, 0, iGridSizeX - 1);

			int loY = (int)((vExtLo.y - vGridExtLow.y) / 300.0f);
			clamp(loY, 0, iGridSizeY - 1);

			int hiX = (int)((vExtHi.x - vGridExtHi.x) / 300.0f);
			clamp(hiX, 0, iGridSizeX - 1);

			int hiY = (int)((vExtHi.y - vGridExtHi.y) / 300.0f);
			clamp(hiY, 0, iGridSizeY - 1);

			for (int y = loY; y <= hiY; y++)
			{
				for (int x = loX; x <= hiX; x++)
				{
					int iGridIndex = x + y * iGridSizeX;
					int iIndex = PushArrayCell(gridlist, areaIndex);
					SetArrayCell(gridlist, iIndex, iGridIndex, NavMeshGridList_Owner);
				}
			}
		}
	}

	INavMeshGrid *grid = new CNavMeshGrid(vGridExtLow.x, vGridExtLow.y, vGridExtHi.x, vGridExtHi.y, iGridSizeX, iGridSizeY, gridareas, gridlist);

	INavMesh *mesh = new CNavMesh(magicNumber, version, navMeshSubVersion, saveBspSize, isMeshAnalyzed, places, areas, ladders, grid);

	smutils->LogMessage(myself, "Total bytes read: %d", this->bytesRead);
	delete fileHandle;
	return mesh;
}
