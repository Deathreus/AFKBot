#include "List.h"
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
#include "NavMeshHint.h"
#include "NavMeshVisibleArea.h"
#include "NavMeshGrid.h"
#include "NavMeshLoader.h"


#define NAV_MAGIC_NUMBER 0xFEEDFACE

inline FILE *MkDirIfNotExist(const char *szPath, const char *szMode)
{
	FILE *file = fopen(szPath, szMode);

	if(!file || ferror(file))
	{
	#ifndef __linux__
		char *delimiter = "\\";
	#else
		char *delimiter = "/";
	#endif

		char szFolderName[1024];
		int folderNameSize = 0;
		szFolderName[0] = 0;

		int iLen = strlen(szPath);

		int i = 0;

		while(i < iLen)
		{
			while((i < iLen) && (szPath[i] != *delimiter))
			{
				szFolderName[folderNameSize++] = szPath[i];
				i++;
			}

			if(i == iLen)
				break;

			i++;
			szFolderName[folderNameSize++] = *delimiter;//next
			szFolderName[folderNameSize] = 0;

		#ifndef __linux__
			mkdir(szFolderName);
		#else
			mkdir(szFolderName, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
		#endif   
		}

		file = fopen(szPath, szMode);

		if(!file || ferror(file))
		{
			smutils->LogError(myself, "Failed to create file at '%s' with mode %s", szPath, szMode);
			return NULL;
		}
	}

	return file;
}

INavMesh *CNavMeshLoader::Load(char *error, int errorMaxlen)
{
	ke::SafeStrcpy(error, errorMaxlen, "");

	const char *mapName = gpGlobals->mapname.ToCStr();

	char navPath[MAX_PATH];
	smutils->BuildPath(Path_SM, navPath, sizeof(navPath), "data\\afkbot\\nav\\%s.nav", mapName);

	CUtlBuffer fileBuffer(4096, 1024*1024, CUtlBuffer::READ_ONLY);
	if (!filesystem->ReadFile(navPath, "MOD", fileBuffer))
	{
		AFKBot::DebugMessage("Failed to find nav file in %s, attempting to find it within vpk's...", navPath);

		smutils->Format(navPath, sizeof(navPath), "maps\\%s.nav", mapName);
		if (!filesystem->ReadFile(navPath, "GAME", fileBuffer))
		{
			ke::SafeSprintf(error, errorMaxlen, "Unable to find navigation mesh: %s", navPath);
			return nullptr;
		}
		else AFKBot::DebugMessage("Successfully found the nav file within a vpk!");
	}

	unsigned int magicNumber = fileBuffer.GetUnsignedInt();

	if (!fileBuffer.IsValid())
	{
		ke::SafeSprintf(error, errorMaxlen, "Error reading magic number value from navigation mesh: %s", navPath);
		return nullptr;
	}

	if (magicNumber != NAV_MAGIC_NUMBER)
	{
		ke::SafeSprintf(error, errorMaxlen, "Invalid magic number value from navigation mesh: %s [%u]", navPath, magicNumber);
		return nullptr;
	}

	unsigned int navMeshVersion = fileBuffer.GetUnsignedInt();

	if (!fileBuffer.IsValid())
	{
		ke::SafeSprintf(error, errorMaxlen, "Error reading version number from navigation mesh: %s", navPath);
		return nullptr;
	}

	if (navMeshVersion < 6 || navMeshVersion > 16)
	{
		ke::SafeSprintf(error, errorMaxlen, "Invalid version number value from navigation mesh: %s [%d]", navPath, navMeshVersion);
		return nullptr;
	}

	unsigned int navMeshSubVersion = 0;
	if (navMeshVersion >= 10)
	{
		navMeshSubVersion = fileBuffer.GetUnsignedInt();
		if(navMeshSubVersion < 5)
			smutils->LogMessage(myself, "Loaded NavMesh has been unmodified, navigation may experience errors. Expected sub-version 5, got %ud", navMeshSubVersion);
	}

	unsigned int saveBspSize = fileBuffer.GetUnsignedInt();

	char bspPath[MAX_PATH];
	ke::SafeSprintf(bspPath, sizeof(bspPath), "maps\\%s.bsp", mapName);
	unsigned int actualBspSize = filesystem->Size(bspPath, "GAME");

	if (saveBspSize != actualBspSize)
	{
		ke::SafeSprintf(error, errorMaxlen, "Navigation mesh was not built with the same version of the map [%d vs %d].", saveBspSize, actualBspSize);
		return nullptr;
	}

	AFKBot::DebugMessage("Nav version: %d; BSPSize: %d; MagicNumber: %p; SubVersion: %d [v10+only]", navMeshVersion, saveBspSize, magicNumber, navMeshSubVersion);

	unsigned char meshAnalyzed = 0;
	if (navMeshVersion > 14)
	{
		meshAnalyzed = fileBuffer.GetUnsignedChar();
	}

	unsigned short placeCount = fileBuffer.GetUnsignedShort();
	CList<INavMeshPlace*> places;
	for (unsigned int placeIndex = 0; placeIndex < placeCount; placeIndex++)
	{
		unsigned short placeSize = fileBuffer.GetUnsignedShort();

		char *placeName = new char[placeSize + 1];
		fileBuffer.Get(placeName, placeSize);

		places.AddToTail(new CNavMeshPlace(placeIndex+1, placeName));

		AFKBot::VerboseDebugMessage("Place \"%s\" [%i]", placeName, placeIndex);
	}

	AFKBot::DebugMessage("%u places parsed, %u places created.", placeCount, places.Size());

	unsigned char unnamedAreas = 0;
	if (navMeshVersion > 11)
	{
		unnamedAreas = fileBuffer.GetUnsignedChar();
	}

	unsigned int areaCount = fileBuffer.GetUnsignedInt();
	if (areaCount < 1)
	{
		ke::SafeStrcpy(error, errorMaxlen, "Navigation mesh exists but contains no areas.");
		return nullptr;
	}

	Vector2D vGridExtLow(1e+10f, 1e+10f);
	Vector2D vGridExtHi(-1e+10f, -1e+10f);

	CList<INavMeshArea*> areas;
	for (unsigned int areaIndex = 0; areaIndex < areaCount; areaIndex++)
	{
		AFKBot::VerboseDebugMessage("Begin area read:");

		unsigned int areaID = fileBuffer.GetUnsignedInt();

		AFKBot::VerboseDebugMessage("Area ID: %d", areaID);

		unsigned int areaFlags = 0;
		if (navMeshVersion <= 8)
			areaFlags = fileBuffer.GetUnsignedChar();
		else if (navMeshVersion < 13)
			areaFlags = fileBuffer.GetUnsignedShort();
		else
			areaFlags = fileBuffer.GetUnsignedInt();

		AFKBot::VerboseDebugMessage("Area Flags: %d", areaFlags);

		float areaExtLoX = fileBuffer.GetFloat();
		float areaExtLoY = fileBuffer.GetFloat();
		float areaExtLoZ = fileBuffer.GetFloat();
		
		float areaExtHiX = fileBuffer.GetFloat();
		float areaExtHiY = fileBuffer.GetFloat();
		float areaExtHiZ = fileBuffer.GetFloat();

		AFKBot::VerboseDebugMessage("Area extent: (%f, %f, %f), (%f, %f, %f)", areaExtLoX, areaExtLoY, areaExtLoZ, areaExtHiX, areaExtHiY, areaExtHiZ);

		if (areaExtLoX < vGridExtLow.x)
			vGridExtLow.x = areaExtLoX;

		if (areaExtLoY < vGridExtLow.y)
			vGridExtLow.y = areaExtLoY;

		if (areaExtHiX > vGridExtHi.x)
			vGridExtHi.x = areaExtHiX;

		if (areaExtHiY > vGridExtHi.y)
			vGridExtHi.y = areaExtHiY;

		AFKBot::VerboseDebugMessage("New Grid extents: (%f, %f), (%f, %f)", vGridExtLow.x, vGridExtLow.y, vGridExtHi.x, vGridExtHi.y);

		float northEastCornerZ = fileBuffer.GetFloat();
		float southWestCornerZ = fileBuffer.GetFloat();

		AFKBot::VerboseDebugMessage("Corners: NW(%f), SW(%f)", northEastCornerZ, southWestCornerZ);

		CList<CList<INavMeshConnection*>> connections(NAV_DIR_COUNT);
		for (int dir = 0; dir < NAV_DIR_COUNT; dir++)
		{
			unsigned int connectionCount = fileBuffer.GetUnsignedInt();

			if (!fileBuffer.IsValid())
			{
				ke::SafeStrcpy(error, errorMaxlen, "Bad data encountered reading areas.");
				return nullptr;
			}

			AFKBot::VerboseDebugMessage("Connection count: %d", connectionCount);

			for (unsigned int connectionIndex = 0; connectionIndex < connectionCount; connectionIndex++)
			{
				unsigned int connectingAreaID = fileBuffer.GetUnsignedInt();

				connections[dir].AddToTail(new CNavMeshConnection(connectingAreaID, (eNavDir)dir));
			}
		}

#if defined _DEBUG
		for(int dir = 0; dir < NAV_DIR_COUNT; dir++)
		{
			CList<INavMeshConnection*> list = connections[dir];
			AFKBot::VerboseDebugMessage("%u connections created", list.Count());
		}
#endif

		unsigned char hidingSpotCount = fileBuffer.GetUnsignedChar();

		AFKBot::VerboseDebugMessage("Hiding Spot Count: %d", hidingSpotCount);

		CList<INavMeshHidingSpot*> hidingSpots;
		for (unsigned int hidingSpotIndex = 0; hidingSpotIndex < hidingSpotCount; hidingSpotIndex++)
		{
			unsigned int hidingSpotID = fileBuffer.GetUnsignedInt();

			float hidingSpotX = fileBuffer.GetFloat();
			float hidingSpotY = fileBuffer.GetFloat();
			float hidingSpotZ = fileBuffer.GetFloat();

			unsigned char hidingSpotFlags = fileBuffer.GetUnsignedChar();

			hidingSpots.AddToTail(new CNavMeshHidingSpot(hidingSpotID, hidingSpotX, hidingSpotY, hidingSpotZ, hidingSpotFlags));

			AFKBot::VerboseDebugMessage("Parsed hiding spot (%f, %f, %f) with ID [%p] and flags [%p]", hidingSpotX, hidingSpotY, hidingSpotZ, hidingSpotID, hidingSpotFlags);
		}

		AFKBot::VerboseDebugMessage("%u hiding spots created", hidingSpots.Count());

		// These are old but we just need to read the data.
		if (navMeshVersion < 15)
		{
			unsigned char approachAreaCount = fileBuffer.GetUnsignedChar();

			for (unsigned int approachAreaIndex = 0; approachAreaIndex < approachAreaCount; approachAreaIndex++)
			{
				fileBuffer.GetUnsignedInt();
				fileBuffer.GetUnsignedInt();
				fileBuffer.GetUnsignedChar();
				fileBuffer.GetUnsignedInt();
				fileBuffer.GetUnsignedChar();
			}
		}

		unsigned int encounterPathCount = fileBuffer.GetUnsignedInt();

		AFKBot::VerboseDebugMessage("Encounter Path Count: %d", encounterPathCount);

		CList<INavMeshEncounterPath*> encounterPaths;
		for (unsigned int encounterPathIndex = 0; encounterPathIndex < encounterPathCount; encounterPathIndex++)
		{
			unsigned int encounterFromID = fileBuffer.GetUnsignedInt();
			unsigned char encounterFromDirection = fileBuffer.GetUnsignedChar();

			unsigned int encounterToID = fileBuffer.GetUnsignedInt();
			unsigned char encounterToDirection = fileBuffer.GetUnsignedChar();

			unsigned char encounterSpotCount = fileBuffer.GetUnsignedChar();

			AFKBot::VerboseDebugMessage("Encounter [from ID %d] [from dir %p] [to ID %d] [to dir %p] [spot count %d]", encounterFromID, encounterFromDirection, encounterToID, encounterToDirection, encounterSpotCount);

			CList<INavMeshEncounterSpot*> encounterSpots;
			for (int encounterSpotIndex = 0; encounterSpotIndex < encounterSpotCount; encounterSpotIndex++)
			{
				unsigned int encounterSpotOrderId = fileBuffer.GetUnsignedInt();

				unsigned char encounterSpotT = fileBuffer.GetUnsignedChar();

				float encounterSpotParametricDistance = (float)encounterSpotT / 255.0f;

				encounterSpots.AddToTail(new CNavMeshEncounterSpot(encounterSpotOrderId, encounterSpotParametricDistance));

				AFKBot::VerboseDebugMessage("Encounter spot [order id %d] and [T %p]", encounterSpotOrderId, encounterSpotT);
			}

			AFKBot::VerboseDebugMessage("%u encounter spots created", encounterSpots.Count());

			INavMeshEncounterPath *encounterPath = new CNavMeshEncounterPath(encounterFromID, (eNavDir)encounterFromDirection, encounterToID, (eNavDir)encounterToDirection, encounterSpots);
			encounterPaths.AddToTail(encounterPath);
		}

		AFKBot::VerboseDebugMessage("%u encounter paths created", encounterPaths.Count());

		unsigned short placeID = fileBuffer.GetUnsignedShort();

		AFKBot::VerboseDebugMessage("Place ID: %d", placeID);

		CList<CList<INavMeshLadderConnection*>> ladderConnections(NAV_LADDER_DIR_COUNT);
		for (int dir = 0; dir < NAV_LADDER_DIR_COUNT; dir++)
		{
			unsigned int ladderConnectionCount = fileBuffer.GetUnsignedInt();

			AFKBot::VerboseDebugMessage("Ladder Connection Count: %d", ladderConnectionCount);

			for (unsigned int ladderConnectionIndex = 0; ladderConnectionIndex < ladderConnectionCount; ladderConnectionIndex++)
			{
				unsigned int ladderConnectID = fileBuffer.GetUnsignedInt();

				ladderConnections[dir].AddToTail(new CNavMeshLadderConnection(ladderConnectID, (eNavLadderDir)dir));

				AFKBot::VerboseDebugMessage("Parsed ladder connect [ID %d]", ladderConnectID);
			}
		}

#if defined _DEBUG
		for(int dir = 0; dir < NAV_LADDER_DIR_COUNT; dir++)
		{
			CList<INavMeshLadderConnection*> list = ladderConnections[dir];
			AFKBot::VerboseDebugMessage("%u ladder connections created", list.Count());
		}
#endif

		float earliestOccupyTimeFirstTeam = fileBuffer.GetFloat();
		float earliestOccupyTimeSecondTeam = fileBuffer.GetFloat();

		AFKBot::VerboseDebugMessage("Earliest occupy times; 1: %f, 2: %f", earliestOccupyTimeFirstTeam, earliestOccupyTimeSecondTeam);

		CList<INavMeshCornerLightIntensity*> cornerLightIntensities;
		CList<INavMeshVisibleArea*> visibleAreas;

		unsigned int inheritVisibilityFrom = 0;
		
		if (navMeshVersion >= 11)
		{
			for (int navCornerIndex = 0; navCornerIndex < NAV_CORNER_COUNT; navCornerIndex++)
			{
				float navCornerLightIntensity = fileBuffer.GetFloat();

				cornerLightIntensities.AddToTail(new CNavMeshCornerLightIntensity((eNavCorner)navCornerIndex, navCornerLightIntensity));

				AFKBot::VerboseDebugMessage("Light intensity: [%f] [idx %d]", navCornerLightIntensity, navCornerIndex);
			}

			if (navMeshVersion >= 16)
			{
				unsigned int visibleAreaCount = fileBuffer.GetUnsignedInt();

				AFKBot::VerboseDebugMessage("Visible area count: %d", visibleAreaCount);

				for (unsigned int visibleAreaIndex = 0; visibleAreaIndex < visibleAreaCount; visibleAreaIndex++)
				{
					unsigned int visibleAreaID = fileBuffer.GetUnsignedInt();

					unsigned char visibleAreaAttributes = fileBuffer.GetUnsignedChar();

					visibleAreas.AddToTail(new CNavMeshVisibleArea(visibleAreaID, visibleAreaAttributes));

					AFKBot::VerboseDebugMessage("Parsed visible area [%d] with attr [%p]", visibleAreaID, visibleAreaAttributes);
				}

				AFKBot::VerboseDebugMessage("%u visible areas created", visibleAreas.Count());

				inheritVisibilityFrom = fileBuffer.GetUnsignedInt();

				AFKBot::VerboseDebugMessage("Inherit visibilty from: %d", inheritVisibilityFrom);
			}
		}

		INavMeshArea *area = new CNavMeshArea(areaID, areaFlags, placeID, areaExtLoX, areaExtLoY, areaExtLoZ, areaExtHiX, areaExtHiY, areaExtHiZ,
			northEastCornerZ, southWestCornerZ, connections, hidingSpots, encounterPaths, ladderConnections,
			cornerLightIntensities, visibleAreas, inheritVisibilityFrom, earliestOccupyTimeFirstTeam, earliestOccupyTimeSecondTeam);

	#if(SOURCE_ENGINE == SE_TF2)
		area->SetTFAttribs(fileBuffer.GetUnsignedInt());
		AFKBot::VerboseDebugMessage("TF2 specific flags: %d", area->GetTFAttribs());
	#endif

		areas.AddToTail(area);
	}

	AFKBot::DebugMessage("%u areas parsed, %u areas created.", areaCount, areas.Count());

	unsigned int ladderCount = fileBuffer.GetUnsignedInt();
	CList<INavMeshLadder*> ladders;
	for (unsigned int ladderIndex = 0; ladderIndex < ladderCount; ladderIndex++)
	{
		AFKBot::VerboseDebugMessage("Begin ladder read:");

		unsigned int ladderID = fileBuffer.GetUnsignedInt();

		AFKBot::VerboseDebugMessage("Ladder ID: %d", ladderID);

		float ladderWidth = fileBuffer.GetFloat();

		AFKBot::VerboseDebugMessage("Ladder width: %f", ladderWidth);

		float ladderTopX = fileBuffer.GetFloat();
		float ladderTopY = fileBuffer.GetFloat();
		float ladderTopZ = fileBuffer.GetFloat();

		float ladderBottomX = fileBuffer.GetFloat();
		float ladderBottomY = fileBuffer.GetFloat();
		float ladderBottomZ = fileBuffer.GetFloat();

		AFKBot::VerboseDebugMessage("Ladder positions: top (%f, %f, %f), bottom (%f, %f, %f)", ladderTopX, ladderTopY, ladderTopZ, ladderBottomX, ladderBottomY, ladderBottomZ);

		float ladderLength = fileBuffer.GetFloat();

		AFKBot::VerboseDebugMessage("Ladder length: %f");

		unsigned int ladderDirection = fileBuffer.GetUnsignedInt();

		AFKBot::VerboseDebugMessage("Ladder face direction: %s",
			ladderDirection == NAV_DIR_NORTH ? "north" :
			ladderDirection == NAV_DIR_SOUTH ? "south" :
			ladderDirection == NAV_DIR_EAST ? "east" :
			ladderDirection == NAV_DIR_WEST ? "west" : "unknown");

		unsigned int ladderTopForwardAreaID = fileBuffer.GetUnsignedInt();

		unsigned int ladderTopLeftAreaID = fileBuffer.GetUnsignedInt();

		unsigned int ladderTopRightAreaID = fileBuffer.GetUnsignedInt();

		unsigned int ladderTopBehindAreaID = fileBuffer.GetUnsignedInt();

		AFKBot::VerboseDebugMessage("Ladder ending areas: forward [%d], back [%d], left [%d], right [%d]", ladderTopForwardAreaID, ladderTopBehindAreaID, ladderTopLeftAreaID, ladderTopRightAreaID);

		unsigned int ladderBottomAreaID = fileBuffer.GetUnsignedInt();

		AFKBot::VerboseDebugMessage("Ladder starting area: %d", ladderBottomAreaID);

		INavMeshLadder *ladder = new CNavMeshLadder(ladderID, ladderWidth, ladderLength, ladderTopX, ladderTopY, ladderTopZ,
			ladderBottomX, ladderBottomY, ladderBottomZ, (eNavDir)ladderDirection,
			ladderTopForwardAreaID, ladderTopLeftAreaID, ladderTopRightAreaID, ladderTopBehindAreaID, ladderBottomAreaID);

		ladders.AddToTail(ladder);
	}

	AFKBot::DebugMessage("%u ladders parsed, %u ladders created.", ladderCount, ladders.Count());

	CList<INavMeshHint*> hints;
	if (navMeshSubVersion > 3) // My own navmesh version
	{
		unsigned int hintCount = fileBuffer.GetUnsignedInt();
		for (unsigned int hintIndex = 0; hintIndex < hintCount; hintIndex++)
		{
			unsigned int hintID = fileBuffer.GetUnsignedInt();

			float hintX = fileBuffer.GetFloat();
			float hintY = fileBuffer.GetFloat();
			float hintZ = fileBuffer.GetFloat();

			float hintYaw = fileBuffer.GetFloat();

			unsigned char hintAttributes = fileBuffer.GetUnsignedChar();

			hints.AddToTail(new CNavMeshHint(hintID, hintX, hintY, hintZ, hintYaw, hintAttributes));
		}

		AFKBot::DebugMessage("Found valid hint data: %u parsed, %u created.", hintCount, hints.Count());
	}

	CNavMesh::m_iHintCount = hints.Count();

	// Grid work converted from Kit o' Rifty and the Source SDK

	int iGridSizeX = (int)((vGridExtHi.x - vGridExtLow.x) / GridCellSize) + 1;
	int iGridSizeY = (int)((vGridExtHi.y - vGridExtLow.y) / GridCellSize) + 1;

	CList<CList<INavMeshArea*>> gridlist(iGridSizeX * iGridSizeY);
	for (unsigned int areaIndex = 0; areaIndex < areaCount; areaIndex++)
	{
		INavMeshArea *area = areas.Element(areaIndex);
		if (area)
		{
			Vector2D vExtLo, vExtHi;
			vExtLo.Init(area->GetNWExtentX(), area->GetNWExtentY());
			vExtHi.Init(area->GetSEExtentX(), area->GetSEExtentY());

			int loX = clamp((int)((vExtLo.x - vGridExtLow.x) / GridCellSize), 0, iGridSizeX - 1);

			int loY = clamp((int)((vExtLo.y - vGridExtLow.y) / GridCellSize), 0, iGridSizeY - 1);

			int hiX = clamp((int)((vExtHi.x - vGridExtLow.x) / GridCellSize), 0, iGridSizeX - 1);

			int hiY = clamp((int)((vExtHi.y - vGridExtLow.y) / GridCellSize), 0, iGridSizeY - 1);

			for (int y = loY; y <= hiY; y++)
			{
				for (int x = loX; x <= hiX; x++)
				{
					gridlist[x + y * iGridSizeX].AddToTail(area);
				}
			}
		}
		else
		{
			if (areaIndex < areaCount - 1)
				AFKBot::DebugMessage("Warning! Degenerate nav area %d found while building the grid!", areaIndex);
		}
	}

#if defined _DEBUG
	AFKBot::VerboseDebugMessage("Lists in grid: %u", gridlist.Size());
	for(int index = 0; index < gridlist.Count(); index++)
	{
		CList<INavMeshArea*> list = gridlist.Element(index);
		AFKBot::VerboseDebugMessage("List at grid index %i has %u areas allocated.", index, list.Size());
	}
#endif

	INavMeshGrid *grid = new CNavMeshGrid(vGridExtLow, vGridExtHi, iGridSizeX, iGridSizeY, gridlist);

	bool hasUnnamedAreas = unnamedAreas != 0;
	AFKBot::DebugMessage("Has unnamed areas: %s", hasUnnamedAreas ? "yes" : "no");

	bool isMeshAnalyzed = meshAnalyzed != 0;
	AFKBot::DebugMessage("Is mesh analyzed: %s", isMeshAnalyzed ? "yes" : "no");

	INavMesh *mesh = new CNavMesh(magicNumber, navMeshVersion, navMeshSubVersion, saveBspSize, isMeshAnalyzed, hasUnnamedAreas, places, areas, hints, ladders, grid);

	return mesh;
}

bool CNavMeshLoader::Save(INavMesh *pNavMesh)
{
	if (!pNavMesh)
	{
		smutils->LogError(myself, "Can't save a non-existant NavMesh, you sure it's loaded?");
		return false;
	}

	const char *mapName = gpGlobals->mapname.ToCStr();

	char navPath[MAX_PATH];
	smutils->BuildPath(Path_SM, navPath, sizeof(navPath), "data\\afkbot\\nav\\%s.nav", mapName);

	FILE *file = MkDirIfNotExist(navPath, "wb");
	if (!file || ferror(file))
	{
		fclose(file);
		return false;
	}
	fclose(file);	// Only needed to create and verify the existance of the new nav file

	CUtlBuffer fileBuffer(4096, 1024*1024);

	fileBuffer.PutUnsignedInt(NAV_MAGIC_NUMBER);

	unsigned int version = pNavMesh->GetVersion();
	fileBuffer.PutUnsignedInt(version);

	if (version > 9)
	{
		unsigned int subVersion = pNavMesh->GetSubVersion();
		if (subVersion < 5)
		{
			AFKBot::DebugMessage("Nav sub version was %d, upgrading version.", subVersion);

			subVersion = 5;
		}
		fileBuffer.PutUnsignedInt(subVersion);
	}

	char bspPath[MAX_PATH];
	ke::SafeSprintf(bspPath, sizeof(bspPath), "maps\\%s.bsp", mapName);

	unsigned int bspSize = filesystem->Size(bspPath, "GAME");
	fileBuffer.PutUnsignedInt(bspSize);
	AFKBot::VerboseDebugMessage("Size of bsp file '%s' is %u bytes.", bspPath, bspSize);

	if (version > 13)
	{
		fileBuffer.PutUnsignedChar(pNavMesh->IsMeshAnalyzed());
	}

	unsigned short placeCount = pNavMesh->GetPlaces()->Count();
	fileBuffer.PutUnsignedShort(placeCount);

	CList<INavMeshPlace*> *places = pNavMesh->GetPlaces();
	for (unsigned short i = 0; i < placeCount; i++)
	{
		INavMeshPlace *place = places->Element(i);
		if (place)
		{
			const char *placeName = place->GetName();
			unsigned short placeSize = strlen(placeName) + 1;
			fileBuffer.PutUnsignedShort(placeSize);
			fileBuffer.Put(placeName, placeSize);
		}
	}

	if (version > 11)
	{
		fileBuffer.PutUnsignedChar(pNavMesh->HasUnnamedAreas());
	}

	unsigned int areaCount = pNavMesh->GetAreas()->Count();
	fileBuffer.PutUnsignedInt(areaCount);

	CList<INavMeshArea*> *areas = pNavMesh->GetAreas();
	for (unsigned int i = 0; i < areaCount; i++)
	{
		INavMeshArea *area = areas->Element(i);
		if (area)
		{
			fileBuffer.PutUnsignedInt(area->GetID());

			if (version <= 8)
				fileBuffer.PutUnsignedChar(area->GetAttributes());
			else if (version < 13)
				fileBuffer.PutUnsignedShort(area->GetAttributes());
			else
				fileBuffer.PutUnsignedInt(area->GetAttributes());

			fileBuffer.PutFloat(area->GetNWExtentX());
			fileBuffer.PutFloat(area->GetNWExtentY());
			fileBuffer.PutFloat(area->GetNWExtentZ());

			fileBuffer.PutFloat(area->GetSEExtentX());
			fileBuffer.PutFloat(area->GetSEExtentY());
			fileBuffer.PutFloat(area->GetSEExtentZ());

			fileBuffer.PutFloat(area->GetNECornerZ());
			fileBuffer.PutFloat(area->GetSWCornerZ());

			for (int dir = 0; dir < NAV_DIR_COUNT; dir++)
			{
				unsigned int connectionCount = area->GetConnections((eNavDir)dir)->Count();
				fileBuffer.PutUnsignedInt(connectionCount);

				CList<INavMeshConnection*> *connections = area->GetConnections((eNavDir)dir);
				for (unsigned int j = 0; j < connectionCount; j++)
				{
					INavMeshConnection *connection = connections->Element(j);
					if (connection)
					{
						fileBuffer.PutUnsignedInt(connection->GetConnectingAreaID());
					}
				}
			}

			unsigned char hidingSpotCount = area->GetHidingSpots()->Count();
			fileBuffer.PutUnsignedChar(hidingSpotCount);

			CList<INavMeshHidingSpot*> *hidingspots = area->GetHidingSpots();
			for (unsigned int j = 0; j < hidingSpotCount; j++)
			{
				INavMeshHidingSpot *hidingspot = hidingspots->Element(j);
				if (hidingspot)
				{
					fileBuffer.PutUnsignedInt(hidingspot->GetID());

					fileBuffer.PutFloat(hidingspot->GetX());
					fileBuffer.PutFloat(hidingspot->GetY());
					fileBuffer.PutFloat(hidingspot->GetZ());

					fileBuffer.PutUnsignedChar(hidingspot->GetFlags());
				}
			}

			// Spit in some garbage if we're old
			if (version <= 14)
			{
				fileBuffer.PutUnsignedChar(0xF);

				for (unsigned char approachAreaIndex = 0; approachAreaIndex < 0xF; approachAreaIndex++)
				{
					fileBuffer.PutUnsignedInt(0);
					fileBuffer.PutUnsignedInt(0);
					fileBuffer.PutUnsignedChar(0);
					fileBuffer.PutUnsignedInt(0);
					fileBuffer.PutUnsignedChar(0);
				}
			}

			unsigned int encounterPathCount = area->GetEncounterPaths()->Count();
			fileBuffer.PutUnsignedInt(encounterPathCount);

			CList<INavMeshEncounterPath*> *encounterpaths = area->GetEncounterPaths();
			for (unsigned int j = 0; j < encounterPathCount; j++)
			{
				INavMeshEncounterPath *encounterpath = encounterpaths->Element(j);
				if (encounterpath)
				{
					fileBuffer.PutUnsignedInt(encounterpath->GetFromAreaID());
					fileBuffer.PutUnsignedChar(encounterpath->GetFromDirection());

					fileBuffer.PutUnsignedInt(encounterpath->GetToAreaID());
					fileBuffer.PutUnsignedChar(encounterpath->GetToDirection());

					unsigned char encounterSpotCount = encounterpath->GetEncounterSpots()->Count();
					fileBuffer.PutUnsignedChar(encounterSpotCount);

					CList<INavMeshEncounterSpot*> *encounterspots = encounterpath->GetEncounterSpots();
					for (int k = 0; k < encounterSpotCount; k++)
					{
						INavMeshEncounterSpot *encounterspot = encounterspots->Element(k);
						if (encounterspot)
						{
							fileBuffer.PutUnsignedInt(encounterspot->GetOrderID());

							unsigned char encounterSpotT = (unsigned char)(255 * encounterspot->GetParametricDistance());
							fileBuffer.PutUnsignedChar(encounterSpotT);
						}
					}
				}
			}

			fileBuffer.PutUnsignedShort(area->GetPlaceID());

			for (int dir = 0; dir < NAV_LADDER_DIR_COUNT; dir++)
			{
				unsigned int ladderConnectionCount = area->GetLadderConnections((eNavLadderDir)dir)->Count();
				fileBuffer.PutUnsignedInt(ladderConnectionCount);

				CList<INavMeshLadderConnection*> *ladderconnections = area->GetLadderConnections((eNavLadderDir)dir);
				for (unsigned int j = 0; j < ladderConnectionCount; j++)
				{
					INavMeshLadderConnection *ladderconnection = ladderconnections->Element(j);
					if (ladderconnection)
					{
						fileBuffer.PutUnsignedInt(ladderconnection->GetConnectingLadderID());
					}
				}
			}

			fileBuffer.PutFloat(area->GetEarliestOccupyTimeFirstTeam());
			fileBuffer.PutFloat(area->GetEarliestOccupyTimeSecondTeam());

			if (version >= 11)
			{
				CList<INavMeshCornerLightIntensity*> *cornerlightintensities = area->GetCornerLightIntensities();
				for (int corner = 0; corner < NAV_CORNER_COUNT; corner++)
				{
					for (int j = 0; j < cornerlightintensities->Count(); j++)
					{
						INavMeshCornerLightIntensity *cornerlightintensity = cornerlightintensities->Element(j);
						if (cornerlightintensity)
						{
							if ((eNavCorner)corner == cornerlightintensity->GetCornerType())
							{
								fileBuffer.PutFloat(cornerlightintensity->GetLightIntensity());
							}
						}
					}
				}

				if (version >= 16)
				{
					unsigned int visibleAreaCount = area->GetVisibleAreas()->Count();
					fileBuffer.PutUnsignedInt(visibleAreaCount);

					CList<INavMeshVisibleArea*> *visibleareas = area->GetVisibleAreas();
					for (unsigned int j = 0; j < visibleAreaCount; j++)
					{
						INavMeshVisibleArea *visiblearea = visibleareas->Element(j);
						if (visiblearea)
						{
							fileBuffer.PutUnsignedInt(visiblearea->GetVisibleAreaID());

							fileBuffer.PutUnsignedChar(visiblearea->GetAttributes());
						}
					}

					fileBuffer.PutUnsignedInt(area->GetInheritVisibilityFromAreaID());

				#if(SOURCE_ENGINE == SE_TF2)
					fileBuffer.PutUnsignedInt(area->GetTFAttribs());
				#endif
				}
			}
		}
	}

	unsigned int ladderCount = pNavMesh->GetLadders()->Count();
	fileBuffer.PutUnsignedInt(ladderCount);

	CList<INavMeshLadder*> *ladders = pNavMesh->GetLadders();
	for (unsigned int i = 0; i < ladderCount; i++)
	{
		INavMeshLadder *ladder = ladders->Element(i);
		if (ladder)
		{
			fileBuffer.PutUnsignedInt(ladder->GetID());

			fileBuffer.PutFloat(ladder->GetWidth());

			fileBuffer.PutFloat(ladder->GetTopX());
			fileBuffer.PutFloat(ladder->GetTopY());
			fileBuffer.PutFloat(ladder->GetTopZ());

			fileBuffer.PutFloat(ladder->GetBottomX());
			fileBuffer.PutFloat(ladder->GetBottomY());
			fileBuffer.PutFloat(ladder->GetBottomZ());

			fileBuffer.PutFloat(ladder->GetLength());

			fileBuffer.PutUnsignedInt(ladder->GetDirection());

			fileBuffer.PutUnsignedInt(ladder->GetTopForwardAreaID());
			fileBuffer.PutUnsignedInt(ladder->GetTopLeftAreaID());
			fileBuffer.PutUnsignedInt(ladder->GetTopRightAreaID());
			fileBuffer.PutUnsignedInt(ladder->GetTopBehindAreaID());
			fileBuffer.PutUnsignedInt(ladder->GetBottomAreaID());
		}
	}

	unsigned int hintCount = pNavMesh->GetHints()->Count();
	fileBuffer.PutUnsignedInt(hintCount);

	CList<INavMeshHint*> *hints = pNavMesh->GetHints();
	for (unsigned int i = 0; i < hintCount; i++)
	{
		INavMeshHint *hint = hints->Element(i);
		if (hint)
		{
			fileBuffer.PutUnsignedInt(hint->GetID());

			fileBuffer.PutFloat(hint->GetX());
			fileBuffer.PutFloat(hint->GetY());
			fileBuffer.PutFloat(hint->GetZ());

			fileBuffer.PutFloat(hint->GetYaw());

			fileBuffer.PutUnsignedChar(hint->GetFlags());
		}
	}

	if (!filesystem->WriteFile(navPath, "MOD", fileBuffer))
	{
		smutils->LogError(myself, "Unable to save %d bytes to '%s'!", fileBuffer.Size(), navPath);
		return false;
	}

	unsigned int navSize = filesystem->Size(navPath);
	AFKBot::DebugMessage("Saved %u bytes to '%s'.", navSize, navPath);

	return true;
}