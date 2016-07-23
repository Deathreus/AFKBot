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
#include "bot.h"
#include "bot_client.h"
#include "bot_waypoint_locations.h"
#include "bot_accessclient.h"
#include "bot_commands.h"
#include "bot_globals.h"
#include "bot_waypoint.h"
#include "ndebugoverlay.h"
#include "bot_menu.h"
// autowaypoint
#include "bot_getprop.h"
#include "bot_waypoint_locations.h"
//#include "bot_hooks.h"
#include "in_buttons.h"
#include "../extension.h"

// setup static client array
CClient CClients::m_Clients[MAX_PLAYERS];
CClient *CClients::m_pListenServerClient = NULL;
bool CClients::m_bClientsDebugging = false;

void CToolTip::Send(edict_t *pPlayer)
{
	//CRCBotPlugin::HudTextMessage(pPlayer,m_pszMessage);

	AFKBot::HudTextMessage(pPlayer, m_pszMessage);

	if (m_pszSound)
		engine->ClientCommand(pPlayer, "play %s", m_pszSound);
}

void CClient::Init()
{
	m_iWaypointShowFlags = 0;
	m_fMonitorHighFiveTime = 0;
	m_fSpeed = 0;
	m_pPlayer = NULL;
	m_szSteamID = NULL;
	m_bWaypointOn = false;
	m_iCurrentWaypoint = -1;
	m_iAccessLevel = 0;
	m_bAutoPaths = true;
	m_iPathFrom = -1;
	m_iPathTo = -1;
	m_bPathWaypointOn = false;
	m_iDebugLevels = 0;
	m_pPlayerInfo = NULL;
	m_iWptArea = 0;
	m_iWaypointDrawType = 3;
	m_bShowMenu = false;
	m_fUpdatePos = 0.0f;
	m_pDebugBot = NULL;
	m_fCopyWptRadius = 0.0f;
	m_iCopyWptFlags = 0;
	m_iCopyWptArea = 0;
	m_pMenu = NULL;
	m_iMenuCommand = -1;
	m_fNextUpdateMenuTime = 0.0f;

	while (!m_NextTooltip.empty())
		m_NextTooltip.pop();

	m_fNextBotServerMessage = 0;
	m_bSentWelcomeMessage = false;
	m_fSpeed = 0;
	m_fUpdatePos = 0;
}

bool CClient::NeedToRenderMenu()
{
	return m_fNextUpdateMenuTime < engine->Time();
}

void CClient::UpdateRenderMenuTime()
{
	extern ConVar bot_menu_update_time2;
	m_fNextUpdateMenuTime = engine->Time() + bot_menu_update_time2.GetFloat();
}

void CClient::SetEdict(edict_t *pPlayer)
{
	m_pPlayer = pPlayer;
	m_pPlayerInfo = playerinfomanager->GetPlayerInfo(pPlayer);
}

void CClient::SetupMenuCommands()
{
	/*engine->ClientCommand(m_pPlayer,"alias \"rcbot_setup\" \"bind 0 menuselect0\"");
	engine->ClientCommand(m_pPlayer,"rcbot_setup");bind 2 \"menuselect 2\"");*/
	engine->ClientCommand(m_pPlayer, "bind 1 \"menuselect 1\"");
	engine->ClientCommand(m_pPlayer, "bind 2 \"menuselect 2\"");
	engine->ClientCommand(m_pPlayer, "bind 3 \"menuselect 3\"");
	engine->ClientCommand(m_pPlayer, "bind 4 \"menuselect 4\"");
	engine->ClientCommand(m_pPlayer, "bind 5 \"menuselect 5\"");
	engine->ClientCommand(m_pPlayer, "bind 6 \"menuselect 6\"");
	engine->ClientCommand(m_pPlayer, "bind 7 \"menuselect 7\"");
	engine->ClientCommand(m_pPlayer, "bind 8 \"menuselect 8\"");
	engine->ClientCommand(m_pPlayer, "bind 9 \"menuselect 9\"");
	engine->ClientCommand(m_pPlayer, "bind 0 \"menuselect 0\"");
}

void CClient::ResetMenuCommands()
{
	/*engine->ClientCommand(m_pPlayer,"alias \"rcbot_reset\" \"bind 0 slot10\"");
	engine->ClientCommand(m_pPlayer,"rcbot_reset");bind 2 \"menuselect 2\"");*/
	engine->ClientCommand(m_pPlayer, "bind 1 \"slot1\"");
	engine->ClientCommand(m_pPlayer, "bind 2 \"slot2\"");
	engine->ClientCommand(m_pPlayer, "bind 3 \"slot3\"");
	engine->ClientCommand(m_pPlayer, "bind 4 \"slot4\"");
	engine->ClientCommand(m_pPlayer, "bind 5 \"slot5\"");
	engine->ClientCommand(m_pPlayer, "bind 6 \"slot6\"");
	engine->ClientCommand(m_pPlayer, "bind 7 \"slot7\"");
	engine->ClientCommand(m_pPlayer, "bind 8 \"slot8\"");
	engine->ClientCommand(m_pPlayer, "bind 9 \"slot9\"");
	engine->ClientCommand(m_pPlayer, "bind 0 \"slot10\"");
}

void CClient::PlaySound(const char *pszSound)
{
	extern ConVar bot_cmd_enable_wpt_sounds;

	if (IsWaypointOn())
	{
		if (bot_cmd_enable_wpt_sounds.GetBool())
			sprintf(m_szSoundToPlay, "play \"%s\"", pszSound);
	}
}

void CClient::AutoEventWaypoint(int iType, float fRadius, bool bAtOtherOrigin, int iTeam, Vector vOrigin, bool bIgnoreTeam, bool bAutoType)
{
	m_iAutoEventWaypoint = iType;
	m_fAutoEventWaypointRadius = fRadius;

	CBotMod *pMod = CBotGlobals::GetCurrentMod();

	m_bAutoEventWaypointAutoType = bAutoType;

	if (bAtOtherOrigin)
	{
		m_vAutoEventWaypointOrigin = vOrigin;
	}
	else
	{
		m_vAutoEventWaypointOrigin = GetOrigin();
		iTeam = CClassInterface::GetTeam(m_pPlayer);
	}

	if (bIgnoreTeam)
		m_iAutoEventWaypointTeam = 0;
	else
	{
		pMod->GetTeamOnlyWaypointFlags(iTeam, &m_iAutoEventWaypointTeamOn, &m_iAutoEventWaypointTeamOff);
		m_iAutoEventWaypointTeam = iTeam;
	}
}

void CClient::TeleportTo(Vector vOrigin)
{
	m_bIsTeleporting = true;
	m_fTeleportTime = engine->Time() + 0.1f;

	Vector *v_origin = CClassInterface::GetOrigin(m_pPlayer);

	byte *pMoveType = CClassInterface::GetMoveTypePointer(m_pPlayer);
	int *pPlayerFlags = CClassInterface::GetPlayerFlagsPointer(m_pPlayer);

	*pMoveType &= ~15;
	*pMoveType |= MOVETYPE_FLYGRAVITY;

	*pPlayerFlags &= ~FL_ONGROUND;
	*pPlayerFlags |= FL_FLY;

	*v_origin = vOrigin;
}

class CBotFunc_HighFiveSearch : public IBotFunction
{
public:
	CBotFunc_HighFiveSearch(edict_t *pPlayer, int iTeam)
	{
		m_pPlayer = pPlayer;
		m_iTeam = iTeam;
		m_pNearestBot = NULL;
		m_fNearestDist = 0;
	}

	void Execute(CBot *pBot)
	{
		if ((pBot->GetEdict() != m_pPlayer) && (pBot->GetTeam() == m_iTeam) && pBot->IsVisible(m_pPlayer))
		{
			float fDist = pBot->DistanceFrom(m_pPlayer);

			if (!m_pNearestBot || (fDist < m_fNearestDist))
			{
				m_pNearestBot = pBot;
				m_fNearestDist = fDist;
			}
		}
	}

	CBot *GetNearestBot()
	{
		return m_pNearestBot;
	}

private:
	edict_t *m_pPlayer;
	int m_iTeam;
	CBot *m_pNearestBot;
	float m_fNearestDist;
};

// called each frame
void CClient::Think()
{
	extern ConVar bot_cmd_enable_wpt_sounds;

	//if ( m_pPlayer  )
	//	HookGiveNamedItem(m_pPlayer);

	/*if ( m_pPlayer.get() == NULL )
	{
	clientDisconnected();
	return;
	}
	*/

	//if ( m_fMonitorHighFiveTime > engine->Time() )
	//{

	if ((m_pPlayer != NULL) && (m_pPlayerInfo == NULL))
	{
		m_pPlayerInfo = playerinfomanager->GetPlayerInfo(m_pPlayer);
	}

	if (CBotGlobals::IsMod(MOD_TF2))
	{
		if ((m_fMonitorHighFiveTime < engine->Time()) && (m_pPlayer != NULL) && (m_pPlayerInfo != NULL) && m_pPlayerInfo->IsConnected() &&
			!m_pPlayerInfo->IsDead() && m_pPlayerInfo->IsPlayer() && !m_pPlayerInfo->IsObserver() &&
			CClassInterface::GetTF2HighFiveReady(m_pPlayer))
		{
			m_fMonitorHighFiveTime = engine->Time() + 0.25f;

			if (CClassInterface::GetHighFivePartner(m_pPlayer) == NULL)
			{
				// wanting high five partner
				// search for bots nearby who can see this player
				CBotFunc_HighFiveSearch *newFunc = new CBotFunc_HighFiveSearch(m_pPlayer, CClassInterface::GetTeam(m_pPlayer));

				CBots::BotFunction(newFunc);

				CBot *pBot = newFunc->GetNearestBot();

				if (pBot != NULL)
				{
					((CBotTF2*)pBot)->HighFivePlayer(m_pPlayer, CClassInterface::GetTF2TauntYaw(m_pPlayer));
					m_fMonitorHighFiveTime = engine->Time() + 3.0f;
				}

				delete newFunc;
			}
		}
	}

	if (m_szSoundToPlay[0] != 0)
	{
		if (bot_cmd_enable_wpt_sounds.GetBool())
			engine->ClientCommand(m_pPlayer, m_szSoundToPlay);

		m_szSoundToPlay[0] = 0;
	}

	if (m_bIsTeleporting)
	{
		if (m_fTeleportTime < engine->Time())
		{
			m_bIsTeleporting = false;
			m_fTeleportTime = 0;
			//reset movetypes
			byte *pMoveType = CClassInterface::GetMoveTypePointer(m_pPlayer);
			int *pPlayerFlags = CClassInterface::GetPlayerFlagsPointer(m_pPlayer);

			*pMoveType &= ~15;
			*pMoveType |= MOVETYPE_WALK;

			*pPlayerFlags &= ~FL_FLY;
			*pPlayerFlags |= FL_ONGROUND;
		}
	}

	if (m_bShowMenu)
	{
		m_bShowMenu = false;
		engine->ClientCommand(m_pPlayer, "cancelselect");
	}

	if (m_pMenu != NULL)
	{
		if (NeedToRenderMenu())
			m_pMenu->Render(this);
		//CBotMenuList::render(pClient);
	}

	if (IsWaypointOn())
		CWaypoints::DrawWaypoints(this);

	if (m_fUpdatePos < engine->Time())
	{
		m_vVelocity = (GetOrigin() - m_vLastPos);
		m_fSpeed = m_vVelocity.Length();
		m_vLastPos = GetOrigin();

		m_fUpdatePos = engine->Time() + 1.0f;
	}

	if (IsDebugging())
	{
		IPlayerInfo *p = playerinfomanager->GetPlayerInfo(m_pPlayer);


		if (IsDebugOn(BOT_DEBUG_SPEED))
		{
			CBotGlobals::BotMessage(m_pPlayer, 0, "speed = %0.0f", m_fSpeed);
		}

		if (IsDebugOn(BOT_DEBUG_USERCMD))
		{

			if (p)
			{
				CUserCmd cmd = *CBots::GetBotPointer(m_pDebugBot)->GetUserCMD();

				CBotGlobals::BotMessage(m_pPlayer, 0, "Btns = %d, cmd_no = %d, impulse = %d, weapselect = %d, weapsub = %d", cmd.buttons, cmd.command_number, cmd.impulse, cmd.weaponselect, cmd.weaponsubtype);

			}
		}


		if ((m_pDebugBot != NULL) && IsDebugOn(BOT_DEBUG_HUD))
		{
			if (m_fNextPrintDebugInfo < engine->Time())
			{
				char msg[1024];
				CBot *pBot = CBots::GetBotPointer(m_pDebugBot);

				QAngle eyes = CBotGlobals::PlayerAngles(m_pDebugBot);
				Vector vForward;
				// in fov? Check angle to edict
				AngleVectors(eyes, &vForward);

				vForward = vForward / vForward.Length(); // normalize
				Vector vLeft = (vForward - p->GetAbsOrigin()).Cross(Vector(0, 0, 1));
				vLeft = vLeft / vLeft.Length();

				Vector vDisplay = p->GetAbsOrigin() + vForward*300.0f;
				vDisplay = vDisplay - vLeft*300.0f;

				// get debug message
				pBot->DebugBot(msg);

#ifndef __linux__
				int i = 0;
				int n = 0;
				char line[256];
				int linenum = 0;
				int iIndex = ENTINDEX(m_pDebugBot);

				do
				{
					while ((msg[i] != 0) && (msg[i] != '\n'))
						line[n++] = msg[i++];

					line[n] = 0;
					debugoverlay->AddEntityTextOverlay(iIndex, linenum++, 1.0f, 255, 255, 255, 255, line);
					n = 0;

					if (msg[i] == 0)
						break;
					i++;
				} while (1);
				//int ent_index, int line_offset, float duration, int r, int g, int b, int a, const char *format, ...
				//	debugoverlay->AddEntityTextOverlay();
#endif
				m_fNextPrintDebugInfo = engine->Time() + 1.0f;
			}
		}
		//this->cm_pDebugBot->getTaskDebug();
		//m_pDebugBot->canAvoid();
	}

	if (m_fNextBotServerMessage < engine->Time())
	{
		if (!m_NextTooltip.empty())
		{
			CToolTip *pTooltip = m_NextTooltip.front();

			pTooltip->Send(m_pPlayer);

			m_NextTooltip.pop();

			delete pTooltip;

			m_fNextBotServerMessage = engine->Time() + 11.0f;
		}
		else
			m_fNextBotServerMessage = engine->Time() + 1.0f;
	}


	/***** Autowaypoint stuff below borrowed and converted from RCBot1 *****/

	if (m_bAutoWaypoint)
	{
		if (!m_pPlayerInfo)
			m_pPlayerInfo = playerinfomanager->GetPlayerInfo(m_pPlayer);

		if (!m_bSetUpAutoWaypoint || !m_pPlayerInfo || m_pPlayerInfo->IsDead())
		{
			int i;
			int start = 0;

			if (!m_pPlayerInfo->IsDead())
				start = 1; // grab one location


			m_fLastAutoWaypointCheckTime = engine->Time() + 0.5f;

			if (!m_pPlayerInfo->IsDead())
				m_vLastAutoWaypointCheckPos[0].SetVector(GetOrigin());

			for (i = start; i < MAX_STORED_AUTOWAYPOINT; i++)
			{
				m_vLastAutoWaypointCheckPos[i].UnSetPoint();
			}

			m_vLastAutoWaypointPlacePos = GetOrigin();
			m_bSetUpAutoWaypoint = TRUE;
			m_fCanPlaceJump = 0;
			m_iLastButtons = 0;

			m_iLastJumpWaypointIndex = -1;
			m_iLastLadderWaypointIndex = -1;
			m_iLastMoveType = 0;
			m_fCanPlaceLadder = 0;
			m_iJoinLadderWaypointIndex = -1;
		}
		else
		{
			int iMoveType = CClassInterface::GetMoveType(m_pPlayer);
			int iPlayerFlags = CClassInterface::GetPlayerFlags(m_pPlayer);
			CUserCmd cmd = *CBots::GetBotPointer(m_pPlayer)->GetUserCMD();

			Vector vPlayerOrigin = GetOrigin();

			// ****************************************************
			// Jump waypoint
			//
			// wait until checkJump is not -1 (meaning that bot is in the air)
			// set checkJump to time+0.5 after landing 
			// 
			// ****************************************************

			if (m_iAutoEventWaypoint != 0)
			{
				int iWpt = CWaypointLocations::NearestWaypoint(m_vAutoEventWaypointOrigin, m_fAutoEventWaypointRadius, -1, false, false, false, NULL, false, m_iAutoEventWaypointTeam, false, false, Vector(0, 0, 0), m_iAutoEventWaypoint);

				CWaypoint *pWpt = CWaypoints::GetWaypoint(iWpt);

				if (!pWpt)
				{
					//updateCurrentWaypoint();

					//pWpt = CWaypoints::getWaypoint(currentWaypoint());

					//if ( !pWpt || pWpt->distanceFrom(m_vAutoEventWaypointOrigin) > 32.0f )
					//{

					if (m_bAutoEventWaypointAutoType)
					{
						CWaypointType *pMainType = CWaypointTypes::GetTypeByFlags(m_iAutoEventWaypoint);

						if (pMainType)
							CWaypoints::AddWaypoint(this, pMainType->GetName(), "", "", "");
						else
							CWaypoints::AddWaypoint(m_pPlayer, m_vAutoEventWaypointOrigin, (m_iAutoEventWaypointTeam == 0) ? (m_iAutoEventWaypoint) : ((m_iAutoEventWaypoint | m_iAutoEventWaypointTeamOn)&~m_iAutoEventWaypointTeamOff), true, cmd.viewangles.y, 0, 32.0f);
					}
					else
						CWaypoints::AddWaypoint(m_pPlayer, m_vAutoEventWaypointOrigin, (m_iAutoEventWaypointTeam == 0) ? (m_iAutoEventWaypoint) : ((m_iAutoEventWaypoint | m_iAutoEventWaypointTeamOn)&~m_iAutoEventWaypointTeamOff), true, cmd.viewangles.y, 0, 32.0f);
					//}
					/*else
					{
					pWpt->addFlag(m_iAutoEventWaypoint);
					}*/
				}

				m_iAutoEventWaypoint = 0;
			}
			//g_pBotManager->GetBotController(m_pPlayer)->IsEFlagSet();

			if ( /*(pev->waterlevel < 3) &&*/ (m_fCanPlaceJump < engine->Time()))
			{
				Vector v_floor;

				if ((m_fCanPlaceJump != -1) && (m_iLastButtons & IN_JUMP) && !(iPlayerFlags & FL_ONGROUND))
				{
					int iNearestWpt = CWaypointLocations::NearestWaypoint(vPlayerOrigin, 80.0, -1, true, false, false, NULL);

					m_iLastJumpWaypointIndex = -1;

					if (iNearestWpt == -1)
					{
						m_iLastJumpWaypointIndex = CWaypoints::AddWaypoint(m_pPlayer, vPlayerOrigin, CWaypointTypes::W_FL_JUMP, true);
					}
					else
						m_iLastJumpWaypointIndex = iNearestWpt; // can still update a current waypoint for land position

					m_vLastAutoWaypointPlacePos = vPlayerOrigin;

					m_fCanPlaceJump = -1;
				}
				// ****************************************************
				// Join jump waypoint to the landed waypoint
				// ****************************************************
				else if ((m_fCanPlaceJump == -1) && (iPlayerFlags & FL_ONGROUND))
				{
					if (m_iLastJumpWaypointIndex != -1)
					{
						int iNearestWpt = CWaypointLocations::NearestWaypoint(vPlayerOrigin, 80.0, -1, true, false, false, NULL);

						if (iNearestWpt == -1)
						{
							int iNewWpt = CWaypoints::AddWaypoint(m_pPlayer, vPlayerOrigin, 0, true);

							if (iNewWpt != -1)
							{
								CWaypoint *pWpt = CWaypoints::GetWaypoint(iNewWpt);
								CWaypoint *pJumpWpt = CWaypoints::GetWaypoint(m_iLastJumpWaypointIndex);

								pJumpWpt->AddPathTo(iNewWpt);

								pJumpWpt->AddFlag(CWaypointTypes::W_FL_JUMP);

								trace_t *tr;

								Vector v_src = pWpt->GetOrigin();

								CBotGlobals::QuickTraceline(m_pPlayer, v_src, v_src - Vector(0, 0, 144));

								tr = CBotGlobals::GetTraceResult();

								v_floor = tr->endpos;
								float len = v_src.z - tr->endpos.z;

								CBotGlobals::QuickTraceline(m_pPlayer, v_src, v_src + Vector(0, 0, 144));

								len += (tr->endpos.z - v_src.z);

								if (len > 72)
								{
									pWpt->RemoveFlag(CWaypointTypes::W_FL_CROUCH);
									pWpt->Move(v_floor + Vector(0, 0, 36));
								}
								else if (len > 32)
								{
									pWpt->AddFlag(CWaypointTypes::W_FL_CROUCH);
									pWpt->Move(v_floor + Vector(0, 0, 12));
								}
							}
						}
						else if (iNearestWpt != m_iLastJumpWaypointIndex)
						{
							CWaypoint *pJumpWpt = CWaypoints::GetWaypoint(m_iLastJumpWaypointIndex);

							pJumpWpt->AddPathTo(iNearestWpt);
							pJumpWpt->AddFlag(CWaypointTypes::W_FL_JUMP);
						}
					}

					m_iLastJumpWaypointIndex = -1;

					// wait a sec after player lands before checking jump again
					m_fCanPlaceJump = engine->Time() + 0.5;
				}
			}

			bool bCheckDistance = (iMoveType != MOVETYPE_FLY) && (m_fCanPlaceLadder == 0); // always check distance unless ladder climbing

			// ****************************************************
			// Ladder waypoint
			// make the frist waypoint (e.g. bottom waypoint)
			// ****************************************************
			if ((iMoveType == MOVETYPE_FLY) && !(m_iLastMoveType == MOVETYPE_FLY))
			{
				// went ON to a ladder

				int iNearestWpt = CWaypointLocations::NearestWaypoint(vPlayerOrigin, 80.0, -1, true, false, false, NULL);

				m_iLastLadderWaypointIndex = -1;

				if (iNearestWpt == -1)
					m_iLastLadderWaypointIndex = CWaypoints::AddWaypoint(m_pPlayer, vPlayerOrigin, CWaypointTypes::W_FL_LADDER, true);
				else
				{
					m_iLastLadderWaypointIndex = iNearestWpt; // can still update a current waypoint for land position

					CWaypoint *pLadderWpt = CWaypoints::GetWaypoint(m_iLastLadderWaypointIndex);

					pLadderWpt->AddFlag(CWaypointTypes::W_FL_LADDER); // update flags
				}

				m_vLastAutoWaypointPlacePos = vPlayerOrigin;

				bCheckDistance = false;

				m_fCanPlaceLadder = 0;

				// need to unset every check point when going on ladder first time
				for (int i = 0; i < MAX_STORED_AUTOWAYPOINT; i++)
				{
					m_vLastAutoWaypointCheckPos[i].UnSetPoint();
				}
			}
			else if (!(iMoveType == MOVETYPE_FLY) && (m_iLastMoveType == MOVETYPE_FLY))
			{
				// went OFF a ladder
				m_fCanPlaceLadder = engine->Time() + 0.2f;
			}

			// ****************************************************
			// If we have walked off a ladder for a small amount of time
			// Make the top/bottom ladder waypoint
			// ****************************************************
			if (m_fCanPlaceLadder && (m_fCanPlaceLadder < engine->Time()))
			{
				if (m_iLastLadderWaypointIndex != -1)
					// place a ladder waypoint before jumping off
				{
					int iNearestWpt = CWaypointLocations::NearestWaypoint(vPlayerOrigin, 80.0, -1, true, false, false, NULL);

					if (iNearestWpt == -1)
					{
						int iNewWpt = CWaypoints::AddWaypoint(m_pPlayer, vPlayerOrigin, CWaypointTypes::W_FL_LADDER, true);

						if (iNewWpt != -1)
						{
							CWaypoint *pLadderWpt = CWaypoints::GetWaypoint(m_iLastLadderWaypointIndex);

							m_iJoinLadderWaypointIndex = iNewWpt;

							pLadderWpt->AddPathTo(iNewWpt);
						}
					}
					else if (iNearestWpt != m_iLastLadderWaypointIndex)
					{
						CWaypoint *pLadderWpt = CWaypoints::GetWaypoint(m_iJoinLadderWaypointIndex);

						m_iJoinLadderWaypointIndex = iNearestWpt;

						pLadderWpt->AddPathTo(iNearestWpt);
					}
				}

				m_iLastLadderWaypointIndex = -1;

				bCheckDistance = false;

				m_fCanPlaceLadder = 0;
			}

			// ****************************************************
			// Join top ladder waypoint to a ground waypoint
			// ****************************************************
			if ((m_iJoinLadderWaypointIndex != -1) && (iPlayerFlags & FL_ONGROUND) && (iMoveType == MOVETYPE_WALK))
			{
				int iNearestWpt = CWaypointLocations::NearestWaypoint(vPlayerOrigin, 40.0, m_iJoinLadderWaypointIndex, true, false, false, NULL);

				if (iNearestWpt == -1)
				{
					int iNewWpt = CWaypoints::AddWaypoint(m_pPlayer, vPlayerOrigin, 0, true);

					if (iNewWpt != -1)
					{
						CWaypoint *pLadderWpt = CWaypoints::GetWaypoint(m_iJoinLadderWaypointIndex);

						pLadderWpt->AddPathTo(iNewWpt);
					}
				}
				else if (iNearestWpt != m_iJoinLadderWaypointIndex)
				{
					CWaypoint *pLadderWpt = CWaypoints::GetWaypoint(m_iJoinLadderWaypointIndex);

					pLadderWpt->AddPathTo(iNearestWpt);
				}

				m_iJoinLadderWaypointIndex = -1;
			}

			m_iLastButtons = cmd.buttons;
			m_iLastMoveType = iMoveType;

			if (m_fLastAutoWaypointCheckTime < engine->Time())
			{
				// ****************************************
				// Corner - Check
				// ****************************************
				//
				// place a "Check - point" at player origin
				//

				CAutoWaypointCheck *vCurVector;
				Vector vCheckOrigin;

				Vector vPlacePosition;
				int iFlags = 0;
				bool bPlace = false;

				int i;
				int n;

				trace_t *tr;

				int numset = 0;
				int last = 0;

				for (n = 0; n < MAX_STORED_AUTOWAYPOINT; n++)
				{
					if (m_vLastAutoWaypointCheckPos[n].IsVectorSet())
					{
						numset++;
					}
				}

				if (numset == MAX_STORED_AUTOWAYPOINT)
				{
					// move check points down
					for (n = 0; n < (MAX_STORED_AUTOWAYPOINT - 1); n++)
					{
						m_vLastAutoWaypointCheckPos[n] = m_vLastAutoWaypointCheckPos[n + 1];
					}

					last = MAX_STORED_AUTOWAYPOINT - 1;
				}
				else
				{
					last = numset;
				}

				iFlags = 0;

				// sort out flags for this waypoint depending on player
				if ((iPlayerFlags & FL_DUCKING) == FL_DUCKING)
				{
					iFlags |= CWaypointTypes::W_FL_CROUCH;  // crouching waypoint
				}

				if (iMoveType == MOVETYPE_LADDER)
					iFlags |= CWaypointTypes::W_FL_LADDER;  // waypoint on a ladder

				m_vLastAutoWaypointCheckPos[last].SetPoint(vPlayerOrigin, iFlags);

				if ((m_iLastJumpWaypointIndex == -1) && bCheckDistance && ((vPlayerOrigin - m_vLastAutoWaypointPlacePos).Length() > 200))
				{
					extern ConVar bot_autowaypoint_dist;
					int iNearestWpt = CWaypointLocations::NearestWaypoint(vPlayerOrigin, bot_autowaypoint_dist.GetFloat(), -1, true, false, false, NULL);

					if (iNearestWpt == -1)
						CWaypoints::AddWaypoint(this, "", "", "", "");

					// set regardless
					m_vLastAutoWaypointPlacePos = vPlayerOrigin;
				}

				// search for occluded check points from player
				for (i = 0; i < MAX_STORED_AUTOWAYPOINT; i++)
				{
					vCurVector = &m_vLastAutoWaypointCheckPos[i];

					if (!vCurVector->IsVectorSet())
						continue;

					vCheckOrigin = vCurVector->GetVector();

					CBotGlobals::QuickTraceline(m_pPlayer, vPlayerOrigin, vCheckOrigin);
					tr = CBotGlobals::GetTraceResult();

#ifndef __linux__
					if (m_bDebugAutoWaypoint && !engine->IsDedicatedServer())
					{
						debugoverlay->AddLineOverlay(vCheckOrigin + Vector(0, 0, 16), vCheckOrigin - Vector(0, 0, 16), 255, 255, 255, 0, 2);
						debugoverlay->AddLineOverlay(vPlayerOrigin, vCheckOrigin, 255, 255, 255, 0, 2);
					}
#endif					
					if (tr->fraction < 1.0)
					{
						if (tr->m_pEnt)
						{
							Vector vel;
							extern IServerGameEnts *servergameents;
							edict_t *pEdict = servergameents->BaseEntityToEdict(tr->m_pEnt);

							if (CClassInterface::GetVelocity(pEdict, &vel))
							{
								// on a lift/train moving "fast"
								if (vel.Length() > 20.0)
									continue;
							}
						}
						// find next which is visible
						for (n = i + 1; n < MAX_STORED_AUTOWAYPOINT; n++)
						{
							vCurVector = &m_vLastAutoWaypointCheckPos[n];

							if (!vCurVector->IsVectorSet())
								continue;

							vCheckOrigin = vCurVector->GetVector();

							CBotGlobals::QuickTraceline(m_pPlayer, vPlayerOrigin, vCheckOrigin);

#ifndef __linux__
							if (m_bDebugAutoWaypoint)
								debugoverlay->AddLineOverlay(vPlayerOrigin, vCheckOrigin, 255, 255, 255, false, 2);
#endif							
							if (tr->fraction >= 1.0)
							{
								int iNearestWpt = CWaypointLocations::NearestWaypoint(vCheckOrigin, 100.0, -1, true, false, false, NULL);

								if (iNearestWpt == -1)
								{
									bPlace = true;
									vPlacePosition = vCheckOrigin;
									iFlags = vCurVector->GetFlags();

									break;
								}
								else
									continue;
							}
						}

					}
				}

				if (bPlace)
				{
					int inewwpt = CWaypoints::AddWaypoint(m_pPlayer, vPlacePosition, iFlags, true);
					CWaypoint *pWpt = CWaypoints::GetWaypoint(inewwpt);
					Vector v_floor;

					m_vLastAutoWaypointPlacePos = vPlacePosition;
					bool bCanStand;

					trace_t *tr;

					Vector v_src = vPlacePosition;

					CBotGlobals::QuickTraceline(m_pPlayer, v_src, v_src - Vector(0, 0, 144));

					tr = CBotGlobals::GetTraceResult();

					v_floor = tr->endpos;
					float len = v_src.z - tr->endpos.z;

					CBotGlobals::QuickTraceline(m_pPlayer, v_src, v_src + Vector(0, 0, 144));

					len += (tr->endpos.z - v_src.z);

					bCanStand = (len > 72);

					if ((m_iLastJumpWaypointIndex != -1) && bCanStand)
					{
						pWpt->RemoveFlag(CWaypointTypes::W_FL_CROUCH);
						//waypoints[inewwpt].origin = v_floor+Vector(0,0,36);
					}
					//clear from i

					int pos = n;
					int n = 0;

					for (n = 0; pos < MAX_STORED_AUTOWAYPOINT; n++)
					{
						m_vLastAutoWaypointCheckPos[n] = m_vLastAutoWaypointCheckPos[pos];

						pos++;
					}

					for (n = n; n < MAX_STORED_AUTOWAYPOINT; n++)
					{
						m_vLastAutoWaypointCheckPos[n].UnSetPoint();
					}
				}

				m_fLastAutoWaypointCheckTime = engine->Time() + 0.5f;
			}
		}
	}
}

void CClient::GiveMessage(char *msg, float fTime)
{
	extern ConVar bot_tooltips;

	if (bot_tooltips.GetBool())
	{
		m_NextTooltip.push(new CToolTip(msg, NULL));
		m_fNextBotServerMessage = engine->Time() + fTime;
	}
}

void CClients::GiveMessage(char *msg, float fTime, edict_t *pPlayer)
{
	CClient *pClient;

	if (pPlayer != NULL)
	{
		pClient = Get(pPlayer);

		if (pClient)
			pClient->GiveMessage(msg, fTime);
	}
	else
	{
		for (int i = 0; i < 32; i++)
		{
			pClient = Get(i);

			if (pClient)
			{
				pClient->GiveMessage(msg, fTime);
			}
		}
	}
}

const char *CClient::GetName()
{
	IPlayerInfo *playerinfo = playerinfomanager->GetPlayerInfo(m_pPlayer);

	if (playerinfo)
		return playerinfo->GetName();

	return NULL;
}

void CClient::SetTeleportVector()
{
	m_vTeleportVector = GetOrigin();
	m_bTeleportVectorValid = true;
}

void CClient::ClientActive()
{
	// get steam id
	IPlayerInfo *playerinfo = playerinfomanager->GetPlayerInfo(m_pPlayer);

	m_szSteamID = NULL;

	if (playerinfo)
	{
		// store steam id
		m_szSteamID = (char*)playerinfo->GetNetworkIDString();
	}
}
// this player joins with pPlayer edict
void CClient::ClientConnected(edict_t *pPlayer)
{
	Init();
	// set player edict
	SetEdict(pPlayer);
}

void CClient::UpdateCurrentWaypoint()
{
	SetWaypoint(CWaypointLocations::NearestWaypoint(GetOrigin(), 50, -1, false, true, false, NULL, false, 0, false, false, Vector(0, 0, 0), m_iWaypointShowFlags));
}
// this player disconnects
void CClient::ClientDisconnected()
{
	// is bot?
	CBot *pBot = CBots::GetBotPointer(m_pPlayer);

	if (pBot != NULL)
	{
		if (pBot->InUse())
		{
			// free bots memory and other stuff
			pBot->FreeAllMemory();
		}
	}

	if (!engine->IsDedicatedServer())
	{
		if (CClients::IsListenServerClient(this))
		{
			CClients::SetListenServerClient(NULL);
		}
	}

	Init();
}

bool CClient::IsUsed()
{
	return (m_pPlayer != NULL);
}

Vector CClient::GetOrigin()
{
	IPlayerInfo *playerinfo = playerinfomanager->GetPlayerInfo(m_pPlayer);

	if (playerinfo)
	{
		return  playerinfo->GetAbsOrigin() + Vector(0, 0, 32);
	}

	return CBotGlobals::EntityOrigin(m_pPlayer) + Vector(0, 0, 32);//m_pPlayer->GetCollideable()->GetCollisionOrigin();
}

void CClients::ClientActive(edict_t *pPlayer)
{
	CClient *pClient = &m_Clients[SlotOfEdict(pPlayer)];

	pClient->ClientActive();
}

CClient *CClients::ClientConnected(edict_t *pPlayer)
{
	CClient *pClient = &m_Clients[SlotOfEdict(pPlayer)];

	pClient->ClientConnected(pPlayer);

	return pClient;
}

void CClients::Init(edict_t *pPlayer)
{
	m_Clients[SlotOfEdict(pPlayer)].Init();
}

void CClients::ClientDisconnected(edict_t *pPlayer)
{
	CClient *pClient = &m_Clients[SlotOfEdict(pPlayer)];

	pClient->ClientDisconnected();
}

void CClients::ClientThink()
{
	static CClient *pClient;

	edict_t *pPlayer;

	m_bClientsDebugging = false;

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		pClient = &m_Clients[i];

		if (!pClient->IsUsed())
			continue;
		if (!m_bClientsDebugging && pClient->IsDebugging())
			m_bClientsDebugging = true;

		pPlayer = pClient->GetPlayer();

		if (pPlayer && pPlayer->GetIServerEntity())
			pClient->Think();
	}
}

CClient *CClients::FindClientBySteamID(char *szSteamID)
{
	CClient *pClient;

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		pClient = &m_Clients[i];

		if (pClient->IsUsed())
		{
			if (FStrEq(pClient->GetSteamID(), szSteamID))
				return pClient;
		}
	}

	return NULL;
}

void CClients::ClientDebugMsg(CBot *pBot, int iLev, const char *fmt, ...)
{
	va_list argptr;
	static char string[1024];

	va_start(argptr, fmt);
	vsprintf(string, fmt, argptr);
	va_end(argptr);

	ClientDebugMsg(iLev, string, pBot);
}

const char *g_szDebugTags[15] =
{
	"GAME_EVENT",
	"NAV",
	"SPEED",
	"VIS",
	"TASK",
	"BUTTONS",
	"USERCMD",
	"UTIL",
	"PROFILE",
	"EDICTS",
	"THINK",
	"LOOK",
	"HUD",
	"AIM",
	"CHAT"
};


void CClients::ClientDebugMsg(int iLev, const char *szMsg, CBot *pBot)
{
	CClient *pClient;

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		pClient = &m_Clients[i];

		if (!pClient->IsUsed())
			continue;
		if (!pClient->IsDebugOn(iLev))
			continue;
		if (pBot && !pClient->IsDebuggingBot(pBot->GetEdict()))
			continue;

		if (pClient->IsDebugOn(BOT_DEBUG_CHAT)) {
			char logmsg[128] = { 0 };
			snprintf(logmsg, sizeof(logmsg), "[DEBUG %s] %s", g_szDebugTags[iLev], szMsg);
			AFKBot::HudTextMessage(pClient->GetPlayer(), logmsg);
		}

		CBotGlobals::BotMessage(pClient->GetPlayer(), 0, "[DEBUG %s] %s", g_szDebugTags[iLev], szMsg);
	}
}

// get index in array
int CClients::SlotOfEdict(edict_t *pPlayer)
{
	return ENTINDEX(pPlayer) - 1;
}

bool CClients::ClientsDebugging(int iLev)
{
	if (iLev == 0)
		return m_bClientsDebugging;
	else if (m_bClientsDebugging)
	{
		int i;
		CClient *pClient;

		for (i = 0; i < MAX_PLAYERS; i++)
		{
			pClient = CClients::Get(i);

			if (pClient->IsUsed())
			{
				if (pClient->IsDebugOn(iLev))
					return true;
			}
		}
	}

	return false;
}

void CClient::SetWaypointCut(CWaypoint *pWaypoint)
{
	if (pWaypoint)
	{
		register int i = 0;

		SetWaypointCopy(pWaypoint);

		m_WaypointCutPaths.clear();

		for (i = 0; i < pWaypoint->NumPaths(); i++)
		{
			m_WaypointCutPaths.push_back(pWaypoint->GetPath(i));
		}

		m_WaypointCopyType = WPT_COPY_CUT;
	}
}

void CClient::SetWaypointCopy(CWaypoint *pWaypoint)
{
	if (pWaypoint)
	{
		m_fCopyWptRadius = pWaypoint->GetRadius();
		m_iCopyWptFlags = pWaypoint->GetFlags();
		m_iCopyWptArea = pWaypoint->GetArea();
		m_WaypointCopyType = WPT_COPY_COPY;
	}
}