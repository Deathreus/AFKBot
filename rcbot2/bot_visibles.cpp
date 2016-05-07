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
 *    link the code of this program with the Half-Life Game Engine ("HL
 *    Engine") and Modified Game Libraries ("MODs") developed by Valve,
 *    L.L.C ("Valve").  You must obey the GNU General Public License in all
 *    respects for all of the code used other than the HL Engine and MODs
 *    from Valve.  If you modify this file, you may extend this exception
 *    to your version of the file, but you are not obligated to do so.  If
 *    you do not wish to do so, delete this exception statement from your
 *    version.
 *
 */
#include "engine_wrappers.h"

#include "bot.h"
#include "bot_visibles.h"
#include "bot_genclass.h"
#include "bot_globals.h"
#include "bot_profile.h"
#include "bot_client.h"
#include "bot_profiling.h"
#include "bot_getprop.h"

#include "ndebugoverlay.h"

extern IVDebugOverlay *debugoverlay;
extern ConVar bot_visrevs;
extern ConVar bot_visrevs_clients;
////////////////////////////////////////////

byte CBotVisibles :: m_bPvs[MAX_MAP_CLUSTERS/8];

////////////////////////////////////////

/*
void CTF2FindFlagFunc :: execute ( edict_t *pEntity )
{
	if ( m_pBot->
	if ( strcmp(pEntity->GetClassName(),"");
}

void CTF2FindFlagFunc :: init ()
{
	m_pBest = NULL;
	m_fBestFactor = 0;
}*/


////////////////////////////////////////

void CFindEnemyFunc :: execute ( edict_t *pEntity )
{
	if ( m_pBot->isEnemy(pEntity) )
	{
		float fFactor = getFactor(pEntity);

		if ( !m_pBest || (fFactor < m_fBestFactor) )
		{
			m_pBest = pEntity;
			m_fBestFactor = fFactor;
		}
	}
}

float CFindEnemyFunc :: getFactor ( edict_t *pEntity )
{
	return m_pBot->getEnemyFactor(pEntity);
}

void CFindEnemyFunc :: setOldEnemy ( edict_t *pEntity )
{
	m_pBest = pEntity;
	m_fBestFactor = getFactor(pEntity);
}

void CFindEnemyFunc :: init ()
{
	m_pBest = NULL;
	m_fBestFactor = 0;
}

///////////////////////////////////////////

CBotVisibles :: CBotVisibles ( CBot *pBot ) 
{
	m_pBot = pBot;
	m_iMaxIndex = m_pBot->maxEntityIndex();
	m_iMaxSize = (m_iMaxIndex/8)+1;
	m_iIndicesVisible = new unsigned char [m_iMaxSize];
	reset();
}

CBotVisibles :: ~CBotVisibles () 
{
	m_pBot = NULL;
	delete[] m_iIndicesVisible;
	m_iIndicesVisible = NULL;
}

void CBotVisibles :: eachVisible ( CVisibleFunc *pFunc )
{
	dataStack<edict_t*> tempStack = m_VisibleList;
	edict_t *pEnt;

	while ( !tempStack.IsEmpty() )
	{
		pEnt = tempStack.ChooseFromStack();

		if ( !pEnt->IsFree() )
			pFunc->execute(pEnt);
	}
}

void CBotVisibles :: reset ()
{
	memset(m_iIndicesVisible,0,sizeof(unsigned char)*m_iMaxSize);
	m_VisibleList.Destroy();
	m_iCurrentIndex = CBotGlobals::maxClients()+1;
	m_iCurPlayer = 1;
}

void CBotVisibles :: debugString ( char *string )
{
	//char szEntities[1024];
	char szNum[10];

	string[0] = 0;

	dataStack<edict_t*> tempStack = m_VisibleList;

	while ( !tempStack.IsEmpty() )
	{
		edict_t *pEnt = tempStack.ChooseFromStack();

		if ( !pEnt )
			continue;

		sprintf(szNum,"%d,",ENTINDEX(pEnt));
		strcat(string,szNum);
	}
}
/*
@param	pEntity		entity to check
@param	iTicks		the pointer to the bot's traceline ticker
@param	bVisible	returns if the entity is visible or not
@param  iIndex      saves recalling INDEXENT
*/
void CBotVisibles :: checkVisible ( edict_t *pEntity, int *iTicks, bool *bVisible, int &iIndex, bool bCheckHead )
{
	// make these static, calling a function with data many times	
	//static Vector vectorSurroundMins, vectorSurroundMaxs;
	static Vector vEntityOrigin;
	static int clusterIndex;
	static bool playerInPVS;

	// reset
	*bVisible = false;

	// update
	if ( CBotGlobals::entityIsValid(pEntity) )
	{
		//if ( CClients::clientsDebugging() && CClients::get(0)->isDebuggingBot(m_pBot) && (ENTINDEX(pEntity)<CBotGlobals::maxClients()) )
		//	debugoverlay->AddLineOverlay(m_pBot->getOrigin(),CBotGlobals::entityOrigin(pEntity),255,255,255,false,1);			

		// if in view cone
		if ( m_pBot->FInViewCone(pEntity) )
		{
			// from Valve developer community wiki
			// http://developer.valvesoftware.com/wiki/Transforming_the_Multiplayer_SDK_into_Coop

			// update tick -- counts the number of PVS done (cpu intensive)
			*iTicks = *iTicks + 1;

			clusterIndex = engine->GetClusterForOrigin( m_pBot->getOrigin() );
			engine->GetPVSForCluster( clusterIndex, sizeof(m_bPvs), m_bPvs );
			
			vEntityOrigin = CBotGlobals::entityOrigin(pEntity);

			// for some reason the origin is their feet. add body height
			if ( iIndex <= gpGlobals->maxClients )
				vEntityOrigin + Vector(0,0,32);

			playerInPVS = engine->CheckOriginInPVS(vEntityOrigin,m_bPvs,sizeof(m_bPvs));//engine->CheckBoxInPVS( vectorSurroundMins, vectorSurroundMaxs, m_bPvs, sizeof( m_bPvs ) );

			if ( playerInPVS )
			{

				*bVisible = m_pBot->FVisible(pEntity,bCheckHead);

#ifndef __linux__
				if ( *bVisible )
				{
					if ( CClients::clientsDebugging(BOT_DEBUG_VIS) && CClients::get(0)->isDebuggingBot(m_pBot->getEdict()) && (ENTINDEX(pEntity)<=CBotGlobals::maxClients()))
						debugoverlay->AddTextOverlay(CBotGlobals::entityOrigin(pEntity),0,0.1,"VISIBLE");
				}
				else
				{
					if ( CClients::clientsDebugging(BOT_DEBUG_VIS) && CClients::get(0)->isDebuggingBot(m_pBot->getEdict()) && (ENTINDEX(pEntity)<=CBotGlobals::maxClients()))
						debugoverlay->AddTextOverlayRGB(CBotGlobals::entityOrigin(pEntity),0,0.1,255,0,0,200,"INVISIBLE");
				}
#endif
			}
			//else if ( CClients::clientsDebugging() && CClients::get(0)->isDebuggingBot(m_pBot) && (ENTINDEX(pEntity)<CBotGlobals::maxClients()))
			//	debugoverlay->AddTextOverlay(CBotGlobals::entityOrigin(pEntity),0,0.1,"INVISIBLE: playerInPVS false");
		}
		//else if ( CClients::clientsDebugging() && CClients::get(0)->isDebuggingBot(m_pBot) && (ENTINDEX(pEntity)<CBotGlobals::maxClients()) )
		//	debugoverlay->AddTextOverlay(CBotGlobals::entityOrigin(pEntity),0,0.1,"INVISIBLE: FInViewCone false");
	}
}

void CBotVisibles :: updateVisibles ()
{
	static bool bVisible;
	static edict_t *pEntity;
	static edict_t *pGroundEntity;
	extern ConVar rcbot_supermode;

	static int iTicks;
	static int iMaxTicks;  //m_pBot->getProfile()->getVisionTicks();
	static int iStartIndex;
	static int iMaxClientTicks; 
	static int iStartPlayerIndex;
	static int iSpecialIndex;

	//update ground entity
	pGroundEntity = CClassInterface::getGroundEntity(m_pBot->getEdict());

	if ( pGroundEntity && (ENTINDEX(pGroundEntity) > 0) )
	{
		setVisible(pGroundEntity,true);
		m_pBot->setVisible(pGroundEntity,true);
	}

	iTicks = 0;
	
	if ( rcbot_supermode.GetBool() )
		iMaxTicks = 100;
	else
		iMaxTicks = m_pBot->getProfile()->m_iVisionTicks;// bot_visrevs.GetInt();

	iStartIndex = m_iCurrentIndex;

	if ( rcbot_supermode.GetBool() )
		iMaxClientTicks = (gpGlobals->maxClients/2)+1;
	else
		iMaxClientTicks =m_pBot->getProfile()->m_iVisionTicksClients; // bot_visrevs_clients.GetInt();

	if ( iMaxTicks <= 2 )
		iMaxTicks = 2;
	if ( iMaxClientTicks < 1 )
		iMaxClientTicks = 1;

#ifdef _DEBUG
	CProfileTimer *timer = CProfileTimers::getTimer(BOT_VISION_TIMER);

	if ( CClients::clientsDebugging(BOT_DEBUG_PROFILE) )
	{
		timer->Start();
	}
#endif

	iStartPlayerIndex = m_iCurPlayer;

	if ( m_pBot->moveToIsValid() )
	{
		Vector vMoveTo = m_pBot->getMoveTo();
		if ( m_pBot->FVisible(vMoveTo) )
			m_pBot->updateCondition(CONDITION_SEE_WAYPOINT);
		else
			m_pBot->removeCondition(CONDITION_SEE_WAYPOINT);
	}

	// we'll start searching some players first for quick player checking
	while ( iTicks < iMaxClientTicks )
	{
		pEntity = INDEXENT(m_iCurPlayer);

		if ( pEntity != pGroundEntity )
		{
			if ( CBotGlobals::entityIsValid(pEntity) && (pEntity != m_pBot->getEdict()) )
			{
				checkVisible(pEntity,&iTicks,&bVisible,m_iCurPlayer);
				setVisible(pEntity,bVisible);
				m_pBot->setVisible(pEntity,bVisible);
			}
		}

		m_iCurPlayer++;

		if ( m_iCurPlayer > CBotGlobals::maxClients() )
			m_iCurPlayer = 1;

		if ( iStartPlayerIndex == m_iCurPlayer )
			break;
	}

	if ( iMaxTicks > m_iMaxIndex )
		iMaxTicks = m_iMaxIndex;

	if ( m_iCurPlayer >= m_iCurrentIndex )
		return;

	// get entities belonging to players too
	// we've captured them elsewhere in another data structure which is quicker to find 
	pEntity = m_pBot->getVisibleSpecial();
	iSpecialIndex = 0;

	if ( pEntity )
	{
		if ( CBotGlobals::entityIsValid(pEntity) )
		{		
			iSpecialIndex = ENTINDEX(pEntity);
			checkVisible(pEntity,&iTicks,&bVisible,iSpecialIndex,true);

			setVisible(pEntity,bVisible);
			m_pBot->setVisible(pEntity,bVisible);
		}
	}

	while ( iTicks < iMaxTicks )
	{
		bVisible = false;

		pEntity = INDEXENT(m_iCurrentIndex);

		if ( (pEntity != pGroundEntity) && (m_iCurrentIndex != iSpecialIndex)  )
		{
			if ( CBotGlobals::entityIsValid(pEntity) )
			{		
				checkVisible(pEntity,&iTicks,&bVisible,m_iCurrentIndex);

				setVisible(pEntity,bVisible);
				m_pBot->setVisible(pEntity,bVisible);
			}
		}

		m_iCurrentIndex ++;

		if ( m_iCurrentIndex >= m_iMaxIndex )
			m_iCurrentIndex = CBotGlobals::maxClients()+1; // back to start of non clients

		if ( m_iCurrentIndex == iStartIndex )
			break; // back to where we started
	}


#ifdef _DEBUG
	if ( CClients::clientsDebugging(BOT_DEBUG_PROFILE) )
	{
		timer->Stop();
	}
#endif
}

bool CBotVisibles :: isVisible ( edict_t *pEdict ) 
{ 
	static int iIndex;
	static int iByte;
	static int iBit;

	iIndex = ENTINDEX(pEdict)-1;
	iByte = iIndex/8;
	iBit = iIndex%8;

	if ( iIndex < 0 )
		return false;

	if ( iByte > m_iMaxSize )
		return false;

	return ( (*(m_iIndicesVisible+iByte))&(1<<iBit))==(1<<iBit);
}

void CBotVisibles :: setVisible ( edict_t *pEdict, bool bVisible ) 
{ 
	static int iIndex;
	static int iByte;
	static int iBit;
	static int iFlag;
	
	iIndex = ENTINDEX(pEdict)-1;
	iByte = iIndex/8;
	iBit = iIndex%8;
	iFlag = 1<<iBit;

	if ( bVisible )
	{
		// visible now
		if ( ((*(m_iIndicesVisible+iByte) & iFlag)!=iFlag) )
			m_VisibleList.Push(pEdict);

		*(m_iIndicesVisible+iByte) |= iFlag;		
	}
	else
	{
		// not visible anymore
		if ( pEdict && ((*(m_iIndicesVisible+iByte) & iFlag)==iFlag) )
			m_VisibleList.Remove(pEdict);

		*(m_iIndicesVisible+iByte) &= ~iFlag;		
	}
}
