#include "engine_wrappers.h"

#include "bot_wpt_dist.h"
#include "bot_globals.h"
#include "bot_waypoint.h"

typedef struct
{
	int version;
	int numwaypoints;
	int maxwaypoints;
}wpt_dist_hdr_t;

int CWaypointDistances::m_Distances[CWaypoints::MAX_WAYPOINTS][CWaypoints::MAX_WAYPOINTS];
float CWaypointDistances::m_fSaveTime = 0;

void CWaypointDistances::Load()
{
	char filename[1024];
	wpt_dist_hdr_t hdr;
	char *szMapName = CBotGlobals::GetMapName();

	if (szMapName  && *szMapName)
	{
		smutils->BuildPath(Path_SM, filename, sizeof(filename), "data\\afkbot\\waypoints\\%s.%s", szMapName, BOT_WAYPOINT_DISTANCE_EXTENSION);

		FILE *bfp = CBotGlobals::OpenFile(filename, "rb");

		if (bfp == NULL)
		{
			return; // give up
		}

		fread(&hdr, sizeof(wpt_dist_hdr_t), 1, bfp);

		if ((hdr.maxwaypoints == CWaypoints::MAX_WAYPOINTS) && (hdr.numwaypoints == CWaypoints::NumWaypoints()) && (hdr.version == WPT_DIST_VER))
		{
			fread(m_Distances, sizeof(int), CWaypoints::MAX_WAYPOINTS * CWaypoints::MAX_WAYPOINTS, bfp);
		}

		m_fSaveTime = engine->Time() + 100.0f;

		fclose(bfp);
	}
}

void CWaypointDistances::Save()
{
	//if ( m_fSaveTime < engine->Time() )
	//{
	char filename[1024];
	char *szMapName = CBotGlobals::GetMapName();

	if (szMapName && *szMapName)
	{
		wpt_dist_hdr_t hdr;

		smutils->BuildPath(Path_SM, filename, sizeof(filename), "data\\afkbot\\%s\\%s.%s", BOT_WAYPOINT_FOLDER, szMapName, BOT_WAYPOINT_DISTANCE_EXTENSION);

		FILE *bfp = CBotGlobals::OpenFile(filename, "wb");

		if (bfp == NULL)
		{
			m_fSaveTime = engine->Time() + 100.0f;
			return; // give up
		}

		hdr.maxwaypoints = CWaypoints::MAX_WAYPOINTS;
		hdr.numwaypoints = CWaypoints::NumWaypoints();
		hdr.version = WPT_DIST_VER;

		fwrite(&hdr, sizeof(wpt_dist_hdr_t), 1, bfp);

		fwrite(m_Distances, sizeof(int), CWaypoints::MAX_WAYPOINTS * CWaypoints::MAX_WAYPOINTS, bfp);

		m_fSaveTime = engine->Time() + 100.0f;

		fclose(bfp);
	}
	//}
}

float CWaypointDistances::GetDistance(int iFrom, int iTo)
{
	if (m_Distances[iFrom][iTo] == -1)
		return (CWaypoints::GetWaypoint(iFrom)->GetOrigin() - CWaypoints::GetWaypoint(iTo)->GetOrigin()).Length();

	return (float)m_Distances[iFrom][iTo];
}