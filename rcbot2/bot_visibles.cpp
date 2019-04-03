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
#include "bot_visibles.h"
#include "bot_genclass.h"
#include "bot_globals.h"
#include "bot_profile.h"
//#include "bot_profiling.h"
#include "bot_getprop.h"


#ifndef __linux__
#include <ndebugoverlay.h>
#endif

extern ConVar bot_visrevs;
extern ConVar bot_visrevs_clients;
////////////////////////////////////////////

byte CBotVisibles::m_bPvs[MAX_MAP_CLUSTERS / 8];

////////////////////////////////////////

/*
void CTF2FindFlagFunc::Execute(edict_t *pEntity)
{
if (m_pBot->
if (strcmp(pEntity->GetClassName(),"");
}

void CTF2FindFlagFunc::Init()
{
m_pBest = NULL;
m_fBestFactor = 0;
}*/


////////////////////////////////////////

void CFindEnemyFunc::Execute(edict_t *pEntity)
{
	if (m_pBot->IsEnemy(pEntity))
	{
		float fFactor = GetFactor(pEntity);

		if (!m_pBest || (fFactor < m_fBestFactor))
		{
			m_pBest = pEntity;
			m_fBestFactor = fFactor;
		}
	}
}

float CFindEnemyFunc::GetFactor(edict_t *pEntity)
{
	return m_pBot->GetEnemyFactor(pEntity);
}

void CFindEnemyFunc::SetOldEnemy(edict_t *pEntity)
{
	m_pBest = pEntity;
	m_fBestFactor = GetFactor(pEntity);
}

void CFindEnemyFunc::Init()
{
	m_pBest = NULL;
	m_fBestFactor = 0;
}

///////////////////////////////////////////

CBotVisibles::CBotVisibles(CBot *pBot)
{
	m_pBot = pBot;
	m_iMaxIndex = gpGlobals->maxEntities;
	m_iMaxSize = (m_iMaxIndex / 8) + 1;
	m_iIndicesVisible = new unsigned char[m_iMaxSize];
	Reset();
}

CBotVisibles::~CBotVisibles()
{
	m_pBot = NULL;
	delete[] m_iIndicesVisible;
	m_iIndicesVisible = NULL;
}

void CBotVisibles::EachVisible(CVisibleFunc *pFunc)
{
	dataStack<edict_t*> tempStack = m_VisibleList;
	edict_t *pEnt;

	while (!tempStack.IsEmpty())
	{
		pEnt = tempStack.ChooseFromStack();

		if (!pEnt->IsFree())
			pFunc->Execute(pEnt);
	}
}

void CBotVisibles::Reset()
{
	memset(m_iIndicesVisible, 0, sizeof(unsigned char)*m_iMaxSize);
	m_VisibleList.Destroy();
	m_iCurrentIndex = gpGlobals->maxClients + 1;
	m_iCurPlayer = 1;
}

void CBotVisibles::DebugString(char *string)
{
	//char szEntities[1024];
	char szNum[10];

	string[0] = 0;

	dataStack<edict_t*> tempStack = m_VisibleList;

	while (!tempStack.IsEmpty())
	{
		edict_t *pEnt = tempStack.ChooseFromStack();

		if (!pEnt)
			continue;

		sprintf(szNum, "%d,", ENTINDEX(pEnt));
		strcat(string, szNum);
	}
}
/*
@param	pEntity		entity to check
@param	iTicks		the pointer to the bot's traceline ticker
@param	bVisible	returns if the entity is visible or not
@param  iIndex      saves recalling INDEXENT
*/
void CBotVisibles::CheckVisible(edict_t *pEntity, int *iTicks, bool *bVisible, int &iIndex, bool bCheckHead)
{
	// make these static, calling a function with data many times	
	//static Vector vectorSurroundMins, vectorSurroundMaxs;
	static Vector vEntityOrigin;
	static int clusterIndex;
	static bool playerInPVS;

	// reset
	*bVisible = false;

	// update
	if (CBotGlobals::EntityIsValid(pEntity))
	{
		// if in view cone
		if (m_pBot->FInViewCone(pEntity))
		{
			// update tick -- counts the number of PVS done (cpu intensive)
			*iTicks = *iTicks + 1;

			*bVisible = m_pBot->FVisible(pEntity, bCheckHead);
		}
	}
}

void CBotVisibles::UpdateVisibles()
{
	static bool bVisible;
	static edict_t *pEntity;
	static edict_t *pGroundEntity;
	extern ConVar bot_supermode;

	static int iTicks;
	static int iMaxTicks;  //m_pBot->getProfile()->getVisionTicks();
	static int iStartIndex;
	static int iMaxClientTicks;
	static int iStartPlayerIndex;
	static int iSpecialIndex;

	//update ground entity
	pGroundEntity = CClassInterface::GetGroundEntity(m_pBot->GetEdict());

	if (pGroundEntity && (ENTINDEX(pGroundEntity) > 0))
	{
		SetVisible(pGroundEntity, true);
		m_pBot->SetVisible(pGroundEntity, true);
	}

	iTicks = 0;

	if (bot_supermode.GetBool())
		iMaxTicks = 100;
	else
		iMaxTicks = m_pBot->GetProfile()->m_iVisionTicks;// bot_visrevs.GetInt();

	iStartIndex = m_iCurrentIndex;

	if (bot_supermode.GetBool())
		iMaxClientTicks = (MAX_PLAYERS / 2) + 1;
	else
		iMaxClientTicks = m_pBot->GetProfile()->m_iVisionTicksClients; // bot_visrevs_clients.GetInt();

	if (iMaxTicks <= 2)
		iMaxTicks = 2;
	if (iMaxClientTicks < 1)
		iMaxClientTicks = 1;

	iStartPlayerIndex = m_iCurPlayer;

	if (m_pBot->MoveToIsValid())
	{
		Vector vMoveTo = *m_pBot->GetMoveTo();
		if (m_pBot->FVisible(vMoveTo))
			m_pBot->UpdateCondition(CONDITION_SEE_WAYPOINT);
		else
			m_pBot->RemoveCondition(CONDITION_SEE_WAYPOINT);
	}

	// we'll start searching some players first for quick player checking
	while (iTicks < iMaxClientTicks)
	{
		pEntity = INDEXENT(m_iCurPlayer);

		if (pEntity != pGroundEntity)
		{
			if (CBotGlobals::EntityIsValid(pEntity) && (pEntity != m_pBot->GetEdict()))
			{
				CheckVisible(pEntity, &iTicks, &bVisible, m_iCurPlayer);
				SetVisible(pEntity, bVisible);
				m_pBot->SetVisible(pEntity, bVisible);
			}
		}

		m_iCurPlayer++;

		if (m_iCurPlayer > gpGlobals->maxClients)
			m_iCurPlayer = 1;

		if (iStartPlayerIndex == m_iCurPlayer)
			break;
	}

	if (iMaxTicks > m_iMaxIndex)
		iMaxTicks = m_iMaxIndex;

	if (m_iCurPlayer >= m_iCurrentIndex)
		return;

	// get entities belonging to players too
	// we've captured them elsewhere in another data structure which is quicker to find 
	pEntity = m_pBot->GetVisibleSpecial();
	iSpecialIndex = 0;

	if (pEntity)
	{
		if (CBotGlobals::EntityIsValid(pEntity))
		{
			iSpecialIndex = ENTINDEX(pEntity);
			CheckVisible(pEntity, &iTicks, &bVisible, iSpecialIndex, true);

			SetVisible(pEntity, bVisible);
			m_pBot->SetVisible(pEntity, bVisible);
		}
	}

	while (iTicks < iMaxTicks)
	{
		bVisible = false;

		pEntity = INDEXENT(m_iCurrentIndex);

		if ((pEntity != pGroundEntity) && (m_iCurrentIndex != iSpecialIndex))
		{
			if (CBotGlobals::EntityIsValid(pEntity))
			{
				CheckVisible(pEntity, &iTicks, &bVisible, m_iCurrentIndex);

				SetVisible(pEntity, bVisible);
				m_pBot->SetVisible(pEntity, bVisible);
			}
		}

		m_iCurrentIndex++;

		if (m_iCurrentIndex >= m_iMaxIndex)
			m_iCurrentIndex = gpGlobals->maxClients + 1; // back to start of non clients

		if (m_iCurrentIndex == iStartIndex)
			break; // back to where we started
	}
}

bool CBotVisibles::IsVisible(edict_t *pEdict)
{
	static int iIndex;
	static int iByte;
	static int iBit;

	iIndex = ENTINDEX(pEdict) - 1;
	iByte = iIndex / 8;
	iBit = iIndex % 8;

	if (iIndex < 0)
		return false;

	if (iByte > m_iMaxSize)
		return false;

	return ((*(m_iIndicesVisible + iByte))&(1 << iBit)) == (1 << iBit);
}

void CBotVisibles::SetVisible(edict_t *pEdict, bool bVisible)
{
	static int iIndex;
	static int iByte;
	static int iBit;
	static int iFlag;

	iIndex = ENTINDEX(pEdict) - 1;
	iByte = iIndex / 8;
	iBit = iIndex % 8;
	iFlag = 1 << iBit;

	if (bVisible)
	{
		// visible now
		if (((*(m_iIndicesVisible + iByte) & iFlag) != iFlag))
			m_VisibleList.Push(pEdict);

		*(m_iIndicesVisible + iByte) |= iFlag;
	}
	else
	{
		// not visible anymore
		if (pEdict && ((*(m_iIndicesVisible + iByte) & iFlag) == iFlag))
			m_VisibleList.Remove(pEdict);

		*(m_iIndicesVisible + iByte) &= ~iFlag;
	}
}