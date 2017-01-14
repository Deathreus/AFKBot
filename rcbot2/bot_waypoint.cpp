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

#include "engine_wrappers.h"

#include "bot_base.h"

#include "in_buttons.h"

#include "bot_globals.h"
#include "bot_navigator.h"
#include "bot_waypoint.h"
#include "bot_waypoint_locations.h"
#include "bot_waypoint_visibility.h"
#include "bot_profile.h"
#include "bot_schedule.h"
#include "bot_getprop.h"
#include "bot_fortress.h"
#include "bot_wpt_dist.h"


#include <vector>    //bir3yk
using namespace std;    //bir3yk

int CWaypoints::m_iNumWaypoints = 0;
CWaypoint CWaypoints::m_theWaypoints[CWaypoints::MAX_WAYPOINTS];
float CWaypoints::m_fNextDrawWaypoints = 0;
int CWaypoints::m_iWaypointTexture = 0;
CWaypointVisibilityTable * CWaypoints::m_pVisibilityTable = NULL;
vector<CWaypointType*> CWaypointTypes::m_Types;
char CWaypoints::m_szAuthor[32];
char CWaypoints::m_szModifiedBy[32];
char CWaypoints::m_szWelcomeMessage[128];

extern ConVar bot_belief_fade;

extern IPlayerInfoManager *playerinfomanager;
extern IVDebugOverlay* debugoverlay;

///////////////////////////////////////////////////////////////
// initialise
void CWaypointNavigator::Init()
{
	m_pBot = NULL;

	m_vOffset = Vector(0, 0, 0);
	m_bOffsetApplied = false;

	m_iCurrentWaypoint = -1;
	m_iNextWaypoint = -1;
	m_iGoalWaypoint = -1;

	m_currentRoute.Destroy();
	while (!m_oldRoute.empty())
		m_oldRoute.pop();

	m_iLastFailedWpt = -1;
	m_iPrevWaypoint = -1;
	m_bWorkingRoute = false;

	Q_memset(m_fBelief, 0, sizeof(float)*CWaypoints::MAX_WAYPOINTS);

	m_iFailedGoals.Destroy();//.clear();//Destroy();
}

bool CWaypointNavigator::BeliefLoad()
{
	int iSize;
	int iDesiredSize;
	register unsigned short int i;
	register unsigned short int num;
	unsigned short int filebelief[CWaypoints::MAX_WAYPOINTS];

	char filename[1024];

	m_bLoadBelief = false;
	m_iBeliefTeam = m_pBot->GetTeam();

	smutils->BuildPath(Path_SM, filename, sizeof(filename), "data\\afkbot\\%s\\%s%d.%s", BOT_WAYPOINT_FOLDER, CBotGlobals::GetMapName(), m_iBeliefTeam, BOT_WAYPOINT_BELIEF_EXTENTION);

	FILE *bfp = CBotGlobals::OpenFile(filename, "rb");

	if (bfp == NULL)
	{
		Msg(" *** Can't open Waypoint belief array for reading!\n");
		return false;
	}

	fseek(bfp, 0, SEEK_END); // seek at end

	iSize = ftell(bfp); // Get file size
	iDesiredSize = CWaypoints::NumWaypoints()*sizeof(unsigned short int);

	// size not right, return false to re workout table
	if (iSize != iDesiredSize)
	{
		fclose(bfp);
		return false;
	}

	fseek(bfp, 0, SEEK_SET); // seek at start

	memset(filebelief, 0, sizeof(unsigned short int)*CWaypoints::MAX_WAYPOINTS);

	fread(filebelief, sizeof(unsigned short int), CWaypoints::NumWaypoints(), bfp);

	// convert from short int to float

	num = (unsigned short int)CWaypoints::NumWaypoints();

	// quick loop
	for (i = 0; i < num; i++)
	{
		m_fBelief[i] = (((float)filebelief[i]) / 32767) * MAX_BELIEF;
	}

	fclose(bfp);

	return true;
}
// update belief array with averaged belief for this team
bool CWaypointNavigator::BeliefSave(bool bOverride)
{
	int iSize;
	int iDesiredSize;
	register unsigned short int i;
	register unsigned short int num;
	unsigned short int filebelief[CWaypoints::MAX_WAYPOINTS];
	char filename[1024];

	if ((m_pBot->GetTeam() == m_iBeliefTeam) && !bOverride)
		return false;

	memset(filebelief, 0, sizeof(unsigned short int)*CWaypoints::MAX_WAYPOINTS);

	// m_iBeliefTeam is the team we've been using -- we might have changed team now
	// so would need to change files if a different team
	// stick to the current team we've been using
	smutils->BuildPath(Path_SM, filename, sizeof(filename), "data\\afkbot\\%s\\%s%d.%s", BOT_WAYPOINT_FOLDER, CBotGlobals::GetMapName(), m_iBeliefTeam, BOT_WAYPOINT_BELIEF_EXTENTION);

	FILE *bfp = CBotGlobals::OpenFile(filename, "rb");

	if (bfp != NULL)
	{
		fseek(bfp, 0, SEEK_END); // seek at end

		iSize = ftell(bfp); // Get file size
		iDesiredSize = CWaypoints::NumWaypoints()*sizeof(unsigned short int);

		// size not right, return false to re workout table
		if (iSize != iDesiredSize)
		{
			fclose(bfp);
		}
		else
		{
			fseek(bfp, 0, SEEK_SET); // seek at start

			if (bfp)
				fread(filebelief, sizeof(unsigned short int), CWaypoints::NumWaypoints(), bfp);

			fclose(bfp);
		}
	}

	bfp = CBotGlobals::OpenFile(filename, "wb");

	if (bfp == NULL)
	{
		m_bLoadBelief = true;
		m_iBeliefTeam = m_pBot->GetTeam();
		Msg(" *** Can't open Waypoint Belief array for writing!\n");
		return false;
	}

	// convert from short int to float

	num = (unsigned short int)CWaypoints::NumWaypoints();

	// quick loop
	for (i = 0; i < num; i++)
	{
		filebelief[i] = (filebelief[i] / 2) + ((unsigned short int)((m_fBelief[i] / MAX_BELIEF) * 16383));
	}

	fseek(bfp, 0, SEEK_SET); // seek at start

	fwrite(filebelief, sizeof(unsigned short int), num, bfp);

	fclose(bfp);

	// new team -- load belief 
	m_iBeliefTeam = m_pBot->GetTeam();
	m_bLoadBelief = true;
	m_bBeliefChanged = false; // saved

	return true;
}

bool CWaypointNavigator::WantToSaveBelief()
{
	// playing on this map for more than a normal load time
	return (m_bBeliefChanged && (m_iBeliefTeam != m_pBot->GetTeam()));
}

int CWaypointNavigator::numPaths()
{
	if (m_iCurrentWaypoint != -1)
		return CWaypoints::GetWaypoint(m_iCurrentWaypoint)->NumPaths();

	return 0;
}

bool CWaypointNavigator::RandomDangerPath(Vector *vec)
{
	float fMaxDanger = 0;
	float fTotal;
	float fBelief;
	float fRand;
	short int i;
	CWaypoint *pWpt;
	CWaypoint *pNext;
	CWaypoint *pOnRouteTo = NULL;

	if (m_iCurrentWaypoint == -1)
		return false;

	if (!m_currentRoute.IsEmpty())
	{
		static int *head;
		static CWaypoint *pW;

		head = m_currentRoute.GetHeadInfoPointer();

		if (head && (*head != -1))
		{
			pOnRouteTo = CWaypoints::GetWaypoint(*head);
		}
	}

	pWpt = CWaypoints::GetWaypoint(m_iCurrentWaypoint);

	if (pWpt == NULL)
		return false;

	fTotal = 0;

	for (i = 0; i < pWpt->NumPaths(); i++)
	{
		pNext = CWaypoints::GetWaypoint(pWpt->GetPath(i));
		fBelief = GetBelief(CWaypoints::GetWaypointIndex(pNext));

		if (pNext == pOnRouteTo)
			fBelief *= pWpt->NumPaths();

		if (fBelief > fMaxDanger)
			fMaxDanger = fBelief;

		fTotal += fBelief;
	}

	if (fMaxDanger < 10)
		return false; // not useful enough

	fRand = RandomFloat(0, fTotal);

	for (i = 0; i < pWpt->NumPaths(); i++)
	{
		pNext = CWaypoints::GetWaypoint(pWpt->GetPath(i));
		fBelief = GetBelief(CWaypoints::GetWaypointIndex(pNext));

		if (pNext == pOnRouteTo)
			fBelief *= pWpt->NumPaths();

		fTotal += fBelief;

		if (fRand < fTotal)
		{
			*vec = pNext->GetOrigin();
			return true;
		}
	}

	return false;

}

Vector CWaypointNavigator::GetPath(int pathid)
{
	return CWaypoints::GetWaypoint(CWaypoints::GetWaypoint(m_iCurrentWaypoint)->GetPath(pathid))->GetOrigin();
}


int CWaypointNavigator::GetPathFlags(int iPath)
{
	CWaypoint *pWpt = CWaypoints::GetWaypoint(m_iCurrentWaypoint);

	return CWaypoints::GetWaypoint(pWpt->GetPath(iPath))->GetFlags();
}

bool CWaypointNavigator::NextPointIsOnLadder()
{
	if (m_iCurrentWaypoint != -1)
	{
		CWaypoint *pWaypoint;

		if ((pWaypoint = CWaypoints::GetWaypoint(m_iCurrentWaypoint)) != NULL)
		{
			return pWaypoint->HasFlag(CWaypointTypes::W_FL_LADDER);
		}
	}

	return false;
}

float CWaypointNavigator::GetNextYaw()
{
	if (m_iCurrentWaypoint != -1)
		return CWaypoints::GetWaypoint(m_iCurrentWaypoint)->GetAimYaw();

	return false;
}

// best waypoints are those with lowest danger
CWaypoint *CWaypointNavigator::ChooseBestFromBeliefBetweenAreas(dataUnconstArray<AStarNode*> *goals, bool bHighDanger, bool bIgnoreBelief)
{
	int i;
	CWaypoint *pWpt = NULL;
	//	CWaypoint *pCheck;

	float fBelief = 0;
	float fSelect;

	// simple checks
	switch (goals->Size())
	{
	case 0:return NULL;
	case 1:return CWaypoints::GetWaypoint(goals->ReturnValueFromIndex(0)->GetWaypoint());
	default:
	{
		AStarNode *node;

		for (i = 0; i < goals->Size(); i++)
		{
			node = goals->ReturnValueFromIndex(i);

			if (bIgnoreBelief)
			{
				if (bHighDanger)
					fBelief += node->GetHeuristic();
				else
					fBelief += (131072.0f - node->GetHeuristic());
			}
			else if (bHighDanger)
				fBelief += m_fBelief[node->GetWaypoint()] + node->GetHeuristic();
			else
				fBelief += MAX_BELIEF - m_fBelief[node->GetWaypoint()] + (131072.0f - node->GetHeuristic());
		}

		fSelect = RandomFloat(0, fBelief);

		fBelief = 0;

		for (i = 0; i < goals->Size(); i++)
		{
			node = goals->ReturnValueFromIndex(i);

			if (bIgnoreBelief)
			{
				if (bHighDanger)
					fBelief += node->GetHeuristic();
				else
					fBelief += (131072.0f - node->GetHeuristic());
			}
			else if (bHighDanger)
				fBelief += m_fBelief[node->GetWaypoint()] + node->GetHeuristic();
			else
				fBelief += MAX_BELIEF - m_fBelief[node->GetWaypoint()] + (131072.0f - node->GetHeuristic());

			if (fSelect <= fBelief)
			{
				pWpt = CWaypoints::GetWaypoint(node->GetWaypoint());
				break;
			}
		}

		if (pWpt == NULL)
			pWpt = CWaypoints::GetWaypoint(goals->Random()->GetWaypoint());
	}
	}

	return pWpt;
}

// best waypoints are those with lowest danger
CWaypoint *CWaypointNavigator::ChooseBestFromBelief(dataUnconstArray<CWaypoint*> *goals, bool bHighDanger, int iSearchFlags, int iTeam)
{
	int i;
	CWaypoint *pWpt = NULL;
	CWaypoint *pCheck;

	float fBelief = 0;
	float fSelect;
	float bBeliefFactor = 1.0f;

	// simple checks
	switch (goals->Size())
	{
	case 0:return NULL;
	case 1:return goals->ReturnValueFromIndex(0);
	default:
	{
		for (i = 0; i < goals->Size(); i++)
		{
			bBeliefFactor = 1.0f;

			if (iSearchFlags & WPT_SEARCH_AVOID_SENTRIES)
			{
				for (int j = gpGlobals->maxClients; j > 0; j--)
				{
					edict_t *pSentry = CTeamFortress2Mod::GetSentryGun(j - 1);

					if (pSentry != NULL)
					{
						if (goals->ReturnValueFromIndex(i)->DistanceFrom(CBotGlobals::EntityOrigin(pSentry)) < 200.0f)
						{
							bBeliefFactor *= 0.1f;
						}
					}
				}
			}

			if (iSearchFlags & WPT_SEARCH_AVOID_SNIPERS)
			{
				for (int j = gpGlobals->maxClients; j > 0; j--)
				{
					edict_t *pPlayer = INDEXENT(j);

					if ((pPlayer != NULL) && !pPlayer->IsFree() && (CClassInterface::GetTF2Class(pPlayer) == TF_CLASS_SNIPER))
					{
						if ((iTeam == 0) || (iTeam == CClassInterface::GetTeam(pPlayer)))
						{
							if (goals->ReturnValueFromIndex(i)->DistanceFrom(CBotGlobals::EntityOrigin(pPlayer)) < 200.0f)
							{
								bBeliefFactor *= 0.1f;
							}
						}
					}
				}
			}

			if (iSearchFlags & WPT_SEARCH_AVOID_TEAMMATE)
			{
				for (int j = gpGlobals->maxClients; j > 0; j--)
				{
					edict_t *pPlayer = INDEXENT(j);

					if ((pPlayer != NULL) && !pPlayer->IsFree())
					{
						if ((iTeam == 0) || (iTeam == CClassInterface::GetTeam(pPlayer)))
						{
							if (goals->ReturnValueFromIndex(i)->DistanceFrom(CBotGlobals::EntityOrigin(pPlayer)) < 200.0f)
							{
								bBeliefFactor *= 0.1f;
							}
						}
					}
				}
			}

			if (bHighDanger)
			{
				fBelief += bBeliefFactor * (1.0f + (m_fBelief[CWaypoints::GetWaypointIndex((*goals)[i])]));
			}
			else
			{
				fBelief += bBeliefFactor * (1.0f + (MAX_BELIEF - (m_fBelief[CWaypoints::GetWaypointIndex((*goals)[i])])));
			}
		}

		fSelect = RandomFloat(0, fBelief);

		fBelief = 0;

		for (i = 0; i < goals->Size(); i++)
		{
			pCheck = goals->ReturnValueFromIndex(i);

			bBeliefFactor = 1.0f;

			if (iSearchFlags & WPT_SEARCH_AVOID_SENTRIES)
			{
				for (int j = gpGlobals->maxClients; j > 0; j--)
				{
					edict_t *pSentry = CTeamFortress2Mod::GetSentryGun(j);

					if (pSentry != NULL)
					{
						if (goals->ReturnValueFromIndex(i)->DistanceFrom(CBotGlobals::EntityOrigin(pSentry)) < 200.0f)
						{
							bBeliefFactor *= 0.1f;
						}
					}
				}
			}

			if (iSearchFlags & WPT_SEARCH_AVOID_SNIPERS)
			{
				for (int j = gpGlobals->maxClients; j > 0; j--)
				{
					edict_t *pPlayer = INDEXENT(j);

					if ((pPlayer != NULL) && !pPlayer->IsFree() && (CClassInterface::GetTF2Class(pPlayer) == TF_CLASS_SNIPER))
					{
						if (goals->ReturnValueFromIndex(i)->DistanceFrom(CBotGlobals::EntityOrigin(pPlayer)) < 200.0f)
						{
							bBeliefFactor *= 0.1f;
						}
					}
				}
			}

			if (iSearchFlags & WPT_SEARCH_AVOID_TEAMMATE)
			{
				for (int j = gpGlobals->maxClients; j > 0; j--)
				{
					edict_t *pPlayer = INDEXENT(i);

					if ((pPlayer != NULL) && !pPlayer->IsFree())
					{
						if (goals->ReturnValueFromIndex(i)->DistanceFrom(CBotGlobals::EntityOrigin(pPlayer)) < 200.0f)
						{
							bBeliefFactor *= 0.1f;
						}
					}
				}
			}

			if (bHighDanger)
			{
				fBelief += bBeliefFactor * (1.0f + (m_fBelief[CWaypoints::GetWaypointIndex((*goals)[i])]));
			}
			else
			{
				fBelief += bBeliefFactor * (1.0f + (MAX_BELIEF - (m_fBelief[CWaypoints::GetWaypointIndex((*goals)[i])])));
			}

			if (fSelect <= fBelief)
			{
				pWpt = pCheck;
				break;
			}
		}

		if (pWpt == NULL)
			pWpt = goals->Random();
	}
	}

	return pWpt;
}

// Get the covering waypoint vector vCover
bool CWaypointNavigator::GetCoverPosition(Vector vCoverOrigin, Vector *vCover)
{
	int iWpt;

	iWpt = CWaypointLocations::GetCoverWaypoint(m_pBot->GetOrigin(), vCoverOrigin, NULL);

	CWaypoint *pWaypoint = CWaypoints::GetWaypoint(iWpt);

	if (pWaypoint == NULL)
		return false;

	*vCover = pWaypoint->GetOrigin();

	return true;
}

void CWaypointNavigator::BeliefOne(int iWptIndex, BotBelief iBeliefType, float fDist)
{
	if (iBeliefType == BELIEF_SAFETY)
	{
		if (m_fBelief[iWptIndex] > 0)
			m_fBelief[iWptIndex] *= bot_belief_fade.GetFloat();
		if (m_fBelief[iWptIndex] < 0)
			m_fBelief[iWptIndex] = 0;
	}
	else // danger	
	{
		if (m_fBelief[iWptIndex] < MAX_BELIEF)
			m_fBelief[iWptIndex] += (2048.0f / fDist);
		if (m_fBelief[iWptIndex] > MAX_BELIEF)
			m_fBelief[iWptIndex] = MAX_BELIEF;
	}

	m_bBeliefChanged = true;
}

// Get belief nearest to current origin using waypoints to store belief
void CWaypointNavigator::Belief(Vector vOrigin, Vector vOther, float fBelief,
	float fStrength, BotBelief iType)
{
	static int i;
	static float factor;
	static float fEDist;
	static int iWptIndex;
	CWaypoint *pWpt;
	dataUnconstArray<int> m_iVisibles;
	dataUnconstArray<int> m_iInvisibles;
	static int iWptFrom;
	static int iWptTo;

	// Get nearest waypoint visible to others
	iWptFrom = CWaypointLocations::NearestWaypoint(vOrigin, 2048.0, -1, true, true, false, NULL, false, 0, false, true, vOther);
	iWptTo = CWaypointLocations::NearestWaypoint(vOther, 2048.0, -1, true, true, false, NULL, false, 0, false, true, vOrigin);

	// no waypoint information
	if ((iWptFrom == -1) || (iWptTo == -1))
		return;

	fEDist = (vOrigin - vOther).Length(); // range

	m_iVisibles.Add(iWptFrom);
	m_iVisibles.Add(iWptTo);

	CWaypointLocations::GetAllVisible(iWptFrom, iWptTo, vOrigin, vOther, fEDist, &m_iVisibles, &m_iInvisibles);
	CWaypointLocations::GetAllVisible(iWptFrom, iWptTo, vOther, vOrigin, fEDist, &m_iVisibles, &m_iInvisibles);

	for (i = 0; i < m_iVisibles.Size(); i++)
	{
		pWpt = CWaypoints::GetWaypoint(m_iVisibles[i]);
		iWptIndex = CWaypoints::GetWaypointIndex(pWpt);

		if (iType == BELIEF_SAFETY)
		{
			if (m_fBelief[iWptIndex] > 0)
				m_fBelief[iWptIndex] *= bot_belief_fade.GetFloat();//(fStrength / (vOrigin-pWpt->GetOrigin()).Length())*fBelief;
			if (m_fBelief[iWptIndex] < 0)
				m_fBelief[iWptIndex] = 0;

			//debugoverlay->AddTextOverlayRGB(pWpt->GetOrigin(),0,5.0f,0.0,150,0,200,"Safety");
		}
		else if (iType == BELIEF_DANGER)
		{
			if (m_fBelief[iWptIndex] < MAX_BELIEF)
				m_fBelief[iWptIndex] += (fStrength / (vOrigin - pWpt->GetOrigin()).Length())*fBelief;
			if (m_fBelief[iWptIndex] > MAX_BELIEF)
				m_fBelief[iWptIndex] = MAX_BELIEF;

			//debugoverlay->AddTextOverlayRGB(pWpt->GetOrigin(),0,5.0f,255,0,0,200,"Danger %0.2f",m_fBelief[iWptIndex]);
		}
	}

	for (i = 0; i < m_iInvisibles.Size(); i++)
	{
		pWpt = CWaypoints::GetWaypoint(m_iInvisibles[i]);
		iWptIndex = CWaypoints::GetWaypointIndex(pWpt);

		// this waypoint is safer from this danger
		if (iType == BELIEF_DANGER)
		{
			if (m_fBelief[iWptIndex] > 0)
				m_fBelief[iWptIndex] *= 0.9;//(fStrength / (vOrigin-pWpt->GetOrigin()).Length())*fBelief;

			//debugoverlay->AddTextOverlayRGB(pWpt->GetOrigin(),1,5.0f,0.0,150,0,200,"Safety INV");
		}
		else if (iType == BELIEF_SAFETY)
		{
			if (m_fBelief[iWptIndex] < MAX_BELIEF)
				m_fBelief[iWptIndex] += (fStrength / (vOrigin - pWpt->GetOrigin()).Length())*fBelief*0.5f;
			if (m_fBelief[iWptIndex] > MAX_BELIEF)
				m_fBelief[iWptIndex] = MAX_BELIEF;

			//debugoverlay->AddTextOverlayRGB(pWpt->GetOrigin(),1,5.0f,255,0,0,200,"Danger INV %0.2f",m_fBelief[iWptIndex]);
		}
	}


	/*
		i = m_oldRoute.size();

		while ( !m_oldRoute.empty() )
		{
		iWptIndex = m_oldRoute.front();

		factor = ((float)i)/m_oldRoute.size();
		i--;

		if ( iWptIndex >= 0 )
		{
		if ( iType == BELIEF_SAFETY )
		{
		if ( m_fBelief[iWptIndex] > 0)
		m_fBelief[iWptIndex] *= bot_belief_fade.GetFloat()*factor;//(fStrength / (vOrigin-pWpt->GetOrigin()).Length())*fBelief;
		if ( m_fBelief[iWptIndex] < 0 )
		m_fBelief[iWptIndex] = 0;
		}
		else if ( iType == BELIEF_DANGER )
		{
		if ( m_fBelief[iWptIndex] < MAX_BELIEF )
		m_fBelief[iWptIndex] += factor*fBelief;
		if ( m_fBelief[iWptIndex] > MAX_BELIEF )
		m_fBelief[iWptIndex] = MAX_BELIEF;
		}
		}

		m_oldRoute.pop();
		}*/

	m_iVisibles.Destroy();
	m_iInvisibles.Destroy();

	m_bBeliefChanged = true;
}

int CWaypointNavigator::GetCurrentFlags()
{
	if (m_iCurrentWaypoint != -1)
		return CWaypoints::GetWaypoint(m_iCurrentWaypoint)->GetFlags();

	return 0;
}

float CWaypointNavigator::GetCurrentBelief()
{
	if (m_iCurrentWaypoint >= 0)
	{
		return m_fBelief[m_iCurrentWaypoint];
	}

	return 0;
}
/*
bool CWaypointNavigator :: GetCrouchHideSpot ( Vector vCoverOrigin, Vector *vCover )
{

}
*/
// Get the hide spot position (vCover) from origin vCoverOrigin
bool CWaypointNavigator::GetHideSpotPosition(Vector vCoverOrigin, Vector *vCover)
{
	int iWpt;

	if (m_pBot->HasGoal())
		iWpt = CWaypointLocations::GetCoverWaypoint(m_pBot->GetOrigin(), vCoverOrigin, NULL, m_pBot->GetGoalOrigin());
	else
		iWpt = CWaypointLocations::GetCoverWaypoint(m_pBot->GetOrigin(), vCoverOrigin, NULL);

	CWaypoint *pWaypoint = CWaypoints::GetWaypoint(iWpt);

	if (pWaypoint == NULL)
		return false;

	*vCover = pWaypoint->GetOrigin();

	return true;
}
// AStar Algorithm : open a waypoint
void CWaypointNavigator::Open(AStarNode *pNode)
{
	if (!pNode->IsOpen())
	{
		pNode->Open();
		//m_theOpenList.push_back(pNode);
		m_theOpenList.Add(pNode);
	}
}
// AStar Algorithm : Get the waypoint with lowest cost
AStarNode *CWaypointNavigator::NextNode()
{
	AStarNode *pNode = NULL;

	pNode = m_theOpenList.Top();
	m_theOpenList.Pop();

	return pNode;
}

// clears the AStar open list
void CWaypointNavigator::ClearOpenList()
{
	m_theOpenList.Destroy();


	//for ( unsigned int i = 0; i < m_theOpenList.size(); i ++ )
	//	m_theOpenList[i]->unOpen();

	//m_theOpenList.clear();
}

void CWaypointNavigator::FailMove()
{
	m_iLastFailedWpt = m_iCurrentWaypoint;

	m_lastFailedPath.bValid = true;
	m_lastFailedPath.iFrom = m_iPrevWaypoint;
	m_lastFailedPath.iTo = m_iCurrentWaypoint;
	m_lastFailedPath.bSkipped = false;

	if (!m_iFailedGoals.IsMember(m_iGoalWaypoint))
	{
		m_iFailedGoals.Add(m_iGoalWaypoint);
		m_fNextClearFailedGoals = engine->Time() + RandomFloat(8.0f, 30.0f);
	}
}

float CWaypointNavigator::DistanceTo(Vector vOrigin)
{
	int iGoal;

	if (m_iCurrentWaypoint == -1)
		m_iCurrentWaypoint = CWaypointLocations::NearestWaypoint(m_pBot->GetOrigin(), CWaypointLocations::REACHABLE_RANGE, -1, true, false, true, NULL, false, m_pBot->GetTeam());

	if (m_iCurrentWaypoint != -1)
	{
		iGoal = CWaypointLocations::NearestWaypoint(vOrigin, CWaypointLocations::REACHABLE_RANGE, -1, true, false, true, NULL, false, m_pBot->GetTeam());

		if (iGoal != -1)
			return CWaypointDistances::GetDistance(m_iCurrentWaypoint, iGoal);
	}

	return m_pBot->DistanceFrom(vOrigin);
}

float CWaypointNavigator::DistanceTo(CWaypoint *pWaypoint)
{
	return DistanceTo(pWaypoint->GetOrigin());
}

// find route using A* algorithm
bool CWaypointNavigator::WorkRoute(Vector vFrom,
	Vector vTo,
	bool *bFail,
	bool bRestart,
	bool bNoInterruptions,
	int iGoalId,
	int iConditions, int iDangerId)
{
	extern ConVar bot_debug_show_route;

	if (bRestart)
	{
		CWaypoint *pGoalWaypoint;

		if (WantToSaveBelief())
			BeliefSave();
		if (WantToLoadBelief())
			BeliefLoad();

		*bFail = false;

		m_bWorkingRoute = true;

		if (iGoalId == -1)
			m_iGoalWaypoint = CWaypointLocations::NearestWaypoint(vTo, CWaypointLocations::REACHABLE_RANGE, m_iLastFailedWpt, true, false, true, &m_iFailedGoals, false, m_pBot->GetTeam());
		else
			m_iGoalWaypoint = iGoalId;

		pGoalWaypoint = CWaypoints::GetWaypoint(m_iGoalWaypoint);

		if (m_iGoalWaypoint == -1)
		{
			*bFail = true;
			m_bWorkingRoute = false;
			return true;
		}

		m_vPreviousPoint = vFrom;
		// Get closest waypoint -- ignore previous failed waypoint
		Vector vIgnore;
		float fIgnoreSize;

		bool bIgnore = m_pBot->GetIgnoreBox(&vIgnore, &fIgnoreSize) && (pGoalWaypoint->DistanceFrom(vFrom) > (fIgnoreSize * 2));

		m_iCurrentWaypoint = CWaypointLocations::NearestWaypoint(vFrom, CWaypointLocations::REACHABLE_RANGE, m_iLastFailedWpt,
			true, false, true, NULL, false, m_pBot->GetTeam(), true, false, vIgnore, 0, NULL, bIgnore, fIgnoreSize);

		// no nearest waypoint -- find nearest waypoint
		if (m_iCurrentWaypoint == -1)
		{
			// don't ignore this time
			m_iCurrentWaypoint = CWaypointLocations::NearestWaypoint(vFrom, CWaypointLocations::REACHABLE_RANGE, -1, true, false, true, NULL, false, m_pBot->GetTeam(), false, false, Vector(0, 0, 0), 0, m_pBot->GetEdict());

			if (m_iCurrentWaypoint == -1)
			{
				*bFail = true;
				m_bWorkingRoute = false;
				return true;
			}
		}

		// reset
		m_iLastFailedWpt = -1;

		ClearOpenList();
		Q_memset(paths, 0, sizeof(AStarNode)*CWaypoints::MAX_WAYPOINTS);
		
		AStarNode *curr = &paths[m_iCurrentWaypoint];
		curr->SetWaypoint(m_iCurrentWaypoint);
		curr->SetHeuristic(m_pBot->DistanceFrom(vTo));
		Open(curr);
	}
	/////////////////////////////////
	if (m_iGoalWaypoint == -1)
	{
		*bFail = true;
		m_bWorkingRoute = false;
		return true;
	}
	if (m_iCurrentWaypoint == -1)
	{
		*bFail = true;
		m_bWorkingRoute = false;
		return true;
	}
	///////////////////////////////

	int iLoops = 0;
	int iMaxLoops = this->m_pBot->GetProfile()->m_iPathTicks;

	if (iMaxLoops <= 0)
		iMaxLoops = 200;

	if (bNoInterruptions)
		iMaxLoops *= 2; // "less" interruptions, however dont want to hang, or use massive cpu

	int iCurrentNode; // node selected

	bool bFoundGoal = false;

	CWaypoint *currWpt;
	CWaypoint *succWpt;
	CWaypointVisibilityTable *pVisTable = CWaypoints::GetVisiblity();

	float fCost;
	float fOldCost;

	Vector vOrigin;

	int iPath;
	int iMaxPaths;
	int iSucc;

	int iLastNode = -1;

	float fBeliefSensitivity = 1.5f;

	if (iConditions & CONDITION_COVERT)
		fBeliefSensitivity = 2.0f;

	while (!bFoundGoal && !m_theOpenList.Empty() && (iLoops < iMaxLoops))
	{
		iLoops++;

		curr = this->NextNode();

		if (!curr)
			break;

		iCurrentNode = curr->GetWaypoint();

		bFoundGoal = (iCurrentNode == m_iGoalWaypoint);

		if (bFoundGoal)
			break;

		// can Get here now
		m_iFailedGoals.Remove(iCurrentNode);//.Remove(iCurrentNode);

		currWpt = CWaypoints::GetWaypoint(iCurrentNode);

		vOrigin = currWpt->GetOrigin();

		iMaxPaths = currWpt->NumPaths();

		succ = NULL;

		for (iPath = 0; iPath < iMaxPaths; iPath++)
		{
			iSucc = currWpt->GetPath(iPath);

			if (iSucc == iLastNode)
				continue;
			if (iSucc == iCurrentNode) // argh?
				continue;
			if (m_lastFailedPath.bValid)
			{
				if (m_lastFailedPath.iFrom == iCurrentNode)
				{
					// failed this path last time
					if (m_lastFailedPath.iTo == iSucc)
					{
						m_lastFailedPath.bSkipped = true;
						continue;
					}
				}
			}

			succ = &paths[iSucc];
			succWpt = CWaypoints::GetWaypoint(iSucc);

			if ((iSucc != m_iGoalWaypoint) && !m_pBot->CanGotoWaypoint(vOrigin, succWpt, currWpt))
				continue;

			if (currWpt->HasFlag(CWaypointTypes::W_FL_TELEPORT_CHEAT))
				fCost = curr->GetCost();
			else if (succWpt->HasFlag(CWaypointTypes::W_FL_TELEPORT_CHEAT))
				fCost = succWpt->DistanceFrom(vOrigin);
			else
				fCost = curr->GetCost() + (succWpt->DistanceFrom(vOrigin));

			if (!CWaypointDistances::IsSet(m_iCurrentWaypoint, iSucc) || (CWaypointDistances::GetDistance(m_iCurrentWaypoint, iSucc) > fCost))
				CWaypointDistances::SetDistance(m_iCurrentWaypoint, iSucc, fCost);

			if (succ->IsOpen() || succ->IsClosed())
			{
				if (succ->GetParent() != -1)
				{
					fOldCost = succ->GetCost();

					if (fCost >= fOldCost)
						continue; // ignore route
				}
				else
					continue;
			}

			succ->UnClose();

			succ->SetParent(iCurrentNode);

			if (fBeliefSensitivity > 1.6f)
			{
				if ((m_pBot->GetEnemy() != NULL) && CBotGlobals::IsPlayer(m_pBot->GetEnemy()) && (m_pBot->IsVisible(m_pBot->GetEnemy())))
				{
					if (CBotGlobals::DotProductFromOrigin(m_pBot->GetEnemy(), succWpt->GetOrigin()) > 0.96f)
						succ->SetCost(fCost + CWaypointLocations::REACHABLE_RANGE);
					else
						succ->SetCost(fCost);

					if (iDangerId != -1)
					{
						if (pVisTable->GetVisibilityFromTo(iDangerId, iSucc))
							succ->SetCost(succ->GetCost() + (m_fBelief[iSucc] * fBeliefSensitivity * 2));
					}
				}
				else if (iDangerId != -1)
				{
					if (!pVisTable->GetVisibilityFromTo(iDangerId, iSucc))
						succ->SetCost(fCost);
					else
						succ->SetCost(fCost + (m_fBelief[iSucc] * fBeliefSensitivity * 2));
				}
				else
					succ->SetCost(fCost + (m_fBelief[iSucc] * fBeliefSensitivity));
				//succ->setCost(fCost-(MAX_BELIEF-m_fBelief[iSucc]));
				//succ->setCost(fCost-((MAX_BELIEF*fBeliefSensitivity)-(m_fBelief[iSucc]*(fBeliefSensitivity-m_pBot->GetProfile()->m_fBraveness))));	
			}
			else
				succ->SetCost(fCost + (m_fBelief[iSucc] * (fBeliefSensitivity - m_pBot->GetProfile()->m_fBraveness)));

			succ->SetWaypoint(iSucc);

			if (!succ->HeuristicSet())
			{
				if (fBeliefSensitivity > 1.6f)
					succ->SetHeuristic(m_pBot->DistanceFrom(succWpt->GetOrigin()) + succWpt->DistanceFrom(vTo) + (m_fBelief[iSucc] * 2));
				else
					succ->SetHeuristic(m_pBot->DistanceFrom(succWpt->GetOrigin()) + succWpt->DistanceFrom(vTo));
			}

			// Fix: do this AFTER setting heuristic and cost!!!!
			if (!succ->IsOpen())
			{
				Open(succ);
			}

		}

		curr->Close(); // close chosen node

		iLastNode = iCurrentNode;
	}
	/////////
	if (iLoops == iMaxLoops)
	{
		//*bFail = true;

		return false; // not finished yet, wait for next iteration
	}

	m_bWorkingRoute = false;

	ClearOpenList(); // finished

	if (!bFoundGoal)
	{
		*bFail = true;

		//no other path
		if (m_lastFailedPath.bSkipped)
			m_lastFailedPath.bValid = false;

		if (!m_iFailedGoals.IsMember(m_iGoalWaypoint))
		{
			m_iFailedGoals.Add(m_iGoalWaypoint);
			m_fNextClearFailedGoals = engine->Time() + RandomFloat(8.0f, 30.0f);
		}

		return true; // waypoint not found but searching is complete
	}

	while (!m_oldRoute.empty())
		m_oldRoute.pop();

	iCurrentNode = m_iGoalWaypoint;

	m_currentRoute.Destroy();

	iLoops = 0;

	int iNumWaypoints = CWaypoints::NumWaypoints();
	float fDistance = 0.0;
	int iParent;

	while ((iCurrentNode != -1) && (iCurrentNode != m_iCurrentWaypoint) && (iLoops <= iNumWaypoints))
	{
		iLoops++;

		m_currentRoute.Push(iCurrentNode);
		m_oldRoute.push(iCurrentNode);

		iParent = paths[iCurrentNode].GetParent();

		// crash bug fix
		if (iParent != -1)
			fDistance += (CWaypoints::GetWaypoint(iCurrentNode)->GetOrigin() - CWaypoints::GetWaypoint(iParent)->GetOrigin()).Length();

		iCurrentNode = iParent;
	}

	CWaypointDistances::SetDistance(m_iCurrentWaypoint, m_iGoalWaypoint, fDistance);
	m_fGoalDistance = fDistance;

	// erh??
	if (iLoops > iNumWaypoints)
	{
		while (!m_oldRoute.empty())
			m_oldRoute.pop();

		m_currentRoute.Destroy();
		*bFail = true;
	}
	else
	{
		m_vGoal = CWaypoints::GetWaypoint(m_iGoalWaypoint)->GetOrigin();
	}

	return true;
}
// if bot has a current position to walk to return the boolean
bool CWaypointNavigator::HasNextPoint()
{
	return m_iCurrentWaypoint != -1;
}
// return the vector of the next point
Vector CWaypointNavigator::GetNextPoint()
{
	return CWaypoints::GetWaypoint(m_iCurrentWaypoint)->GetOrigin();
}

bool CWaypointNavigator::GetNextRoutePoint(Vector *point)
{
	if (!m_currentRoute.IsEmpty())
	{
		static int *head;
		static CWaypoint *pW;

		head = m_currentRoute.GetHeadInfoPointer();

		if (head && (*head != -1))
		{
			pW = CWaypoints::GetWaypoint(*head);
			*point = pW->GetOrigin();// + pW->applyRadius();

			return true;
		}
	}

	return false;
}

bool CWaypointNavigator::CanGetTo(Vector vOrigin)
{
	int iwpt = CWaypointLocations::NearestWaypoint(vOrigin, 100, -1, true, false, true, NULL, false, m_pBot->GetTeam());

	if (iwpt >= 0)
	{
		if (m_iFailedGoals.IsMember(iwpt))
			return false;
	}
	else
		return false;

	return true;
}

void CWaypointNavigator::RollBackPosition()
{
	m_vPreviousPoint = m_pBot->GetOrigin();
	m_iCurrentWaypoint = CWaypointLocations::NearestWaypoint(m_vPreviousPoint, CWaypointLocations::REACHABLE_RANGE, m_iLastFailedWpt, true, false, true, NULL, false, m_pBot->GetTeam());

	while (!m_currentRoute.IsEmpty()) // reached goal!!
	{
		if (m_iCurrentWaypoint == m_currentRoute.Pop())
		{
			if (!m_currentRoute.IsEmpty())
				m_iCurrentWaypoint = m_currentRoute.Pop();
		}
	}

	if (m_iCurrentWaypoint == -1)
		m_iCurrentWaypoint = CWaypointLocations::NearestWaypoint(m_pBot->GetOrigin(), CWaypointLocations::REACHABLE_RANGE, -1, true, false, true, NULL, false, m_pBot->GetTeam());
	// find waypoint in route
}
// update the bots current walk vector
void CWaypointNavigator::UpdatePosition()
{
	static Vector vWptOrigin;
	static float fRadius;
	static float fPrevBelief, fBelief;

	static QAngle aim;
	static Vector vaim;

	static bool bTouched;

	fPrevBelief = 0;
	fBelief = 0;

	if (m_iCurrentWaypoint == -1) // invalid
	{
		m_pBot->StopMoving();
		m_bOffsetApplied = false;
		return;
	}

	CWaypoint *pWaypoint = CWaypoints::GetWaypoint(m_iCurrentWaypoint);

	if (pWaypoint == NULL)
	{
		m_bOffsetApplied = false;
		return;
	}

	aim = QAngle(0, pWaypoint->GetAimYaw(), 0);
	AngleVectors(aim, &vaim);

	fRadius = pWaypoint->GetRadius();

	vWptOrigin = pWaypoint->GetOrigin();

	if (!m_bWorkingRoute)
	{
		bool movetype_ok = CClassInterface::IsMoveType(m_pBot->GetEdict(), MOVETYPE_LADDER) || CClassInterface::IsMoveType(m_pBot->GetEdict(), MOVETYPE_FLYGRAVITY);

		//bTouched = false;

		bTouched = pWaypoint->Touched(m_pBot->GetOrigin(), m_vOffset, m_pBot->GetTouchDistance(), !m_pBot->IsUnderWater());

		if (pWaypoint->HasFlag(CWaypointTypes::W_FL_LADDER))
			bTouched = bTouched && movetype_ok;

		if (bTouched)
		{
			int iWaypointID = CWaypoints::GetWaypointIndex(pWaypoint);
			int iWaypointFlagsPrev = 0;

			fPrevBelief = GetBelief(iWaypointID);

			// Bot passed into this waypoint safely, update belief

			bot_statistics_t *stats = m_pBot->GetStats();

			if (stats)
			{
				if ((stats->stats.m_iEnemiesVisible > stats->stats.m_iTeamMatesVisible) && (stats->stats.m_iEnemiesInRange > 0))
					BeliefOne(iWaypointID, BELIEF_DANGER, 100.0f);
				else if ((stats->stats.m_iTeamMatesVisible > 0) && (stats->stats.m_iTeamMatesInRange > 0))
					BeliefOne(iWaypointID, BELIEF_SAFETY, 100.0f);
			}

			m_bOffsetApplied = false;

			m_bDangerPoint = false;


			if (m_currentRoute.IsEmpty()) // reached goal!!
			{
				// fix: bots jumping at wrong positions
				m_pBot->TouchedWpt(pWaypoint, -1);


				m_vPreviousPoint = m_pBot->GetOrigin();
				m_iPrevWaypoint = m_iCurrentWaypoint;
				m_iCurrentWaypoint = -1;

				if (m_pBot->GetSchedule()->IsCurrentSchedule(SCHED_RUN_FOR_COVER) ||
					m_pBot->GetSchedule()->IsCurrentSchedule(SCHED_GOOD_HIDE_SPOT))
					m_pBot->ReachedCoverSpot(pWaypoint->GetFlags());
			}
			else
			{
				iWaypointFlagsPrev = CWaypoints::GetWaypoint(m_iCurrentWaypoint)->GetFlags();
				int iPrevWpt = m_iPrevWaypoint;
				m_vPreviousPoint = m_pBot->GetOrigin();
				m_iPrevWaypoint = m_iCurrentWaypoint;
				m_iCurrentWaypoint = m_currentRoute.Pop();

				// fix: bots jumping at wrong positions
				m_pBot->TouchedWpt(pWaypoint, m_iCurrentWaypoint, iPrevWpt);


				// fix : update pWaypoint as Current Waypoint
				pWaypoint = CWaypoints::GetWaypoint(m_iCurrentWaypoint);

				if (pWaypoint)
				{
				}
				if (m_iCurrentWaypoint != -1)
				{ // random point, but more chance of choosing the most dangerous point
					m_bDangerPoint = RandomDangerPath(&m_vDangerPoint);
				}

				fBelief = GetBelief(m_iCurrentWaypoint);
			}
		}
	}
	else
		m_bOffsetApplied = false;

	m_pBot->WalkingTowardsWaypoint(pWaypoint, &m_bOffsetApplied, m_vOffset);

	// fix for bots not finding goals
	if (m_fNextClearFailedGoals && (m_fNextClearFailedGoals < engine->Time()))
	{
		m_iFailedGoals.Destroy();
		m_fNextClearFailedGoals = 0;
	}

	m_pBot->SetMoveTo(vWptOrigin + m_vOffset);

	if (pWaypoint && pWaypoint->IsAiming())
		m_pBot->SetAiming(vWptOrigin + (vaim * 1024));

	/*if ( !m_pBot->HasEnemy() && (fBelief >= (fPrevBelief+10.0f)) )
		m_pBot->setLookAtTask(LOOK_LAST_ENEMY);
		else if ( !m_pBot->HasEnemy() && (fPrevBelief > (fBelief+10.0f)) )
		{
		m_pBot->SetLookVector(pWaypoint->GetOrigin() + pWaypoint->ApplyRadius());
		m_pBot->SetLookAtTask(LOOK_VECTOR,RandomFloat(1.0f,2.0f));
		}*/
}

void CWaypointNavigator::Clear()
{
	m_currentRoute.Destroy();
	m_iFailedGoals.Destroy();//.clear();//Destroy();
}
// free up memory
void CWaypointNavigator::FreeMapMemory()
{
	BeliefSave(true);
	Clear();
}

void CWaypointNavigator::FreeAllMemory()
{
	FreeMapMemory();
}

bool CWaypointNavigator::RouteFound()
{
	return !m_currentRoute.IsEmpty();
}

/////////////////////////////////////////////////////////
// checks if a waypoint is touched
bool CWaypoint::Touched(Vector vOrigin, Vector vOffset, float fTouchDist, bool onground)
{
	static Vector v_dynamic;
	extern ConVar bot_ladder_offs;

	v_dynamic = m_vOrigin + vOffset;

	if (HasFlag(CWaypointTypes::W_FL_TELEPORT_CHEAT))
		return ((vOrigin - GetOrigin()).Length()) < (MAX(fTouchDist, GetRadius()));

	// on ground or ladder
	if (onground)
	{
		if ((vOrigin - v_dynamic).Length2D() <= fTouchDist)
		{
			if (HasFlag(CWaypointTypes::W_FL_LADDER))
				return ((vOrigin.z + bot_ladder_offs.GetFloat()) > v_dynamic.z);

			return fabs(vOrigin.z - v_dynamic.z) <= WAYPOINT_HEIGHT;
		}
	}
	else // swimming
	{
		if ((vOrigin - v_dynamic).Length() < fTouchDist)
			return true;
	}

	return false;
}
// clear the waypoints possible paths
void CWaypoint::ClearPaths()
{
	m_thePaths.Clear();
}
// Get the distance from this waypoint from vector position vOrigin
float CWaypoint::DistanceFrom(Vector vOrigin)
{
	return (m_vOrigin - vOrigin).Length();
}
///////////////////////////////////////////////////
void CWaypoints::UpdateWaypointPairs(vector<edict_wpt_pair_t> *pPairs, int iWptFlag, const char *szClassname)
{
	register short int iSize = NumWaypoints();
	CWaypoint *pWpt;
	edict_wpt_pair_t pair;
	CTraceFilterWorldAndPropsOnly filter;
	trace_t *trace_result;

	pWpt = m_theWaypoints;
	trace_result = CBotGlobals::GetTraceResult();

	Vector vOrigin;

	for (register short int i = 0; i < iSize; i++)
	{
		if (pWpt->IsUsed() && pWpt->HasFlag(iWptFlag))
		{
			pair.pWaypoint = pWpt;
			pair.pEdict = CClassInterface::FindEntityByClassnameNearest(pWpt->GetOrigin(), szClassname, 300.0f);

			if (pair.pEdict != NULL)
			{
				vOrigin = CBotGlobals::EntityOrigin(pair.pEdict);

				CBotGlobals::TraceLine(vOrigin, vOrigin - Vector(0, 0, CWaypointLocations::REACHABLE_RANGE), MASK_SOLID_BRUSHONLY, &filter);
				// updates trace_result

				pair.v_ground = trace_result->endpos + Vector(0, 0, 48.0f);

				pPairs->push_back(pair);
			}
		}

		pWpt++;
	}
}
/////////////////////////////////////////////////////////////////////////////////////
// save waypoints (visibilitymade saves having to work out visibility again)
// pPlayer is the person who called the command to save, NULL if automatic
bool CWaypoints::Save(bool bVisiblityMade, edict_t *pPlayer, const char *pszAuthor, const char *pszModifier)
{
	char filename[1024];
	char szAuthorName[32];

	smutils->BuildPath(Path_SM, filename, sizeof(filename), "data\\afkbot\\%s\\%s.%s", BOT_WAYPOINT_FOLDER, CBotGlobals::GetMapName(), BOT_WAYPOINT_EXTENSION);

	FILE *bfp = CBotGlobals::OpenFile(filename, "wb");

	if (bfp == NULL)
	{
		return false; // give up
	}

	int iSize = NumWaypoints();

	// write header
	// ----
	CWaypointHeader header;
	CWaypointAuthorInfo authorinfo;

	int flags = 0;

	if (bVisiblityMade)
		flags |= W_FILE_FL_VISIBILITY;

	//////////////////////////////////////////////
	header.iFlags = flags;
	header.iNumWaypoints = iSize;
	header.iVersion = WAYPOINT_VERSION;

	if (pszAuthor != NULL)
		strncpy(authorinfo.szAuthor, pszAuthor, 31);
	else
	{
		strncpy(authorinfo.szAuthor, CWaypoints::GetAuthor(), 31);
	}

	if (pszModifier != NULL)
		strncpy(authorinfo.szModifiedBy, pszModifier, 31);
	else
	{
		strncpy(authorinfo.szModifiedBy, CWaypoints::GetModifier(), 31);
	}

	authorinfo.szAuthor[31] = 0;
	authorinfo.szModifiedBy[31] = 0;

	if (!bVisiblityMade && (pszAuthor == NULL) && (pszModifier == NULL))
	{
		strcpy(szAuthorName, "(unknown)");

		if (pPlayer != NULL)
		{
			strcpy(szAuthorName, playerinfomanager->GetPlayerInfo(pPlayer)->GetName());
		}

		if (authorinfo.szAuthor[0] == 0) // no author
		{
			strncpy(authorinfo.szAuthor, szAuthorName, 31);
			authorinfo.szAuthor[31] = 0;

			memset(authorinfo.szModifiedBy, 0, 32);
		}
		else if (strcmp(szAuthorName, authorinfo.szAuthor))
		{
			// modified
			strncpy(authorinfo.szModifiedBy, szAuthorName, 31);
			authorinfo.szModifiedBy[31] = 0;
		}
	}

	strcpy(header.szFileType, BOT_WAYPOINT_FILE_TYPE);
	strcpy(header.szMapName, CBotGlobals::GetMapName());
	//////////////////////////////////////////////

	fwrite(&header, sizeof(CWaypointHeader), 1, bfp);
	fwrite(&authorinfo, sizeof(CWaypointAuthorInfo), 1, bfp);

	for (int i = 0; i < iSize; i++)
	{
		CWaypoint *pWpt = &m_theWaypoints[i];

		// save individual waypoint and paths
		pWpt->Save(bfp);
	}

	fclose(bfp);

	//CWaypointDistances::reset();

	CWaypointDistances::Save();

	return true;
}

// load waypoints
bool CWaypoints::Load(const char *szMapName)
{
	char filename[1024];

	strcpy(m_szWelcomeMessage, "No waypoints for this map");

	// open explicit map name waypoints
	if (szMapName == NULL)
		smutils->BuildPath(Path_SM, filename, sizeof(filename), "data\\afkbot\\%s\\%s.%s", BOT_WAYPOINT_FOLDER, CBotGlobals::GetMapName(), BOT_WAYPOINT_EXTENSION);
	else
		smutils->BuildPath(Path_SM, filename, sizeof(filename), "data\\afkbot\\%s\\%s.%s", BOT_WAYPOINT_FOLDER, szMapName, BOT_WAYPOINT_EXTENSION);

	FILE *bfp = CBotGlobals::OpenFile(filename, "rb");

	if (bfp == NULL)
	{
		return false; // give up
	}

	CWaypointHeader header;
	CWaypointAuthorInfo authorinfo;

	memset(authorinfo.szAuthor, 0, 31);
	memset(authorinfo.szModifiedBy, 0, 31);

	// read header
	// -----------

	fread(&header, sizeof(CWaypointHeader), 1, bfp);

	if (!FStrEq(header.szFileType, BOT_WAYPOINT_FILE_TYPE))
	{
		CBotGlobals::BotMessage(NULL, 0, "Error loading waypoints: File type mismatch");
		fclose(bfp);
		return false;
	}
	if (header.iVersion > WAYPOINT_VERSION)
	{
		CBotGlobals::BotMessage(NULL, 0, "Error loading waypoints: Waypoint version too new");
		fclose(bfp);
		return false;
	}

	if (szMapName)
	{
		if (!FStrEq(header.szMapName, szMapName))
		{
			CBotGlobals::BotMessage(NULL, 0, "Error loading waypoints: Map name mismatch");
			fclose(bfp);
			return false;
		}
	}
	else if (!FStrEq(header.szMapName, CBotGlobals::GetMapName()))
	{
		CBotGlobals::BotMessage(NULL, 0, "Error loading waypoints: Map name mismatch");
		fclose(bfp);
		return false;
	}

	if (header.iVersion > 3)
	{
		// load author information
		fread(&authorinfo, sizeof(CWaypointAuthorInfo), 1, bfp);

		sprintf(m_szWelcomeMessage, "Waypoints by %s", authorinfo.szAuthor);

		if (authorinfo.szModifiedBy[0] != 0)
		{
			strcat(m_szWelcomeMessage, " modified by ");
			strcat(m_szWelcomeMessage, authorinfo.szModifiedBy);
		}
	}
	else
		sprintf(m_szWelcomeMessage, "Waypoints Loaded");

	int iSize = header.iNumWaypoints;

	// ok lets read the waypoints
	// initialize

	CWaypoints::Init(authorinfo.szAuthor, authorinfo.szModifiedBy);

	m_iNumWaypoints = iSize;

	bool bWorkVisibility = true;

	// if we're loading from another map, just load visibility, save effort!
	if ((szMapName == NULL) && (header.iFlags & W_FILE_FL_VISIBILITY))
		bWorkVisibility = (!m_pVisibilityTable->ReadFromFile(iSize));

	for (int i = 0; i < iSize; i++)
	{
		CWaypoint *pWpt = &m_theWaypoints[i];

		pWpt->Load(bfp, header.iVersion);

		if (pWpt->IsUsed()) // not a deleted waypoint
		{
			// add to waypoint locations for fast searching and drawing
			CWaypointLocations::AddWptLocation(pWpt, i);
		}
	}

	fclose(bfp);

	m_pVisibilityTable->SetWorkVisiblity(bWorkVisibility);

	if (bWorkVisibility) // say a message
		Msg(" *** No waypoint visibility file ***\n *** Working out waypoint visibility information... ***\n");

	// if we're loading from another map just do this again!
	if (szMapName == NULL)
		CWaypointDistances::Load();

	// script coupled to waypoints too
	//CPoints::loadMapScript();

	return true;
}

void CWaypoint::Init()
{
	//m_thePaths.clear();
	m_iFlags = 0;
	m_vOrigin = Vector(0, 0, 0);
	m_bUsed = false; // ( == "deleted" )
	SetAim(0);
	m_thePaths.Clear();
	m_iArea = 0;
	m_fRadius = 0;
	m_bUsed = true;
	m_fNextCheckGroundTime = 0;
	m_bHasGround = false;
	m_fRadius = 0;
	m_OpensLaterInfo.Clear();
	m_bIsReachable = true;
	m_fCheckReachableTime = 0;
}

void CWaypoint::Save(FILE *bfp)
{
	fwrite(&m_vOrigin, sizeof(Vector), 1, bfp);
	// aim of vector (used with certain waypoint types)
	fwrite(&m_iAimYaw, sizeof(int), 1, bfp);
	fwrite(&m_iFlags, sizeof(int), 1, bfp);
	// not deleted
	fwrite(&m_bUsed, sizeof(bool), 1, bfp);

	int iPaths = NumPaths();
	fwrite(&iPaths, sizeof(int), 1, bfp);

	for (int n = 0; n < iPaths; n++)
	{
		int iPath = GetPath(n);
		fwrite(&iPath, sizeof(int), 1, bfp);
	}

	if (CWaypoints::WAYPOINT_VERSION >= 2)
	{
		fwrite(&m_iArea, sizeof(int), 1, bfp);
	}

	if (CWaypoints::WAYPOINT_VERSION >= 3)
	{
		fwrite(&m_fRadius, sizeof(float), 1, bfp);
	}
}

void CWaypoint::Load(FILE *bfp, int iVersion)
{
	int iPaths;

	fread(&m_vOrigin, sizeof(Vector), 1, bfp);
	// aim of vector (used with certain waypoint types)
	fread(&m_iAimYaw, sizeof(int), 1, bfp);
	fread(&m_iFlags, sizeof(int), 1, bfp);
	// not deleted
	fread(&m_bUsed, sizeof(bool), 1, bfp);
	fread(&iPaths, sizeof(int), 1, bfp);

	for (int n = 0; n < iPaths; n++)
	{
		int iPath;
		fread(&iPath, sizeof(int), 1, bfp);
		AddPathTo(iPath);
	}

	if (iVersion >= 2)
	{
		fread(&m_iArea, sizeof(int), 1, bfp);
	}

	if (iVersion >= 3)
	{
		fread(&m_fRadius, sizeof(float), 1, bfp);
	}
}

bool CWaypoint::CheckGround()
{
	if (m_fNextCheckGroundTime < engine->Time())
	{
		CBotGlobals::QuickTraceline(NULL, m_vOrigin, m_vOrigin - Vector(0, 0, 80.0f));
		m_bHasGround = (CBotGlobals::GetTraceResult()->fraction < 1.0f);
		m_fNextCheckGroundTime = engine->Time() + 1.0f;
	}

	return m_bHasGround;
}

void CWaypoints::Init(const char *pszAuthor, const char *pszModifiedBy)
{
	if (pszAuthor != NULL)
	{
		strncpy(m_szAuthor, pszAuthor, 31);
		m_szAuthor[31] = 0;
	}
	else
		m_szAuthor[0] = 0;

	if (pszModifiedBy != NULL)
	{
		strncpy(m_szModifiedBy, pszModifiedBy, 31);
		m_szModifiedBy[31] = 0;
	}
	else
		m_szModifiedBy[0] = 0;

	m_iNumWaypoints = 0;
	m_fNextDrawWaypoints = 0;

	for (int i = 0; i < MAX_WAYPOINTS; i++)
		m_theWaypoints[i].Init();

	Q_memset(m_theWaypoints, 0, sizeof(CWaypoint)*MAX_WAYPOINTS);

	CWaypointLocations::Init();
	CWaypointDistances::Reset();
	m_pVisibilityTable->ClearVisibilityTable();
}

void CWaypoints::SetupVisibility()
{
	m_pVisibilityTable = new CWaypointVisibilityTable();
	m_pVisibilityTable->Init();
}

void CWaypoints::FreeMemory()
{
	if (m_pVisibilityTable)
	{
		m_pVisibilityTable->FreeVisibilityTable();

		delete m_pVisibilityTable;
	}
	m_pVisibilityTable = NULL;
}

///////////////////////////////////////////////////////
// return nearest waypoint not visible to pinch point
CWaypoint *CWaypoints::GetPinchPointFromWaypoint(Vector vPlayerOrigin, Vector vPinchOrigin)
{
	int iWpt = CWaypointLocations::GetCoverWaypoint(vPlayerOrigin, vPinchOrigin, NULL, &vPinchOrigin);

	return GetWaypoint(iWpt);
}

CWaypoint *CWaypoints::GetNestWaypoint(int iTeam, int iArea, bool bForceArea, CBot *pBot)
{
	//m_theWaypoints
	return NULL;
}

void CWaypoints::DeleteWaypoint(int iIndex)
{
	// mark as not used
	m_theWaypoints[iIndex].SetUsed(false);
	m_theWaypoints[iIndex].ClearPaths();

	// remove from waypoint locations
	Vector vOrigin = m_theWaypoints[iIndex].GetOrigin();
	float fOrigin[3] = { vOrigin.x, vOrigin.y, vOrigin.z };
	CWaypointLocations::DeleteWptLocation(iIndex, fOrigin);

	// delete any paths pointing to this waypoint
	DeletePathsTo(iIndex);
}

void CWaypoints::ShiftVisibleAreas(edict_t *pPlayer, int from, int to)
{
	for (int i = 0; i < m_iNumWaypoints; i++)
	{
		CWaypoint *pWpt = &(m_theWaypoints[i]);

		if (!pWpt->IsUsed())
			continue;

		if (pWpt->GetArea() == from)
		{
			CBotGlobals::QuickTraceline(pPlayer, CBotGlobals::EntityOrigin(pPlayer), pWpt->GetOrigin());

			if ((CBotGlobals::GetTraceResult()->fraction >= 1.0f))
				pWpt->SetArea(to);
		}
	}
}

void CWaypoints::ShiftAreas(int val)
{
	for (int i = 0; i < m_iNumWaypoints; i++)
	{
		CWaypoint *pWpt = &m_theWaypoints[i];

		if (pWpt->GetFlags() > 0)
		{
			pWpt->SetArea(pWpt->GetArea() + val);
		}
	}
}

int CWaypoints::GetClosestFlagged(int iFlags, Vector &vOrigin, int iTeam, float *fReturnDist, unsigned char *failedwpts)
{
	int i = 0;
	int size = NumWaypoints();

	float fDist = 8192.0;
	float distance;
	int iwpt = -1;
	int iFrom = CWaypointLocations::NearestWaypoint(vOrigin, fDist, -1, true, false, true, NULL, false, iTeam);

	CWaypoint *pWpt;

	for (i = 0; i < size; i++)
	{
		pWpt = &m_theWaypoints[i];

		if (i == iFrom)
			continue;

		if (failedwpts[i] == 1)
			continue;

		if (pWpt->IsUsed() && pWpt->ForTeam(iTeam))
		{
			if (pWpt->HasFlag(iFlags))
			{
				if (!CTeamFortress2Mod::m_ObjectiveResource.IsWaypointAreaValid(pWpt->GetArea()))
					continue;

				if ((iFrom == -1))
					distance = (pWpt->GetOrigin() - vOrigin).Length();
				else
					distance = CWaypointDistances::GetDistance(iFrom, i);

				if (distance < fDist)
				{
					fDist = distance;
					iwpt = i;
				}
			}
		}
	}

	if (fReturnDist)
		*fReturnDist = fDist;

	return iwpt;
}

void CWaypoints::DeletePathsTo(int iWpt)
{
	CWaypoint *pWaypoint = CWaypoints::GetWaypoint(iWpt);

	int iNumPathsTo = pWaypoint->NumPathsToThisWaypoint();
	dataUnconstArray<int> m_PathsTo;

	// this will go into an evil loop unless we do this first
	// and use a temporary copy as a side effect of performing
	// a remove will affect the original array
	for (int i = 0; i < iNumPathsTo; i++)
	{
		m_PathsTo.Add(pWaypoint->GetPathToThisWaypoint(i));
	}

	iNumPathsTo = m_PathsTo.Size();

	for (int i = 0; i < iNumPathsTo; i++)
	{
		int iOther = m_PathsTo.ReturnValueFromIndex(i);

		CWaypoint *pOther = GetWaypoint(iOther);

		pOther->RemovePathTo(iWpt);
	}
	/*

	short int iNumWaypoints = (short int)NumWaypoints();

	for ( register short int i = 0; i < iNumWaypoints; i ++ )
	m_theWaypoints[i].removePathTo(iWpt);*/
}

// Fixed; 23/01
void CWaypoints::DeletePathsFrom(int iWpt)
{
	m_theWaypoints[iWpt].ClearPaths();
}

int CWaypoints::AddWaypoint(edict_t *pPlayer, Vector vOrigin, int iFlags, bool bAutoPath, int iYaw, int iArea, float fRadius)
{
	int iIndex = FreeWaypointIndex();
	extern ConVar bot_wpt_autoradius;

	if (iIndex == -1)
	{
		Msg("Waypoints full!");
		return -1;
	}

	if ((fRadius == 0) && (bot_wpt_autoradius.GetFloat() > 0))
		fRadius = bot_wpt_autoradius.GetFloat();

	///////////////////////////////////////////////////
	m_theWaypoints[iIndex] = CWaypoint(vOrigin, iFlags);
	m_theWaypoints[iIndex].SetAim(iYaw);
	m_theWaypoints[iIndex].SetArea(iArea);
	m_theWaypoints[iIndex].SetRadius(fRadius);
	// increase max waypoints used
	if (iIndex == m_iNumWaypoints)
		m_iNumWaypoints++;
	///////////////////////////////////////////////////

	float fOrigin[3] = { vOrigin.x, vOrigin.y, vOrigin.z };

	CWaypointLocations::AddWptLocation(iIndex, fOrigin);
	m_pVisibilityTable->WorkVisibilityForWaypoint(iIndex, true);

	if (bAutoPath && !(iFlags & CWaypointTypes::W_FL_UNREACHABLE))
	{
		CWaypointLocations::AutoPath(pPlayer, iIndex);
	}

	return iIndex;
}

void CWaypoints::RemoveWaypoint(int iIndex)
{
	if (iIndex >= 0)
		m_theWaypoints[iIndex].SetUsed(false);
}

int CWaypoints::NumWaypoints()
{
	return m_iNumWaypoints;
}

///////////

int CWaypoints::NearestWaypointGoal(int iFlags, Vector &origin, float fDist, int iTeam)
{
	register short int i;
	static int size;

	float distance;
	int iwpt = -1;

	CWaypoint *pWpt;

	size = NumWaypoints();

	for (i = 0; i < size; i++)
	{
		pWpt = &m_theWaypoints[i];

		if (pWpt->IsUsed() && pWpt->ForTeam(iTeam))
		{
			if ((iFlags == -1) || pWpt->HasFlag(iFlags))
			{
				if (CTeamFortress2Mod::m_ObjectiveResource.IsWaypointAreaValid(pWpt->GetArea()))
				{
					if ((distance = pWpt->DistanceFrom(origin)) < fDist)
					{
						fDist = distance;
						iwpt = i;
					}
				}
			}
		}
	}

	return iwpt;
}

CWaypoint *CWaypoints::RandomRouteWaypoint(CBot *pBot, Vector vOrigin, Vector vGoal, int iTeam, int iArea)
{
	register short int i;
	static short int size;
	static CWaypoint *pWpt;
	static CWaypointNavigator *pNav;

	pNav = (CWaypointNavigator*)pBot->GetNavigator();

	size = NumWaypoints();

	dataUnconstArray<CWaypoint*> goals;

	for (i = 0; i < size; i++)
	{
		pWpt = &m_theWaypoints[i];

		if (pWpt->IsUsed() && pWpt->ForTeam(iTeam))// && (pWpt->GetArea() == iArea) )
		{
			if (pWpt->HasFlag(CWaypointTypes::W_FL_ROUTE))
			{
				if ((pWpt->GetArea() != iArea))
					continue;

				// CHECK THAT ROUTE WAYPOINT IS USEFUL...

				Vector vRoute = pWpt->GetOrigin();

				if ((vRoute - vOrigin).Length() < ((vGoal - vOrigin).Length() + 128.0f))
				{
					//if ( CWaypointDistances::GetDistance() )
					/*Vector vecLOS;
					float flDot;
					Vector vForward;
					// in fov? Check angle to edict
					vForward = vGoal - vOrigin;
					vForward = vForward/vForward.Length(); // normalise

					vecLOS = vRoute - vOrigin;
					vecLOS = vecLOS/vecLOS.Length(); // normalise

					flDot = DotProduct (vecLOS , vForward );

					if ( flDot > 0.17f ) // 80 degrees*/
					goals.Add(pWpt);
				}
			}
		}
	}

	pWpt = NULL;

	if (!goals.IsEmpty())
	{
		pWpt = pNav->ChooseBestFromBelief(&goals);
	}

	goals.Clear();

	return pWpt;
}

#define MAX_DEPTH 10
/*
void CWaypointNavigator::runAwayFrom ( int iId )
{
CWaypoint *pRunTo = CWaypoints::GetNextCoverPoint(CWaypoints::GetWaypoint(m_iCurrentWaypoint),CWaypoints::GetWaypoint(iId)) ;

if ( pRunTo )
{
if ( pRunTo->touched(m_pBot->GetOrigin(),Vector(0,0,0),48.0f) )
m_iCurrentWaypoint = CWaypoints::GetWaypointIndex(pRunTo);
else
m_pBot->setMoveTo(pRunTo->GetOrigin());
}

}*/

CWaypoint *CWaypoints::GetNextCoverPoint(CBot *pBot, CWaypoint *pCurrent, CWaypoint *pBlocking)
{
	int iMaxDist = -1;
	int iNext;
	float fMaxDist = 0.0f;
	float fDist = 0.0f;
	CWaypoint *pNext;

	for (int i = 0; i < pCurrent->NumPaths(); i++)
	{
		iNext = pCurrent->GetPath(i);
		pNext = CWaypoints::GetWaypoint(iNext);

		if (pNext == pBlocking)
			continue;

		if (!pBot->CanGotoWaypoint(pCurrent->GetOrigin(), pNext, pCurrent))
			continue;

		if ((iMaxDist == -1) || ((fDist = pNext->DistanceFrom(pBlocking->GetOrigin())) > fMaxDist))
		{
			fMaxDist = fDist;
			iMaxDist = iNext;
		}
	}

	if (iMaxDist == -1)
		return NULL;

	return CWaypoints::GetWaypoint(iMaxDist);
}

CWaypoint *CWaypoints::NearestPipeWaypoint(Vector vTarGet, Vector vOrigin, int *iAiming)
{
	// 1 : find nearest waypoint to vTarGet
	// 2 : loop through waypoints find visible waypoints to vTarGet
	// 3 : loop through visible waypoints find another waypoint invisible to vTarGet but visible to waypoint 2

	short int iTarGet = (short int)CWaypointLocations::NearestWaypoint(vTarGet, BLAST_RADIUS, -1, true, true);
	CWaypoint *pTarGet = CWaypoints::GetWaypoint(iTarGet);
	//vector<short int> waypointlist;
	int inearest = -1;

	if (pTarGet == NULL)
		return NULL;

	CWaypointVisibilityTable *pTable = CWaypoints::GetVisiblity();

	register short int NumWaypoints = (short int)m_iNumWaypoints;

	float finearestdist = 9999.0f;
	float fjnearestdist = 9999.0f;
	float fidist;
	float fjdist;

	CWaypoint *pTempi, *pTempj;

	for (register short int i = 0; i < NumWaypoints; i++)
	{
		if (iTarGet == i)
			continue;

		pTempi = CWaypoints::GetWaypoint(i);

		if ((fidist = pTarGet->DistanceFrom(pTempi->GetOrigin())) > finearestdist)
			continue;

		if (pTable->GetVisibilityFromTo((int)iTarGet, (int)i))
		{
			for (register short int j = 0; j < NumWaypoints; j++)
			{
				if (j == i)
					continue;
				if (j == iTarGet)
					continue;

				pTempj = CWaypoints::GetWaypoint(j);

				if ((fjdist = pTempj->DistanceFrom(vOrigin)) > fjnearestdist)
					continue;

				if (pTable->GetVisibilityFromTo((int)i, (int)j) && !pTable->GetVisibilityFromTo((int)iTarGet, (int)j))
				{
					finearestdist = fidist;
					fjnearestdist = fjdist;
					inearest = j;
					*iAiming = i;
				}
			}
		}
	}

	return CWaypoints::GetWaypoint(inearest);

}

void CWaypoints::AutoFix(bool bAutoFixNonArea)
{
	int *iNumAreas = CTeamFortress2Mod::m_ObjectiveResource.m_iNumControlPoints;
	int iNumCps;

	if (iNumAreas == NULL)
		return;

	iNumCps = *iNumAreas + 1;

	for (int i = 0; i < NumWaypoints(); i++)
	{
		if (m_theWaypoints[i].IsUsed() && (m_theWaypoints[i].GetFlags() > 0))
		{
			if ((m_theWaypoints[i].GetArea() > iNumCps) || (bAutoFixNonArea && (m_theWaypoints[i].GetArea() == 0) && m_theWaypoints[i].HasSomeFlags(CWaypointTypes::W_FL_SENTRY | CWaypointTypes::W_FL_DEFEND | CWaypointTypes::W_FL_SNIPER | CWaypointTypes::W_FL_CAPPOINT | CWaypointTypes::W_FL_TELE_EXIT)))
			{
				m_theWaypoints[i].SetArea(CTeamFortress2Mod::m_ObjectiveResource.NearestArea(m_theWaypoints[i].GetOrigin()));
				CBotGlobals::BotMessage(NULL, 0, "Changed Waypoint id %d area to (area = %d)", i, m_theWaypoints[i].GetArea());
			}
		}
	}
}

void CWaypoints::CheckAreas(edict_t *pActivator)
{
	int *iNumAreas = CTeamFortress2Mod::m_ObjectiveResource.m_iNumControlPoints;
	int iNumCps;

	if (iNumAreas == NULL)
		return;

	iNumCps = *iNumAreas + 1;

	for (int i = 0; i < NumWaypoints(); i++)
	{
		if (m_theWaypoints[i].IsUsed() && (m_theWaypoints[i].GetFlags() > 0))
		{
			if (m_theWaypoints[i].GetArea() > iNumCps)
			{
				CBotGlobals::BotMessage(pActivator, 0, "Invalid Waypoint id %d (area = %d)", i, m_theWaypoints[i].GetArea());
			}
		}
	}
}

CWaypoint *CWaypoints::RandomWaypointGoalNearestArea(int iFlags, int iTeam, int iArea, bool bForceArea, CBot *pBot, bool bHighDanger, Vector *origin, int iIgnore, bool bIgnoreBelief, int iWpt1)
{
	register short int i;
	static short int size;
	CWaypoint *pWpt;
	AStarNode *node;
	float fDist;

	size = NumWaypoints();

	dataUnconstArray<AStarNode*> goals;

	if (iWpt1 == -1)
		iWpt1 = CWaypointLocations::NearestWaypoint(*origin, 200, -1);

	for (i = 0; i < size; i++)
	{
		if (i == iIgnore)
			continue;

		pWpt = &m_theWaypoints[i];

		if (pWpt->IsUsed() && pWpt->ForTeam(iTeam))// && (pWpt->GetArea() == iArea) )
		{
			if ((iFlags == -1) || pWpt->HasSomeFlags(iFlags))
			{
				if (!bForceArea && !CTeamFortress2Mod::m_ObjectiveResource.IsWaypointAreaValid(pWpt->GetArea()))
					continue;
				else if (bForceArea && (pWpt->GetArea() != iArea))
					continue;

				node = new AStarNode();

				if (iWpt1 != -1)
				{
					fDist = CWaypointDistances::GetDistance(iWpt1, i);
				}
				else
				{
					fDist = pWpt->DistanceFrom(*origin);
				}

				if (fDist == 0.0f)
					fDist = 0.1f;

				node->SetWaypoint(i);
				node->SetHeuristic(131072.0f / (fDist*fDist));

				goals.Add(node);
			}
		}
	}

	pWpt = NULL;

	if (!goals.IsEmpty())
	{
		if (pBot)
		{
			CWaypointNavigator *pNav;

			pNav = (CWaypointNavigator*)pBot->GetNavigator();

			pWpt = pNav->ChooseBestFromBeliefBetweenAreas(&goals, bHighDanger, bIgnoreBelief);
		}
		else
			pWpt = CWaypoints::GetWaypoint(goals.Random()->GetWaypoint());

		//pWpt = goals.Random();
	}

	for (i = 0; i < goals.Size(); i++)
	{
		node = goals.ReturnValueFromIndex(i);

		delete node;
	}

	goals.Clear();

	return pWpt;
}

CWaypoint *CWaypoints::RandomWaypointGoalBetweenArea(int iFlags, int iTeam, int iArea, bool bForceArea, CBot *pBot, bool bHighDanger, Vector *org1, Vector *org2, bool bIgnoreBelief, int iWpt1, int iWpt2)
{
	register short int i;
	static short int size;
	CWaypoint *pWpt;
	AStarNode *node;
	float fCost = 0;

	if (iWpt1 == -1)
		iWpt1 = CWaypointLocations::NearestWaypoint(*org1, 200, -1);
	if (iWpt2 == -1)
		iWpt2 = CWaypointLocations::NearestWaypoint(*org2, 200, -1);

	size = NumWaypoints();

	dataUnconstArray<AStarNode*> goals;

	for (i = 0; i < size; i++)
	{
		pWpt = &m_theWaypoints[i];

		if (pWpt->IsUsed() && pWpt->ForTeam(iTeam))// && (pWpt->GetArea() == iArea) )
		{
			if ((iFlags == -1) || pWpt->HasSomeFlags(iFlags))
			{

				if (!bForceArea && !CTeamFortress2Mod::m_ObjectiveResource.IsWaypointAreaValid(pWpt->GetArea()))
					continue;
				else if (bForceArea && (pWpt->GetArea() != iArea))
					continue;

				fCost = 0;

				node = new AStarNode();

				node->SetWaypoint(i);

				if (iWpt1 != -1)
					fCost = 131072.0f / CWaypointDistances::GetDistance(iWpt1, i);
				else
					fCost = 131072.0f / pWpt->DistanceFrom(*org1);

				if (iWpt2 != -1)
					fCost += 131072.0f / CWaypointDistances::GetDistance(iWpt2, i);
				else
					fCost += 131072.0f / pWpt->DistanceFrom(*org2);

				node->SetHeuristic(fCost);

				goals.Add(node);
			}
		}
	}

	pWpt = NULL;

	if (!goals.IsEmpty())
	{
		if (pBot)
		{
			CWaypointNavigator *pNav;

			pNav = (CWaypointNavigator*)pBot->GetNavigator();

			pWpt = pNav->ChooseBestFromBeliefBetweenAreas(&goals, bHighDanger, bIgnoreBelief);
		}
		else
			pWpt = CWaypoints::GetWaypoint(goals.Random()->GetWaypoint());

		//pWpt = goals.Random();
	}

	for (i = 0; i < goals.Size(); i++)
	{
		node = goals.ReturnValueFromIndex(i);

		delete node;
	}

	goals.Clear();

	return pWpt;
}

CWaypoint *CWaypoints::RandomWaypointGoal(int iFlags, int iTeam, int iArea, bool bForceArea, CBot *pBot, bool bHighDanger, int iSearchFlags, int iIgnore)
{
	register short int i;
	static short int size;
	CWaypoint *pWpt;

	size = NumWaypoints();

	dataUnconstArray<CWaypoint*> goals;

	for (i = 0; i < size; i++)
	{
		if (iIgnore == i)
			continue;

		pWpt = &m_theWaypoints[i];

		if (pWpt->IsUsed() && pWpt->ForTeam(iTeam))// && (pWpt->GetArea() == iArea) )
		{
			if ((iFlags == -1) || pWpt->HasSomeFlags(iFlags))
			{
				if (!bForceArea && !CTeamFortress2Mod::m_ObjectiveResource.IsWaypointAreaValid(pWpt->GetArea()))
					continue;
				else if (bForceArea && (pWpt->GetArea() != iArea))
					continue;

				goals.Add(pWpt);
			}
		}
	}

	pWpt = NULL;

	if (!goals.IsEmpty())
	{
		if (pBot)
		{
			CWaypointNavigator *pNav;

			pNav = (CWaypointNavigator*)pBot->GetNavigator();

			pWpt = pNav->ChooseBestFromBelief(&goals, bHighDanger, iSearchFlags);
		}
		else
			pWpt = goals.Random();

		//pWpt = goals.Random();
	}

	goals.Clear();

	return pWpt;
}

int CWaypoints::RandomFlaggedWaypoint(int iTeam)
{
	return GetWaypointIndex(RandomWaypointGoal(-1, iTeam));
}

///////////

// Get the next free slot to save a waypoint to
int CWaypoints::FreeWaypointIndex()
{
	for (int i = 0; i < MAX_WAYPOINTS; i++)
	{
		if (!m_theWaypoints[i].IsUsed())
			return i;
	}

	return -1;
}

bool CWaypoint::CheckReachable()
{
	if (m_fCheckReachableTime < engine->Time())
	{
		CWaypoint *pOther;
		int NumPathsTo = NumPathsToThisWaypoint();
		int i;

		for (i = 0; i < NumPathsTo; i++)
		{
			pOther = CWaypoints::GetWaypoint(GetPathToThisWaypoint(i));

			if (pOther->GetFlags() == 0)
				break;

			if (pOther->GetFlags() & CWaypointTypes::W_FL_WAIT_GROUND)
			{
				if (pOther->CheckGround())
					break;
			}

			if (GetFlags() & CWaypointTypes::W_FL_OPENS_LATER)
			{
				if (pOther->IsPathOpened(m_vOrigin))
					break;
			}
		}

		m_bIsReachable = !(i == NumPathsTo);
		m_fCheckReachableTime = engine->Time() + 1.0f;
	}

	return m_bIsReachable;
}

int CWaypoint::NumPaths()
{
	return m_thePaths.Size();
}

int CWaypoint::GetPath(int i)
{
	return m_thePaths.ReturnValueFromIndex(i);
}

bool CWaypoint::IsPathOpened(Vector vPath)
{
	wpt_opens_later_t *info;

	for (int i = 0; i < m_OpensLaterInfo.Size(); i++)
	{
		info = m_OpensLaterInfo.ReturnPointerFromIndex(i);

		if (info->vOrigin == vPath)
		{
			if (info->fNextCheck < engine->Time())
			{
				info->bVisibleLastCheck = CBotGlobals::CheckOpensLater(m_vOrigin, vPath);

				info->fNextCheck = engine->Time() + 2.0f;
			}

			return info->bVisibleLastCheck;
		}
	}

	// not found -- add now
	wpt_opens_later_t newinfo;

	newinfo.fNextCheck = engine->Time() + 2.0f;
	newinfo.vOrigin = vPath;
	newinfo.bVisibleLastCheck = CBotGlobals::CheckOpensLater(m_vOrigin, vPath);

	m_OpensLaterInfo.Add(newinfo);

	return newinfo.bVisibleLastCheck;
}

void CWaypoint::AddPathFrom(int iWaypointIndex)
{
	m_PathsTo.Add(iWaypointIndex);
}

void CWaypoint::RemovePathFrom(int iWaypointIndex)
{
	m_PathsTo.Remove(iWaypointIndex);
}

int CWaypoint::NumPathsToThisWaypoint()
{
	return m_PathsTo.Size();
}

int CWaypoint::GetPathToThisWaypoint(int i)
{
	return m_PathsTo.ReturnValueFromIndex(i);
}

bool CWaypoint::AddPathTo(int iWaypointIndex)
{
	CWaypoint *pTo = CWaypoints::GetWaypoint(iWaypointIndex);

	if (pTo == NULL)
		return false;
	// already in list
	if (m_thePaths.IsMember(iWaypointIndex))
		return false;
	// dont have a path loop
	if (this == pTo)
		return false;

	m_thePaths.Add(iWaypointIndex);

	pTo->AddPathFrom(CWaypoints::GetWaypointIndex(this));

	return true;
}

Vector CWaypoint::ApplyRadius()
{
	if (m_fRadius > 0)
		return Vector(RandomFloat(-m_fRadius, m_fRadius), RandomFloat(m_fRadius, m_fRadius), 0);

	return Vector(0, 0, 0);
}

void CWaypoint::RemovePathTo(int iWaypointIndex)
{
	CWaypoint *pOther = CWaypoints::GetWaypoint(iWaypointIndex);

	if (pOther != NULL)
	{
		m_thePaths.Remove(iWaypointIndex);

		pOther->RemovePathFrom(CWaypoints::GetWaypointIndex(this));
	}

	return;
}

void CWaypoint::Info(edict_t *pEdict)
{
	CWaypointTypes::PrintInfo(this, pEdict);
}

bool CWaypoint::IsAiming()
{
	return (m_iFlags & (CWaypointTypes::W_FL_DEFEND |
		CWaypointTypes::W_FL_ROCKET_JUMP |
		CWaypointTypes::W_FL_DOUBLEJUMP |
		CWaypointTypes::W_FL_SENTRY | // or machine gun (DOD)
		CWaypointTypes::W_FL_SNIPER |
		CWaypointTypes::W_FL_TELE_EXIT |
		CWaypointTypes::W_FL_TELE_ENTRANCE)) > 0;
}

/////////////////////////////////////
// Waypoint Types
/////////////////////////////////////

CWaypointType *CWaypointTypes::GetType(const char *szType)
{
	for (unsigned int i = 0; i < m_Types.size(); i++)
	{
		if (FStrEq(m_Types[i]->GetName(), szType))
			return m_Types[i];
	}

	return NULL;
}

void CWaypointTypes::AddType(CWaypointType *type)
{
	m_Types.push_back(type);
}

CWaypointType *CWaypointTypes::GetTypeByIndex(unsigned int iIndex)
{
	if (iIndex < m_Types.size())
	{
		return m_Types[iIndex];
	}
	else
		return NULL;
}

CWaypointType *CWaypointTypes::GetTypeByFlags(int iFlags)
{
	CBotMod *pMod = CBotGlobals::GetCurrentMod();

	for (unsigned int i = 0; i < m_Types.size(); i++)
	{
		if (!m_Types[i]->ForMod(pMod->GetModId()))
			continue;

		if (m_Types[i]->GetBits() == iFlags)
			return m_Types[i];
	}

	return NULL;
}

unsigned int CWaypointTypes::GetNumTypes()
{
	return m_Types.size();
}

void CWaypointTypes::Setup()
{
	AddType(new CWaypointType(W_FL_NOBLU, "noblueteam", "TF2 blue team can't use this waypoint", (1 << MOD_TF2)));
	AddType(new CWaypointType(W_FL_NOALLIES, "noallies", "DOD allies team can't use this waypoint", (1 << MOD_DOD)));

	AddType(new CWaypointType(W_FL_FLAG, "flag", "bot will find a flag here", (1 << MOD_TF2)));
	AddType(new CWaypointType(W_FL_HEALTH, "health", "bot can sometimes Get health here", (1 << MOD_TF2) | (1 << MOD_HLDM2)));
	AddType(new CWaypointType(W_FL_ROCKET_JUMP, "rocketjump", "TF2 a bot can rocket jump here", (1 << MOD_TF2)));
	AddType(new CWaypointType(W_FL_AMMO, "ammo", "bot can sometimes Get ammo here", (1 << MOD_TF2) | (1 << MOD_HLDM2)));
	AddType(new CWaypointType(W_FL_RESUPPLY, "resupply", "TF2 bot can always Get ammo and health here", (1 << MOD_TF2)));
	AddType(new CWaypointType(W_FL_SENTRY, "sentry", "TF2 engineer bot can build here", (1 << MOD_TF2)));
	AddType(new CWaypointType(W_FL_DOUBLEJUMP, "doublejump", "TF2 scout can double jump here", (1 << MOD_TF2)));
	AddType(new CWaypointType(W_FL_TELE_ENTRANCE, "teleentrance", "TF2 engineer bot can build tele entrance here", (1 << MOD_TF2)));
	AddType(new CWaypointType(W_FL_TELE_EXIT, "teleexit", "TF2 engineer bot can build tele exit here", (1 << MOD_TF2)));
	AddType(new CWaypointType(W_FL_AREAONLY, "areaonly", "bot will only use this waypoint at certain areas of map", (1 << MOD_TF2)));
	AddType(new CWaypointType(W_FL_ROUTE, "route", "bot will attempt to go through one of these", (1 << MOD_TF2) | (1 << MOD_DOD)));
	AddType(new CWaypointType(W_FL_NO_FLAG, "noflag", "TF2 bot will lose flag if he goes thorugh here", (1 << MOD_TF2)));
	AddType(new CWaypointType(W_FL_COVER_RELOAD, "cover_reload", "DOD:S bots can take cover here while shooting an enemy and reload. They can also stand up and shoot the enemy after reloading", (1 << MOD_DOD)));
	AddType(new CWaypointType(W_FL_FLAGONLY, "flagonly", "TF2 bot needs the flag to go through here", (1 << MOD_TF2)));

	AddType(new CWaypointType(W_FL_NORED, "noredteam", "TF2 red team can't use this waypoint", (1 << MOD_TF2)));
	AddType(new CWaypointType(W_FL_NOAXIS, "noaxis", "DOD axis team can't use this waypoint", (1 << MOD_DOD)));
	AddType(new CWaypointType(W_FL_DEFEND, "defend", "bot will defend at this position"));
	AddType(new CWaypointType(W_FL_SNIPER, "sniper", "a bot can snipe here"));
	AddType(new CWaypointType(W_FL_MACHINEGUN, "machinegun", "DOD machine gunner will deploy gun here", (1 << MOD_DOD)));
	AddType(new CWaypointType(W_FL_CROUCH, "crouch", "bot will duck here"));
	AddType(new CWaypointType(W_FL_PRONE, "prone", "DOD:S bots prone here", (1 << MOD_DOD)));
	AddType(new CWaypointType(W_FL_JUMP, "jump", "bot will jump here"));

	AddType(new CWaypointType(W_FL_UNREACHABLE, "unreachable", "bot can't go here (used for visibility purposes only)"));
	AddType(new CWaypointType(W_FL_LADDER, "ladder", "bot will climb a ladder here"));
	AddType(new CWaypointType(W_FL_FALL, "fall", "Bots might kill themselves if they fall down here with low health"));
	AddType(new CWaypointType(W_FL_CAPPOINT, "capture", "TF2/DOD bot will find a capture point here"));
	AddType(new CWaypointType(W_FL_BOMBS_HERE, "bombs", "DOD bots can pickup bombs here", (1 << MOD_DOD)));
	AddType(new CWaypointType(W_FL_BOMB_TO_OPEN, "bombtoopen", "DOD:S bot needs to blow up this point to move on", (1 << MOD_DOD)));
	AddType(new CWaypointType(W_FL_BREAKABLE, "breakable", "Bots need to break something with a rocket to Get through here", (1 << MOD_DOD)));
	AddType(new CWaypointType(W_FL_OPENS_LATER, "openslater", "this waypoint is available when a door is open only"));
	AddType(new CWaypointType(W_FL_WAIT_GROUND, "waitground", "bot will wait until there is ground below"));
	AddType(new CWaypointType(W_FL_LIFT, "lift", "bot needs to wait on a lift here"));

	AddType(new CWaypointType(W_FL_SPRINT, "sprint", "bots will sprint here", ((1 << MOD_DOD) | (1 << MOD_HLDM2))));
	AddType(new CWaypointType(W_FL_TELEPORT_CHEAT, "teleport", "bots will teleport to the next waypoint (cheat)"));
	AddType(new CWaypointType(W_FL_OWNER_ONLY, "owneronly", "only bot teams who own the area of the waypoint can use it"));

	//AddType(new CWaypointType(W_FL_ATTACKPOINT,"squad_attackpoint","Tactical waypoint -- each squad will go to different attack points and signal others to go",WptColor(90,90,90)));
}

void CWaypointTypes::FreeMemory()
{
	for (unsigned int i = 0; i < m_Types.size(); i++)
	{
		delete m_Types[i];
		m_Types[i] = NULL;
	}

	m_Types.clear();
}

void CWaypointTypes::PrintInfo(CWaypoint *pWpt, edict_t *pPrintTo, float duration)
{
	CBotMod *pCurrentMod = CBotGlobals::GetCurrentMod();
	char szMessage[1024];
	Q_snprintf(szMessage, 1024, "Waypoint ID %d (Area = %d | Radius = %0.1f)[", CWaypoints::GetWaypointIndex(pWpt), pWpt->GetArea(), pWpt->GetRadius());

	if (pWpt->GetFlags())
	{
		bool bComma = false;

		for (unsigned int i = 0; i < m_Types.size(); i++)
		{
			if (m_Types[i]->ForMod(pCurrentMod->GetModId()) && m_Types[i]->IsBitsInFlags(pWpt->GetFlags()))
			{
				if (bComma)
					strcat(szMessage, ",");

				strcat(szMessage, m_Types[i]->GetName());
				//strcat(szMessage," (");
				//strcat(szMessage,m_Types[i]->GetDescription());
				//strcat(szMessage,")");				
				bComma = true;
			}
		}
	}
	else
	{
		strcat(szMessage, "No Waypoint Types");
	}

	strcat(szMessage, "]");

#ifndef __linux__
	debugoverlay->AddTextOverlay(pWpt->GetOrigin() + Vector(0, 0, 24), duration, szMessage);
#endif
	//CRCBotPlugin :: HudTextMessage (pPrintTo,"wptinfo","Waypoint Info",szMessage,Color(255,0,0,255),1,2);
}

CWaypointType::CWaypointType(int iBit, const char *szName, const char *szDescription, int iModBits, int iImportance)
{
	m_iBit = iBit;
	m_szName = CStrings::GetString(szName);
	m_szDescription = CStrings::GetString(szDescription);
	m_iMods = iModBits;
	m_iImportance = iImportance;
}

bool CWaypoint::ForTeam(int iTeam)
{
	CBotMod *pMod = CBotGlobals::GetCurrentMod();

	return pMod->CheckWaypointForTeam(this, iTeam);
}

class CTestBot : public CBotTF2
{
public:
	CTestBot(edict_t *pEdict, int iTeam, int iClass)
	{
		Init();
		strcpy(m_szBotName, "Test Bot");
		m_iClass = (TFClass)iClass;
		m_iTeam = iTeam;
		m_pEdict = pEdict;
		Setup();
	}
};

void CWaypointTest::Go(edict_t *pPlayer)
{
	int i, j;
	int iCheck = 0;
	//int iCurrentArea = 0;
	CWaypoint *pWpt1;
	CWaypoint *pWpt2;

	IBotNavigator *pNav;
	CBot *pBots[2];
	CBot *pBot;

	pBots[0] = new CTestBot(pPlayer, 2, 9);
	pBots[1] = new CTestBot(pPlayer, 3, 9);

	int iBot = 0;

	for (iBot = 0; iBot < 2; iBot++)
	{
		pBot = pBots[iBot];

		pNav = pBot->GetNavigator();

		for (i = 0; i < CWaypoints::MAX_WAYPOINTS; i++)
		{
			pWpt1 = CWaypoints::GetWaypoint(i);

			iCheck = 0;

			if (!pWpt1->ForTeam(iBot + 2))
				continue;

			if (!pBot->CanGotoWaypoint(Vector(0, 0, 0), pWpt1))
				continue;

			// simulate bot situations on the map
			// e.g. bot is at sentry point A wanting more ammo at resupply X
			if (pWpt1->HasFlag(CWaypointTypes::W_FL_SENTRY))
				iCheck = CWaypointTypes::W_FL_RESUPPLY | CWaypointTypes::W_FL_AMMO;
			if (pWpt1->HasSomeFlags(CWaypointTypes::W_FL_RESUPPLY | CWaypointTypes::W_FL_AMMO))
				iCheck = CWaypointTypes::W_FL_SENTRY | CWaypointTypes::W_FL_TELE_ENTRANCE | CWaypointTypes::W_FL_TELE_EXIT;

			if (iCheck != 0)
			{
				for (j = 0; j < CWaypoints::MAX_WAYPOINTS; j++)
				{

					if (i == j)
						continue;

					pWpt2 = CWaypoints::GetWaypoint(j);

					if (!pWpt2->ForTeam(iBot + 2))
						continue;

					pWpt2 = CWaypoints::GetWaypoint(j);

					if (!pBot->CanGotoWaypoint(Vector(0, 0, 0), pWpt2))
						continue;

					if ((pWpt2->GetArea() != 0) && (pWpt2->GetArea() != pWpt1->GetArea()))
						continue;

					if (pWpt2->HasSomeFlags(iCheck))
					{
						bool bfail = false;
						bool brestart = true;
						bool bnointerruptions = true;

						while (pNav->WorkRoute(
							pWpt1->GetOrigin(),
							pWpt2->GetOrigin(),
							&bfail,
							brestart,
							bnointerruptions, j)
							==
							false
							);

						if (bfail)
						{
							// log this one
							CBotGlobals::BotMessage(pPlayer, 0, "Waypoint Test: Route fail from '%d' to '%d'", i, j);
						}
					}
				}
			}
		}

	}

	delete pBots[0];
	delete pBots[1];
}
