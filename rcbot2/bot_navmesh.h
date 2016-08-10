//	NavMesh parser and functions written by -W3E- El Diablo

#ifndef __RCBOT_NAVMESH_H__
#define __RCBOT_NAVMESH_H__

#include "bot_navigator.h"

enum eNavCorner
{
	NAV_CORNER_NORTH_WEST = 0,
	NAV_CORNER_NORTH_EAST = 1,
	NAV_CORNER_SOUTH_EAST = 2,
	NAV_CORNER_SOUTH_WEST = 3,

	NAV_CORNER_COUNT
};

enum eNavDir
{
	NAV_DIR_NORTH = 0,
	NAV_DIR_EAST = 1,
	NAV_DIR_SOUTH = 2,
	NAV_DIR_WEST = 3,

	NAV_DIR_COUNT
};

enum eNavLadder
{
	NAV_LADDER_DIR_UP = 0,
	NAV_LADDER_DIR_DOWN = 1,

	NAV_LADDER_DIR_COUNT
};

class CNavMeshNavigator : public IBotNavigator
{
public:
	bool Init(char *error, size_t maxlen);

	void FreeMapMemory();
	void FreeAllMemory();

private:
	INavMesh *m_pNavMesh;
};

template <class T>
class IList
{
public:
	virtual bool Insert(T item, unsigned int index) = 0;
	virtual void Append(T item) = 0;
	virtual void Prepend(T item) = 0;
	virtual T At(unsigned int index) = 0;
	virtual size_t Size() = 0;
	virtual unsigned int Find(T item) = 0;
};

class INavMesh
{
public:
	virtual unsigned int GetMagicNumber() = 0;
	virtual unsigned int GetVersion() = 0;
	virtual unsigned int GetSubVersion() = 0;
	virtual unsigned int GetSaveBSPSize() = 0;
	virtual bool IsMeshAnalyzed() = 0;
	virtual IList<INavMeshPlace*> *GetPlaces() = 0;
	virtual IList<INavMeshArea*> *GetAreas() = 0;
	virtual IList<INavMeshLadder*> *GetLadders() = 0;
};

class INavMeshArea
{
public:
	virtual unsigned int GetID() = 0;
	virtual unsigned int GetFlags() = 0;
	virtual unsigned int GetPlaceID() = 0;

	virtual float GetNWExtentX() = 0;
	virtual float GetNWExtentY() = 0;
	virtual float GetNWExtentZ() = 0;

	virtual float GetSEExtentX() = 0;
	virtual float GetSEExtentY() = 0;
	virtual float GetSEExtentZ() = 0;

	virtual float GetEarliestOccupyTimeFirstTeam() = 0;
	virtual float GetEarliestOccupyTimeSecondTeam() = 0;

	virtual float GetNECornerZ() = 0;
	virtual float GetSWCornerZ() = 0;

	virtual IList<INavMeshConnection*> *GetConnections() = 0;
	virtual IList<INavMeshHidingSpot*> *GetHidingSpots() = 0;
	virtual IList<INavMeshEncounterPath*> *GetEncounterPaths() = 0;
	virtual IList<INavMeshLadderConnection*> *GetLadderConnections() = 0;
	virtual IList<INavMeshCornerLightIntensity*> *GetCornerLightIntensities() = 0;
	virtual IList<INavMeshVisibleArea*> *GetVisibleAreas() = 0;

	virtual unsigned int GetInheritVisibilityFromAreaID() = 0;

	virtual unsigned char GetUnk01() = 0;
};

class INavMeshConnection
{
public:
	virtual unsigned int GetConnectingAreaID() = 0;
	virtual eNavDir GetDirection() = 0;
};

class INavMeshCornerLightIntensity
{
public:
	virtual eNavCorner GetCornerType() = 0;
	virtual float GetLightIntensity() = 0;
};

class INavMeshEncounterPath
{
public:
	virtual unsigned int GetFromAreaID() = 0;
	virtual eNavDir GetFromDirection() = 0;
	virtual unsigned int GetToAreaID() = 0;
	virtual eNavDir GetToDirection() = 0;
	virtual IList<INavMeshEncounterSpot*> *GetEncounterSpots() = 0;
};

class INavMeshEncounterSpot
{
public:
	virtual unsigned int GetOrderID() = 0;
	virtual float GetParametricDistance() = 0;
};

class INavMeshHidingSpot
{
public:
	virtual unsigned int GetID() = 0;

	virtual float GetX() = 0;
	virtual float GetY() = 0;
	virtual float GetZ() = 0;

	virtual unsigned char GetFlags() = 0;
};

class INavMeshLadder
{
public:
	virtual unsigned int GetID() = 0;
	virtual float GetWidth() = 0;
	virtual float GetLength() = 0;

	virtual float GetTopX() = 0;
	virtual float GetTopY() = 0;
	virtual float GetTopZ() = 0;

	virtual float GetBottomX() = 0;
	virtual float GetBottomY() = 0;
	virtual float GetBottomZ() = 0;

	virtual eNavDir GetDirection() = 0;

	virtual unsigned int GetTopForwardAreaID() = 0;
	virtual unsigned int GetTopLeftAreaID() = 0;
	virtual unsigned int GetTopRightAreaID() = 0;
	virtual unsigned int GetTopBehindAreaID() = 0;

	virtual unsigned int GetBottomAreaID() = 0;
};

class INavMeshLadderConnection
{
public:
	virtual unsigned int GetConnectingLadderID() = 0;
	virtual eNavLadder GetDirection() = 0;
};

class INavMeshPlace
{
public:
	virtual const char *GetName() = 0;
	virtual unsigned int GetID() = 0;
};

class INavMeshVisibleArea
{
public:
	virtual unsigned int GetVisibleAreaID() = 0;
	virtual unsigned char GetAttributes() = 0;
};

class INavMeshLoader
{
public:
	virtual INavMesh *Load(char *error, size_t errorMaxlen) = 0;
};

template <class T>
class List : public IList<T>
{
public:
	List() { this->items = new SourceHook::CVector<T>(); }

	~List() { delete this->items; }

	bool Insert(T item, unsigned int index)
	{
		size_t size = this->items->size();

		if (index < 0 || index > size)
			return false;

		this->items->insert(this->items->iterAt(index), item);
		return true;
	}

	void Append(T item) { this->items->insert(this->items->end(), item); }

	void Prepend(T item) { this->items->insert(this->items->begin(), item); }

	T At(unsigned int index) { return this->items->at(index); }

	size_t Size() { return this->items->size(); }

	unsigned int Find(T item)
	{
		size_t size = this->items->size();

		for (unsigned int i = 0; i < size; i++)
		{
			if (this->items->at(i) != item)
				continue;

			return i;
		}

		return -1;
	}

private:
	SourceHook::CVector<T> *items;
};

class CNavMesh : public INavMesh
{
public:
	CNavMesh(unsigned int magicNumber, unsigned int version, unsigned int subVersion, unsigned int saveBSPSize, bool isMeshAnalyzed,
		IList<INavMeshPlace*> *places, IList<INavMeshArea*> *areas, IList<INavMeshLadder*> *ladders);

	~CNavMesh();

	unsigned int GetMagicNumber();
	unsigned int GetVersion();
	unsigned int GetSubVersion();
	unsigned int GetSaveBSPSize();

	bool IsMeshAnalyzed();

	IList<INavMeshPlace*> *GetPlaces();
	IList<INavMeshArea*> *GetAreas();
	IList<INavMeshLadder*> *GetLadders();

private:
	unsigned int magicNumber;
	unsigned int version;
	unsigned int subVersion;
	unsigned int saveBSPSize;
	bool isMeshAnalyzed;
	IList<INavMeshPlace*> *places;
	IList<INavMeshArea*> *areas;
	IList<INavMeshLadder*> *ladders;
};

class CNavMeshArea : public INavMeshArea
{
public:
	CNavMeshArea(unsigned int id, unsigned int flags, unsigned int placeID,
		float nwExtentX, float nwExtentY, float nwExtentZ,
		float seExtentX, float seExtentY, float seExtentZ,
		float neCornerZ, float swCornerZ,
		IList<INavMeshConnection*> *connections, IList<INavMeshHidingSpot*> *hidingSpots, IList<INavMeshEncounterPath*> *encounterPaths,
		IList<INavMeshLadderConnection*> *ladderConnections, IList<INavMeshCornerLightIntensity*> *cornerLightIntensities,
		IList<INavMeshVisibleArea*> *visibleAreas, unsigned int inheritVisibilityFromAreaID,
		float earliestOccupyTimeFirstTeam, float earliestOccupyTimeSecondTeam, unsigned char unk01);

	~CNavMeshArea();

	unsigned int GetID();
	unsigned int GetFlags();
	unsigned int GetPlaceID();

	float GetNWExtentX();
	float GetNWExtentY();
	float GetNWExtentZ();

	float GetSEExtentX();
	float GetSEExtentY();
	float GetSEExtentZ();

	float GetEarliestOccupyTimeFirstTeam();
	float GetEarliestOccupyTimeSecondTeam();

	float GetNECornerZ();
	float GetSWCornerZ();

	IList<INavMeshConnection*> *GetConnections();
	IList<INavMeshHidingSpot*> *GetHidingSpots();
	IList<INavMeshEncounterPath*> *GetEncounterPaths();
	IList<INavMeshLadderConnection*> *GetLadderConnections();
	IList<INavMeshCornerLightIntensity*> *GetCornerLightIntensities();
	IList<INavMeshVisibleArea*> *GetVisibleAreas();

	unsigned int GetInheritVisibilityFromAreaID();

	unsigned char GetUnk01();

private:
	unsigned int id;
	unsigned int flags;
	unsigned int placeID;

	float nwExtentX;
	float nwExtentY;
	float nwExtentZ;

	float seExtentX;
	float seExtentY;
	float seExtentZ;

	float neCornerZ;
	float swCornerZ;

	IList<INavMeshConnection*> *connections;
	IList<INavMeshHidingSpot*> *hidingSpots;
	IList<INavMeshEncounterPath*> *encounterPaths;
	IList<INavMeshLadderConnection*> *ladderConnections;
	IList<INavMeshCornerLightIntensity*> *cornerLightIntensities;
	IList<INavMeshVisibleArea*> *visibleAreas;

	float earliestOccupyTimeFirstTeam;
	float earliestOccupyTimeSecondTeam;

	unsigned int inheritVisibilityFromAreaID;

	unsigned char unk01;
};

class CNavMeshConnection : public INavMeshConnection
{
public:
	CNavMeshConnection(unsigned int connectingAreaID, eNavDir direction);
	~CNavMeshConnection();

	unsigned int GetConnectingAreaID();

	eNavDir GetDirection();

private:
	unsigned int connectingAreaID;
	eNavDir direction;
};

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

class CNavMeshEncounterPath : public INavMeshEncounterPath
{
public:
	CNavMeshEncounterPath(unsigned int fromAreaID, eNavDir fromDirection, unsigned int toAreaID, eNavDir toDirection, IList<INavMeshEncounterSpot*> *encounterSpots);
	~CNavMeshEncounterPath();

	unsigned int GetFromAreaID();
	eNavDir GetFromDirection();

	unsigned int GetToAreaID();
	eNavDir GetToDirection();

	IList<INavMeshEncounterSpot*> *GetEncounterSpots();

private:
	unsigned int fromAreaID;
	eNavDir fromDirection;
	unsigned int toAreaID;
	eNavDir toDirection;
	IList<INavMeshEncounterSpot*> *encounterSpots;
};

class CNavMeshEncounterSpot : public INavMeshEncounterSpot
{
public:
	CNavMeshEncounterSpot(unsigned int orderID, float parametricDistance);
	~CNavMeshEncounterSpot();

	unsigned int GetOrderID();

	float GetParametricDistance();

private:
	unsigned int orderID;
	float parametricDistance;
};

class CNavMeshHidingSpot : public INavMeshHidingSpot
{
public:
	CNavMeshHidingSpot(unsigned int id, float x, float y, float z, unsigned char flags);
	~CNavMeshHidingSpot();

	unsigned int GetID();

	float GetX();
	float GetY();
	float GetZ();

	unsigned char GetFlags();

private:
	unsigned int id;
	float x;
	float y;
	float z;
	unsigned char flags;
};

class CNavMeshLadder : public INavMeshLadder
{
public:
	CNavMeshLadder(unsigned int id, float width, float length, float topX, float topY, float topZ,
		float bottomX, float bottomY, float bottomZ, eNavDir direction,
		unsigned int topForwardAreaID, unsigned int topLeftAreaID, unsigned int topRightAreaID, unsigned int topBehindAreaID, unsigned int bottomAreaID);

	~CNavMeshLadder();

	unsigned int GetID();

	float GetWidth();
	float GetLength();

	float GetTopX();
	float GetTopY();
	float GetTopZ();

	float GetBottomX();
	float GetBottomY();
	float GetBottomZ();

	eNavDir GetDirection();

	unsigned int GetTopForwardAreaID();
	unsigned int GetTopLeftAreaID();
	unsigned int GetTopRightAreaID();
	unsigned int GetTopBehindAreaID();

	unsigned int GetBottomAreaID();

private:
	unsigned int id;
	float width;
	float length;
	float topX;
	float topY;
	float topZ;
	float bottomX;
	float bottomY;
	float bottomZ;
	eNavDir direction;
	unsigned int topForwardAreaID;
	unsigned int topLeftAreaID;
	unsigned int topRightAreaID;
	unsigned int topBehindAreaID;
	unsigned int bottomAreaID;
};

class CNavMeshLadderConnection : public INavMeshLadderConnection
{
public:
	CNavMeshLadderConnection(unsigned int connectingLadderID, eNavLadder direction);
	~CNavMeshLadderConnection();

	unsigned int GetConnectingLadderID();

	eNavLadder GetDirection();

private:
	unsigned int connectingLadderID;
	eNavLadder direction;
};

class CNavMeshPlace : public INavMeshPlace
{
public:
	CNavMeshPlace(unsigned int id, const char *name);
	~CNavMeshPlace();

	const char *GetName();

	unsigned int GetID();

private:
	unsigned int id;
	char name[256];
};

class CNavMeshVisibleArea : public INavMeshVisibleArea
{
public:
	CNavMeshVisibleArea(unsigned int visibleAreaID, unsigned char attributes);
	~CNavMeshVisibleArea();

	unsigned int GetVisibleAreaID();

	unsigned char GetAttributes();

private:
	unsigned int visibleAreaID;
	unsigned char attributes;
};

class CNavMeshLoader : public INavMeshLoader
{
public:
	CNavMeshLoader(const char *mapName);
	~CNavMeshLoader();

	INavMesh *Load(char *error, size_t errorMaxlen);

private:
	unsigned int ReadData(void *output, unsigned int elementSize, unsigned int elementCount, FILE *fileHandle);

private:
	char mapName[100];
	unsigned int bytesRead;
};

#endif
