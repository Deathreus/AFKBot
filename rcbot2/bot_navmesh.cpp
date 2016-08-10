/*
 *    This file is part of RCBot.
 *
 *    RCBot by Paul Murphy adapted from Botman's HPB Bot 2 template.
 *
 *    RCBot is free software; you can redistribute it and/or modify it
 *    under the terms of the GNU General Public License as published by the
 *    Free Software Foundation; either version 2 of the License, or (at
 *    your option) any later version.
 *
 *    RCBot is distributed in the hope that it will be useful, but
 *    WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with RCBot; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    In addition, as a special exception, the author gives permission to
 *    link the code of this program with the Half-Life Game engine ("HL
 *    engine") and Modified Game Libraries ("MODs") developed by Valve,
 *    L.L.C ("Valve").  You must obey the GNU General Public License in all
 *    respects for all of the code used other than the HL engine and MODs
 *    from Valve.  If you modify this file, you may extend this exception
 *    to your version of the file, but you are not obligated to do so.  If
 *    you do not wish to do so, delete this exception statement from your
 *    version.
 *
 */

#include "bot_navmesh.h"
#include <iostream>
#include <sys/stat.h>

void CNavMeshNavigator::FreeMapMemory()
{
	delete m_pNavMesh;
	m_pNavMesh = NULL;
}

void CNavMeshNavigator::FreeAllMemory()
{
	FreeMapMemory();
}

bool CNavMeshNavigator::Init(char *error, size_t maxlen)
{
	INavMeshLoader *loader = new CNavMeshLoader(STRING(gpGlobals->mapname));
	this->m_pNavMesh = loader->Load(error, maxlen);

	return m_pNavMesh != NULL;
}

CNavMesh::CNavMesh(unsigned int magicNumber, unsigned int version, unsigned int subVersion, unsigned int saveBSPSize, bool isMeshAnalyzed,
	IList<INavMeshPlace*> *places, IList<INavMeshArea*> *areas, IList<INavMeshLadder*> *ladders)
{
	this->magicNumber = magicNumber;
	this->version = version;
	this->subVersion = subVersion;
	this->saveBSPSize = saveBSPSize;
	this->isMeshAnalyzed = isMeshAnalyzed;
	this->places = places;
	this->areas = areas;
	this->ladders = ladders;
}

unsigned int CNavMesh::GetMagicNumber() { return this->magicNumber; }
unsigned int CNavMesh::GetVersion() { return this->version; }
unsigned int CNavMesh::GetSubVersion() { return this->subVersion; }
unsigned int CNavMesh::GetSaveBSPSize() { return this->saveBSPSize; }
bool CNavMesh::IsMeshAnalyzed() { return this->isMeshAnalyzed; }

IList<INavMeshPlace*> *CNavMesh::GetPlaces() { return this->places; }
IList<INavMeshArea*> *CNavMesh::GetAreas() { return this->areas; }
IList<INavMeshLadder*> *CNavMesh::GetLadders() { return this->ladders; }

CNavMeshArea::CNavMeshArea(unsigned int id, unsigned int flags, unsigned int placeID,
	float nwExtentX, float nwExtentY, float nwExtentZ,
	float seExtentX, float seExtentY, float seExtentZ,
	float neCornerZ, float swCornerZ,
	IList<INavMeshConnection*> *connections, IList<INavMeshHidingSpot*> *hidingSpots, IList<INavMeshEncounterPath*> *encounterPaths,
	IList<INavMeshLadderConnection*> *ladderConnections, IList<INavMeshCornerLightIntensity*> *cornerLightIntensities,
	IList<INavMeshVisibleArea*> *visibleAreas, unsigned int inheritVisibilityFromAreaID,
	float earliestOccupyTimeFirstTeam, float earliestOccupyTimeSecondTeam, unsigned char unk01)
{
	this->id = id;
	this->flags = flags;
	this->placeID = placeID;
	this->nwExtentX = nwExtentX;
	this->nwExtentY = nwExtentY;
	this->nwExtentZ = nwExtentZ;
	this->seExtentX = seExtentX;
	this->seExtentY = seExtentY;
	this->seExtentZ = seExtentZ;
	this->neCornerZ = neCornerZ;
	this->swCornerZ = swCornerZ;
	this->connections = connections;
	this->hidingSpots = hidingSpots;
	this->encounterPaths = encounterPaths;
	this->ladderConnections = ladderConnections;
	this->cornerLightIntensities = cornerLightIntensities;
	this->visibleAreas = visibleAreas;
	this->inheritVisibilityFromAreaID = inheritVisibilityFromAreaID;
	this->earliestOccupyTimeFirstTeam = earliestOccupyTimeFirstTeam;
	this->earliestOccupyTimeSecondTeam = earliestOccupyTimeSecondTeam;
	this->unk01 = unk01;
}

unsigned int CNavMeshArea::GetID() { return this->id; }
unsigned int CNavMeshArea::GetFlags() { return this->flags; }
unsigned int CNavMeshArea::GetPlaceID() { return this->placeID; }

float CNavMeshArea::GetNWExtentX() { return this->nwExtentX; }
float CNavMeshArea::GetNWExtentY() { return this->nwExtentY; }
float CNavMeshArea::GetNWExtentZ() { return this->nwExtentZ; }

float CNavMeshArea::GetSEExtentX() { return this->seExtentX; }
float CNavMeshArea::GetSEExtentY() { return this->seExtentY; }
float CNavMeshArea::GetSEExtentZ() { return this->seExtentZ; }

float CNavMeshArea::GetEarliestOccupyTimeFirstTeam() { return this->earliestOccupyTimeFirstTeam; }
float CNavMeshArea::GetEarliestOccupyTimeSecondTeam() { return this->earliestOccupyTimeSecondTeam; }

float CNavMeshArea::GetNECornerZ() { return this->neCornerZ; }
float CNavMeshArea::GetSWCornerZ() { return this->swCornerZ; }

IList<INavMeshConnection*> *CNavMeshArea::GetConnections() { return this->connections; }
IList<INavMeshHidingSpot*> *CNavMeshArea::GetHidingSpots() { return this->hidingSpots; }
IList<INavMeshEncounterPath*> *CNavMeshArea::GetEncounterPaths() { return this->encounterPaths; }
IList<INavMeshLadderConnection*> *CNavMeshArea::GetLadderConnections() { return this->ladderConnections; }
IList<INavMeshCornerLightIntensity*> *CNavMeshArea::GetCornerLightIntensities() { return this->cornerLightIntensities; }
IList<INavMeshVisibleArea*> *CNavMeshArea::GetVisibleAreas() { return this->visibleAreas; }

unsigned int CNavMeshArea::GetInheritVisibilityFromAreaID() { return this->inheritVisibilityFromAreaID; }

unsigned char CNavMeshArea::GetUnk01() { return this->unk01; }

CNavMeshConnection::CNavMeshConnection(unsigned int connectingAreaID, eNavDir direction)
{
	this->connectingAreaID = connectingAreaID;
	this->direction = direction;
}

unsigned int CNavMeshConnection::GetConnectingAreaID() { return this->connectingAreaID; }

eNavDir CNavMeshConnection::GetDirection() { return this->direction; }

CNavMeshCornerLightIntensity::CNavMeshCornerLightIntensity(eNavCorner cornerType, float lightIntensity)
{
	this->cornerType = cornerType;
	this->lightIntensity = lightIntensity;
}

eNavCorner CNavMeshCornerLightIntensity::GetCornerType() { return this->cornerType; }

float CNavMeshCornerLightIntensity::GetLightIntensity() { return this->lightIntensity; }

CNavMeshEncounterPath::CNavMeshEncounterPath(unsigned int fromAreaID, eNavDir fromDirection,
	unsigned int toAreaID, eNavDir toDirection, IList<INavMeshEncounterSpot*> *encounterSpots)
{
	this->fromAreaID = fromAreaID;
	this->fromDirection = fromDirection;
	this->toAreaID = toAreaID;
	this->toDirection = toDirection;
	this->encounterSpots = encounterSpots;
}

unsigned int CNavMeshEncounterPath::GetFromAreaID() { return this->fromAreaID; }
eNavDir CNavMeshEncounterPath::GetFromDirection() { return this->fromDirection; }

unsigned int CNavMeshEncounterPath::GetToAreaID() { return this->toAreaID; }
eNavDir CNavMeshEncounterPath::GetToDirection() { return this->toDirection; }

IList<INavMeshEncounterSpot*> *CNavMeshEncounterPath::GetEncounterSpots() { return this->encounterSpots; }

CNavMeshEncounterSpot::CNavMeshEncounterSpot(unsigned int orderID, float parametricDistance)
{
	this->orderID = orderID;
	this->parametricDistance = parametricDistance;
}

unsigned int CNavMeshEncounterSpot::GetOrderID() { return this->orderID; }

float CNavMeshEncounterSpot::GetParametricDistance() { return this->parametricDistance; }

CNavMeshHidingSpot::CNavMeshHidingSpot(unsigned int id, float x, float y, float z, unsigned char flags)
{
	this->id = id;
	this->x = x;
	this->y = y;
	this->z = z;
	this->flags = flags;
}

unsigned int CNavMeshHidingSpot::GetID() { return this->id; }

float CNavMeshHidingSpot::GetX() { return this->x; }
float CNavMeshHidingSpot::GetY() { return this->y; }
float CNavMeshHidingSpot::GetZ() { return this->z; }

unsigned char CNavMeshHidingSpot::GetFlags() { return this->flags; }

CNavMeshLadder::CNavMeshLadder(unsigned int id, float width, float length, float topX, float topY, float topZ,
	float bottomX, float bottomY, float bottomZ, eNavDir direction,
	unsigned int topForwardAreaID, unsigned int topLeftAreaID, unsigned int topRightAreaID, unsigned int topBehindAreaID, unsigned int bottomAreaID)
{
	this->id = id;
	this->width = width;
	this->length = length;
	this->topX = topX;
	this->topY = topY;
	this->topZ = topZ;
	this->bottomX = bottomX;
	this->bottomY = bottomY;
	this->bottomZ = bottomZ;
	this->direction = direction;
	this->topForwardAreaID = topForwardAreaID;
	this->topLeftAreaID = topLeftAreaID;
	this->topRightAreaID = topRightAreaID;
	this->topBehindAreaID = topBehindAreaID;
	this->bottomAreaID = bottomAreaID;
}

unsigned int CNavMeshLadder::GetID() { return this->id; }

float CNavMeshLadder::GetWidth() { return this->width; }
float CNavMeshLadder::GetLength() { return this->length; }

float CNavMeshLadder::GetTopX() { return this->topX; }
float CNavMeshLadder::GetTopY() { return this->topY; }
float CNavMeshLadder::GetTopZ() { return this->topZ; }

float CNavMeshLadder::GetBottomX() { return this->bottomX; }
float CNavMeshLadder::GetBottomY() { return this->bottomY; }
float CNavMeshLadder::GetBottomZ() { return this->bottomZ; }

eNavDir CNavMeshLadder::GetDirection() { return this->direction; }

unsigned int CNavMeshLadder::GetTopForwardAreaID() { return this->topForwardAreaID; }
unsigned int CNavMeshLadder::GetTopLeftAreaID() { return this->topLeftAreaID; }
unsigned int CNavMeshLadder::GetTopRightAreaID() { return this->topRightAreaID; }
unsigned int CNavMeshLadder::GetTopBehindAreaID() { return this->topBehindAreaID; }
unsigned int CNavMeshLadder::GetBottomAreaID() { return this->bottomAreaID; }

CNavMeshLadderConnection::CNavMeshLadderConnection(unsigned int connectingLadderID, eNavLadder direction)
{
	this->connectingLadderID = connectingLadderID;
	this->direction = direction;
}

unsigned int CNavMeshLadderConnection::GetConnectingLadderID() { return this->connectingLadderID; }

eNavLadder CNavMeshLadderConnection::GetDirection() { return this->direction; }

CNavMeshPlace::CNavMeshPlace(unsigned int id, const char *name)
{
	this->id = id;
	strcpy_s(this->name, sizeof(this->name), name);
}

const char *CNavMeshPlace::GetName() { return this->name; }
unsigned int CNavMeshPlace::GetID() { return this->id; }

CNavMeshVisibleArea::CNavMeshVisibleArea(unsigned int visibleAreaID, unsigned char attributes)
{
	this->visibleAreaID = visibleAreaID;
	this->attributes = attributes;
}

unsigned int CNavMeshVisibleArea::GetVisibleAreaID() { return this->visibleAreaID; }
unsigned char CNavMeshVisibleArea::GetAttributes() { return this->attributes; }

CNavMeshLoader::CNavMeshLoader(const char *mapName)
{
	strcpy_s(this->mapName, sizeof(this->mapName), mapName);
	this->bytesRead = 0;
}

INavMesh *CNavMeshLoader::Load(char *error, size_t maxlen)
{
	strcpy_s(error, maxlen, "");

	char navPath[MAX_PATH];
	g_pSM->BuildPath(Path_SM, navPath, sizeof(navPath), "configs\\afkbot\\nav\\%s.nav", this->mapName);

	FILE *fileHandle = fopen(navPath, "rb");

	if (!fileHandle)
	{
		sprintf_s(error, maxlen, "Unable to find navigation mesh: %s", navPath);
		return NULL;
	}

	unsigned int magicNumber;
	int elementsRead = this->ReadData(&magicNumber, sizeof(unsigned int), 1, fileHandle);

	if (elementsRead != 1)
	{
		fclose(fileHandle);
		sprintf_s(error, maxlen, "Error reading magic number value from navigation mesh: %s", navPath);
		return NULL;
	}

	if (magicNumber != 0xFEEDFACE)
	{
		fclose(fileHandle);
		sprintf_s(error, maxlen, "Invalid magic number value from navigation mesh: %s [%p]", navPath, magicNumber);
		return NULL;
	}

	unsigned int version;
	elementsRead = this->ReadData(&version, sizeof(unsigned int), 1, fileHandle);

	if (elementsRead != 1)
	{
		fclose(fileHandle);
		sprintf_s(error, maxlen, "Error reading version number from navigation mesh: %s", navPath);
		return NULL;
	}

	if (version < 6 || version > 16)
	{
		fclose(fileHandle);
		sprintf_s(error, maxlen, "Invalid version number value from navigation mesh: %s [%d]", navPath, version);
		return NULL;
	}

	unsigned int navMeshSubVersion = 0;

	if (version >= 10)
		this->ReadData(&navMeshSubVersion, sizeof(unsigned int), 1, fileHandle);

	unsigned int saveBspSize;
	this->ReadData(&saveBspSize, sizeof(unsigned int), 1, fileHandle);

	char bspPath[MAX_PATH];
	g_pSM->BuildPath(Path_Game, bspPath, sizeof(bspPath), "maps\\%s.bsp", this->mapName);

	unsigned int actualBspSize = 0;

#ifdef PLATFORM_WINDOWS
	struct _stat s;
	_stat(bspPath, &s);
	actualBspSize = s.st_size;
#elif defined PLATFORM_POSIX
	struct stat s;
	stat(bspPath, &s);
	actualBspSize = s.st_size;
#endif

	if (actualBspSize != saveBspSize)
	{
		META_CONPRINTF("WARNING: Navigation mesh was not built with the same version of the map [%d vs %d].\n", actualBspSize, saveBspSize);
	}

	unsigned char meshAnalyzed = 0;

	if (version >= 14)
		this->ReadData(&meshAnalyzed, sizeof(unsigned char), 1, fileHandle);

	bool isMeshAnalyzed = meshAnalyzed != 0;

	//META_CONPRINTF("Is mesh analyzed: %s\n", isMeshAnalyzed ? "yes" : "no");

	unsigned short placeCount;
	this->ReadData(&placeCount, sizeof(unsigned short), 1, fileHandle);

	//META_CONPRINTF("Nav version: %d; BSPSize: %d; MagicNumber: %p; SubVersion: %d [v10+only]; Place Count: %d\n", version, saveBspSize, magicNumber, navMeshSubVersion, placeCount);

	List<INavMeshPlace*> *places = new List<INavMeshPlace*>();

	for (unsigned int placeIndex = 0; placeIndex < placeCount; placeIndex++)
	{
		unsigned short placeSize;

		this->ReadData(&placeSize, sizeof(unsigned short), 1, fileHandle);

		char placeName[256];
		this->ReadData(placeName, sizeof(unsigned char), placeSize, fileHandle);

		places->Append(new CNavMeshPlace(placeIndex, placeName));
		//META_CONPRINTF("Parsed place: %s [%d]\n", placeName, placeIndex);
	}

	unsigned char unnamedAreas = 0;
	if (version > 11)
		this->ReadData(&unnamedAreas, sizeof(unsigned char), 1, fileHandle);

	bool hasUnnamedAreas = unnamedAreas != 0;

	//META_CONPRINTF("Has unnamed areas: %s\n", hasUnnamedAreas ? "yes" : "no");

	IList<INavMeshArea*> *areas = new List<INavMeshArea*>();

	unsigned int areaCount;
	this->ReadData(&areaCount, sizeof(unsigned int), 1, fileHandle);

	//META_CONPRINTF("Area count: %d\n", areaCount);

	for (unsigned int areaIndex = 0; areaIndex < areaCount; areaIndex++)
	{
		unsigned int areaID;
		float x1, y1, z1, x2, y2, z2;
		unsigned int areaFlags = 0;
		IList<INavMeshConnection*> *connections = new List<INavMeshConnection*>();
		IList<INavMeshHidingSpot*> *hidingSpots = new List<INavMeshHidingSpot*>();
		IList<INavMeshEncounterPath*> *encounterPaths = new List<INavMeshEncounterPath*>();
		IList<INavMeshLadderConnection*> *ladderConnections = new List<INavMeshLadderConnection*>();
		IList<INavMeshCornerLightIntensity*> *cornerLightIntensities = new List<INavMeshCornerLightIntensity*>();
		IList<INavMeshVisibleArea*> *visibleAreas = new List<INavMeshVisibleArea*>();
		unsigned int inheritVisibilityFrom = 0;
		unsigned char hidingSpotCount = 0;
		unsigned int visibleAreaCount = 0;
		float earliestOccupyTimeFirstTeam = 0.0f;
		float earliestOccupyTimeSecondTeam = 0.0f;
		float northEastCornerZ;
		float southWestCornerZ;
		unsigned short placeID = 0;
		unsigned char unk01 = 0;

		this->ReadData(&areaID, sizeof(unsigned int), 1, fileHandle);

		//META_CONPRINTF("Area ID: %d\n", areaID);

		if (version <= 8)
			this->ReadData(&areaFlags, sizeof(unsigned char), 1, fileHandle);
		else if (version < 13)
			this->ReadData(&areaFlags, sizeof(unsigned short), 1, fileHandle);
		else
			this->ReadData(&areaFlags, sizeof(unsigned int), 1, fileHandle);

		//META_CONPRINTF("Area Flags: %d\n", areaFlags);
		this->ReadData(&x1, sizeof(float), 1, fileHandle);
		this->ReadData(&y1, sizeof(float), 1, fileHandle);
		this->ReadData(&z1, sizeof(float), 1, fileHandle);
		this->ReadData(&x2, sizeof(float), 1, fileHandle);
		this->ReadData(&y2, sizeof(float), 1, fileHandle);
		this->ReadData(&z2, sizeof(float), 1, fileHandle);

		//META_CONPRINTF("Area extent: (%f, %f, %f), (%f, %f, %f)\n", x1, y1, z1, x2, y2, z2);

		this->ReadData(&northEastCornerZ, sizeof(float), 1, fileHandle);
		this->ReadData(&southWestCornerZ, sizeof(float), 1, fileHandle);

		//META_CONPRINTF("Corners: NW(%f), SW(%f)\n", northEastCornerZ, southWestCornerZ);

		// CheckWaterLevel() are we underwater in this area?

		for (unsigned int direction = 0; direction < NAV_DIR_COUNT; direction++)
		{
			unsigned int connectionCount;
			this->ReadData(&connectionCount, sizeof(unsigned int), 1, fileHandle);

			//META_CONPRINTF("Connection count: %d (%p)\n", connectionCount, connectionCount);

			for (int connectionIndex = 0; connectionIndex < connectionCount; connectionIndex++)
			{
				unsigned int connectingAreaID;
				this->ReadData(&connectingAreaID, sizeof(unsigned int), 1, fileHandle);

				INavMeshConnection *connection = new CNavMeshConnection(connectingAreaID, (eNavDir)direction);
				connections->Append(connection);
			}
		}

		this->ReadData(&hidingSpotCount, sizeof(unsigned char), 1, fileHandle);
		//META_CONPRINTF("Hiding Spot Count: %d\n", hidingSpotCount);

		for (unsigned int hidingSpotIndex = 0; hidingSpotIndex < hidingSpotCount; hidingSpotIndex++)
		{
			unsigned int hidingSpotID;
			this->ReadData(&hidingSpotID, sizeof(unsigned int), 1, fileHandle);

			float hidingSpotX, hidingSpotY, hidingSpotZ;
			this->ReadData(&hidingSpotX, sizeof(float), 1, fileHandle);
			this->ReadData(&hidingSpotY, sizeof(float), 1, fileHandle);
			this->ReadData(&hidingSpotZ, sizeof(float), 1, fileHandle);

			unsigned char hidingSpotFlags;
			this->ReadData(&hidingSpotFlags, sizeof(unsigned char), 1, fileHandle);

			INavMeshHidingSpot *hidingSpot = new CNavMeshHidingSpot(hidingSpotID, hidingSpotX, hidingSpotY, hidingSpotZ, hidingSpotFlags);
			hidingSpots->Append(hidingSpot);
			//META_CONPRINTF("Parsed hiding spot (%f, %f, %f) with ID [%p] and flags [%p]\n", hidingSpotX, hidingSpotY, hidingSpotZ, hidingSpotID, hidingSpotFlags);
		}

		// These are old but we just need to read the data.
		if (version < 15)
		{
			unsigned char approachAreaCount;
			this->ReadData(&approachAreaCount, sizeof(unsigned char), 1, fileHandle);

			for (unsigned int approachAreaIndex = 0; approachAreaIndex < approachAreaCount; approachAreaIndex++)
			{
				unsigned int approachHereID;
				this->ReadData(&approachHereID, sizeof(unsigned int), 1, fileHandle);

				unsigned int approachPrevID;
				this->ReadData(&approachPrevID, sizeof(unsigned int), 1, fileHandle);

				unsigned char approachType;
				this->ReadData(&approachType, sizeof(unsigned char), 1, fileHandle);

				unsigned int approachNextID;
				this->ReadData(&approachNextID, sizeof(unsigned int), 1, fileHandle);

				unsigned char approachHow;
				this->ReadData(&approachHow, sizeof(unsigned char), 1, fileHandle);
			}
		}

		unsigned int encounterPathCount;
		this->ReadData(&encounterPathCount, sizeof(unsigned int), 1, fileHandle);
		//META_CONPRINTF("Encounter Path Count: %d\n", encounterPathCount);

		for (unsigned int encounterPathIndex = 0; encounterPathIndex < encounterPathCount; encounterPathIndex++)
		{
			unsigned int encounterFromID;
			this->ReadData(&encounterFromID, sizeof(unsigned int), 1, fileHandle);

			unsigned char encounterFromDirection;
			this->ReadData(&encounterFromDirection, sizeof(unsigned char), 1, fileHandle);

			unsigned int encounterToID;
			this->ReadData(&encounterToID, sizeof(unsigned int), 1, fileHandle);

			unsigned char encounterToDirection;
			this->ReadData(&encounterToDirection, sizeof(unsigned char), 1, fileHandle);

			unsigned char encounterSpotCount;
			this->ReadData(&encounterSpotCount, sizeof(unsigned char), 1, fileHandle);

			//META_CONPRINTF("Encounter [from ID %d] [from dir %p] [to ID %d] [to dir %p] [spot count %d]\n", encounterFromID, encounterFromDirection, encounterToID, encounterToDirection, encounterSpotCount);
			IList<INavMeshEncounterSpot*> *encounterSpots = new List<INavMeshEncounterSpot*>();

			for (int encounterSpotIndex = 0; encounterSpotIndex < encounterSpotCount; encounterSpotIndex++)
			{
				unsigned int encounterSpotOrderId;
				this->ReadData(&encounterSpotOrderId, sizeof(unsigned int), 1, fileHandle);

				unsigned char encounterSpotT;
				this->ReadData(&encounterSpotT, sizeof(unsigned char), 1, fileHandle);

				float encounterSpotParametricDistance = (float)encounterSpotT / 255.0f;

				INavMeshEncounterSpot *encounterSpot = new CNavMeshEncounterSpot(encounterSpotOrderId, encounterSpotParametricDistance);
				encounterSpots->Append(encounterSpot);
				//META_CONPRINTF("Encounter spot [order id %d] and [T %p]\n", encounterSpotOrderId, encounterSpotT);
			}

			INavMeshEncounterPath *encounterPath = new CNavMeshEncounterPath(encounterFromID, (eNavDir)encounterFromDirection, encounterToID, (eNavDir)encounterToDirection, encounterSpots);
			encounterPaths->Append(encounterPath);
		}

		this->ReadData(&placeID, sizeof(unsigned short), 1, fileHandle);

		//META_CONPRINTF("Place ID: %d\n", placeID);

		for (unsigned int ladderDirection = 0; ladderDirection < NAV_LADDER_DIR_COUNT; ladderDirection++)
		{
			unsigned int ladderConnectionCount;
			this->ReadData(&ladderConnectionCount, sizeof(unsigned int), 1, fileHandle);

			//META_CONPRINTF("Ladder Connection Count: %d\n", ladderConnectionCount);

			for (unsigned int ladderConnectionIndex = 0; ladderConnectionIndex < ladderConnectionCount; ladderConnectionIndex++)
			{
				unsigned int ladderConnectID;
				this->ReadData(&ladderConnectID, sizeof(unsigned int), 1, fileHandle);

				INavMeshLadderConnection *ladderConnection = new CNavMeshLadderConnection(ladderConnectID, (eNavLadder)ladderDirection);
				ladderConnections->Append(ladderConnection);
				//META_CONPRINTF("Parsed ladder connect [ID %d]\n", ladderConnectID);
			}
		}

		this->ReadData(&earliestOccupyTimeFirstTeam, sizeof(float), 1, fileHandle);
		this->ReadData(&earliestOccupyTimeSecondTeam, sizeof(float), 1, fileHandle);

		if (version >= 11)
		{
			for (int navCornerIndex = 0; navCornerIndex < NAV_CORNER_COUNT; navCornerIndex++)
			{
				float navCornerLightIntensity;
				this->ReadData(&navCornerLightIntensity, sizeof(float), 1, fileHandle);

				INavMeshCornerLightIntensity *cornerLightIntensity = new CNavMeshCornerLightIntensity((eNavCorner)navCornerIndex, navCornerLightIntensity);
				cornerLightIntensities->Append(cornerLightIntensity);
				//META_CONPRINTF("Light intensity: [%f] [idx %d]\n", navCornerLightIntensity, navCornerIndex);
			}

			if (version >= 16)
			{
				this->ReadData(&visibleAreaCount, sizeof(unsigned int), 1, fileHandle);

				//META_CONPRINTF("Visible area count: %d\n", visibleAreaCount);

				for (unsigned int visibleAreaIndex = 0; visibleAreaIndex < visibleAreaCount; visibleAreaIndex++)
				{
					unsigned int visibleAreaID;
					this->ReadData(&visibleAreaID, sizeof(unsigned int), 1, fileHandle);

					unsigned char visibleAreaAttributes;
					this->ReadData(&visibleAreaAttributes, sizeof(unsigned char), 1, fileHandle);

					INavMeshVisibleArea *visibleArea = new CNavMeshVisibleArea(visibleAreaID, visibleAreaAttributes);
					visibleAreas->Append(visibleArea);
					//META_CONPRINTF("Parsed visible area [%d] with attr [%p]\n", visibleAreaID, visibleAreaAttributes);
				}

				this->ReadData(&inheritVisibilityFrom, sizeof(unsigned int), 1, fileHandle);

				//META_CONPRINTF("Inherit visibilty from: %d\n", inheritVisibilityFrom);

				this->ReadData(&unk01, sizeof(unsigned char), 1, fileHandle);
			}
		}

		INavMeshArea *area = new CNavMeshArea(areaID, areaFlags, placeID, x1, y1, z1, x2, y2, z2,
			northEastCornerZ, southWestCornerZ, connections, hidingSpots, encounterPaths, ladderConnections,
			cornerLightIntensities, visibleAreas, inheritVisibilityFrom, earliestOccupyTimeFirstTeam, earliestOccupyTimeSecondTeam, unk01);

		areas->Append(area);
	}

	unsigned int ladderCount;
	this->ReadData(&ladderCount, sizeof(unsigned int), 1, fileHandle);

	//META_CONPRINTF("Ladder count: %d\n", ladderCount);
	IList<INavMeshLadder*> *ladders = new List<INavMeshLadder*>();

	for (unsigned int ladderIndex = 0; ladderIndex < ladderCount; ladderIndex++)
	{
		unsigned int ladderID;
		this->ReadData(&ladderID, sizeof(unsigned int), 1, fileHandle);

		float ladderWidth;
		this->ReadData(&ladderWidth, sizeof(float), 1, fileHandle);

		float ladderTopX, ladderTopY, ladderTopZ, ladderBottomX, ladderBottomY, ladderBottomZ;

		this->ReadData(&ladderTopX, sizeof(float), 1, fileHandle);
		this->ReadData(&ladderTopY, sizeof(float), 1, fileHandle);
		this->ReadData(&ladderTopZ, sizeof(float), 1, fileHandle);
		this->ReadData(&ladderBottomX, sizeof(float), 1, fileHandle);
		this->ReadData(&ladderBottomY, sizeof(float), 1, fileHandle);
		this->ReadData(&ladderBottomZ, sizeof(float), 1, fileHandle);

		float ladderLength;
		this->ReadData(&ladderLength, sizeof(float), 1, fileHandle);

		unsigned int ladderDirection;
		this->ReadData(&ladderDirection, sizeof(unsigned int), 1, fileHandle);

		unsigned int ladderTopForwardAreaID;
		this->ReadData(&ladderTopForwardAreaID, sizeof(unsigned int), 1, fileHandle);

		unsigned int ladderTopLeftAreaID;
		this->ReadData(&ladderTopLeftAreaID, sizeof(unsigned int), 1, fileHandle);

		unsigned int ladderTopRightAreaID;
		this->ReadData(&ladderTopRightAreaID, sizeof(unsigned int), 1, fileHandle);

		unsigned int ladderTopBehindAreaID;
		this->ReadData(&ladderTopBehindAreaID, sizeof(unsigned int), 1, fileHandle);

		unsigned int ladderBottomAreaID;
		this->ReadData(&ladderBottomAreaID, sizeof(unsigned int), 1, fileHandle);

		INavMeshLadder *ladder = new CNavMeshLadder(ladderID, ladderWidth, ladderLength, ladderTopX, ladderTopY, ladderTopZ,
			ladderBottomX, ladderBottomY, ladderBottomZ, (eNavDir)ladderDirection,
			ladderTopForwardAreaID, ladderTopLeftAreaID, ladderTopRightAreaID, ladderTopBehindAreaID, ladderBottomAreaID);

		ladders->Append(ladder);
	}

	fclose(fileHandle);
	INavMesh *mesh = new CNavMesh(magicNumber, version, navMeshSubVersion, saveBspSize, isMeshAnalyzed, places, areas, ladders);

	return mesh;
}

unsigned int CNavMeshLoader::ReadData(void *output, unsigned int elementSize, unsigned int elementCount, FILE *fileHandle)
{
	unsigned int count = fread(output, elementSize, elementCount, fileHandle);

	unsigned int byteCount = elementCount * elementSize;

	this->bytesRead += byteCount;

	return count;
}
