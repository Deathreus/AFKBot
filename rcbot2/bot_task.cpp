/*
 *    part of https://rcbot2.svn.sourceforge.net/svnroot/rcbot2
 *
 *    ThIs file Is part of RCBot.
 *
 *    RCBot by Paul Murphy adapted from Botman's HPB Bot 2 template.
 *
 *    RCBot Is free software; you can redIstribute it and/or modify it
 *    under the terms of the GNU General Public License as publIshed by the
 *    Free Software Foundation; either version 2 of the License, or (at
 *    your option) any later version.
 *
 *    RCBot Is dIstributed in the hope that it will be useful, but
 *    WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with RCBot; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    In addition, as a special exception, the author gives permIssion to
 *    link the code of thIs program with the Half-Life Game engine ("HL
 *    engine") and Modified Game Libraries ("MODs") developed by Valve,
 *    L.L.C ("Valve").  You must obey the GNU General Public License in all
 *    respects for all of the code used other than the HL engine and MODs
 *    from Valve.  If you modify thIs file, you may extend thIs exception
 *    to your version of the file, but you are not obligated to do so.  If
 *    you do not wIsh to do so, delete thIs exception statement from your
 *    version.
 *
 */
#include "engine_wrappers.h"

//#include "bot_mtrand.h"
#include "bot.h"
#include "bot_schedule.h"
#include "bot_task.h"
#include "bot_navigator.h"
#include "bot_waypoint_locations.h"
#include "bot_globals.h"
#include "in_buttons.h"
#include "bot_weapons.h"
//#include "bot_hldm_bot.h"
#include "bot_fortress.h"
//#include "bot_profiling.h"
#include "bot_Getprop.h"
//#include "bot_dod_bot.h"
#include "bot_squads.h"
#include "bot_waypoint_vIsibility.h"

extern ConVar *sv_gravity;
// desx and desy must be normalized
// desx = dIstance (should be 2d)
// desy = height offSet
void GetGrenadeAngle(float v, float g, float desx, float desy, float *fa1, float *fa2)
{
	//normalize
	float fmax = MAX(v, MAX(g, MAX(desx, desy)));

	v /= fmax;
	g /= fmax;
	desx /= fmax;
	desy /= fmax;
	double vsquared = v*v;
	double vfourth = vsquared * vsquared;
	double x2 = desx * desx;
	double gx2 = g * x2;
	double twoyv2 = 2 * desy*vsquared;
	double fourabplusa = vfourth - g*(gx2 + twoyv2);
	double topplus = vsquared + sqrt(fourabplusa);
	double topminus = vsquared - sqrt(fourabplusa);
	double bottom = g*desx;

	*fa1 = (float)atan(topplus / bottom);
	*fa2 = (float)atan(topminus / bottom);

	*fa1 = RAD2DEG(*fa1);
	*fa2 = RAD2DEG(*fa2);

	return;
}

float GetGrenadeZ(edict_t *pShooter, edict_t *pTarGet, Vector vOrigin, Vector vTarGet, float fInitialSpeed)
{
	float fAngle1, fAngle2;

	Vector vForward;
	QAngle angles;

	Vector v_comp = (vTarGet - vOrigin);

	float fDIst2D = v_comp.Length2D();
	float fDIst3D = v_comp.Length();

	GetGrenadeAngle(fInitialSpeed, sv_gravity->GetFloat(), fDIst2D, vTarGet.z - vOrigin.z, &fAngle1, &fAngle2);

	angles = QAngle(0, 0, 0);
	// do a quick traceline to Check which angle to choose (minimum angle = straight)

	if (CBotGlobals::IsShotVisible(pShooter, vOrigin, vTarGet, pTarGet))
		angles.x = -MIN(fAngle1, fAngle2);
	else
		angles.x = -MAX(fAngle1, fAngle2);

	AngleVectors(angles, &vForward);

	// add gravity height
	return (vOrigin + (vForward*fDIst3D)).z;
}
////////////////////////////////////////////////////////////////////////////////////////////
// Tasks

CBotTF2MedicHeal::CBotTF2MedicHeal()
{
	m_pHeal = NULL;
	m_bHealerJumped = false;
}

void CBotTF2MedicHeal::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	edict_t *pHeal;
	CBotTF2 *pBotTF2;

	pBot->WantToShoot(false);
	pBot->WantToListen(false);

	if (!pBot->IsTF2())
	{
		Fail();
		return;
	}

	pBotTF2 = (CBotTF2*)pBot;

	pHeal = pBotTF2->GetHealingEntity();

	if (pHeal && !m_bHealerJumped)
	{
		Vector vVelocity;

		if (!CClassInterface::GetVelocity(pHeal, &vVelocity) || (vVelocity.Length2D() > 1.0f))
		{
			IPlayerInfo *p = playerinfomanager->GetPlayerInfo(pHeal);

			if (p)
			{
				if (p->GetLastUserCommand().buttons & IN_JUMP)
				{
					m_bHealerJumped = true;
					m_vJump = CBotGlobals::EntityOrigin(pHeal);
				}
			}
		}
	}

	if (!pHeal)
	{
		// because the medic would have followed thIs guy, he would have lost hIs own waypoint
		pBot->GetNavigator()->RollBackPosition();
		Fail();
	}
	else if (pBot->GetCurrentWeapon() == NULL)
	{
		pBotTF2->ClearHealingEntity();
		pBot->GetNavigator()->RollBackPosition();
		Fail();
	}
	else if (pBotTF2->GetHealFactor(pHeal) == 0.0f)
	{
		pBotTF2->ClearHealingEntity();
		pBot->GetNavigator()->RollBackPosition();
		Fail();
	}/*
	else if ( !pBot->IsVisible(pHeal) )
	{
	pBot->GetNavigator()->RollBackPosition();
	((CBotFortress*)pBot)->ClearHealingEntity();
	Fail();
	}*/
	else if (pBot->DistanceFrom(pHeal) > 416)
	{
		pBotTF2->ClearHealingEntity();
		pBot->GetNavigator()->RollBackPosition();
		Fail();
	}
	else if (pBot->GetCurrentWeapon()->GetWeaponInfo()->GetID() != TF2_WEAPON_MEDIGUN)
	{
		pBot->Select_CWeapon(CWeapons::GetWeapon(TF2_WEAPON_MEDIGUN));
	}
	else
	{
		Vector vVelocity;

		CClassInterface::GetVelocity(pBot->GetEdict(), &vVelocity);

		pBot->ClearFailedWeaponSelect();

		if (m_bHealerJumped && (pBot->DistanceFrom(m_vJump) < 64.0f) && (vVelocity.Length() > 1.0f))
		{
			pBot->Jump();
			m_bHealerJumped = false;
		}

		if (!pBotTF2->HealPlayer())
		{
			pBot->GetNavigator()->RollBackPosition();
			pBotTF2->ClearHealingEntity();
			Fail();
		}
	}
}

///////////


CBotTF2ShootLastEnemyPosition::CBotTF2ShootLastEnemyPosition(Vector vPosition, edict_t *pEnemy, Vector m_vVelocity)
{
	float len = m_vVelocity.Length();

	m_vPosition = vPosition;

	if (len > 0)
		m_vPosition = m_vPosition - ((m_vVelocity / m_vVelocity.Length()) * 16);

	m_pEnemy = pEnemy;
	m_fTime = 0;
}

void CBotTF2ShootLastEnemyPosition::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	CBotWeapon *pWeapon = pBot->GetCurrentWeapon();
	CBotTF2 *pTF2Bot = (CBotTF2*)pBot;
	CWeapon *pChange = NULL;
	CBotWeapon *pChanGeto = NULL;

	if (m_fTime == 0)
		m_fTime = engine->Time() + RandomFloat(2.0f, 4.5f);

	if (m_fTime < engine->Time())
	{
		Complete();
		return;
	}

	if (!CBotGlobals::EntityIsValid(m_pEnemy) || !CBotGlobals::EntityIsAlive(m_pEnemy))
	{
		Complete();
		return;
	}

	if (pBot->GetEnemy() && pBot->HasSomeConditions(CONDITION_SEE_CUR_ENEMY))
	{
		Fail();
		return;
	}

	pBot->WantToShoot(false);
	pBot->WantToChangeWeapon(false);
	pBot->WantToListen(false);

	if (pTF2Bot->GetClass() == TF_CLASS_SOLDIER)
	{
		pChange = CWeapons::GetWeapon(TF2_WEAPON_ROCKETLAUNCHER);
	}
	else if (pTF2Bot->GetClass() == TF_CLASS_DEMOMAN)
	{
		pChange = CWeapons::GetWeapon(TF2_WEAPON_GRENADELAUNCHER);
	}

	if (!pChange)
	{
		Fail();
		return;
	}

	pChanGeto = pBot->GetWeapons()->GetWeapon(pChange);

	if (pChanGeto->GetAmmo(pBot) < 1)
	{
		Complete();
		return;
	}

	if (pChanGeto != pWeapon)
	{
		pBot->SelectBotWeapon(pChanGeto);
	}
	else
	{
		if (RandomInt(0, 1))
			pBot->PrimaryAttack(false);
	}

	pBot->SetLookVector(m_vPosition);
	pBot->SetLookAtTask(LOOK_VECTOR);

}


/////////////

CBotTF2WaitHealthTask::CBotTF2WaitHealthTask(Vector vOrigin)
{
	m_vOrigin = vOrigin;
	m_fWaitTime = 0;
}

void CBotTF2WaitHealthTask::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	if (!m_fWaitTime)
		m_fWaitTime = engine->Time() + RandomFloat(5.0f, 10.0f);

	if (!pBot->HasSomeConditions(CONDITION_NEED_HEALTH))
		Complete();
	else if (m_fWaitTime < engine->Time())
		Fail();
	else
	{
		// TO DO
		/*edict_t *pOtherPlayer = CBotGlobals::findNearestPlayer(m_vOrigin,50.0,pBot->GetEdict());

		if ( pOtherPlayer )
		{
		Fail();
		return;
		}*/

		pBot->SetLookAtTask(LOOK_AROUND);

		if (pBot->DistanceFrom(m_vOrigin) > 50)
			pBot->SetMoveTo((m_vOrigin));
		else
			pBot->StopMoving();

		if (pBot->IsTF())
		{
			((CBotTF2*)pBot)->Taunt();

			if (((CBotTF2*)pBot)->IsBeingHealed())
				Complete();
		}
	}
}


CBotTF2WaitFlagTask::CBotTF2WaitFlagTask(Vector vOrigin, bool bFind)
{
	m_vOrigin = vOrigin;
	m_fWaitTime = 0;
	m_bFind = bFind;
}

void CBotTF2WaitFlagTask::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	if (!m_fWaitTime)
	{
		if (m_bFind)
			m_fWaitTime = engine->Time() + 5.0f;
		else
			m_fWaitTime = engine->Time() + 10.0f;
	}

	if (((CBotTF2*)pBot)->HasFlag())
		Complete();
	else if (pBot->GetHealthPercent() < 0.2)
	{
		Fail();
	}
	else if (m_fWaitTime < engine->Time())
	{
		((CBotFortress*)pBot)->FlagReset();
		Fail();
	}
	else if (!pBot->IsTF())
	{
		Fail();
	}
	else
	{
		if (!((CBotFortress*)pBot)->WaitForFlag(&m_vOrigin, &m_fWaitTime, m_bFind))
		{
			Fail();
		}
	}
}

//////////
/*CBotDODBomb :: CBotDODBomb ( int iBombType, int iBombID, edict_t *pBomb, Vector vPosition, int iPrevOwner )
{
m_iType = iBombType;
m_iBombID = iBombID;
m_fTime = 0;

if ( m_iBombID == -1 )
m_iBombID = CDODMod::m_Flags.GetBombID(pBomb);

m_pBombTarGet = pBomb;
m_vOrigin = vPosition;
m_iPrevTeam = iPrevOwner;
}

void CBotDODBomb :: Execute (CBot *pBot,CBotSchedule *pSchedule)
{
bool bWorking = false;

pBot->WantToShoot(false);

if ( m_fTime == 0 )
{
m_fTime = engine->Time() + RandomFloat(8.0f,12.0f);
((CDODBot*)pBot)->SetNearestBomb(m_pBombTarGet);

if ( (m_iType == DOD_BOMB_PLANT) || (m_iType == DOD_BOMB_PATH_PLANT) )
{
if ( CDODMod::m_Flags.IsTeamMatePlanting(pBot->GetEdict(),pBot->GetTeam(),m_iBombID) )
Fail();
}
else
{
if ( CDODMod::m_Flags.IsTeamMateDefusing(pBot->GetEdict(),pBot->GetTeam(),m_iBombID) )
Fail();
}
}
else if ( m_fTime < engine->Time() )
{
Fail();
}

if ( m_iType == DOD_BOMB_PLANT)
{
bWorking = CClassInterface::IsPlayerPlantingBomb_DOD(pBot->GetEdict());

if ( CDODMod::m_Flags.IsBombPlanted(m_iBombID) )
{
Complete();
}
else if ( CDODMod::m_Flags.IsTeamMatePlanting(pBot->GetEdict(),pBot->GetTeam(),m_iBombID) )
Complete(); // team mate doing my job

//else if ( !CClassInterface::IsPlayerPlantingBomb_DOD(pBot->GetEdict()) )// it Is still planted
//	Complete(); // bomb Is being defused by someone else - give up
}
else if ( m_iType == DOD_BOMB_DEFUSE)
{
bWorking = CClassInterface::IsPlayerDefusingBomb_DOD(pBot->GetEdict());

if ( !CDODMod::m_Flags.IsBombPlanted(m_iBombID) )
Complete();
else if ( CDODMod::m_Flags.IsTeamMateDefusing(pBot->GetEdict(),pBot->GetTeam(),m_iBombID) )
Complete(); // team mate doing my job

//else if ( CDODMod::m_Flags.IsBombBeingDefused(m_iBombID) && !CClassInterface::IsPlayerDefusingBomb_DOD(pBot->GetEdict()) )// it Is still planted
//	Complete(); // bomb Is being defused by someone else - give up
}
else if ( m_iType == DOD_BOMB_PATH_PLANT ) // a bomb that's in the way
{
bWorking = CClassInterface::IsPlayerPlantingBomb_DOD(pBot->GetEdict());

if ( CClassInterface::GetDODBombState(m_pBombTarGet) != DOD_BOMB_STATE_AVAILABLE )
{
//CDODBot *pDODBot = (CDODBot*)pBot;

//pDODBot->removeBomb();

Complete();
}
else if ( CDODMod::m_Flags.IsTeamMatePlanting(pBot->GetEdict(),pBot->GetTeam(),CBotGlobals::EntityOrigin(m_pBombTarGet)) )
Complete(); // team mate doing my job

}
else if ( m_iType == DOD_BOMB_PATH_DEFUSE ) // a bomb that's in the way
{
bWorking = CClassInterface::IsPlayerDefusingBomb_DOD(pBot->GetEdict());

if ( CClassInterface::GetDODBombState(m_pBombTarGet) == DOD_BOMB_STATE_AVAILABLE )
Complete();
else if ( CDODMod::m_Flags.IsTeamMateDefusing(pBot->GetEdict(),pBot->GetTeam(),CBotGlobals::EntityOrigin(m_pBombTarGet)) )
Complete(); // team mate doing my job
}

pBot->SetLookVector(m_vOrigin);
pBot->SetLookAtTask(LOOK_VECTOR);

pBot->use();

if ( !bWorking )
{
pBot->SetMoveTo(m_vOrigin);
pBot->SetMoveSpeed(CClassInterface::GetMaxSpeed(pBot->GetEdict())/4);
}
else
{
pBot->StopMoving();
}
}

void CBotDODBomb :: DebugString ( char *string )
{
sprintf(string,"CBotDODBomb\nm_iType = %d\nm_iBombID = %d\nm_fTime = %0.2f\nm_iPrevTeam = %d",m_iType, m_iBombID,m_fTime,m_iPrevTeam);
}

//////
void CDODWaitForGrenadeTask :: Execute (CBot *pBot,CBotSchedule *pSchedule)
{

if ( m_pGrenade.Get() == NULL )
{
Complete();
}
else if ( !CBotGlobals::EntityIsAlive(m_pGrenade) )
Complete();
else if ( m_fTime == 0 )
{
CDODBot *pDODBot = (CDODBot*)pBot;

m_fTime = engine->Time() + RandomFloat(3.0f,5.0f);

pDODBot->prone();
}
else if ( m_fTime < engine->Time() )
{
CDODBot *pDODBot = (CDODBot*)pBot;

Complete();

pDODBot->unProne();
}
}

void CDODWaitForGrenadeTask :: DebugString ( char *string )
{
sprintf(string,"CDODWaitForGrenadeTask");
}
//////////

void CDODWaitForBombTask :: Execute (CBot *pBot,CBotSchedule *pSchedule)
{
if ( m_fTime == 0.0f )
{
CWaypoint *pCurrent;
pBot->UpdateCondition(CONDITION_RUN);
pCurrent = CWaypoints::GetWaypoint(CWaypointLocations::NearestWaypoint(pBot->GetOrigin(),400.0f,CWaypoints::GetWaypointIndex(m_pBlocking),true,false,true));

if ( pCurrent == NULL )
pCurrent = m_pBlocking;

m_fTime = engine->Time() + RandomFloat(2.0f,5.0f);
m_pRunTo = CWaypoints::GetNextCoverPoint(pBot,pCurrent,m_pBlocking) ;
}

if ( m_pBombTarGet.Get() == NULL )
{
Complete();
return;
}

if ( m_pBombTarGet.Get()->GetUnknown() == NULL )
{
Complete();
return;
}

pBot->UpdateCondition(CONDITION_RUN);

if ( m_pRunTo )
{
if (m_pRunTo->touched(pBot->GetOrigin(),Vector(0,0,0),48.0f) )
{
if ( pBot->DistanceFrom(m_pBombTarGet) > (BLAST_RADIUS*2) )
pBot->StopMoving();
else
m_pRunTo = CWaypoints::GetNextCoverPoint(pBot,m_pRunTo,m_pBlocking) ;
}
else
pBot->SetMoveTo(m_pRunTo->GetOrigin());
}

if ( m_fTime < engine->Time() )
{
Complete();
return;
}

if ( CClassInterface::GetDODBombState (m_pBombTarGet) != 2 )
{
Complete();
return;
}

pBot->LookAtEdict(m_pBombTarGet);
pBot->SetLookAtTask(LOOK_EDICT);
}

void CDODWaitForBombTask :: DebugString ( char *string )
{
sprintf(string,"CDODWaitForBombTask");
}

//////////

CBotDODAttackPoint :: CBotDODAttackPoint ( int iFlagID, Vector vOrigin, float fRadius )
{
m_vOrigin = vOrigin;
m_fAttackTime = 0;
m_fTime = 0;
m_iFlagID = iFlagID;
m_fRadius = fRadius;
}

void CBotDODAttackPoint :: Execute (CBot *pBot,CBotSchedule *pSchedule)
{
static int iTeam;

iTeam = pBot->GetTeam();

if ( CDODMod::m_Flags.ownsFlag(m_iFlagID,iTeam) )
{
Complete();

pBot->ResetAreaClear();

if ( pBot->inSquad() && pBot->IsSquadLeader() )
{
float fDanger = MAX_BELIEF;
IBotNavigator *pNav = pBot->GetNavigator();

fDanger = pNav->GetBelief(CDODMod::m_Flags.GetWaypointAtFlag(m_iFlagID));

if ( RandomFloat(0.0f,MAX_BELIEF) < fDanger )
{
pBot->addVoiceCommand(DOD_VC_HOLD);
pBot->UpdateCondition(CONDITION_DEFENSIVE);
}
}

return;
}
else if ( m_fAttackTime == 0 )
{
m_fAttackTime = engine->Time() + RandomFloat(30.0,60.0);
}
else if ( m_fAttackTime < engine->Time() )
{
Complete();
return;
}
else
{
if ( m_fTime == 0 )
{
m_fTime = engine->Time() + RandomFloat(2.0,4.0);
m_vMoveTo = m_vOrigin + Vector(RandomFloat(-m_fRadius,m_fRadius),RandomFloat(-m_fRadius,m_fRadius),0);
m_bProne = (RandomFloat(0,1) * (1.0f-pBot->GetHealthPercent())) > 0.75f;

if (  CDODMod::m_Flags.numFriendliesAtCap(m_iFlagID,iTeam) < CDODMod::m_Flags.numCappersRequired(m_iFlagID,iTeam) )
{
// count players I see
CDODBot *pDODBot = (CDODBot*)pBot;

pDODBot->addVoiceCommand(DOD_VC_NEED_BACKUP);
}
}
else if ( m_fTime < engine->Time() )
{
m_fTime = 0;
}
else
{
static float fdIst;

fdIst = pBot->DistanceFrom(m_vMoveTo);

if ( m_bProne && !pBot->HasSomeConditions(CONDITION_RUN) )
pBot->duck();

if ( fdIst < m_fRadius )
{
pBot->StopMoving();
pBot->SetLookAtTask(LOOK_AROUND);
}
else if ( fdIst > 400 )
Fail();
else
{
pBot->SetMoveTo(m_vMoveTo);
}
}
}
}

void CBotDODAttackPoint :: DebugString ( char *string )
{
sprintf(string,"CBotDODAttackPoint\nm_iFlagID = %d\n m_vOrigin = (%0.1f,%0.1f,%0.1f,radius = %0.1f)",m_iFlagID,m_vOrigin.x,m_vOrigin.y,m_vOrigin.z,m_fRadius);
}*/

///////////

CBotTF2AttackPoint::CBotTF2AttackPoint(int iArea, Vector vOrigin, int iRadius)
{
	m_vOrigin = vOrigin;
	m_fAttackTime = 0;
	m_fTime = 0;
	m_iArea = iArea;
	m_iRadius = iRadius;
}

void CBotTF2AttackPoint::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	int iCpIndex = CTeamFortress2Mod::m_ObjectiveResource.m_WaypointAreaToIndexTranslation[m_iArea];
	int iTeam = pBot->GetTeam();
	CBotTF2 *pTF2Bot = (CBotTF2*)pBot;

	if (CTeamFortress2Mod::TF2_IsPlayerInvuln(pBot->GetEdict()))
		Fail();

	pBot->WantToInvestigateSound(false);

	if (m_iArea && (CTeamFortress2Mod::m_ObjectiveResource.GetOwningTeam(iCpIndex) == iTeam))
	{
		Complete(); // done
		pTF2Bot->UpdateAttackDefendPoints();
	}
	else if (m_iArea && !CTeamFortress2Mod::m_ObjectiveResource.IsCPValid(iCpIndex, iTeam, TF2_POINT_ATTACK))
	{
		Fail(); // too slow
		pTF2Bot->UpdateAttackDefendPoints();
	}
	else if (m_fAttackTime == 0)
		m_fAttackTime = engine->Time() + RandomFloat(30.0, 60.0);
	else if (m_fAttackTime < engine->Time())
		Complete();
	else
	{
		if (m_fTime == 0)
		{

			m_fTime = engine->Time() + RandomFloat(5.0, 10.0);
			m_vMoveTo = m_vOrigin + Vector(RandomFloat(-m_iRadius, m_iRadius), RandomFloat(-m_iRadius, m_iRadius), 0);
		}
		else if (m_fTime < engine->Time())
		{
			m_fTime = 0;
		}
		else
		{
			static float fdIst;

			fdIst = pBot->DistanceFrom(m_vMoveTo);

			if (pTF2Bot->GetClass() == TF_CLASS_SPY)
			{
				if (pTF2Bot->IsDisguised())
					pBot->PrimaryAttack(); // remove dIsguIse to capture

				pTF2Bot->WantToDisguise(false);

				// block cloaking
				if (pTF2Bot->IsCloaked())
				{
					// uncloak
					pTF2Bot->SpyUnCloak();
				}
				else
				{
					pBot->LetGoOfButton(IN_ATTACK2);
				}

				pTF2Bot->WaitCloak();

			}

			if (fdIst < 52)
			{
				pBot->StopMoving();
			}
			else if (fdIst > 400)
				Fail();
			else
			{
				pBot->SetMoveTo((m_vMoveTo));
			}

			pBot->SetLookAtTask(LOOK_AROUND);

			if (((CBotTF2*)pBot)->CheckAttackPoint())
				Complete();
		}
	}
}

////////////////////////
void CPrimaryAttack::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	pBot->PrimaryAttack();
	Complete();
}

////////////////////////////

CBotTF2PushPayloadBombTask::CBotTF2PushPayloadBombTask(edict_t * pPayloadBomb)
{
	m_pPayloadBomb = pPayloadBomb;
	m_fPushTime = 0;
	m_fTime = 0;
	m_vRandomOffset = Vector(0, 0, 0);
}

void CBotTF2PushPayloadBombTask::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	pBot->WantToInvestigateSound(false);

	if (m_fPushTime == 0)
	{
		m_fPushTime = engine->Time() + RandomFloat(10.0, 30.0);
		m_vRandomOffset = Vector(RandomFloat(-50, 50), RandomFloat(-50, 50), 0);
	}
	else if (m_fPushTime < engine->Time())
	{
		Complete();
	}
	else if (m_pPayloadBomb.Get() == NULL)
	{
		Complete();
		return;
	}
	else
	{

		m_vOrigin = CBotGlobals::EntityOrigin(m_pPayloadBomb);
		//m_vMoveTo = m_vOrigin + Vector(RandomFloat(-10,10),RandomFloat(-10,10),0);
		m_vMoveTo = m_vOrigin + m_vRandomOffset;

		if (pBot->DistanceFrom(m_vMoveTo) < 100)
		{
			if ((((CBotTF2*)pBot)->GetClass() == TF_CLASS_SPY) && (((CBotTF2*)pBot)->IsDisguised()))
				pBot->PrimaryAttack(); // remove dIsguIse to capture

			((CBotFortress*)pBot)->WantToDisguise(false);

		}
		else
			pBot->SetMoveTo(m_vMoveTo);

		pBot->SetLookAtTask(LOOK_AROUND);
	}

}

////////////////////////////////////////////////////////////////////////

CBotTF2DefendPayloadBombTask::CBotTF2DefendPayloadBombTask(edict_t * pPayloadBomb)
{
	m_pPayloadBomb = pPayloadBomb;
	m_fDefendTime = 0;
	m_fTime = 0;
	m_vRandomOffset = Vector(0, 0, 0);
}

void CBotTF2DefendPayloadBombTask::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	if (m_fDefendTime == 0)
	{
		m_fDefendTime = engine->Time() + RandomFloat(10.0f, 30.0f);
		m_vRandomOffset = Vector(RandomFloat(-150.0f, 150.0f), RandomFloat(-150.0f, 150.0f), 0);
	}
	else if (m_fDefendTime < engine->Time())
	{
		Complete();
	}
	else if (m_pPayloadBomb.Get() == NULL)
	{
		Complete();
		return;
	}
	else
	{
		m_vOrigin = CBotGlobals::EntityOrigin(m_pPayloadBomb);
		m_vMoveTo = m_vOrigin + m_vRandomOffset;

		if (pBot->DistanceFrom(m_vMoveTo) > 200)
			pBot->SetMoveTo(m_vMoveTo);
		else
			pBot->StopMoving();

		pBot->SetLookAtTask(LOOK_EDICT);
		pBot->LookAtEdict(m_pPayloadBomb);
	}

}

//////////////////////
CBotTF2DefendPoint::CBotTF2DefendPoint(int iArea, Vector vOrigin, int iRadius)
{
	m_vOrigin = vOrigin;
	m_fDefendTime = 0;
	m_fTime = 0;
	m_iArea = iArea;
	m_iRadius = iRadius;
}

void CBotTF2DefendPoint::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	int iCpIndex = CTeamFortress2Mod::m_ObjectiveResource.m_WaypointAreaToIndexTranslation[m_iArea];
	int iTeam = pBot->GetTeam();

	if (m_iArea && (CTeamFortress2Mod::m_ObjectiveResource.GetOwningTeam(iCpIndex) != iTeam))
	{
		// doesn't belong to us/can't defend anymore
		((CBotTF2*)pBot)->UpdateAttackDefendPoints();
		Complete(); // done
	}
	else if (m_iArea && !CTeamFortress2Mod::m_ObjectiveResource.IsCPValid(iCpIndex, iTeam, TF2_POINT_DEFEND))
	{
		((CBotTF2*)pBot)->UpdateAttackDefendPoints();
		Fail(); // too slow
	}
	else if (m_fDefendTime == 0)
	{
		m_fDefendTime = engine->Time() + RandomFloat(30.0, 60.0);
		pBot->ResetLookAroundTime();
	}
	else if (m_fDefendTime < engine->Time())
		Complete();
	else
	{
		if (m_fTime == 0)
		{
			float fdIst;

			m_fTime = engine->Time() + RandomFloat(5.0, 10.0);
			m_vMoveTo = m_vOrigin + Vector(RandomFloat(-m_iRadius, m_iRadius), RandomFloat(-m_iRadius, m_iRadius), 0);
			fdIst = pBot->DistanceFrom(m_vMoveTo);

			if (fdIst < 32)
				pBot->StopMoving();
			else if (fdIst > 400)
				Fail();
			else
			{
				pBot->SetMoveTo(m_vMoveTo);
			}
		}
		else if (m_fTime < engine->Time())
		{
			m_fTime = 0;
		}
		pBot->SetLookAtTask(LOOK_SNIPE);
	}
}

///////////
CBotTF2UpgradeBuilding::CBotTF2UpgradeBuilding(edict_t *pBuilding)
{
	m_pBuilding = pBuilding;
	m_fTime = 0.0f;
}

void CBotTF2UpgradeBuilding::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	edict_t *pBuilding = m_pBuilding.Get();
	edict_t *pOwner = NULL;
	pBot->WantToShoot(false);
	pBot->WantToInvestigateSound(false);
	pBot->WantToListen(false);

	if (!m_fTime)
		m_fTime = engine->Time() + RandomFloat(9.0f, 11.0f);

	if (m_fTime < engine->Time())
	{
		Complete();
	}// Fix 16/07/09
	else if (pBuilding == NULL)
	{
		Fail();
		return;
	}
	else if (((pOwner = CTeamFortress2Mod::GetSentryOwner(pBuilding)) != NULL) &&
		CClassInterface::IsCarryingObj(pOwner) && (CClassInterface::GetCarriedObj(pOwner) == pBuilding))
	{
		// Owner Is carrying it
		Fail();
		return;
	}
	else if (!pBot->IsVisible(pBuilding))
	{
		if (pBot->DistanceFrom(pBuilding) > 200)
			Fail();
		else if (pBot->DistanceFrom(pBuilding) > 100)
			pBot->SetMoveTo(CBotGlobals::EntityOrigin(pBuilding));

		pBot->SetLookAtTask(LOOK_EDICT, 0.15f);
		pBot->LookAtEdict(pBuilding);
	}
	else if (CBotGlobals::EntityIsValid(pBuilding) && CBotGlobals::EntityIsAlive(pBuilding))
	{
		if (!((CBotFortress*)pBot)->UpgradeBuilding(pBuilding))
			Fail();
	}
	else
		Fail();
}

/*void CBotHL2DMUseButton :: Execute (CBot *pBot,CBotSchedule *pSchedule)
{
static Vector vOrigin;

if ( m_pButton.Get() == NULL )
{
Fail();
return;
}

vOrigin = CBotGlobals::EntityOrigin(m_pButton);

if ( m_fTime == 0.0f )
{
m_fTime = engine->Time() + RandomFloat(4.0f,6.0f);
}

if ( m_fTime < engine->Time() )
Complete();

//if ( CClassInterface::GetAnimCycle(m_pCharger) == 1.0f )
//	Complete();

pBot->SetLookVector(vOrigin);
pBot->SetLookAtTask(LOOK_VECTOR);

if ( pBot->DistanceFrom(m_pButton) > 96 )
{
pBot->SetMoveTo(vOrigin);
}
else if ( pBot->IsFacing(vOrigin) )
{
pBot->use();
Complete();
}
}

void CBotHL2DMUseCharger :: Execute (CBot *pBot,CBotSchedule *pSchedule)
{
static Vector vOrigin;

if ( m_pCharger.Get() == NULL )
{
Fail();
return;
}

vOrigin = CBotGlobals::EntityOrigin(m_pCharger);

if ( m_fTime == 0.0f )
{
m_fTime = engine->Time() + RandomFloat(4.0f,6.0f);
}

if ( m_fTime < engine->Time() )
Complete();

if ( CClassInterface::GetAnimCycle(m_pCharger) == 1.0f )
Complete();

if ( ( m_iType == CHARGER_HEALTH ) && ( pBot->GetHealthPercent() >= 0.99f ) )
Complete();
else if ( ( m_iType == CHARGER_ARMOR ) && ( ((CHLDMBot*)pBot)->GetArmorPercent() >= 0.99f ) )
Complete();

pBot->SetLookVector(vOrigin);
pBot->SetLookAtTask(LOOK_VECTOR);

if ( pBot->DistanceFrom(m_pCharger) > 96 )
{
pBot->SetMoveTo(vOrigin);
}
else if ( pBot->IsFacing(vOrigin) )
{
pBot->use();
}
}

void CBotGravGunPickup :: Execute(CBot *pBot,CBotSchedule *pSchedule)
{
static Vector vOrigin;
static Vector vBotOrigin;

if ( m_fTime == 0.0f )
{
m_fSecAttTime = 0;
m_fTime = engine->Time() + RandomFloat(2.0f,4.0f);
}

if ( m_fTime < engine->Time() )
{
CHLDMBot *HL2DMBot = ((CHLDMBot*)pBot);

if (HL2DMBot->GetFailedObject() && (HL2DMBot->DistanceFrom(HL2DMBot->GetFailedObject())<=(pBot->DistanceFrom(m_Prop)+48)) )
pBot->PrimaryAttack();

HL2DMBot->SetFailedObject(m_Prop);

Fail();
return;
}

if ( !CBotGlobals::EntityIsValid(m_Prop) || !pBot->IsVisible(m_Prop) )
{
((CHLDMBot*)pBot)->SetFailedObject(m_Prop);
Fail();
return;
}

pBot->WantToChangeWeapon(false);

vBotOrigin = pBot->GetOrigin();
vOrigin = CBotGlobals::EntityOrigin(m_Prop);

CBotWeapon *pWeapon = pBot->GetCurrentWeapon();

if ( !pWeapon || ( pWeapon->GetID() != HL2DM_WEAPON_PHYSCANNON)  )
{
if ( !pBot->Select_CWeapon(CWeapons::GetWeapon(HL2DM_WEAPON_PHYSCANNON)) )
{
Fail();
return;
}
}
else if ( pBot->DistanceFrom(vOrigin) > 100 )
pBot->SetMoveTo(vOrigin);
else if ( ((vOrigin-vBotOrigin).Length2D() < 16) && (vOrigin.z < vBotOrigin.z) )
pBot->SetMoveTo(vBotOrigin + (vBotOrigin-vOrigin)*100);
else
pBot->StopMoving();

if ( pWeapon )
m_Weapon = INDEXENT(pWeapon->GetWeaponIndex());
else
{
Fail();
return;
}

pBot->SetMoveLookPriority(MOVELOOK_OVERRIDE);
pBot->SetLookVector(vOrigin);
pBot->SetLookAtTask(LOOK_VECTOR);
pBot->SetMoveLookPriority(MOVELOOK_TASK);

if ( pBot->IsFacing(vOrigin) )
{
edict_t *pPhys = CClassInterface::gravityGunObject(m_Weapon);

if ( pPhys == m_Prop.Get() )
Complete();
else if ( pPhys || CClassInterface::gravityGunOpen(m_Weapon) )
{
if ( m_fSecAttTime < engine->Time() )
{
pBot->SecondaryAttack();
m_fSecAttTime = engine->Time() + RandomFloat(0.25,0.75);
}
}
}
}*/

///////////////////////////////////////////////////////////////////////
/*
// Protect SG from Enemy
CBotTFEngiTankSentry :: CBotTFEngiTankSentry ( edict_t *pSentry, edict_t *pEnemy )
{
m_pEnemy = pEnemy;
m_pSentry = pSentry;
}

void CBotTFEngiTankSentry :: Execute (CBot *pBot,CBotSchedule *pSchedule)
{

CBotFortress *tfBot;

if ( !pBot->IsTF() )
{
Fail();
return;
}

tfBot = (CBotFortress*)pBot;


if ( !CBotGlobals::EntityIsAlive(m_pEnemy) )
Complete();
else if ( !CBotGlobals::EntityIsAlive(m_pSentry) || !CBotGlobals::EntityIsValid(m_pSentry) || !CTeamFortress2Mod::IsSentry(m_pSentry) )
Fail();
else
{
Vector vOrigin;
Vector vComp;

vComp = CBotGlobals::EntityOrigin(m_pEnemy) - CBotGlobals::EntityOrigin(m_pSentry);
vComp = vComp / vComp.Length(); // NormalIse

// find task position behind Sentry
vOrigin = CBotGlobals::EntityOrigin(m_pSentry) - (vComp*80);

if ( pBot->DistanceFrom(vOrigin) > 32  )
{
// Get into position!
pBot->SetMoveTo((vOrigin));
}
else
{
if ( !pBot->currentWeapon("tf_wrench") )
pBot->SelectWeaponName("tf_wrench");
else
{
SetTaskEntity();
pBot->SetLookAt(TSK_ENTITY,2);

// Tank!!!
pBot->duck();
pBot->tapButton(IN_ATTACK);
}

}
}

}
*/

////////////////////////

////////////////////////


CBotTF2WaitAmmoTask::CBotTF2WaitAmmoTask(Vector vOrigin)
{
	m_vOrigin = vOrigin;
	m_fWaitTime = 0.0f;
}

void CBotTF2WaitAmmoTask::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	if (!m_fWaitTime)
		m_fWaitTime = engine->Time() + RandomFloat(5.0f, 10.0f);

	if (!pBot->HasSomeConditions(CONDITION_NEED_AMMO))
	{
		Complete();
	}
	else if (m_fWaitTime < engine->Time())
		Fail();
	else if (pBot->DistanceFrom(m_vOrigin) > 100)
	{
		pBot->SetMoveTo(m_vOrigin);
	}
	else
	{
		pBot->StopMoving();
	}
}

///////////////////////////
CBotTaskEngiPickupBuilding::CBotTaskEngiPickupBuilding(edict_t *pBuilding)
{
	m_pBuilding = pBuilding;
	m_fTime = 0.0f;
}
// move Building / move Sentry / move dIsp / move tele
void CBotTaskEngiPickupBuilding::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	CBotWeapon *pWeapon = pBot->GetCurrentWeapon();

	if (m_fTime == 0.0f)
		m_fTime = engine->Time() + 6.0f;

	if (!m_pBuilding.Get())
	{
		Fail();
		return;
	}

	// don't pick up Sentry now because my Sentry Is in good use!!!
	if (((CBotTF2*)pBot)->SentryRecentlyHadEnemy())
	{
		Fail();
		return;
	}

	pBot->WantToShoot(false);
	pBot->LookAtEdict(m_pBuilding.Get());
	pBot->SetLookAtTask(LOOK_EDICT);

	((CBotTF2*)pBot)->UpdateCarrying();

	if (((CBotTF2*)pBot)->IsCarrying()) //if ( CBotGlobals::EntityOrigin(m_pBuilding) == CBotGlobals::EntityOrigin(pBot->GetEdict()) )
		Complete();
	else if (m_fTime < engine->Time())
		Fail();
	else if (!pWeapon)
		Fail();
	else if (pWeapon->GetID() != TF2_WEAPON_WRENCH)
	{
		if (!pBot->Select_CWeapon(CWeapons::GetWeapon(TF2_WEAPON_WRENCH)))
			Fail();
	}
	else if (pBot->DistanceFrom(m_pBuilding) < 100)
	{
		if (CBotGlobals::YawAngleFromEdict(pBot->GetEdict(), CBotGlobals::EntityOrigin(m_pBuilding)) < 25)
		{
			if (RandomInt(0, 1))
				pBot->SecondaryAttack();
			else
				pBot->LetGoOfButton(IN_ATTACK2);

			((CBotTF2*)pBot)->ResetCarryTime();
		}
	}
	else
		pBot->SetMoveTo((CBotGlobals::EntityOrigin(m_pBuilding)));
}

/////////////////
CBotTaskEngiPlaceBuilding::CBotTaskEngiPlaceBuilding(eEngiBuild iObject, Vector vOrigin)
{
	m_vOrigin = vOrigin;
	m_fTime = 0.0f;
	m_iState = 1; // BEGIN HERE , otherwIse bot will try to destroy the Building
	m_iObject = iObject;
	m_iTries = 0;
}

// unused
void CBotTaskEngiPlaceBuilding::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	pBot->WantToInvestigateSound(false);

	if (m_fTime == 0.0f)
	{
		m_fTime = engine->Time() + 6.0f;
		pBot->ResetLookAroundTime();

		if (CTeamFortress2Mod::BuildingNearby(pBot->GetTeam(), m_vOrigin))
			m_vOrigin = m_vOrigin + Vector(RandomFloat(-200.0f, 200.0f), RandomFloat(-200.0f, 200.0f), 0);
	}

	pBot->SetLookVector(m_vOrigin);
	pBot->SetLookAtTask(LOOK_VECTOR);

	((CBotTF2*)pBot)->UpdateCarrying();

	if (!(((CBotTF2*)pBot)->IsCarrying()))
		Complete();
	else if (m_fTime < engine->Time())
		Fail();
	else if (pBot->DistanceFrom(m_vOrigin) < 100)
	{
		if (CBotGlobals::YawAngleFromEdict(pBot->GetEdict(), m_vOrigin) < 25)
		{
			int state = ((CBotTF2*)pBot)->EngiBuildObject(&m_iState, m_iObject, &m_fTime, &m_iTries);

			if (state == 1)
				Complete();
			else if (state == 0)
				Fail();

			//pBot->PrimaryAttack();
		}
	}
	else
		pBot->SetMoveTo(m_vOrigin);

	if (pBot->HasEnemy())
	{
		if (RandomInt(0, 1))
			pBot->PrimaryAttack();
	}
}

/////////////////////////////
CBotUseLunchBoxDrink::CBotUseLunchBoxDrink()
{
	m_fTime = 0.0f;
}
void CBotUseBuffItem::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	CBotWeapon *pWeapon;

	pBot->WantToShoot(false);
	pBot->WantToChangeWeapon(false);

	if (m_fTime == 0)
		m_fTime = engine->Time() + 5.0f;
	else if (m_fTime < engine->Time())
		Fail();

	pWeapon = pBot->GetCurrentWeapon();

	if (CClassInterface::GetRageMeter(pBot->GetEdict()) < 100.0f)
	{
		Fail();
		return;
	}

	if (!pWeapon || (pWeapon->GetID() != TF2_WEAPON_BUFF_ITEM))
	{
		if (!pBot->Select_CWeapon(CWeapons::GetWeapon(TF2_WEAPON_BUFF_ITEM)))
			Fail();
	}
	else
	{
		if (RandomInt(0, 1))
			pBot->PrimaryAttack();
	}
}

CBotUseBuffItem::CBotUseBuffItem()
{
	m_fTime = 0.0f;
}
void CBotUseLunchBoxDrink::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	CBotWeapon *pWeapon;

	pBot->WantToShoot(false);
	pBot->WantToChangeWeapon(false);

	if (m_fTime == 0)
		m_fTime = engine->Time() + 5.0f;
	else if (m_fTime < engine->Time())
		Fail();

	pWeapon = pBot->GetCurrentWeapon();

	if (CClassInterface::TF2_GetEnergyDrinkMeter(pBot->GetEdict()) < 100.0f)
	{
		Fail();
		return;
	}

	if (!pWeapon || (pWeapon->GetID() != TF2_WEAPON_LUNCHBOX_DRINK))
	{
		if (!pBot->Select_CWeapon(CWeapons::GetWeapon(TF2_WEAPON_LUNCHBOX_DRINK)))
			Fail();
	}
	else
	{
		if (RandomInt(0, 1))
			pBot->PrimaryAttack();
	}
}
/////////////////////////////
CBotBackstab::CBotBackstab(edict_t *_pEnemy)
{
	m_fTime = 0.0f;
	pEnemy = _pEnemy;
}

void CBotBackstab::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	Vector vrear;
	Vector vangles;
	CBotWeapon *pBotWeapon;
	CWeapon *pWeapon;
	CBotTF2 *pTF2Bot = (CBotTF2*)pBot;

	pBot->WantToChangeWeapon(false);
	pBot->WantToShoot(false);
	pBot->WantToListen(false);

	pBotWeapon = pBot->GetCurrentWeapon();

	if (!pBotWeapon)
	{
		Fail();
		pTF2Bot->WaitBackstab();
		return;
	}

	pWeapon = pBotWeapon->GetWeaponInfo();

	if (pWeapon == NULL)
	{
		Fail();
		pTF2Bot->WaitBackstab();
		return;
	}

	if (!CBotGlobals::IsAlivePlayer(pEnemy))
		Fail();

	if (!m_fTime)
		m_fTime = engine->Time() + RandomFloat(5.0f, 10.0f);

	pBot->SetLookAtTask(LOOK_EDICT);
	pBot->LookAtEdict(pEnemy);

	if (m_fTime < engine->Time())
	{
		Fail();
		pTF2Bot->WaitBackstab();
		return;
	}
	else if (!pEnemy || !CBotGlobals::EntityIsValid(pEnemy) || !CBotGlobals::EntityIsAlive(pEnemy))
	{
		if (pBot->GetEnemy() && (pEnemy != pBot->GetEnemy()) && pBot->HasSomeConditions(CONDITION_SEE_CUR_ENEMY) && CBotGlobals::IsAlivePlayer(pBot->GetEnemy()))
		{
			pEnemy = pBot->GetEnemy();
		}
		else
		{
			Fail();
		}

		pTF2Bot->WaitBackstab();
		return;
	}
	else if (!pBot->IsVisible(pEnemy))
	{
		// thIs guy will do
		if (pBot->GetEnemy() && (pEnemy != pBot->GetEnemy()) && pBot->HasSomeConditions(CONDITION_SEE_CUR_ENEMY) && CBotGlobals::IsAlivePlayer(pBot->GetEnemy()))
		{
			pEnemy = pBot->GetEnemy();
		}
		else
		{
			Fail();
		}

		pTF2Bot->WaitBackstab();
		return;
	}
	else if (pWeapon->GetID() != TF2_WEAPON_KNIFE)
	{
		if (!pBot->Select_CWeapon(CWeapons::GetWeapon(TF2_WEAPON_KNIFE)))
		{
			Fail();
			pTF2Bot->WaitBackstab();
			return;
		}
	}

	AngleVectors(CBotGlobals::EntityEyeAngles(pEnemy), &vangles);
	vrear = CBotGlobals::EntityOrigin(pEnemy) - (vangles * 45) + Vector(0, 0, 32);

	if (pBot->DistanceFrom(vrear) > 40)
	{
		pBot->SetMoveTo(vrear);
	}
	else
	{
		// uncloak
		if (CTeamFortress2Mod::TF2_IsPlayerCloaked(pBot->GetEdict()))
			pTF2Bot->SpyUnCloak();
		else
			pTF2Bot->HandleAttack(pBotWeapon, pEnemy);
	}
}

void CBotInvestigateTask::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	if (m_fTime == 0)
	{
		CWaypoint *pWaypoint = CWaypoints::GetWaypoint(CWaypointLocations::NearestWaypoint(m_vOrigin, m_fRadius, -1));

		if (pWaypoint == NULL)
		{
			// can't investigate
			// but other tasks depend on thIs, so Complete it
			Complete();
			return;
		}

		if (pWaypoint->NumPaths() > 0)
		{
			for (int i = 0; i < pWaypoint->NumPaths(); i++)
				m_InvPoints.push_back(CWaypoints::GetWaypoint(pWaypoint->GetPath(i))->GetOrigin());

			m_iCurPath = RandomInt(0, pWaypoint->NumPaths() - 1);
		}

		m_fTime = engine->Time() + m_fMaxTime;
	}

	if (m_fTime < engine->Time())
		Complete();

	if (m_InvPoints.size() > 0)
	{
		Vector vPoint;

		if (m_iState == 0) // goto inv point
			vPoint = m_InvPoints[m_iCurPath];
		else if (m_iState == 1) // goto origin
			vPoint = m_vOrigin;

		if ((pBot->DistanceFrom(vPoint) < 80) || ((m_iState == 0) && (pBot->DistanceFrom(m_vOrigin) > m_fRadius)))
		{
			m_iState = (!m_iState) ? 1 : 0;

			if (m_iState == 0)
				m_iCurPath = RandomInt(0, m_InvPoints.size() - 1);
		}
		else
			pBot->SetMoveTo(vPoint);
	}
	else
		pBot->StopMoving();
	// walk
	pBot->SetMoveSpeed(CClassInterface::GetMaxSpeed(pBot->GetEdict()) / 8);
	//pBot->SetLookVector();

	if (m_bHasPOV)
	{
		pBot->SetLookVector(m_vPOV);
		pBot->SetLookAtTask(LOOK_VECTOR);
	}
	else
		pBot->SetLookAtTask(LOOK_AROUND);

}
///////////////////

void CBotDefendTask::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	static float fDIst;

	if (m_fTime == 0)
	{
		if (pBot->InSquad() && pBot->IsSquadLeader())
		{
			m_fTime = engine->Time() + RandomFloat(15.0f, 45.0f);
		}
		else if (m_fMaxTime > 0)
			m_fTime = engine->Time() + m_fMaxTime;
		else
		{
			m_fTime = engine->Time() + RandomFloat(20.0f, 90.0f);

			if (pBot->IsTF2())
			{
				if (CTeamFortress2Mod::IsMapType(TF_MAP_MVM))
				{
					Vector vFlag;

					if (CTeamFortress2Mod::GetFlagLocation(TF2_TEAM_BLUE, &vFlag))
					{
						// FOR DEBUGGING
						float fDIst = (vFlag - m_vOrigin).Length();
						float fWaitTime = (90.0f - (fDIst / 45)); // MAX DIst 4050
						//
						m_fTime = engine->Time() + fWaitTime;
					}
				}
			}
		}
	}

	fDIst = pBot->DistanceFrom(m_vOrigin);

	if (fDIst > 200)
		Fail(); // too far -- bug
	else if (fDIst > 100)
		pBot->SetMoveTo(m_vOrigin);
	else
	{
		pBot->Defending();

		pBot->StopMoving();

		if (m_iWaypointType & CWaypointTypes::W_FL_CROUCH)
			pBot->Duck();

		if (m_bDefendOrigin)
		{
			pBot->SetAiming(m_vDefendOrigin);
			pBot->SetLookVector(m_vDefendOrigin);
		}

		pBot->SetLookAtTask(m_LookTask);

		//pBot->SetAiming(m_vDefendOrigin);

		/*if ( m_bDefendOrigin )
		{
		pBot->SetLookAtTask(LOOK_AROUND);
		//pBot->SetAiming(m_vDefendOrigin);
		}
		else
		pBot->SetLookAtTask(LOOK_SNIPE);*/
	}

	/*if ( m_fTime < engine->Time() )
	{
	if ( pBot->inSquad() && pBot->IsSquadLeader() )
	pBot->addVoiceCommand(DOD_VC_GOGOGO);

	Complete();
	}*/
}

//////////////////////
void CBotTF2EngiLookAfter::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	CBotFortress *tfBot = (CBotFortress*)pBot;

	if (!m_fTime)
	{
		m_fTime = engine->Time() + RandomFloat(21.0f, 60.0f);
		m_fHitSentry = engine->Time() + RandomFloat(1.0f, 3.0f);
	}
	else if (m_fTime < engine->Time())
	{
		tfBot->NextLookAfterSentryTime(engine->Time() + RandomFloat(20.0f, 50.0f));
		Complete();
	}
	else
	{
		CBotWeapon *pWeapon = pBot->GetCurrentWeapon();
		edict_t *pSentry = m_pSentry.Get();

		pBot->WantToListen(false);

		pBot->SetLookAtTask(LOOK_AROUND);

		if (!pWeapon)
		{
			Fail();
			return;
		}
		else if (pWeapon->GetID() != TF2_WEAPON_WRENCH)
		{
			if (!pBot->Select_CWeapon(CWeapons::GetWeapon(TF2_WEAPON_WRENCH)))
			{
				Fail();
				return;
			}
		}

		if (pSentry != NULL)
		{
			if (pBot->DistanceFrom(pSentry) > 100)
				pBot->SetMoveTo(CBotGlobals::EntityOrigin(pSentry));
			else
			{
				pBot->StopMoving();

				if ((CClassInterface::GetSentryEnemy(pSentry) != NULL) ||
					(CClassInterface::GetSentryHealth(pSentry) < CClassInterface::GetTF2GetBuildingMaxHealth(pSentry)) ||
					(CClassInterface::GetTF2SentryRockets(pSentry) < 20) ||
					(CClassInterface::GetTF2SentryShells(pSentry) < 150))
				{
					pBot->PrimaryAttack();
				}


				pBot->Duck(true); // crouch too

			}

			pBot->LookAtEdict(pSentry);
			pBot->SetLookAtTask(LOOK_EDICT); // LOOK_EDICT fix engineers not looking at their Sentry

		}
		else
			Fail();
	}
}

////////////////////////
CBotTFEngiBuildTask::CBotTFEngiBuildTask(eEngiBuild iObject, CWaypoint *pWaypoint)
{
	m_iObject = iObject;
	m_vOrigin = pWaypoint->GetOrigin() + pWaypoint->ApplyRadius();
	m_iState = 0;
	m_fTime = 0;
	m_iTries = 0;
	m_fNextUpdateAngle = 0.0f;
	QAngle ang = QAngle(0, pWaypoint->GetAimYaw(), 0);
	Vector vForward;
	AngleVectors(ang, &vForward);
	m_vAimingVector = m_vOrigin + (vForward*100.0f);
	m_iArea = pWaypoint->GetArea();
	m_vBaseOrigin = m_vOrigin;
	m_fRadius = pWaypoint->GetRadius();
}

void CBotTFEngiBuildTask::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	CBotFortress *tfBot;

	bool bAimingOk = true;

	pBot->WantToInvestigateSound(false);

	//if ( !pBot->IsTF() ) // shouldn't happen ever
	//	Fail();

	if (!CTeamFortress2Mod::m_ObjectiveResource.IsWaypointAreaValid(m_iArea))
		Fail();

	pBot->WantToShoot(false); // don't shoot enemies , Want to build the damn thing
	pBot->WantToChangeWeapon(false); // if enemy just strike them with wrench
	pBot->WantToListen(false); // sometimes bots dont place sentries because they are looking the wrong way due to a noIse

	tfBot = (CBotFortress*)pBot;

	if (tfBot->GetClass() != TF_CLASS_ENGINEER)
	{
		Fail();
		return;
	}
	/*else if ( pBot->HasEnemy() && pBot->HasSomeConditions(CONDITION_SEE_CUR_ENEMY) )
	{
	Fail();
	return;
	}*/

	if (m_fTime == 0.0f)
	{
		pBot->ResetLookAroundTime();

		if (CTeamFortress2Mod::BuildingNearby(pBot->GetTeam(), m_vOrigin))
		{
			if (m_fRadius > 0.0f)
				m_vOrigin = m_vBaseOrigin + Vector(RandomFloat(-m_fRadius, m_fRadius), RandomFloat(-m_fRadius, m_fRadius), 0);
			else
			{
				// can't build here
				Fail();
				return;
			}
		}

		m_fTime = engine->Time() + RandomFloat(4.0f, 8.0f);

		m_fNextUpdateAngle = engine->Time() + 0.5f;
	}
	else if (m_fTime < engine->Time())
		Fail();

	if (m_iObject == ENGI_DISP)
	{
		edict_t *pSentry = tfBot->GetSentry();

		if (pSentry && CBotGlobals::EntityIsValid(pSentry))
		{
			Vector vSentry = CBotGlobals::EntityOrigin(pSentry);
			Vector vOrigin = pBot->GetOrigin();
			Vector vLookAt = vOrigin - (vSentry - vOrigin);

			m_vAimingVector = vLookAt;

			//pBot->SetLookVector(vLookAt);
			//pBot->SetLookAtTask(LOOK_VECTOR);

			//LOOK_VECTOR,11);
			//bAimingOk = pBot->DotProductFromOrigin(vLookAt) > 0.965925f; // 15 degrees // < CBotGlobals::YawAngleFromEdict(pBot->GetEdict(),pBot->GetLookVector()) < 15;
		}
		else
		{
			Vector vSentry = pBot->GetAiming();
			Vector vOrigin = pBot->GetOrigin();
			Vector vLookAt = vOrigin - (vSentry - vOrigin);

			m_vAimingVector = vLookAt;

			/*pBot->SetLookVector(pBot->GetAiming());
			pBot->SetLookAtTask((LOOK_VECTOR));*/
			//bAimingOk = pBot->DotProductFromOrigin(vLookAt) > 0.965925f;
		}
	}

	if ((m_iTries > 1) && (m_fNextUpdateAngle < engine->Time()))
	{
		QAngle angles;
		Vector vforward = m_vAimingVector - pBot->GetOrigin();
		vforward = vforward / vforward.Length(); // normalize

		VectorAngles(vforward, angles);

		angles.y += RandomFloat(-60.0f, 60.0f); // Yaw
		CBotGlobals::FixFloatAngle(&angles.y);

		AngleVectors(angles, &vforward);

		vforward = vforward / vforward.Length();

		m_vAimingVector = pBot->GetOrigin() + vforward*100.0f;

		m_fNextUpdateAngle = engine->Time() + 1.5f;
	}

	pBot->SetLookAtTask(LOOK_VECTOR, 0.2f);
	pBot->SetLookVector(m_vAimingVector);

	bAimingOk = pBot->IsFacing(m_vAimingVector); // 15 degrees

	if (pBot->DistanceFrom(m_vOrigin) > 70.0f)
	{
		if (!CBotGlobals::IsVisible(pBot->GetEdict(), pBot->GetEyePosition(), m_vOrigin))
			Fail();
		else
			pBot->SetMoveTo(m_vOrigin);
	}
	else if (bAimingOk || (m_iTries > 1))
	{
		int state = tfBot->EngiBuildObject(&m_iState, m_iObject, &m_fTime, &m_iTries);

		if (state == 1)
			Complete();
		else if (state == 0)
			Fail();
		else if (state == 3) // Failed , try again 
		{
			if (m_fRadius > 0.0f)
				m_vOrigin = m_vBaseOrigin + Vector(RandomFloat(-m_fRadius, m_fRadius), RandomFloat(-m_fRadius, m_fRadius), 0);
			else
			{
				// can't build here
				Fail();
				return;
			}
		}
	}
}

///////////////////////////////////////////////////////

CFindGoodHideSpot::CFindGoodHideSpot(edict_t *pEntity)
{
	m_vHideFrom = CBotGlobals::EntityOrigin(pEntity);
}

CFindGoodHideSpot::CFindGoodHideSpot(Vector vec)
{
	m_vHideFrom = vec;
}

void CFindGoodHideSpot::Init()
{
	// not required, should have been constructed properly
	//m_vHideFrom = Vector(0,0,0);
}

void CFindGoodHideSpot::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	Vector vFound;

	if (!pBot->GetNavigator()->GetHideSpotPosition(m_vHideFrom, &vFound))
		Fail();
	else
	{
		pSchedule->PassVector(vFound);
		Complete();
	}
}

CFindPathTask::CFindPathTask(int iWaypointId, eLookTask looktask)
{
	m_iWaypointId = iWaypointId;
	m_LookTask = looktask;
	m_vVector = CWaypoints::GetWaypoint(iWaypointId)->GetOrigin();
	m_flags.m_data = 0;
	m_fRange = 0.0f;
	m_iDangerPoint = -1;
	m_bGetPassedIntAsWaypointId = false;
}

void CFindPathTask::Init()
{
	m_flags.m_data = 0;
	m_iInt = 0;
	m_fRange = 0.0f;
	m_iDangerPoint = -1;
	m_bGetPassedIntAsWaypointId = false;
	//SetFailInterrupt(CONDITION_SEE_CUR_ENEMY);
}

CFindPathTask::CFindPathTask(edict_t *pEdict)
{
	m_iWaypointId = -1;
	m_pEdict = pEdict;
	m_vVector = CBotGlobals::EntityOrigin(pEdict);
	m_LookTask = LOOK_WAYPOINT;
	m_flags.m_data = 0;
	m_fRange = 0.0f;
	m_iDangerPoint = -1;
	m_bGetPassedIntAsWaypointId = false;
}

void CFindPathTask::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	bool bFail = false;

	if (m_LookTask == LOOK_NOISE)
		pBot->WantToListen(false); // vector already Set before find path task

	if (m_bGetPassedIntAsWaypointId)
	{
		m_iWaypointId = pSchedule->PassedInt();

		if (m_iWaypointId == -1)
		{
			Fail();
			return;
		}

		m_vVector = CWaypoints::GetWaypoint(m_iWaypointId)->GetOrigin();
	}
	else if (pSchedule->HasPassVector())
	{
		m_vVector = pSchedule->PassedVector();
		pSchedule->ClearPass();
	}

	if ((m_iInt == 0) || (m_iInt == 1))
	{
		IBotNavigator *pNav = pBot->GetNavigator();

		pBot->m_fWaypointStuckTime = 0;

		if (pNav->WorkRoute(pBot->GetOrigin(),
			m_vVector,
			&bFail,
			(m_iInt == 0),
			m_flags.bits.m_bNoInterruptions,
			m_iWaypointId,
			pBot->GetConditions(), m_iDangerPoint))
		{
			pBot->m_fWaypointStuckTime = engine->Time() + RandomFloat(10.0f, 15.0f);
			pBot->MoveFailed(); // Reset
			m_iInt = 2;
		}
		else
			m_iInt = 1;
	}

	if (bFail)
	{
		Fail();
	}
	else if (m_iInt == 2)
	{
		if (pBot->m_fWaypointStuckTime == 0)
			pBot->m_fWaypointStuckTime = engine->Time() + RandomFloat(5.0f, 10.0f);

		if (!pBot->GetNavigator()->HasNextPoint())
		{
			Complete(); // reached goal
		}
		else
		{
			if (pBot->MoveFailed())
			{
				Fail();
				pBot->GetNavigator()->FailMove();
			}

			if (m_pEdict)
			{
				if (CBotGlobals::EntityIsValid(m_pEdict))
				{
					pBot->LookAtEdict(m_pEdict);

					if (m_flags.bits.m_bCompleteInRangeOfEdict && m_flags.bits.m_bCompleteSeeTaskEdict)
					{
						// Complete if inrange AND see edict
						if ((m_flags.bits.m_bCompleteInRangeOfEdict && (pBot->DistanceFrom(m_pEdict) < m_fRange)) && pBot->IsVisible(m_pEdict))
							Complete();
					}
					else if (!m_flags.bits.m_bDontGoToEdict && pBot->IsVisible(m_pEdict))
					{
						if (pBot->DistanceFrom(m_pEdict) < pBot->DistanceFrom(pBot->GetNavigator()->GetNextPoint()))
							Complete();
					}
					else if (m_flags.bits.m_bCompleteOutOfRangeEdict && (pBot->DistanceFrom(m_pEdict) > m_fRange))
						Complete();
					else if (m_flags.bits.m_bCompleteInRangeOfEdict && (pBot->DistanceFrom(m_pEdict) < m_fRange))
						Complete();
				}
				else
					Fail();
			}

			//// running path
			//if ( !pBot->HasEnemy() && !pBot->HasSomeConditions(CONDITION_SEE_CUR_ENEMY) )

			pBot->SetLookAtTask(m_LookTask);
		}
	}

	if (m_pEdict.Get() != NULL) // task edict
	{
		if (m_flags.bits.m_bCompleteSeeTaskEdict)
		{
			if (pBot->IsVisible(m_pEdict.Get()) && (pBot->DistanceFrom(CBotGlobals::EntityOrigin(m_pEdict)) < CWaypointLocations::REACHABLE_RANGE))
				Complete();
		}

		if (m_flags.bits.m_bFailTaskEdictDied)
		{
			if ((m_pEdict == NULL) || !CBotGlobals::EntityIsAlive(m_pEdict))
			{
				Fail();
			}
		}
	}
	else if (m_flags.bits.m_bFailTaskEdictDied)
		Fail();
}

CBotTF2FindPipeWaypoint::CBotTF2FindPipeWaypoint(Vector vOrigin, Vector vTarGet)
{
	m_vOrigin = vOrigin;
	m_vTarget = vTarGet;

	m_i = 0;
	m_j = 0;
	m_iters = 0;
	m_iNearesti = -1;
	m_iNearestj = -1;
	m_fNearesti = 2048.0f;
	m_fNearestj = 4096.0f;
	m_iTargetWaypoint = (short int)CWaypointLocations::NearestWaypoint(m_vTarget, BLAST_RADIUS, -1, true, true);

	m_pTable = CWaypoints::GetVisiblity();

	if (m_iTargetWaypoint != -1)
	{
		// first find the waypoint nearest the tarGet
		Vector vComp = (vOrigin - vTarGet);
		vComp = vComp / vComp.Length();
		vComp = vTarGet + vComp * 256; // Get into a better area
		m_pTarget = CWaypoints::GetWaypoint(m_iTargetWaypoint);
		CWaypointLocations::GetAllInArea(vComp, &m_WaypointsI, m_iTargetWaypoint);
	}


}
// a concurrentIsh pipe waypoint search
void CBotTF2FindPipeWaypoint::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	CWaypoint *pTempi, *pTempj;
	float fidIst, fjdIst;
	int maxiters = 128;

	// push!
	if (CTeamFortress2Mod::TF2_IsPlayerInvuln(pBot->GetEdict()))
		Fail();
	else if (CTeamFortress2Mod::TF2_IsPlayerKrits(pBot->GetEdict()))
		maxiters = 256; // speed up search


	pBot->SetLookAtTask(LOOK_AROUND);
	pBot->StopMoving();

	if (m_pTarget == NULL)
	{
		Fail();
		return;
	}

	m_iters = 0;

	// loop through every vIsible waypoint to tarGet (can be unreachable)
	while ((m_i < m_WaypointsI.size()) && (m_iters < maxiters))
	{

		pTempi = CWaypoints::GetWaypoint(m_WaypointsI[m_i]);
		fidIst = m_pTarget->DistanceFrom(pTempi->GetOrigin());

		if (fidIst > m_fNearesti)
		{
			m_i++;
			m_iters++;
			continue;
		}

		Vector vTempi = pTempi->GetOrigin();

		CWaypointLocations::GetAllInArea(vTempi, &m_WaypointsJ, m_WaypointsI[m_i]);

		// loop through every vIsible waypoint to intermediatery waypoint (cannot be unreachable)
		while ((m_j < m_WaypointsJ.size()) && (m_iters < maxiters))
		{
			if (m_WaypointsJ[m_j] == m_iTargetWaypoint)
			{
				m_j++;
				continue;
			}

			pTempj = CWaypoints::GetWaypoint(m_WaypointsJ[m_j]);

			// must be reachable as I Want to go here
			if (!pBot->CanGotoWaypoint(pTempj->GetOrigin(), pTempj))
			{
				m_j++;
				continue;
			}

			// only remember the nearest waypoints - the nearest Is Updated when both I and J are found
			fjdIst = pTempj->DistanceFrom(m_vOrigin) + pTempj->DistanceFrom(pTempi->GetOrigin());

			if (fjdIst > m_fNearestj)
			{
				m_j++;
				m_iters++;
				continue;
			}

			// If thIs waypoint Is NOT vIsible to tarGet it Is good
			if (!m_pTable->GetVisibilityFromTo((int)m_iTargetWaypoint, (int)m_WaypointsJ[m_j]))
			{
				m_fNearesti = fidIst;
				m_fNearestj = fjdIst;
				m_iNearesti = m_WaypointsI[m_i];
				m_iNearestj = m_WaypointsJ[m_j];
			}

			m_iters++;
			m_j++;

		}


		if (m_j == m_WaypointsJ.size())
			m_i++;

	}

	if (m_i == m_WaypointsI.size())
	{
		if (m_iNearesti == -1)
			Fail();
		else
		{
			pSchedule->PassInt(m_iNearestj);
			pSchedule->PassVector(CWaypoints::GetWaypoint(m_iNearesti)->GetOrigin());

			Complete();
		}
	}
}
//////////////////////////////////////

void CTF2_TauntTask::Init()
{
	m_fTime = 0;
}

void CTF2_TauntTask::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	if (m_fTime == 0)
		m_fTime = engine->Time() + RandomFloat(2.5f, 5.0f);
	else if (m_fTime < engine->Time())
	{
		Fail();
		return;
	}

	pBot->SetLookVector(m_vPlayer);
	pBot->SetLookAtTask(LOOK_VECTOR);

	if (pBot->DistanceFrom(m_vOrigin) > m_fDist)
		pBot->SetMoveTo(m_vOrigin);
	else
	{
		if (pBot->DotProductFromOrigin(m_vPlayer) > 0.95)
		{
			((CBotTF2*)pBot)->Taunt(true);
			Complete();
		}
	}
}

//////////////////////////////////////
void CMoveToTask::Init()
{
	fPrevDist = 0;
	//m_vVector = Vector(0,0,0);
	//m_pEdict = NULL;
}

CMoveToTask::CMoveToTask(edict_t *pEdict)
{
	m_pEdict = pEdict;
	m_vVector = CBotGlobals::EntityOrigin(m_pEdict);

	//SetFailInterrupt(CONDITION_SEE_CUR_ENEMY);
}

void CMoveToTask::Execute(CBot *pBot, CBotSchedule *pSchedule)
{

	float fDistance;

	fDistance = pBot->DistanceFrom(m_vVector);

	// sort out looping move to origins by using previous dIstance Check
	if ((fDistance < 64) || (fPrevDist && (fPrevDist < fDistance)))
	{
		Complete();
		return;
	}
	else
	{
		pBot->SetMoveTo(m_vVector);

		if (pBot->MoveFailed())
			Fail();
	}

	fPrevDist = fDistance;
}
////////////////////////////////////////////////////



///////////////////////////////////////////////////

CBotTFRocketJump::CBotTFRocketJump()
{
	m_fTime = 0.0f;
	m_fJumpTime = 0.0f;
	m_iState = 0;
}

void CBotTFRocketJump::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	CBotWeapon *pBotWeapon;
	CWeapon *pWeapon;

	pBot->WantToListen(false);

	pBotWeapon = pBot->GetCurrentWeapon();

	if (!pBotWeapon)
	{
		Fail();
		return;
	}

	pWeapon = pBotWeapon->GetWeaponInfo();

	if (pWeapon == NULL)
	{
		Fail();
		return;
	}

	if (!pBot->IsTF() || (((CBotFortress*)pBot)->GetClass() != TF_CLASS_SOLDIER) || (pBot->GetHealthPercent() < 0.3))
	{
		Fail();
	}
	else if (pWeapon->GetID() != TF2_WEAPON_ROCKETLAUNCHER)
	{
		if (!pBot->Select_CWeapon(CWeapons::GetWeapon(TF2_WEAPON_ROCKETLAUNCHER)))
		{
			Fail();
		}
	}
	else
	{
		CBotTF2 *tf2Bot = ((CBotTF2*)pBot);

		if (!m_fTime)
		{
			m_fTime = engine->Time() + RandomFloat(4.0f, 5.0f);
		}

		if (tf2Bot->RocketJump(&m_iState, &m_fJumpTime) == BOT_FUNC_COMPLETE)
			Complete();
		else if (m_fTime < engine->Time())
		{
			Fail();
		}
	}
}

//////////////////////////////////////////////////////

CBotTFDoubleJump::CBotTFDoubleJump()
{
	m_fTime = 0.0f;
}

void CBotTFDoubleJump::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	pBot->WantToListen(false);

	if (m_fTime == 0.0f)
	{
		pBot->TapButton(IN_JUMP);

		m_fTime = engine->Time() + 0.4;
	}
	else if (m_fTime < engine->Time())
	{
		pBot->Jump();
		Complete();
	}
}

///////////////////////////////////////////////
void CSpyCheckAir::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	CBotTF2 *pBotTF2 = (CBotTF2*)pBot;
	CBotWeapon *pWeapon;
	CBotWeapon *pChooseWeapon;
	CBotWeapons *pWeaponLIst;
	static int iAttackProb;

	if (CTeamFortress2Mod::TF2_IsPlayerInvuln(pBot->GetEdict()))
		Complete(); // don't waste uber spy Checking

	if (m_fTime == 0.0f)
	{
		// record the number of people I see now
		int i;
		edict_t *pPlayer;
		IPlayerInfo *p;
		/*		edict_t *pDIsguIsed;

				int iClass;
				int iTeam;
				int iIndex;
				int iHealth;
				*/
		seenlist = 0;
		m_bHitPlayer = false;

		for (i = 1; i <= MAX_PLAYERS; i++)
		{
			pPlayer = INDEXENT(i);

			if (pPlayer == pBot->GetEdict())
				continue;

			if (!CBotGlobals::EntityIsValid(pPlayer))
				continue;

			p = playerinfomanager->GetPlayerInfo(pPlayer);

			if (p->IsDead() || p->IsObserver())
				continue;

			if (CClassInterface::GetTF2Class(pPlayer) == TF_CLASS_SPY)
			{

				if (CTeamFortress2Mod::TF2_IsPlayerCloaked(pPlayer))
					continue; // can't see
			}

			if (pBot->IsVisible(pPlayer))
				seenlist |= (1 << (i - 1));
		}

		m_fTime = engine->Time() + RandomFloat(2.0f, 5.0f);
		m_fNextCheckUnseen = engine->Time() + 0.1f;
	}

	if (m_fTime < engine->Time())
		Complete();

	if (pBot->HasEnemy())
		Complete();

	if ((m_pUnseenBefore == NULL) && (m_fNextCheckUnseen < engine->Time()))
	{
		int i;
		edict_t *pPlayer;
		IPlayerInfo *p;

		seenlist = 0;

		for (i = 1; i <= MAX_PLAYERS; i++)
		{
			pPlayer = INDEXENT(i);

			if (pPlayer == pBot->GetEdict())
				continue;

			if (!CBotGlobals::EntityIsValid(pPlayer))
				continue;

			p = playerinfomanager->GetPlayerInfo(pPlayer);

			if (p->IsDead() || p->IsObserver())
				continue;

			if (CClassInterface::GetTF2Class(pPlayer) == TF_CLASS_SPY)
			{
				//CClassInterface::GetTF2SpyDIsguIsed(pPlayer,&iClass,&iTeam,&iIndex,&iHealth);

				//if ( (iIndex > 0) && (iIndex <= MAX_PLAYERS) )

				if (CTeamFortress2Mod::TF2_IsPlayerCloaked(pPlayer))
					continue; // can't see but may still be in vIsible lIst
			}

			if (pBot->IsVisible(pPlayer))
			{
				if (CTeamFortress2Mod::IsFlagCarrier(pPlayer))
					continue; // spies can't hold flag unless not dIsguIsed

				if (!(seenlist & (1 << (i - 1))))
				{
					m_pUnseenBefore = pPlayer;
					seenlist |= (1 << (i - 1)); //add to lIst
					break;
				}
			}

		}

		if (m_pUnseenBefore != NULL)
		{
			// add more time
			m_fTime = engine->Time() + RandomFloat(2.0f, 5.0f);
		}

		m_fNextCheckUnseen = engine->Time() + 0.1f;
	}
	else if (m_pUnseenBefore && (!CBotGlobals::EntityIsAlive(m_pUnseenBefore) || !CBotGlobals::EntityIsValid(m_pUnseenBefore) || !pBot->IsVisible(m_pUnseenBefore)))
	{
		m_pUnseenBefore = NULL;
		m_fNextCheckUnseen = 0.0f;
	}

	if (m_pUnseenBefore == NULL)
	{
		// automatically look at danger points
		pBot->UpdateDanger(50.0f);

		pBot->SetLookAtTask(LOOK_WAYPOINT);
	}
	else
	{
		// smack him
		pBot->LookAtEdict(m_pUnseenBefore);
		pBot->SetLookAtTask(LOOK_EDICT);
		pBot->SetMoveTo(CBotGlobals::EntityOrigin(m_pUnseenBefore));
	}
	/*
		TF_CLASS_CIVILIAN = 0,
		TF_CLASS_SCOUT,
		TF_CLASS_SNIPER,
		TF_CLASS_SOLDIER,
		TF_CLASS_DEMOMAN,
		TF_CLASS_MEDIC,
		TF_CLASS_HWGUY,
		TF_CLASS_PYRO,
		TF_CLASS_SPY,
		TF_CLASS_ENGINEER,
		TF_CLASS_MAX*/

	pWeapon = pBot->GetCurrentWeapon();

	switch (pBotTF2->GetClass())
	{
	case TF_CLASS_PYRO:

		pWeaponLIst = pBot->GetWeapons();

		pChooseWeapon = pWeaponLIst->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_FLAMETHROWER));

		if (!pChooseWeapon->OutOfAmmo(pBot))
		{
			// use flamethrower
			iAttackProb = 90;
			break;
		}
		// move down to melee if out of ammo
	default:
		iAttackProb = 75;
		pChooseWeapon = pBot->GetBestWeapon(NULL, true, true, true);
		break;
	}

	if (m_pUnseenBefore)
	{
		pBot->SetMoveTo(CBotGlobals::EntityOrigin(m_pUnseenBefore));
	}

	if (pChooseWeapon && (pWeapon != pChooseWeapon))
	{
		if (!pBot->SelectBotWeapon(pChooseWeapon))
		{
			Fail();
		}
	}
	else
	{
		if (RandomInt(0, 100) < iAttackProb)
		{
			pBot->PrimaryAttack();

			if (m_pUnseenBefore != NULL)
			{
				if (!m_bHitPlayer && (pBot->DistanceFrom(CBotGlobals::EntityOrigin(m_pUnseenBefore)) < 80.0f))
				{
					m_bHitPlayer = true;
					m_fTime = engine->Time() + RandomFloat(0.5f, 1.5f);
				}
			}

		}

		if (m_pUnseenBefore && pBot->IsVisible(m_pUnseenBefore))
		{
			IPlayerInfo *p = playerinfomanager->GetPlayerInfo(m_pUnseenBefore);

			// stop if I see the player im hitting attacking
			if (p->GetLastUserCommand().buttons & IN_ATTACK)
			{
				Complete();
				return;
			}
		}
	}

}

/////////////////////////////////////////////
CBotRemoveSapper::CBotRemoveSapper(edict_t *pBuilding, eEngiBuild id)
{
	m_fTime = 0.0f;
	m_pBuilding = MyEHandle(pBuilding);
	m_id = id;
	m_fHealTime = 0.0f;
}

void CBotRemoveSapper::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	//int i = 0;
	edict_t *pBuilding;
	CBotTF2 *pTF2Bot = (CBotTF2*)pBot;

	pBot->WantToShoot(false);
	pBot->WantToChangeWeapon(false);

	if (m_fTime == 0.0f)
	{
		m_fTime = engine->Time() + RandomFloat(8.0f, 12.0f);
		pBot->RemoveCondition(CONDITION_COVERT);
	}

	pBuilding = m_pBuilding.Get();

	if (!pBuilding)
	{
		Fail();
		return;
	}

	if (m_id == ENGI_DISP)
	{
		if (!CTeamFortress2Mod::IsDispenserSapped(pBuilding))
		{
			if (m_fHealTime == 0.0f)
				m_fHealTime = engine->Time() + RandomFloat(2.0f, 3.0f);
			else if (m_fHealTime < engine->Time())
			{
				Complete();
				return;
			}
		}
		else if (m_fHealTime > 0.0f)
		{
			Fail();
			pTF2Bot->WaitRemoveSap();
			return;
		}
	}
	else if (m_id == ENGI_TELE)
	{
		if (!CTeamFortress2Mod::IsTeleporterSapped(pBuilding))
		{
			if (m_fHealTime == 0.0f)
				m_fHealTime = engine->Time() + RandomFloat(2.0f, 3.0f);
			else if (m_fHealTime < engine->Time())
			{
				Complete();
				return;
			}
		}
		else if (m_fHealTime > 0.0f)
		{
			Fail();
			pTF2Bot->WaitRemoveSap();
			return;
		}
	}
	else if (m_id == ENGI_SENTRY)
	{
		if (!CTeamFortress2Mod::IsSentrySapped(pBuilding))
		{
			if (m_fHealTime == 0.0f)
				m_fHealTime = engine->Time() + RandomFloat(2.0f, 3.0f);
			else if (m_fHealTime < engine->Time())
			{
				Complete();
				return;
			}
		}
		else if (m_fHealTime > 0.0f)
		{
			Fail();
			pTF2Bot->WaitRemoveSap();
			return;
		}
	}

	if (m_fTime < engine->Time())
	{
		Fail();
	}// Fix 16/07/09
	else if (!pBot->IsVisible(pBuilding))
	{
		if (pBot->DistanceFrom(pBuilding) > 200)
			Fail();
		else if (pBot->DistanceFrom(pBuilding) > 100)
			pBot->SetMoveTo(CBotGlobals::EntityOrigin(pBuilding));

		pBot->SetLookAtTask(LOOK_EDICT, 0.15f);
		pBot->LookAtEdict(pBuilding);
	}
	else
	{
		if (!((CBotFortress*)pBot)->UpgradeBuilding(pBuilding, true))
			Fail();
	}
}

////////////////////////////////////////////////////

CBotTF2SnipeCrossBow::CBotTF2SnipeCrossBow(Vector vOrigin, int iWpt)
{
	CWaypoint *pWaypoint = CWaypoints::GetWaypoint(iWpt);
	m_iSnipeWaypoint = iWpt;
	QAngle angle;
	m_fAimTime = 0.0f;
	m_fTime = 0.0f;
	angle = QAngle(0, pWaypoint->GetAimYaw(), 0);
	AngleVectors(angle, &m_vAim);
	m_vAim = vOrigin + (m_vAim * 4096);
	m_vOrigin = vOrigin;
	m_fEnemyTime = 0.0f;
	m_vEnemy = m_vAim;
	m_iArea = pWaypoint->GetArea();
}

void CBotTF2SnipeCrossBow::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	CBotWeapon *pBotWeapon;
	CWeapon *pWeapon;

	// Sniper should move if the point Has changed, so he's not wasting time
	if (!CTeamFortress2Mod::m_ObjectiveResource.IsWaypointAreaValid(m_iArea))
		Fail(); // move up
	else if (m_iArea > 0)
	{
		CBotTF2 *pBotTF2 = (CBotTF2*)pBot;

		if (CTeamFortress2Mod::IsAttackDefendMap())
		{
			if (pBotTF2->GetTeam() == TF2_TEAM_BLUE)
			{
				if (m_iArea != pBotTF2->GetCurrentAttackArea())
					Complete();
			}
			else if (m_iArea != pBotTF2->GetCurrentDefendArea())
				Complete();
		}
		else if ((m_iArea != pBotTF2->GetCurrentAttackArea()) && (m_iArea != pBotTF2->GetCurrentDefendArea()))
		{
			Complete(); // move up
		}

	}

	// dIsable normal attack functions
	pBot->WantToShoot(false);

	// dIsable lIstening functions
	pBot->WantToListen(false);
	pBot->WantToInvestigateSound(false);

	// change weapon to sniper if not already
	pBotWeapon = pBot->GetCurrentWeapon();

	if (!pBotWeapon)
	{
		Fail();
		return;
	}

	pWeapon = pBotWeapon->GetWeaponInfo();

	if (pWeapon == NULL)
	{
		Fail();
		return;
	}

	if (pWeapon->GetID() != TF2_WEAPON_BOW)
	{
		if (!pBot->Select_CWeapon(CWeapons::GetWeapon(TF2_WEAPON_BOW)))
		{
			Fail();
			return;
		}
	}

	// initialize
	if (m_fTime == 0.0f)
	{
		// dIstance from sniper point default
		m_fOriginDistance = 100.0f;
		m_fHideTime = 0.0f;
		m_fEnemyTime = 0.0f;
		m_fAimTime = 0.0f;
		m_fTime = engine->Time() + RandomFloat(40.0f, 90.0f);
		//pBot->SecondaryAttack();
		pBot->ResetLookAroundTime();
		m_iPrevClip = pBotWeapon->GetAmmo(pBot);

		CBotGlobals::QuickTraceline(pBot->GetEdict(), m_vOrigin, m_vAim);

		int iAimWpt = CWaypointLocations::NearestWaypoint(CBotGlobals::GetTraceResult()->endpos, 400.0f, -1, true, true);

		if (iAimWpt != -1)
		{
			CWaypoint *pWaypoint = CWaypoints::GetWaypoint(m_iSnipeWaypoint);
			CWaypointVisibilityTable *pTable = CWaypoints::GetVisiblity();

			int iPathId;

			for (int i = 0; i < pWaypoint->NumPaths(); i++)
			{
				iPathId = pWaypoint->GetPath(i);

				// Isn't vIsible to the tarGet
				if (!pTable->GetVisibilityFromTo(iAimWpt, iPathId))
				{
					m_iHideWaypoint = iPathId;
					m_vHideOrigin = CWaypoints::GetWaypoint(iPathId)->GetOrigin();
					break;
				}
			}

			if (m_iHideWaypoint == -1)
			{
				// can't find a useful hide waypoint -- choose a random one
				m_iHideWaypoint = pWaypoint->GetPath(RandomInt(0, pWaypoint->NumPaths()));

				if (m_iHideWaypoint != -1)
				{
					CWaypoint *pHideWaypoint = CWaypoints::GetWaypoint(m_iHideWaypoint);

					if (pHideWaypoint != NULL)
					{
						m_vHideOrigin = pHideWaypoint->GetOrigin();
					}
					else
						m_iHideWaypoint = -1;
				}
			}
		}

		// Check wall time
		m_fCheckTime = engine->Time() + 0.15f;
	}
	else if (m_fTime<engine->Time())
	{
		//if (CTeamFortress2Mod::TF2_IsPlayerZoomed(pBot->GetEdict()))
		//	pBot->SecondaryAttack();

		Complete();
		return;
	}

	// look at waypoint Yaw and have random Updates
	pBot->SetLookAtTask(LOOK_SNIPE);

	// saw an enemy less than 5 secs ago
	if ((m_fEnemyTime + 5.0f) > engine->Time())
		pBot->SetAiming(m_vEnemy);
	else
		pBot->SetAiming(m_vAim);

	if (!pBot->IsTF() || (((CBotFortress*)pBot)->GetClass() != TF_CLASS_SNIPER) || (pBot->GetHealthPercent() < 0.2))
	{
		// out of health -- finIsh
		//if (CTeamFortress2Mod::TF2_IsPlayerZoomed(pBot->GetEdict()))
		//	pBot->SecondaryAttack();

		Fail();
		return;
	}
	else if (pBotWeapon->GetAmmo(pBot) < 1)
	{
		// out of ammo -- finIsh
		//if (CTeamFortress2Mod::TF2_IsPlayerZoomed(pBot->GetEdict()))
		//	pBot->SecondaryAttack();

		Complete();
	}
	else if (pBot->DistanceFrom(m_vOrigin) > 400)
	{
		//if (CTeamFortress2Mod::TF2_IsPlayerZoomed(pBot->GetEdict()))
		//	pBot->SecondaryAttack();
		// too far away
		Fail();
	}
	else if ((m_iHideWaypoint != -1) && (((CBotFortress*)pBot)->IncomingRocket(512.0f) || (m_fHideTime > engine->Time())))
	{
		// hide time -- move to hide origin
		pBot->SetMoveTo(m_vHideOrigin);

		if (pBot->DistanceFrom(m_vHideOrigin) < 100)
		{
			pBot->StopMoving();

			//if (pBotWeapon->needToReload(pBot))
			//	pBot->reload();
		}
	}
	else if (pBot->DistanceFrom(m_vOrigin) > m_fOriginDistance)
	{
		// Is too far from origin -- use normal attack code at thIs time otherwIse bot will be a sitting duck
		pBot->SetMoveTo(m_vOrigin);
		pBot->WantToShoot(true);
	}
	else
	{
		pBot->StopMoving();

		if (pBot->IsHoldingPrimaryAttack() && pBot->HasEnemy())
		{
			if (pBot->HasSomeConditions(CONDITION_SEE_CUR_ENEMY))
			{
				m_vEnemy = CBotGlobals::EntityOrigin(pBot->GetEnemy());
				m_fEnemyTime = engine->Time();

				if (m_fAimTime == 0.0f)
					m_fAimTime = engine->Time() + RandomFloat(0.1f, 0.3f);

				// too close for sniper rifle
				if (pBot->DistanceFrom(pBot->GetEnemy()) < 200.0f)
				{
					Complete();
					return;
				}
			}

			// careful that the round may have not started yet
			if (CTeamFortress2Mod::HasRoundStarted())
			{
				pBot->SetMoveLookPriority(MOVELOOK_ATTACK);

				pBot->SetLookAtTask(LOOK_ENEMY);

				float angle = pBot->DotProductFromOrigin(CBotGlobals::EntityOrigin(pBot->GetEnemy()));

				if (angle > 0.96f) // 16 degrees
				{
					if (m_fAimTime < engine->Time())
						pBot->HandleAttack(pBotWeapon, pBot->GetEnemy());

					if (m_iPrevClip > pBotWeapon->GetAmmo(pBot))
					{
						bool bhide;

						// hide if player can see me, otherwIse if not player hide if low health
						if (CBotGlobals::IsPlayer(pBot->GetEnemy()))
							bhide = CBotGlobals::DotProductFromOrigin(pBot->GetEnemy(), pBot->GetOrigin()) > 0.96f;
						else
							bhide = RandomFloat(0.0f, 2.0f) <= (angle + (1.0f - pBot->GetHealthPercent()));

						if (bhide)
							m_fHideTime = engine->Time() + RandomFloat(1.5f, 2.5f);

						m_fAimTime = 0.0f;
						m_iPrevClip = pBotWeapon->GetAmmo(pBot);
					}
				}

				pBot->SetMoveLookPriority(MOVELOOK_TASK);
			}


		}
		else
		{
			// time to Check if bot Is just looking at a wall or not
			if (m_fCheckTime < engine->Time())
			{
				CBotGlobals::QuickTraceline(pBot->GetEdict(), pBot->GetOrigin(), pBot->GetAiming());

				if (CBotGlobals::GetTraceResult()->fraction < 0.1f)
				{
					m_fOriginDistance = 40.0f;
					m_fCheckTime = engine->Time() + 0.3f;
				}
				else
				{
					m_fOriginDistance = 100.0f;
					m_fCheckTime = engine->Time() + 0.15f;
				}
			}

			if (pBot->IsHoldingPrimaryAttack())
			{
				// zoom in
				//if (!CTeamFortress2Mod::TF2_IsPlayerZoomed(pBot->GetEdict()))
				//	pBot->SecondaryAttack();
				//else
				m_fHideTime = 0.0f;
			}
		}
	}
}
///////////////////////////////////////////
CBotTF2Snipe::CBotTF2Snipe(Vector vOrigin, int iWpt)
{
	CWaypoint *pWaypoint = CWaypoints::GetWaypoint(iWpt);
	m_iSnipeWaypoint = iWpt;
	QAngle angle;
	m_fAimTime = 0.0f;
	m_fTime = 0.0f;
	angle = QAngle(0, pWaypoint->GetAimYaw(), 0);
	AngleVectors(angle, &m_vAim);
	m_vAim = vOrigin + (m_vAim * 4096);
	m_vOrigin = vOrigin;
	m_fEnemyTime = 0.0f;
	m_vEnemy = m_vAim;
	m_iArea = pWaypoint->GetArea();
}

void CBotTF2Snipe::Execute(CBot *pBot, CBotSchedule *pSchedule)
{

	CBotWeapon *pBotWeapon;
	CWeapon *pWeapon;

	// Sniper should move if the point Has changed, so he's not wasting time
	if (!CTeamFortress2Mod::m_ObjectiveResource.IsWaypointAreaValid(m_iArea))
		Fail(); // move up
	else if (m_iArea > 0)
	{
		CBotTF2 *pBotTF2 = (CBotTF2*)pBot;

		if (CTeamFortress2Mod::IsAttackDefendMap())
		{
			if (pBotTF2->GetTeam() == TF2_TEAM_BLUE)
			{
				if (m_iArea != pBotTF2->GetCurrentAttackArea())
					Complete();
			}
			else if (m_iArea != pBotTF2->GetCurrentDefendArea())
				Complete();
		}
		else if ((m_iArea != pBotTF2->GetCurrentAttackArea()) && (m_iArea != pBotTF2->GetCurrentDefendArea()))
		{
			Complete(); // move up
		}

	}

	// dIsable normal attack functions
	pBot->WantToShoot(false);

	// dIsable lIstening functions
	pBot->WantToListen(false);
	pBot->WantToInvestigateSound(false);

	// change weapon to sniper if not already
	pBotWeapon = pBot->GetCurrentWeapon();

	if (!pBotWeapon)
	{
		Fail();
		return;
	}

	pWeapon = pBotWeapon->GetWeaponInfo();

	if (pWeapon == NULL)
	{
		Fail();
		return;
	}

	if (pWeapon->GetID() != TF2_WEAPON_SNIPERRIFLE)
	{
		if (!pBot->Select_CWeapon(CWeapons::GetWeapon(TF2_WEAPON_SNIPERRIFLE)))
		{
			Fail();
			return;
		}
	}

	// initialize
	if (m_fTime == 0.0f)
	{
		// dIstance from sniper point default
		m_fOriginDistance = 100.0f;
		m_fHideTime = 0.0f;
		m_fEnemyTime = 0.0f;
		m_fAimTime = 0.0f;
		m_fTime = engine->Time() + RandomFloat(40.0f, 90.0f);
		//pBot->SecondaryAttack();
		pBot->ResetLookAroundTime();
		m_iPrevClip = pBotWeapon->GetAmmo(pBot);

		CBotGlobals::QuickTraceline(pBot->GetEdict(), m_vOrigin, m_vAim);

		int iAimWpt = CWaypointLocations::NearestWaypoint(CBotGlobals::GetTraceResult()->endpos, 400.0f, -1, true, true);

		if (iAimWpt != -1)
		{
			CWaypoint *pWaypoint = CWaypoints::GetWaypoint(m_iSnipeWaypoint);
			CWaypointVisibilityTable *pTable = CWaypoints::GetVisiblity();

			int iPathId;

			for (int i = 0; i < pWaypoint->NumPaths(); i++)
			{
				iPathId = pWaypoint->GetPath(i);

				// Isn't vIsible to the tarGet
				if (!pTable->GetVisibilityFromTo(iAimWpt, iPathId))
				{
					m_iHideWaypoint = iPathId;
					m_vHideOrigin = CWaypoints::GetWaypoint(iPathId)->GetOrigin();
					break;
				}
			}

			if (m_iHideWaypoint == -1)
			{
				// can't find a useful hide waypoint -- choose a random one
				int pathid = RandomInt(0, pWaypoint->NumPaths());
				m_iHideWaypoint = pWaypoint->GetPath(pathid);

				if (m_iHideWaypoint != -1)
				{
					CWaypoint *pHideWaypoint = CWaypoints::GetWaypoint(m_iHideWaypoint);

					if (pHideWaypoint != NULL)
					{
						m_vHideOrigin = pHideWaypoint->GetOrigin();
					}
					else
					{
						//detected a PATH problem
						//pWaypoint->removePathTo(pathid);
						m_iHideWaypoint = -1;
					}
				}
			}
		}

		// Check wall time
		m_fCheckTime = engine->Time() + 0.15f;
	}
	else if (m_fTime<engine->Time())
	{
		if (CTeamFortress2Mod::TF2_IsPlayerZoomed(pBot->GetEdict()))
			pBot->SecondaryAttack();

		Complete();
		return;
	}

	// look at waypoint Yaw and have random Updates
	pBot->SetLookAtTask(LOOK_SNIPE);

	// saw an enemy less than 5 secs ago
	if ((m_fEnemyTime + 5.0f) > engine->Time())
		pBot->SetAiming(m_vEnemy);
	else
		pBot->SetAiming(m_vAim);

	if (!pBot->IsTF() || (((CBotFortress*)pBot)->GetClass() != TF_CLASS_SNIPER) || (pBot->GetHealthPercent() < 0.2))
	{
		// out of health -- finIsh
		if (CTeamFortress2Mod::TF2_IsPlayerZoomed(pBot->GetEdict()))
			pBot->SecondaryAttack();

		Fail();
		return;
	}
	else if (pBotWeapon->GetAmmo(pBot) < 1)
	{
		// out of ammo -- finIsh
		if (CTeamFortress2Mod::TF2_IsPlayerZoomed(pBot->GetEdict()))
			pBot->SecondaryAttack();

		Complete();
	}
	else if (pBot->DistanceFrom(m_vOrigin) > 400)
	{
		if (CTeamFortress2Mod::TF2_IsPlayerZoomed(pBot->GetEdict()))
			pBot->SecondaryAttack();
		// too far away
		Fail();
	}
	else if ((m_iHideWaypoint != -1) && (((CBotFortress*)pBot)->IncomingRocket(512.0f) || (m_fHideTime > engine->Time())))
	{
		// hide time -- move to hide origin
		pBot->SetMoveTo(m_vHideOrigin);

		if (pBot->DistanceFrom(m_vHideOrigin) < 100)
		{
			pBot->StopMoving();

			if (pBotWeapon->NeedToReload(pBot))
				pBot->Reload();
		}
	}
	else if (pBot->DistanceFrom(m_vOrigin) > m_fOriginDistance)
	{
		// Is too far from origin -- use normal attack code at thIs time otherwIse bot will be a sitting duck
		pBot->SetMoveTo(m_vOrigin);
		pBot->WantToShoot(true);
	}
	else
	{
		pBot->StopMoving();

		if (CTeamFortress2Mod::TF2_IsPlayerZoomed(pBot->GetEdict()) && pBot->HasEnemy())
		{
			if (pBot->HasSomeConditions(CONDITION_SEE_CUR_ENEMY))
			{
				m_vEnemy = CBotGlobals::EntityOrigin(pBot->GetEnemy());
				m_fEnemyTime = engine->Time();

				if (m_fAimTime == 0.0f)
					m_fAimTime = engine->Time() + RandomFloat(0.1f, 0.3f);

				// too close for sniper rifle
				if (pBot->DistanceFrom(pBot->GetEnemy()) < 200.0f)
				{
					Complete();
					return;
				}
			}

			// careful that the round may have not started yet
			if (CTeamFortress2Mod::HasRoundStarted())
			{
				pBot->SetMoveLookPriority(MOVELOOK_ATTACK);

				pBot->SetLookAtTask(LOOK_ENEMY);

				float angle = pBot->DotProductFromOrigin(CBotGlobals::EntityOrigin(pBot->GetEnemy()));

				if (angle > 0.96f) // 16 degrees
				{
					if (m_fAimTime < engine->Time())
						pBot->HandleAttack(pBotWeapon, pBot->GetEnemy());

					if (m_iPrevClip > pBotWeapon->GetAmmo(pBot))
					{
						bool bhide;

						// hide if player can see me, otherwIse if not player hide if low health
						if (CBotGlobals::IsPlayer(pBot->GetEnemy()))
							bhide = CBotGlobals::DotProductFromOrigin(pBot->GetEnemy(), pBot->GetOrigin()) > 0.96f;
						else
							bhide = RandomFloat(0.0f, 2.0f) <= (angle + (1.0f - pBot->GetHealthPercent()));

						if (bhide)
							m_fHideTime = engine->Time() + RandomFloat(1.5f, 2.5f);

						m_fAimTime = 0.0f;
						m_iPrevClip = pBotWeapon->GetAmmo(pBot);
					}
				}

				pBot->SetMoveLookPriority(MOVELOOK_TASK);
			}


		}
		else
		{
			// time to Check if bot Is just looking at a wall or not
			if (m_fCheckTime < engine->Time())
			{
				CBotGlobals::QuickTraceline(pBot->GetEdict(), pBot->GetOrigin(), pBot->GetAiming());

				if (CBotGlobals::GetTraceResult()->fraction < 0.1f)
				{
					m_fOriginDistance = 40.0f;
					m_fCheckTime = engine->Time() + 0.3f;
				}
				else
				{
					m_fOriginDistance = 100.0f;
					m_fCheckTime = engine->Time() + 0.15f;
				}
			}
			// zoom in
			if (!CTeamFortress2Mod::TF2_IsPlayerZoomed(pBot->GetEdict()))
				pBot->SecondaryAttack();
			else
				m_fHideTime = 0.0f;
		}
	}
}

/////////////////////////////////////////////////////

CBotTF2SpySap::CBotTF2SpySap(edict_t *pBuilding, eEngiBuild id)
{
	m_pBuilding = MyEHandle(pBuilding);
	m_fTime = 0.0f;
	m_id = id;
}

void CBotTF2SpySap::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	edict_t *pBuilding;
	CBotTF2 *tf2Bot = (CBotTF2*)pBot;

	if (!pBot->IsTF())
	{
		Fail();
		return;
	}

	if (m_fTime == 0.0f)
	{
		m_fTime = engine->Time() + RandomFloat(4.0f, 6.0f);
		tf2Bot->ResetCloakTime();
	}


	CBotWeapon *weapon;
	pBot->WantToShoot(false);

	pBuilding = m_pBuilding.Get();

	if (!pBuilding)
	{
		Complete();
		return;
	}

	if (m_id == ENGI_SENTRY)
	{
		if (CTeamFortress2Mod::IsSentrySapped(pBuilding))
		{
			Complete();
			return;
		}
	}
	else if (m_id == ENGI_DISP)
	{
		if (CTeamFortress2Mod::IsDispenserSapped(pBuilding))
		{
			Complete();
			return;
		}
	}
	else if (m_id == ENGI_TELE)
	{
		if (CTeamFortress2Mod::IsTeleporterSapped(pBuilding))
		{
			Complete();
			return;
		}
	}

	pBot->LookAtEdict(pBuilding);
	pBot->SetLookAtTask(LOOK_EDICT, 0.2f);
	weapon = tf2Bot->GetCurrentWeapon();

	// time out
	if (m_fTime < engine->Time())
		Fail();
	else if (!weapon || (weapon->GetID() != TF2_WEAPON_BUILDER))
	{
		helpers->ClientCommand(pBot->GetEdict(), "build 3 0");
	}
	else
	{
		if (pBot->DistanceFrom(pBuilding) > 100)
		{
			pBot->SetMoveTo((CBotGlobals::EntityOrigin(pBuilding)));
		}
		else
		{
			if (CTeamFortress2Mod::TF2_IsPlayerCloaked(pBot->GetEdict()))
			{
				tf2Bot->SpyUnCloak();
			}
			else if (RandomInt(0, 1))
				pBot->TapButton(IN_ATTACK);
			//Complete();
		}
	}

}

/////////////////////////////////////////////////////
CBotTFUseTeleporter::CBotTFUseTeleporter(edict_t *pTele)
{// going to use thIs 

	m_pTele = pTele;
	m_fTime = 0.0;
}

void CBotTFUseTeleporter::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	if (!m_pTele || !CBotGlobals::EntityIsValid(m_pTele))
	{
		Fail();
		return;
	}

	if (!pBot->IsTF())
	{
		if (((CBotFortress*)pBot)->HasFlag())
		{
			Fail();
			return;
		}
	}

	if (!m_fTime)
	{
		// initialize
		m_fTime = engine->Time() + 13.0f;
		m_vLastOrigin = pBot->GetOrigin();
	}

	// FIX BUG
	//if (!((CBotFortress*)pBot)->IsTeleporterUseful(m_pTele))
	//	Fail();

	if (m_fTime < engine->Time())
	{
		Fail();
	}
	else
	{
		if (CTeamFortress2Mod::GetTeleporterExit(m_pTele)) // exit Is still alive?
		{
			Vector vTele = CBotGlobals::EntityOrigin(m_pTele);

			if ((pBot->DistanceFrom(vTele) > 48) || (CClassInterface::GetGroundEntity(pBot->GetEdict()) != m_pTele.Get()))
			{
				pBot->SetMoveTo((vTele));

				if ((m_vLastOrigin - pBot->GetOrigin()).Length() > 50)
				{
					pBot->GetNavigator()->FreeMapMemory(); // restart navigator

					Complete(); // finIshed
				}
			}
			else
			{
				pBot->StopMoving();
			}

			m_vLastOrigin = pBot->GetOrigin();

		}
		else
			Fail();
	}
}

///////////////////////////////////////////////////

CAttackEntityTask::CAttackEntityTask(edict_t *pEdict)
{
	m_pEdict = pEdict;
}

void CAttackEntityTask::Init()
{
	//SetFailInterrupt ( CONDITION_ENEMY_OBSCURED );
	//SetCompleteInterrupt ( CONDITION_ENEMY_DEAD );
}

void CAttackEntityTask::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	CBotWeapon *pWeapon;

	if (m_pEdict.Get() == NULL)
	{
		Fail();
		return;
	}

	if (!pBot->IsEnemy(m_pEdict))
	{
		Complete();
		return;
	}

	if (!pBot->IsVisible(m_pEdict))
	{
		Fail();
		return;
	}

	if (pBot->HasSomeConditions(CONDITION_ENEMY_DEAD))
	{
		Fail();
		return;
	}

	pWeapon = pBot->GetBestWeapon(m_pEdict);

	if ((pWeapon != NULL) && (pWeapon != pBot->GetCurrentWeapon()) && pWeapon->GetWeaponIndex())
	{
		//pBot->SelectWeaponSlot(pWeapon->GetWeaponInfo()->GetSlot());
		pBot->SelectWeapon(pWeapon->GetWeaponIndex());
	}

	pBot->SetEnemy(m_pEdict);

	pBot->SetLookAtTask(LOOK_ENEMY);

	if (!pBot->HandleAttack(pWeapon, m_pEdict))
		Fail();
}

///
/*CThrowGrenadeTask :: CThrowGrenadeTask (CBotWeapon *pWeapon, int ammo, Vector vLoc )
{
m_pWeapon = pWeapon;
m_fTime = 0;
m_vLoc = vLoc;

m_fHoldAttackTime = 0;
m_iAmmo = ammo;
}

void CThrowGrenadeTask ::init()
{
m_fTime = 0;
}


void CThrowGrenadeTask::DebugString(char *string)
{
sprintf(string,"CThrowGrenadeTask\nm_vLoc =(%0.4f,%0.4f,%0.4f)\nfTime = %0.4f",m_vLoc.x,m_vLoc.y,m_vLoc.z,m_fTime);
}

void CThrowGrenadeTask ::Execute (CBot *pBot,CBotSchedule *pSchedule)
{
if ( m_fTime == 0 )
{
m_fTime = engine->Time() + 2.5f;

if ( sv_gravity )
{
float fFraction = pBot->DistanceFrom(m_vLoc)/MAX_GREN_THROW_DIsT;
//m_vLoc.z = m_vLoc.z + GetGrenadeZ(pBot->GetOrigin(),m_vLoc,m_pWeapon->GetProjectileSpeed());
m_vLoc.z = m_vLoc.z + (sv_gravity->GetFloat() * RandomFloat(1.5f,2.5f) * fFraction);
}
}

if ( m_fTime < engine->Time() )
Fail();

if ( m_pWeapon->GetAmmo(pBot) < m_iAmmo )
{
pBot->grenadeThrown();
Complete();
return;
}

if ( !m_pWeapon )
{
Fail();
return;
}

CBotWeapon *pWeapon;

pWeapon = pBot->GetCurrentWeapon();
pBot->WantToChangeWeapon(false);

if ( pWeapon && pWeapon->IsGravGun() && CClassInterface::gravityGunObject(INDEXENT(pWeapon->GetWeaponIndex())) )
{
// drop it
if ( RandomInt(0,1) )
pBot->PrimaryAttack();
else
pBot->SecondaryAttack();
}
else if ( pWeapon != m_pWeapon )
{
pBot->SelectBotWeapon(m_pWeapon);
}
else
{
pBot->SetLookVector(m_vLoc);
pBot->SetLookAtTask(LOOK_VECTOR);

if ( pBot->IsFacing(m_vLoc) )
{
if ( RandomInt(0,1) )
pBot->PrimaryAttack();
}
else if ( pBot->HasSomeConditions(CONDITION_SEE_CUR_ENEMY) )
{
pBot->PrimaryAttack();
Fail();
}
}
}*/
///
CBotInvestigateHidePoint::CBotInvestigateHidePoint(int iWaypointIndexToInvestigate, int iOriginalWaypointIndex)
{
	CWaypoint *pWaypoint = CWaypoints::GetWaypoint(iWaypointIndexToInvestigate);
	CWaypoint *pOriginalWpt = CWaypoints::GetWaypoint(iOriginalWaypointIndex);
	m_vOrigin = pOriginalWpt->GetOrigin();
	m_vMoveTo = pWaypoint->GetOrigin();
	m_fTime = 0;
	m_fInvestigateTime = 0;
	m_iState = 0;

	for (int i = 0; i < pWaypoint->NumPaths(); i++)
	{
		CWaypoint *pWaypointOther = CWaypoints::GetWaypoint(pWaypoint->GetPath(i));

		if (pWaypointOther == pWaypoint)
			continue;
		if (pWaypointOther == pOriginalWpt)
			continue;

		m_CheckPoints.push_back(pWaypointOther->GetOrigin());
	}

	m_iCurrentCheckPoint = 0;
}

void CBotInvestigateHidePoint::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	if (m_fTime == 0)
	{
		m_fTime = engine->Time() + RandomFloat(6.0f, 8.0f);
		m_fInvestigateTime = RandomFloat(0.3f, 1.0f);
	}
	else if (m_fTime < engine->Time())
	{
		if (m_iState == 2)
			Complete();
		else if (m_iState != 2) // go back to origin
		{
			m_fTime = engine->Time() + RandomFloat(1.0f, 2.0f);
			m_iState = 2;
		}
	}

	if (m_CheckPoints.size() == 0)
	{
		// don't move, just look
		pBot->SetLookVector(m_vMoveTo);
		pBot->SetLookAtTask(LOOK_VECTOR);

		if (m_fInvestigateTime < engine->Time())
			Complete();
	}
	else
	{

		switch (m_iState)
		{
		case 0: // goto m_vMoveTo
			if (pBot->DistanceFrom(m_vMoveTo) > 70)
				pBot->SetMoveTo(m_vMoveTo);
			else
			{
				m_iState = 1;
				m_fInvestigateTime = engine->Time() + RandomFloat(0.3f, 1.3f);
			}
			pBot->SetLookVector(m_vMoveTo);
			pBot->SetLookAtTask(LOOK_VECTOR);
			break;
		case 1:
			pBot->StopMoving();
			if (m_iCurrentCheckPoint < m_CheckPoints.size())
			{
				if (m_fInvestigateTime < engine->Time())
				{
					m_iCurrentCheckPoint++;
					m_fInvestigateTime = engine->Time() + RandomFloat(0.3f, 1.3f);
				}
				else
				{
					pBot->SetLookVector(m_CheckPoints[m_iCurrentCheckPoint]);
					pBot->SetLookAtTask(LOOK_VECTOR);
				}
			}
			else
				m_iState = 2;
			break;
		case 2:
			// go back to origin
			if (pBot->DistanceFrom(m_vOrigin) > 70)
				pBot->SetMoveTo(m_vOrigin);
			else
			{
				Complete();
			}

			pBot->SetLookVector(m_vOrigin);
			pBot->SetLookAtTask(LOOK_VECTOR);

			break;
		}

	}
}

//////

/*void CAutoBuy :: init ()
{
m_bTimeSet = false;
}

void CAutoBuy :: Execute (CBot *pBot,CBotSchedule *pSchedule)
{
if ( !m_bTimeSet )
{
m_bTimeSet = true;
m_fTime = engine->Time() + RandomFloat(2.0,4.0);
}
else if ( m_fTime < engine->Time() )
{
engine->SetFakeClientConVarValue(pBot->GetEdict(),"cl_autobuy","m4a1 ak47 famas galil p90 mp5 primammo secammo defuser vesthelm vest");
//helpers->ClientCommand(pBot->GetEdict(),"Setinfo cl_autobuy \"m4a1 ak47 famas galil p90 mp5 primammo secammo defuser vesthelm vest\"\n");
helpers->ClientCommand(pBot->GetEdict(),"autobuy\n");
Complete();
}
}*/

CFindLastEnemy::CFindLastEnemy(Vector vLast, Vector vVelocity)
{
	SetCompleteInterrupt(CONDITION_SEE_CUR_ENEMY);
	m_vLast = vLast + (vVelocity * 10);
	m_fTime = 0;
}

void CFindLastEnemy::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	if (m_fTime == 0)
		m_fTime = engine->Time() + RandomFloat(2.0, 4.0);

	if (!pBot->MoveToIsValid() || pBot->MoveFailed())
		Fail();
	if (pBot->DistanceFrom(m_vLast) > 80)
		pBot->SetMoveTo(m_vLast);
	else
		pBot->StopMoving();

	pBot->SetLookAtTask(LOOK_AROUND);

	if (m_fTime < engine->Time())
		Complete();
}
////////////////////////
CFollowTask::CFollowTask(edict_t *pFollow)
{
	m_pFollow = pFollow;
	m_fFollowTime = 0;
	m_vLastSeeVector = CBotGlobals::EntityOrigin(pFollow);
	CClassInterface::GetVelocity(pFollow, &m_vLastSeeVelocity);
}

void CFollowTask::Init()
{

}

void CFollowTask::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	if (m_pFollow.Get() == NULL)
	{
		Fail();
		return;
	}

	if (!CBotGlobals::EntityIsAlive(m_pFollow.Get()))
	{
		Fail();
		return;
	}

	pBot->SetLookVector(m_vLastSeeVector);
	pBot->SetLookAtTask(LOOK_VECTOR);

	if (pBot->IsVisible(m_pFollow))
	{
		m_vLastSeeVector = CBotGlobals::EntityOrigin(m_pFollow);

		if (pBot->DistanceFrom(m_pFollow) > 150.0f)
			pBot->SetMoveTo(m_vLastSeeVector);
		else
			pBot->StopMoving();

		CClassInterface::GetVelocity(m_pFollow, &m_vLastSeeVelocity);

		m_vLastSeeVector = m_vLastSeeVector + m_vLastSeeVelocity;

		m_fFollowTime = engine->Time() + 1.0f;
	}

	if (m_fFollowTime < engine->Time())
		Complete();
}

////////////////////////////////////////////////

/*void CDODDropAmmoTask :: DebugString ( char *string )
{
sprintf(string,"CDODDropAmmoTask");
}

void CDODDropAmmoTask :: Execute (CBot *pBot,CBotSchedule *pSchedule)
{
Vector vOrigin = CBotGlobals::EntityOrigin(m_pPlayer.Get());

if ( m_pPlayer.Get() == NULL )
{
Fail();
return;
}

if ( !CBotGlobals::EntityIsAlive(m_pPlayer) )
{
Fail();
return;
}

pBot->SetLookAtTask(LOOK_VECTOR);
pBot->SetLookVector(vOrigin);
pBot->SetMoveTo(vOrigin);

if ( pBot->IsFacing(vOrigin) )
{
if ( !pBot->IsVisible(m_pPlayer) )
{
Fail();
return;
}
}

if ( (pBot->DistanceFrom(m_pPlayer) < 200.0f) && pBot->IsFacing(vOrigin) )
{
CDODBot *pDODBot = ((CDODBot*)pBot);
pDODBot->dropAmmo();
Complete();
return;
}

}*/

////////////////////////////////////////////
CCrouchHideTask::CCrouchHideTask(edict_t *pHideFrom)
{
	m_pHideFrom = pHideFrom;
	m_vLastSeeVector = CBotGlobals::EntityOrigin(pHideFrom);
	m_bCrouching = true; // duck
	m_fChangeTime = 0.0f;
	m_fHideTime = 0.0f;
}

void CCrouchHideTask::Init()
{
	m_bCrouching = true; // duck
	m_fChangeTime = 0.0f;
	m_fHideTime = 0.0f;
}

void CCrouchHideTask::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	pBot->WantToListen(false);

	if (m_pHideFrom.Get() == NULL)
	{
		Complete();
		return;
	}
	if (!CBotGlobals::EntityIsAlive(m_pHideFrom))
	{
		Complete();
		return;
	}

	if (m_fHideTime == 0)
		m_fHideTime = engine->Time() + RandomFloat(7.0f, 14.0f);

	if (m_fChangeTime == 0.0f)
		m_fChangeTime = engine->Time() + RandomFloat(0.9f, 2.9f);

	pBot->StopMoving();

	if (pBot->IsVisible(m_pHideFrom))
		m_vLastSeeVector = CBotGlobals::EntityOrigin(m_pHideFrom);

	pBot->SetLookVector(m_vLastSeeVector);

	pBot->SetLookAtTask(LOOK_VECTOR);

	if (m_fChangeTime < engine->Time())
	{
		m_bCrouching = !m_bCrouching;
		m_fChangeTime = engine->Time() + RandomFloat(1.0f, 3.0f);
	}

	if (m_bCrouching)
	{
		pBot->Duck(true);
		pBot->WantToShoot(false);
	}
	else if (pBot->HasEnemy())
	{
		m_fHideTime = engine->Time() + 10.0f;
	}

	// refrain from proning
	pBot->UpdateCondition(CONDITION_RUN);
	pBot->RemoveCondition(CONDITION_PRONE);

	if (m_fHideTime < engine->Time())
		Complete();

}
////////////////////////////////////////////////////////
CHideTask::CHideTask(Vector vHideFrom)
{
	m_vHideFrom = vHideFrom;
}

void CHideTask::Init()
{
	m_fHideTime = 0;
}

void CHideTask::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	pBot->StopMoving();
	pBot->SetLookVector(m_vHideFrom);
	pBot->SetLookAtTask(LOOK_VECTOR);
	pBot->Duck(true);

	if (m_fHideTime == 0)
		m_fHideTime = engine->Time() + RandomFloat(5.0, 10.0);

	if (m_fHideTime < engine->Time())
		Complete();
}
///////////////////////////////////////////
CBotTF2DemomanPipeJump::CBotTF2DemomanPipeJump(CBot *pBot, Vector vWaypointGround,
	Vector vWaypointNext,
	CBotWeapon *pWeapon)
{
	m_iStartingAmmo = pWeapon->GetClip1(pBot);
	m_vStart = vWaypointGround - Vector(0, 0, 48.0);
	m_vEnd = vWaypointNext;
	m_pPipeBomb = NULL;
	m_fTime = 0;
	m_iState = 0;
	m_pWeapon = pWeapon;
	m_bFired = false;
}

void CBotTF2DemomanPipeJump::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	pBot->WantToListen(false);

	if (m_fTime == 0)
	{
		// init
		m_fTime = engine->Time() + RandomFloat(5.0f, 10.0f);
	}

	if (m_fTime < engine->Time())
		Fail(); // time out

	if (pBot->GetCurrentWeapon() != m_pWeapon)
	{
		pBot->SelectBotWeapon(m_pWeapon);
		return;
	}

	if (m_pPipeBomb)
	{
		if (!CBotGlobals::EntityIsValid(m_pPipeBomb))
		{
			Fail();
			return;
		}
	}

	switch (m_iState)
	{
	case 0:
		if (m_pWeapon->GetClip1(pBot) == 0)
		{
			if (m_pWeapon->GetAmmo(pBot) == 0)
				Fail();
			else if (RandomInt(0, 1))
				pBot->TapButton(IN_RELOAD);
		}
		else
		{
			if (!m_bFired && !m_iStartingAmmo)
				m_iStartingAmmo = m_pWeapon->GetClip1(pBot);
			else if (m_bFired && (m_iStartingAmmo > m_pWeapon->GetClip1(pBot)))
			{
				// find pipe bomb
				m_pPipeBomb = CClassInterface::FindEntityByClassnameNearest(pBot->GetOrigin(), "tf_projectile_pipe_remote", 150.0f, NULL);

				if (m_pPipeBomb)
				{
					// Set thIs up incase of Fail, the bot knows he Has a sticky there
					((CBotTF2*)pBot)->SetStickyTrapType(m_vStart, TF_TRAP_TYPE_ENEMY);
					m_iState++;
				}
				else
					Fail();
			}
			else
			{
				pBot->SetLookVector(m_vStart);
				pBot->SetLookAtTask(LOOK_VECTOR);

				if (pBot->DistanceFrom(m_vStart) < 150)
				{
					if (pBot->DotProductFromOrigin(m_vStart) > 0.99)
					{
						if (RandomInt(0, 1))
						{
							pBot->PrimaryAttack();
							m_bFired = true;
						}
					}
				}
				else
				{
					pBot->SetMoveTo(m_vStart);
				}
			}
		}
		break;
	case 1:
	{
		Vector v_comp;
		Vector v_startrunup;
		Vector v_pipe;
		Vector vel;
		extern ConVar bot_demo_runup_dist;

		if (CClassInterface::GetVelocity(m_pPipeBomb, &vel))
		{
			if (vel.Length() > 1.0)
				break; // wait until the pipe bomb Has rested
		}

		v_comp = m_vEnd - m_vStart;
		v_comp = v_comp / v_comp.Length();

		v_pipe = CBotGlobals::EntityOrigin(m_pPipeBomb);
		v_startrunup = v_pipe - (v_comp * bot_demo_runup_dist.GetFloat());
		v_startrunup.z = v_pipe.z;

		pBot->LookAtEdict(m_pPipeBomb);
		pBot->SetLookAtTask(LOOK_EDICT);

		// m_pPipeBomb != NULL

		// run up and jump time

		if (pBot->DistanceFrom(v_startrunup) < 52.0f)
			m_iState++;

		pBot->SetMoveTo(v_startrunup);
	}
	break;
	case 2:
	{
		Vector v_comp;
		extern ConVar bot_demo_runup_dist;
		Vector v_endrunup;
		Vector v_pipe;

		v_comp = m_vEnd - m_vStart;
		v_comp = v_comp / v_comp.Length();
		v_pipe = CBotGlobals::EntityOrigin(m_pPipeBomb);

		v_endrunup = v_pipe + (v_comp * bot_demo_runup_dist.GetFloat());
		v_endrunup.z = v_pipe.z;

		pBot->SetLookVector(m_vEnd);
		pBot->SetLookAtTask(LOOK_VECTOR);

		// m_pPipeBomb != NULL

		// run up and jump time

		if (pBot->DistanceFrom(v_endrunup) < 48.0f)
		{
			m_iState++;
		}

		pBot->SetMoveTo(v_endrunup);
	}
	break;
	case 3:
		pBot->Jump();
		m_iState++;
		break;
	case 4:
	{
		Vector vel;

		if (CClassInterface::GetVelocity(pBot->GetEdict(), &vel))
		{
			if (vel.z > 10)
			{
				((CBotTF2*)pBot)->DetonateStickies(true);
				Complete();
			}
		}
		else
		{
			((CBotTF2*)pBot)->DetonateStickies(true);
			Complete();
		}
	}
	break;
	default:
		Fail();
	}
}

//////////////////////////////////////////
CBotTF2DemomanPipeEnemy::CBotTF2DemomanPipeEnemy(CBotWeapon *pPipeLauncher, Vector vEnemy, edict_t *pEnemy)
{
	m_vEnemy = vEnemy;
	m_pEnemy = MyEHandle(pEnemy);
	m_fTime = 0.0f;
	m_vAim = vEnemy;
	m_pPipeLauncher = pPipeLauncher;
	m_fHoldAttackTime = 0.0f;
	m_fHeldAttackTime = 0.0f;
}

void CBotTF2DemomanPipeEnemy::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	if (m_pEnemy.Get() == NULL)
	{
		Fail();
		return;
	}

	if (pBot->RecentlyHurt(1.0f))
	{
		// taking damage
		Fail();
		pBot->UpdateUtilTime(pBot->GetCurrentUtil());
		return;
	}

	if (CTeamFortress2Mod::TF2_IsPlayerInvuln(pBot->GetEdict()))
		Fail();

	pBot->WantToListen(false);
	pBot->WantToInvestigateSound(false);

	if (m_fTime == 0)
	{

		m_vStand = CWaypoints::GetWaypoint(pSchedule->PassedInt())->GetOrigin();
		Vector vOtherWaypoint = pSchedule->PassedVector();
		((CBotTF2*)pBot)->SetStickyTrapType(m_vEnemy, TF_TRAP_TYPE_ENEMY);

		// Need to Lob my pipes
		if (vOtherWaypoint.z > (m_vEnemy.z + 32.0f))
		{
			m_vAim = m_vEnemy; //(m_vEnemy - pBot->GetOrigin())/2;
			m_vAim.z = m_vEnemy.z + GetGrenadeZ(pBot->GetEdict(), m_pEnemy, pBot->GetOrigin(), m_vEnemy, TF2_GRENADESPEED);
		}
		else
		{
			// otherwIse just aim at the closest waypoint
			m_vAim = (vOtherWaypoint + m_vEnemy) / 2;
		}

		m_fHoldAttackTime = (pBot->DistanceFrom(m_vEnemy) / 512.0f) - 1.0f;

		if (m_fHoldAttackTime < 0.0f)
			m_fHoldAttackTime = 0.0f;

		/*
		if ( sv_gravity )
		{
		float fFraction = ((m_vAim-m_vStand).Length())/MAX_GREN_THROW_DIST;

		m_vAim.z = m_vAim.z + (sv_gravity->GetFloat() * RandomFloat(1.5f,2.5f) * fFraction);
		}*/

		m_fTime = engine->Time() + RandomFloat(5.0f, 10.0f);
	}

	if (pBot->DistanceFrom(m_vStand) > 200)
	{
		Fail();
		pBot->SetLastEnemy(NULL);
	}


	if (!CBotGlobals::EntityIsValid(m_pEnemy) || !CBotGlobals::EntityIsAlive(m_pEnemy) || (m_fTime < engine->Time()))
	{
		// blow up any grens before we finIsh
		//if ( m_pEnemy.Get() && pBot->IsVisible(m_pEnemy.Get()) )
		((CBotTF2*)pBot)->DetonateStickies(true);

		Complete();
		pBot->SetLastEnemy(NULL);
		return;
	}

	pBot->WantToShoot(false);

	if ((m_pPipeLauncher->GetAmmo(pBot) + m_pPipeLauncher->GetClip1(pBot)) == 0)
	{
		if (pBot->IsVisible(m_pEnemy.Get()))
			((CBotTF2*)pBot)->DetonateStickies(true);

		Complete();
		pBot->SetLastEnemy(NULL);
		return;
	}

	if (pBot->GetCurrentWeapon() != m_pPipeLauncher)
	{
		pBot->SelectBotWeapon(m_pPipeLauncher);
		pBot->SetLastEnemy(NULL);
		return;
	}

	pBot->WantToChangeWeapon(false);

	if (m_pPipeLauncher->GetClip1(pBot) == 0)
	{
		if (RandomInt(0, 1))
			pBot->Reload();
	}
	else if (pBot->DistanceFrom(m_vStand) > 100.0f)
		pBot->SetMoveTo(m_vStand);
	else
	{
		pBot->SetLookAtTask(LOOK_VECTOR);
		pBot->SetLookVector(m_vAim);
		pBot->StopMoving();

		if (pBot->DotProductFromOrigin(m_vAim) > 0.99)
		{
			float fTime = engine->Time();

			if (m_fHeldAttackTime == 0)
				m_fHeldAttackTime = fTime + m_fHoldAttackTime + RandomFloat(0.0, 0.15);

			if (m_fHeldAttackTime > fTime)
				pBot->PrimaryAttack(true);
			else
			{
				if (m_fHeldAttackTime < (fTime - 0.1f))
					m_fHeldAttackTime = 0;

				pBot->LetGoOfButton(IN_ATTACK);
			}

			((CBotTF2*)pBot)->SetStickyTrapType(m_vEnemy, TF_TRAP_TYPE_ENEMY);
		}
	}
}


//////////////////////////////////////////
CBotTF2DemomanPipeTrap::CBotTF2DemomanPipeTrap(eDemoTrapType type, Vector vStand, Vector vLoc, Vector vSpread, bool bAutoDetonate, int wptarea)
{
	m_vPoint = vLoc;
	m_vLocation = vLoc;
	m_vSpread = vSpread;
	m_iState = 0;
	m_iStickies = 6;
	m_iTrapType = type;
	m_vStand = vStand;
	m_fTime = 0.0f;
	m_bAutoDetonate = bAutoDetonate;
	m_iWptArea = wptarea;
}

void CBotTF2DemomanPipeTrap::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	bool bFail = false;
	CBotTF2 *pTF2Bot = (CBotTF2*)pBot;

	pBot->WantToChangeWeapon(false);

	if (pBot->GetEnemy() && pBot->HasSomeConditions(CONDITION_SEE_CUR_ENEMY))
	{
		if (CTeamFortress2Mod::HasRoundStarted())
		{
			pBot->SecondaryAttack();
			Fail();
		}
	}

	if (pTF2Bot->DeployStickies(m_iTrapType, m_vStand, m_vLocation, m_vSpread, &m_vPoint, &m_iState, &m_iStickies, &bFail, &m_fTime, m_iWptArea))
	{
		Complete();

		if (m_bAutoDetonate)
			pBot->SecondaryAttack();
	}

	if (bFail)
		Fail();
}
/////////

CMessAround::CMessAround(edict_t *pFriendly, int iMaxVoiceCmd)
{
	m_fTime = 0.0f;
	m_pFriendly = pFriendly;
	m_iType = RandomInt(0, 3);
	m_iMaxVoiceCmd = iMaxVoiceCmd;
}

void CMessAround::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	if (!m_pFriendly || !CBotGlobals::EntityIsValid(m_pFriendly))
	{
		Fail();
		return;
	}

	// smack the friendly player with my melee attack
	switch (m_iType)
	{
	case 0:
	{
		Vector origin = CBotGlobals::EntityOrigin(m_pFriendly);

		pBot->SetLookVector(origin);
		pBot->SetLookAtTask(LOOK_VECTOR);


		if (pBot->DistanceFrom(m_pFriendly) > 100)
		{
			pBot->SetMoveTo((origin));
		}
		else if (pBot->FInViewCone(m_pFriendly))
		{
			CBotWeapon *pWeapon = pBot->GetBestWeapon(NULL, true, true);

			if (pWeapon)
			{
				pBot->SelectBotWeapon(pWeapon);

				if (RandomInt(0, 1))
					pBot->PrimaryAttack();
			}
		}

		if (!m_fTime)
			m_fTime = engine->Time() + RandomFloat(3.5f, 8.0f);
	}
	break;// Taunt at my friendly player
	case 1:
	{
		Vector origin = CBotGlobals::EntityOrigin(m_pFriendly);
		bool ok = true;

		pBot->SetLookVector(origin);
		pBot->SetLookAtTask(LOOK_VECTOR);

		if (!pBot->FInViewCone(m_pFriendly))
		{
			ok = false;
		}

		if (pBot->DistanceFrom(m_pFriendly) > 100)
		{
			pBot->SetMoveTo((origin));
			ok = false;
		}

		if (ok)
		{
			if (pBot->IsTF2())
				((CBotTF2*)pBot)->Taunt(true);
			//else if ( pBot->IsDOD() )
			//	((CDODBot*)pBot)->Taunt(); pBot->impulse(100);
		}

		if (!m_fTime)
			m_fTime = engine->Time() + RandomFloat(3.5f, 6.5f);

	}
	// say some random voice commands
	break;
	case 2:
	{
		if (!m_fTime)
			pBot->AddVoiceCommand(RandomInt(0, m_iMaxVoiceCmd - 1));

		if (!m_fTime)
			m_fTime = engine->Time() + RandomFloat(1.5f, 3.0f);
	}
	// press some random buttons, such as attack2, jump
	break;
	case 3:
	{
		if (RandomInt(0, 1))
			pBot->Jump();
		else
		{
			if (pBot->IsTF2())
			{
				if (((CBotTF2*)pBot)->GetClass() == TF_CLASS_HWGUY)
					pBot->SecondaryAttack(true);
			}
		}

		if (!m_fTime)
			m_fTime = engine->Time() + RandomFloat(1.5f, 3.0f);
	}
	default:
		break;
	}

	if (m_fTime < engine->Time())
		Complete();

	if (pBot->IsTF2())
	{
		if (CTeamFortress2Mod::HasRoundStarted())
			Complete();
	}
	/*else if ( pBot->IsDOD() )
	{
	if ( CDODMod::m_Flags.GetNumFlags() > 0 )
	Complete();
	}*/

}

///////////
//defensive technique

CBotTF2Spam::CBotTF2Spam(CBot *pBot, Vector vStart, int iYaw, CBotWeapon *pWeapon)
{
	Vector forward;
	QAngle angle = QAngle(0, iYaw, 0);

	AngleVectors(angle, &forward);
	m_vTarget = vStart + forward*2000.0f;
	CBotGlobals::QuickTraceline(pBot->GetEdict(), vStart, m_vTarget);
	m_vTarget = CBotGlobals::GetTraceResult()->endpos - forward;
	m_pWeapon = pWeapon;
	m_vStart = vStart;

	m_fTime = 0.0f;
}

CBotTF2Spam::CBotTF2Spam(Vector vStart, Vector vTarGet, CBotWeapon *pWeapon)
{
	m_vTarget = vTarGet;
	m_pWeapon = pWeapon;
	m_vStart = vStart;

	m_fTime = 0.0f;
}

float CBotTF2Spam::GetDistance()
{
	return (m_vStart - m_vTarget).Length();
}

void CBotTF2Spam::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	pBot->WantToShoot(false);
	pBot->WantToListen(false);

	if (CTeamFortress2Mod::TF2_IsPlayerInvuln(pBot->GetEdict()))
	{
		Fail();
		return;
	}

	if (pBot->RecentlyHurt(1.0f))
	{
		// taking damage
		Fail();
		pBot->UpdateUtilTime(pBot->GetCurrentUtil());
		pBot->WantToShoot(true);
		return;
	}


	if (m_fTime == 0.0f)
	{
		if (m_pWeapon->GetID() == TF2_WEAPON_GRENADELAUNCHER)
		{
			Vector vVIsibleWaypoint = pSchedule->PassedVector();

			if (vVIsibleWaypoint.z > (m_vTarget.z + 32.0f)) // need to lob grenade
				m_vTarget.z += GetGrenadeZ(pBot->GetEdict(), NULL, pBot->GetOrigin(), m_vTarget, m_pWeapon->GetProjectileSpeed());
			else // mid point between waypoint and tarGet as I won't see tarGet
				m_vTarget = (m_vTarget + vVIsibleWaypoint) / 2;
		}

		m_fNextAttack = engine->Time() + RandomFloat(0.2f, 1.0f);
		m_fTime = engine->Time() + RandomFloat(12.0f, 24.0f);
	}
	else if (m_fTime < engine->Time())
	{
		Complete();

		pBot->SetLastEnemy(NULL);

		// prevent bot from keeping doing thIs
		pBot->UpdateUtilTime(BOT_UTIL_SPAM_NEAREST_SENTRY);
		pBot->UpdateUtilTime(BOT_UTIL_SPAM_LAST_ENEMY);
		pBot->UpdateUtilTime(BOT_UTIL_SPAM_LAST_ENEMY_SENTRY);
	}

	if (pBot->DistanceFrom(m_vStart) > 100)
		pBot->SetMoveTo(m_vStart);
	else
		pBot->StopMoving();

	if (pBot->GetCurrentWeapon() != m_pWeapon)
	{
		if (!pBot->SelectBotWeapon(m_pWeapon))
			Fail();
	}

	pBot->SetLookVector(m_vTarget);
	pBot->SetLookAtTask(LOOK_VECTOR);

	if (m_pWeapon->OutOfAmmo(pBot))
	{
		pBot->SetLastEnemy(NULL);
		Complete();
	}
	else if (m_pWeapon->NeedToReload(pBot))
	{
		if (RandomInt(0, 1))
			pBot->Reload();
	}
	else if (m_fNextAttack < engine->Time())
	{
		pBot->PrimaryAttack();
		m_fNextAttack = engine->Time() + RandomFloat(0.2f, 1.0f);
	}

}

///////////

CBotTF2AttackSentryGunTask::CBotTF2AttackSentryGunTask(edict_t *pSentryGun, CBotWeapon *pWeapon)
{
	m_pSentryGun = pSentryGun;
	m_pWeapon = pWeapon;
}

void CBotTF2AttackSentryGunTask::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	pBot->WantToListen(false);
	pBot->WantToInvestigateSound(false);

	if (CTeamFortress2Mod::TF2_IsPlayerInvuln(pBot->GetEdict()))
		Fail();

	if (m_pSentryGun.Get() == NULL)
	{
		Fail();
		return;
	}

	if (m_fTime == 0.0f)
	{
		float fMinDIst = 9999;
		float fDIst;

		CWaypointVisibilityTable *table = CWaypoints::GetVisiblity();

		m_fTime = engine->Time() + RandomFloat(8.0f, 14.0f);

		m_iSentryWaypoint = CWaypointLocations::NearestWaypoint(CBotGlobals::EntityOrigin(m_pSentryGun), 200.0f, -1);

		m_iStartingWaypoint = CWaypointLocations::NearestWaypoint(pBot->GetOrigin(), 200.0f, -1, true, true, true, NULL);

		m_vStart = pBot->GetOrigin();
		m_vHide = m_vStart;

		CWaypoint *pWpt = CWaypoints::GetWaypoint(m_iStartingWaypoint);

		if (pWpt != NULL)
		{
			for (int i = 0; i < pWpt->NumPaths(); i++)
			{
				if (table->GetVisibilityFromTo(pWpt->GetPath(i), m_iSentryWaypoint) == false)
				{
					CWaypoint *pPath = CWaypoints::GetWaypoint(pWpt->GetPath(i));

					if ((fDIst = pPath->DistanceFrom(m_vStart)) < fMinDIst)
					{
						fMinDIst = fDIst;
						m_vHide = pPath->GetOrigin();
					}
				}

				pWpt->GetPath(i);
			}

		}

		// hide waypoint = 
	}
	else if (m_fTime < engine->Time())
	{
		Complete();
	}

	if (pBot->GetCurrentWeapon() != m_pWeapon)
	{
		pBot->SelectBotWeapon(m_pWeapon);
		return;
	}

	pBot->WantToChangeWeapon(false);

	if (m_pWeapon->OutOfAmmo(pBot))
		Complete();

	pBot->LookAtEdict(m_pSentryGun);
	pBot->SetLookAtTask(LOOK_EDICT);

	if (m_pWeapon->NeedToReload(pBot) || (CClassInterface::GetSentryEnemy(m_pSentryGun) == pBot->GetEdict()))
	{
		if (pBot->DistanceFrom(m_vHide) > 80.0f)
			pBot->SetMoveTo(m_vHide);
		else
		{
			if (m_pWeapon->NeedToReload(pBot))
			{
				if (RandomInt(0, 1))
					pBot->Reload();
			}
			pBot->StopMoving();
		}
	}
	else if (pBot->DistanceFrom(m_vStart) > 80.0f)
		pBot->SetMoveTo(m_vStart);
	else
		pBot->StopMoving();

	if (pBot->IsVisible(m_pSentryGun))
	{
		// use thIs shooting method below
		pBot->WantToShoot(false);
		// attack
		pBot->HandleAttack(m_pWeapon, m_pSentryGun);
	}
}

/////////////

void CBotNest::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	CBotTF2 *pBotTF2 = (CBotTF2*)pBot;

	if ((pBotTF2->GetClass() == TF_CLASS_MEDIC) && pBotTF2->SomeoneCalledMedic())
		Fail(); // Follow player

	if (!pBotTF2->WantToNest())
	{
		Complete();
		pBotTF2->AddVoiceCommand(TF_VC_GOGOGO);
		return;
	}
	else if (pBot->HasSomeConditions(CONDITION_PUSH))
	{
		Complete();
		pBot->RemoveCondition(CONDITION_PUSH);
		pBotTF2->AddVoiceCommand(TF_VC_GOGOGO);
		return;
	}

	if (m_fTime == 0)
	{
		m_fTime = engine->Time() + RandomFloat(6.0f, 12.0f);

		if (RandomInt(0, 1))
			pBotTF2->AddVoiceCommand(TF_VC_HELP);
	}
	else if (m_fTime < engine->Time())
	{
		Complete();
		pBotTF2->AddVoiceCommand(TF_VC_GOGOGO);
	}

	// wait around
	// wait for more friendlies
	// heal up
	// 

	pBot->SetLookAtTask(LOOK_AROUND);

	pBot->StopMoving();
}

CBotNest::CBotNest()
{
	m_fTime = 0.0f;
	m_pEnemy = NULL;
}
////////////////////////////////////////////////



/////////////////////////////////////////////

void CBotJoinSquad::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	if (!pBot->InSquad())
	{
		pBot->SetSquad(CBotSquads::AddSquadMember(m_pPlayer, pBot->GetEdict()));
		Complete();
	}
	//CBotSquads::SquadJoin(pBot->GetEdict(),m_pPlayer);
}

void CBotFollowSquadLeader::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	edict_t *pSquadLeader;
	float fDIst;

	if (!pBot->InSquad(m_pSquad))
	{
		Fail();
		return;
	}

	pSquadLeader = m_pSquad->GetLeader();

	if (pSquadLeader == pBot->GetEdict())
	{
		Fail();
		return;
	}

	if (CClassInterface::GetMoveType(pSquadLeader) == MOVETYPE_LADDER)
	{
		pBot->StopMoving();
		return;
	}

	if ((m_fVisibleTime > 0.0f) && !pBot->HasEnemy() && !pBot->IsVisible(pSquadLeader))
	{
		// haven't seen leader for five seconds
		if ((engine->Time() - m_fVisibleTime) > 0.2f)
		{
			Complete();
			return;
		}
	}
	else
		m_fVisibleTime = engine->Time();

	if (pBot->HasSomeConditions(CONDITION_SQUAD_IDLE))
	{
		// squad leader idle for 3 seconds. see what else i can do
		Complete();
		return;
	}

	if (m_fUpdateMovePosTime < engine->Time())
	{
		float fRand;
		Vector vVelocity;

		fRand = RandomFloat(1.0f, 2.0f);

		CClassInterface::GetVelocity(pSquadLeader, &vVelocity);

		m_fLeaderSpeed = (MIN(m_fLeaderSpeed, 100.0f)*0.5f) + (vVelocity.Length()*0.5f);

		m_fUpdateMovePosTime = engine->Time() + (fRand * (1.0f - (vVelocity.Length() / 320)));
		m_vPos = m_pSquad->GetFormationVector(pBot->GetEdict());

		m_vForward = m_vPos + vVelocity;
	}

	fDIst = pBot->DistanceFrom(m_vPos); // More than reachable range

	if (fDIst > 400.0f)
	{
		Fail(); // find a path instead
		return;
	}
	else if (fDIst > m_pSquad->GetSpread())
	{
		pBot->SetMoveTo(m_vPos);
		pBot->SetSquadIdleTime(engine->Time());

		if (pBot->IsVisible(pSquadLeader))
			pBot->SetMoveSpeed(m_fLeaderSpeed);
	}
	else
	{
		// idle
		pBot->StopMoving();
		pBot->SquadInPosition();
	}

	pBot->SetLookVector(m_vForward);
	pBot->SetLookAtTask(LOOK_VECTOR);
}
////////////////////////////////////////////////////

/*CBotDODSnipe :: CBotDODSnipe ( CBotWeapon *pWeaponToUse, Vector vOrigin, float fYaw, bool bUseZ, float z, int iWaypointType )
{
QAngle angle;
m_fEnemyTime = 0.0f;
m_fTime = 0.0f;
angle = QAngle(0,fYaw,0);
AngleVectors(angle,&m_vAim);
m_vAim = vOrigin + (m_vAim*1024);
m_vOrigin = vOrigin;
m_pWeaponToUse = pWeaponToUse;
m_fScopeTime = 0;
m_bUseZ = bUseZ;
m_z = z; // z = ground level
m_iWaypointType = iWaypointType;
m_fTimeout = 0.0f;
}

void CBotDODSnipe :: DebugString ( char *string )
{
sprintf(string,"CBotDODSnipe\nm_fTime = %0.2f\npWeaponToUse = %s\nm_bUseZ = %s\nm_z = %0.2f",m_fTime,m_pWeaponToUse->GetWeaponInfo()->GetWeaponName(),m_bUseZ ? "true":"false",m_z);
}

void CBotDODSnipe :: Execute (CBot *pBot,CBotSchedule *pSchedule)
{
static CBotWeapon *pCurrentWeapon;
static CWeapon *pWeapon;

static bool bDeployedOrZoomed;
static float fDIst;

bDeployedOrZoomed = false;

pBot->WantToShoot(false);
pBot->WantToListen(false);

if ( m_fTime == 0.0f )
{
m_fEnemyTime = 0.0f;
m_fTime = engine->Time() + RandomFloat(20.0f,40.0f);
pBot->ResetLookAroundTime();
}

pCurrentWeapon = pBot->GetCurrentWeapon();

if ( !pCurrentWeapon )
{
Fail();
return;
}

pWeapon = pCurrentWeapon->GetWeaponInfo();

if (pWeapon == NULL)
{
Fail();
return;
}

if ( pCurrentWeapon != m_pWeaponToUse )
{
if ( !pBot->Select_CWeapon(CWeapons::GetWeapon(m_pWeaponToUse->GetID())) )
{
Fail();
}

return;
}
else
{
if ( pCurrentWeapon->IsZoomable() )
bDeployedOrZoomed = CClassInterface::IsSniperWeaponZoomed(pCurrentWeapon->GetWeaponEntity());
else if ( pCurrentWeapon->IsDeployable() )
bDeployedOrZoomed = CClassInterface::IsMachineGunDeployed(pCurrentWeapon->GetWeaponEntity());

if ( m_fScopeTime < engine->Time() )
{
if ( !bDeployedOrZoomed )
{
pBot->SecondaryAttack();

if ( m_fTimeout == 0.0f )
m_fTimeout = engine->Time();
else if ( (m_fTimeout + 3.0f) < engine->Time() )
Fail();
}
else
m_fTimeout = 0.0f;

m_fScopeTime = engine->Time() + RandomFloat(0.5f,1.0f);
}
}

if ( pCurrentWeapon->GetAmmo(pBot) < 1 )
{
if ( bDeployedOrZoomed )
pBot->SecondaryAttack();

Complete();
}
else if ( pBot->DistanceFrom(m_vOrigin) > 200 ) // too far from sniper point
{
if ( bDeployedOrZoomed )
pBot->SecondaryAttack();
// too far away
Fail();
}

if ( (m_fEnemyTime + 5.0f) > engine->Time() )
{
pBot->SetLookAtTask(LOOK_VECTOR);
pBot->SetLookVector(m_vLastEnemy);
}
else if ( m_bUseZ )
{
Vector vAim = Vector(m_vAim.x,m_vAim.y,m_z);
pBot->SetLookAtTask(LOOK_VECTOR);
pBot->SetLookVector(pBot->snipe(vAim));
}
else
{
pBot->SetLookAtTask(LOOK_VECTOR);
pBot->SetLookVector(pBot->snipe(m_vAim));
}

fDIst = (m_vOrigin - pBot->GetOrigin()).Length2D();

if ( (fDIst > 16) || !bDeployedOrZoomed )
{
pBot->SetMoveTo(m_vOrigin);
pBot->SetMoveSpeed(CClassInterface::GetMaxSpeed(pBot->GetEdict())/8);

if ( ( fDIst < 48 ) && ((CDODBot*)pBot)->withinTeammate() )
Fail();
}
else
{

bool unprone = false;

pBot->StopMoving();

if ( m_iWaypointType & CWaypointTypes::W_FL_PRONE )
{
//pBot->UpdateDanger(MAX_BELIEF);
pBot->removeCondition(CONDITION_RUN);
pBot->UpdateCondition(CONDITION_PRONE);

}
else
{
if ( m_iWaypointType & CWaypointTypes::W_FL_CROUCH )
pBot->duck();
// refrain from proning
pBot->UpdateCondition(CONDITION_RUN);
pBot->removeCondition(CONDITION_PRONE);
unprone = true;

}

if ( unprone )
{
CClassInterface::GetPlayerInfoDOD(pBot->GetEdict(),&unprone,NULL);
if ( unprone )
{
CDODBot *pDODBot = (CDODBot *)pBot;
pDODBot->unProne();
}
}

// no enemy for a while
if ( (m_fEnemyTime + m_fTime) < engine->Time() )
{
if ( bDeployedOrZoomed )
pBot->SecondaryAttack();

Complete();
}
}

if ( pBot->HasEnemy() )
{
pBot->SetMoveLookPriority(MOVELOOK_ATTACK);

pBot->SetLookAtTask(LOOK_ENEMY);

pBot->handleAttack(pCurrentWeapon,pBot->GetEnemy());

pBot->SetMoveLookPriority(MOVELOOK_TASK);

// havin' fun
m_fEnemyTime = engine->Time();

m_vLastEnemy = CBotGlobals::EntityOrigin(pBot->GetEnemy());
}
}

//////////////////////////

CBotHL2DMSnipe :: CBotHL2DMSnipe ( CBotWeapon *pWeaponToUse, Vector vOrigin, float fYaw, bool bUseZ, float z, int iWaypointType )
{
QAngle angle;
m_fEnemyTime = 0.0f;
m_fTime = 0.0f;
angle = QAngle(0,fYaw,0);
AngleVectors(angle,&m_vAim);
m_vAim = vOrigin + (m_vAim*1024);
m_vOrigin = vOrigin;
m_pWeaponToUse = pWeaponToUse;
m_fScopeTime = 0;
m_bUseZ = bUseZ;
m_z = z; // z = ground level
m_iWaypointType = iWaypointType;
}

void CBotHL2DMSnipe :: DebugString ( char *string )
{
sprintf(string,"CBotHL2DMSnipe\nm_fTime = %0.2f\npWeaponToUse = %s\nm_bUseZ = %s\nm_z = %0.2f",m_fTime,m_pWeaponToUse->GetWeaponInfo()->GetWeaponName(),m_bUseZ ? "true":"false",m_z);
}

void CBotHL2DMSnipe :: Execute (CBot *pBot,CBotSchedule *pSchedule)
{
static CBotWeapon *pCurrentWeapon;
static CWeapon *pWeapon;

static bool bDeployedOrZoomed;
static float fDIst;

bDeployedOrZoomed = false;

pBot->WantToShoot(false);
pBot->WantToListen(false);

if ( m_fTime == 0.0f )
{
m_fEnemyTime = engine->Time();
m_fTime = m_fEnemyTime + RandomFloat(20.0f,40.0f);
pBot->ResetLookAroundTime();
}

pCurrentWeapon = pBot->GetCurrentWeapon();

if ( !pCurrentWeapon )
{
Fail();
return;
}

pWeapon = pCurrentWeapon->GetWeaponInfo();

if (pWeapon == NULL)
{
Fail();
return;
}

// refrain from proning
pBot->UpdateCondition(CONDITION_RUN);

if ( pCurrentWeapon != m_pWeaponToUse )
{
if ( !pBot->Select_CWeapon(CWeapons::GetWeapon(m_pWeaponToUse->GetID())) )
{
Fail();
}

return;
}

if ( pCurrentWeapon->GetAmmo(pBot) < 1 )
{
Complete();
}
else if ( pBot->DistanceFrom(m_vOrigin) > 200 ) // too far from sniper point
{
// too far away
Fail();
}

if ( m_bUseZ )
{
Vector vAim = Vector(m_vAim.x,m_vAim.y,m_z);
pBot->SetLookAtTask(LOOK_VECTOR);
pBot->SetLookVector(pBot->snipe(vAim));
}
else
{
pBot->SetLookAtTask(LOOK_SNIPE);
pBot->SetLookVector(pBot->snipe(m_vAim));
}

fDIst = (m_vOrigin - pBot->GetOrigin()).Length2D();

if ( fDIst > 16 )
{
pBot->SetMoveTo(m_vOrigin);
pBot->SetMoveSpeed(CClassInterface::GetMaxSpeed(pBot->GetEdict())/8);

//if ( ( fDIst < 48 ) && ((CDODBot*)pBot)->withinTeammate() )
//	Fail();
}
else
{
pBot->StopMoving();

if ( m_iWaypointType & CWaypointTypes::W_FL_CROUCH )
pBot->duck();

// no enemy for a while
if ( (m_fEnemyTime + m_fTime) < engine->Time() )
{
//if ( bDeployedOrZoomed )
//	pBot->SecondaryAttack();

Complete();
}
}

if ( pBot->HasEnemy() )
{
pBot->SetMoveLookPriority(MOVELOOK_ATTACK);
pBot->SetLookAtTask(LOOK_ENEMY);
pBot->handleAttack(pCurrentWeapon,pBot->GetEnemy());
pBot->SetMoveLookPriority(MOVELOOK_TASK);

// havin' fun
m_fEnemyTime = engine->Time();
}
}*/
///////////////////////////////////////////
// interrupts

CBotTF2EngineerInterrupt::CBotTF2EngineerInterrupt(CBot *pBot)
{
	m_pSentryGun = CTeamFortress2Mod::GetMySentryGun(pBot->GetEdict());

	if (m_pSentryGun.Get() != NULL)
	{
		m_fPrevSentryHealth = CClassInterface::GetSentryHealth(m_pSentryGun);
	}
	else
		m_fPrevSentryHealth = 0;
}

bool CBotTF2CoverInterrupt::IsInterrupted(CBot *pBot, bool *bFailed, bool *bCompleted)
{
	if (CTeamFortress2Mod::TF2_IsPlayerInvuln(pBot->GetEdict()))
	{
		*bFailed = true;
		return true;
	}

	return false;
}

CBotTF2HurtInterrupt::CBotTF2HurtInterrupt(CBot *pBot)
{
	m_iHealth = pBot->GetHealthPercent();
}

bool CBotTF2HurtInterrupt::IsInterrupted(CBot *pBot, bool *bFailed, bool *bCompleted)
{
	if (m_iHealth > pBot->GetHealthPercent())
	{
		*bFailed = true;
		pBot->SecondaryAttack();
		return true;
	}

	return false;
}

bool CBotTF2EngineerInterrupt::IsInterrupted(CBot *pBot, bool *bFailed, bool *bCompleted)
{
	if (m_pSentryGun.Get() != NULL)
	{
		if (!CClassInterface::GetTF2BuildingIsMini(m_pSentryGun))
		{
			float m_fCurrentHealth = CClassInterface::GetSentryHealth(m_pSentryGun);

			if ((((CBotFortress*)pBot)->GetMetal() > 75) && (m_fCurrentHealth < m_fPrevSentryHealth))
			{
				*bFailed = true;
				return true;
			}

			m_fPrevSentryHealth = m_fCurrentHealth;
		}
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////
// Base Task
CBotTask::CBotTask()
{
	_Init();
}

bool CBotTask::TimedOut()
{
	return (this->m_fTimeOut != 0) && (engine->Time() < this->m_fTimeOut);
}

eTaskState CBotTask::IsInterrupted(CBot *pBot)
{
	if (m_pInterruptFunc != NULL)
	{
		bool bFailed = false;
		bool bCompleted = false;

		if (m_pInterruptFunc->IsInterrupted(pBot, &bFailed, &bCompleted))
		{
			if (bFailed)
				return STATE_FAIL;

			return STATE_COMPLETE;
		}
	}

	if (m_iCompleteInterruptConditionsHave)
	{
		if (pBot->HasAllConditions(m_iCompleteInterruptConditionsHave))
			return STATE_COMPLETE;
	}

	if (m_iCompleteInterruptConditionsDontHave)
	{
		if (!pBot->HasAllConditions(m_iCompleteInterruptConditionsDontHave))
			return STATE_COMPLETE;
	}

	if (m_iFailInterruptConditionsHave)
	{
		if (pBot->HasAllConditions(m_iFailInterruptConditionsHave))
			return STATE_FAIL;
	}

	if (m_iFailInterruptConditionsDontHave)
	{
		if (!pBot->HasAllConditions(m_iFailInterruptConditionsDontHave))
			return STATE_FAIL;
	}

	return STATE_RUNNING;
}

void CBotTask::_Init()
{
	m_pInterruptFunc = NULL;
	m_iFlags = 0;
	m_iState = STATE_IDLE;
	m_fTimeOut = 0;
	//	m_pEdict = NULL;
	//	m_fFloat = 0;
	//	m_iInt = 0;
	//	m_vVector = Vector(0,0,0);
	m_iFailInterruptConditionsHave = 0;
	m_iFailInterruptConditionsDontHave = 0;
	m_iCompleteInterruptConditionsHave = 0;
	m_iCompleteInterruptConditionsDontHave = 0;
	Init();
}

void CBotTask::Init()
{
	return;
}

void CBotTask::Execute(CBot *pBot, CBotSchedule *pSchedule)
{
	return;
}

bool CBotTask::HasFailed()
{
	return m_iState == STATE_FAIL;
}

bool CBotTask::IsComplete()
{
	return m_iState == STATE_COMPLETE;
}
/*
void CBotTask :: SetVector ( Vector vOrigin )
{
m_vVector = vOrigin;
}

void CBotTask :: SetFloat ( float fFloat )
{
m_fFloat = fFloat;
}

void CBotTask :: SetEdict ( edict_t *pEdict )
{
m_pEdict = pEdict;
}
*/
// if thIs condition Is true it will Complete, if bUnSet Is true, the condition must be false to be Complete
void CBotTask::SetCompleteInterrupt(int iInterruptHave, int iInterruptDontHave)
{
	m_iCompleteInterruptConditionsHave = iInterruptHave;
	m_iCompleteInterruptConditionsDontHave = iInterruptDontHave;
}

void CBotTask::SetFailInterrupt(int iInterruptHave, int iInterruptDontHave)
{
	m_iFailInterruptConditionsHave = iInterruptHave;
	m_iFailInterruptConditionsDontHave = iInterruptDontHave;
}

void CBotTask::Fail()
{
	m_iState = STATE_FAIL;
}

void CBotTask::Complete()
{
	m_iState = STATE_COMPLETE;
}
