/*
 *    part of https://rcbot2.svn.sourceforge.net/svnroot/rcbot2
 *
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

#include "ndebugoverlay.h"

#include "bot_waypoint.h"
#include "bot_menu.h"
#include "bot_wpt_color.h"
#include "bot_globals.h"
#include "bot_client.h"

CBotMenu *CBotMenuList::m_MenuList[BOT_MENU_MAX];

void CWaypointFlagMenuItem::Activate(CClient *pClient)
{
	int iWpt = pClient->CurrentWaypoint();
	CWaypoint *pWpt = CWaypoints::GetWaypoint(iWpt);
	CWaypointType *type = CWaypointTypes::GetTypeByIndex(m_iFlag);

	if (pWpt)
	{
		if (pWpt->HasFlag(type->GetBits()))
			pWpt->RemoveFlag(type->GetBits());
		else
			pWpt->AddFlag(type->GetBits());
	}
}

const char *CWaypointFlagMenu::GetCaption(CClient *pClient, WptColor &color)
{
	pClient->UpdateCurrentWaypoint();

	int iWpt = pClient->CurrentWaypoint();
	CWaypoint *pWpt = CWaypoints::GetWaypoint(iWpt);

	if (pWpt)
	{
		color = CWaypointTypes::GetColour(pWpt->GetFlags());
		sprintf(m_szCaption, "Waypoint Flags ID = [%d]", iWpt);
	}
	else
	{
		color = WptColor::white;
		sprintf(m_szCaption, "No Waypoint");
	}

	return m_szCaption;
}

const char *CWaypointFlagMenuItem::GetCaption(CClient *pClient, WptColor &color)
{
	pClient->UpdateCurrentWaypoint();

	int iWpt = pClient->CurrentWaypoint();
	CWaypoint * pWpt = CWaypoints::GetWaypoint(iWpt);

	CWaypointType *type = CWaypointTypes::GetTypeByIndex(m_iFlag);

	color = type->GetColour();

	sprintf(m_szCaption, "[%s] %s", (pWpt != NULL) ? (pWpt->HasFlag(type->GetBits()) ? "x" : " ") : "No Waypoint", type->GetName());

	return m_szCaption;
}

CWaypointFlagMenu::CWaypointFlagMenu(CBotMenu *pPrev)
{
	int iMod = CBotGlobals::GetCurrentMod()->GetModId();
	// check the number of waypoint types available
	// caption
	// 1.
	// 2.
	// 3.
	// 4.
	// 5.
	// 6.
	// 7.
	// 8. More...
	// 9. Go Back

	int iNumTypes = CWaypointTypes::GetNumTypes();

	int iNumAdded = 0;
	CBotMenu *pParent;
	CBotMenu *pCurrent;

	int i;

	pCurrent = this;
	pParent = pPrev;

	for (i = 0; i < iNumTypes; i++)
	{
		if (!CWaypointTypes::GetTypeByIndex(i)->ForMod(iMod))
			continue;

		pCurrent->AddMenuItem(new CWaypointFlagMenuItem(i));
		iNumAdded++;

		if ((iNumAdded > 7) || (i == (iNumTypes - 1)))
		{
			CBotMenuItem *back = new CBotGotoMenuItem("Back...", pParent);

			pParent = pCurrent;

			if ((iNumAdded > 7) && (i < (iNumTypes - 1)))
			{
				pCurrent = new CBotMenu();
				pCurrent->SetCaption("Waypoint Flags (More)");
				pParent->AddMenuItem(new CBotGotoMenuItem("More...", pCurrent));
			}

			pParent->AddMenuItem(back);

			//	make a new menu

			iNumAdded = 0; // reset

		}
	}

}

void CBotMenuList::SetupMenus()
{
	m_MenuList[BOT_MENU_WPT] = new CWaypointMenu(); //new CWaypointFlagMenu(NULL);
}

const char *CWaypointRadiusMenu::GetCaption(CClient *pClient, WptColor &color)
{
	int iWpt = pClient->CurrentWaypoint();
	CWaypoint *pWpt = CWaypoints::GetWaypoint(iWpt);
	float fRadius = 0;

	if (pWpt)
	{
		fRadius = pWpt->GetRadius();
	}

	sprintf(m_szCaption, "Waypoint Radius (%0.1f)", fRadius);
	color = WptColor::white;

	return m_szCaption;
}

const char *CWaypointAreaMenu::GetCaption(CClient *pClient, WptColor &color)
{
	int iWpt = pClient->CurrentWaypoint();
	CWaypoint *pWpt = CWaypoints::GetWaypoint(iWpt);
	int iArea = 0;

	if (pWpt)
	{
		iArea = pWpt->GetArea();
	}

	sprintf(m_szCaption, "Waypoint Area (%d)", iArea);
	color = WptColor::white;

	return m_szCaption;
}

const char *CWaypointMenu::GetCaption(CClient *pClient, WptColor &color)
{
	int iWpt = pClient->CurrentWaypoint();

	if (iWpt == -1)
		sprintf(m_szCaption, "Waypoint Menu - No waypoint - Walk towards a waypoint");
	else
		sprintf(m_szCaption, "Waypoint Menu [%d]", iWpt);

	color = WptColor::white;

	return m_szCaption;
}

const char *CWaypointYawMenuItem::GetCaption(CClient *pClient, WptColor &color)
{
	CWaypoint *pWpt = CWaypoints::GetWaypoint(pClient->CurrentWaypoint());

	if (pWpt)
		sprintf(m_szCaption, "Yaw = %d degrees (press to update)", (int)pWpt->GetAimYaw());
	else
		sprintf(m_szCaption, "No Waypoint");

	return m_szCaption;
}

void CWaypointYawMenuItem::Activate(CClient *pClient)
{
	CWaypoint *pWpt = CWaypoints::GetWaypoint(pClient->CurrentWaypoint());

	if (pWpt)
		pWpt->SetAim(CBotGlobals::PlayerAngles(pClient->GetPlayer()).y);
}

void CWaypointAreaIncrease::Activate(CClient *pClient)
{
	int iWpt = pClient->CurrentWaypoint();
	CWaypoint *pWpt = CWaypoints::GetWaypoint(iWpt);

	if (pWpt)
	{
		pWpt->SetArea(pWpt->GetArea() + 1);
	}
}

void CWaypointAreaDecrease::Activate(CClient *pClient)
{
	int iWpt = pClient->CurrentWaypoint();
	CWaypoint *pWpt = CWaypoints::GetWaypoint(iWpt);

	if (pWpt)
	{
		pWpt->SetArea(pWpt->GetArea() - 1);
	}
}

void CWaypointRadiusIncrease::Activate(CClient *pClient)
{
	int iWpt = pClient->CurrentWaypoint();
	CWaypoint *pWpt = CWaypoints::GetWaypoint(iWpt);

	if (pWpt)
	{
		float fRadius = pWpt->GetRadius();

		if (fRadius < 200.0f)
			pWpt->SetRadius(fRadius + 32.0f);
		else
			pWpt->SetRadius(200.0f);
	}
}

void CWaypointRadiusDecrease::Activate(CClient *pClient)
{
	int iWpt = pClient->CurrentWaypoint();
	CWaypoint *pWpt = CWaypoints::GetWaypoint(iWpt);

	if (pWpt)
	{
		float fRadius = pWpt->GetRadius();

		if (fRadius > 32.0f)
			pWpt->SetRadius(fRadius - 32.0f);
		else
			pWpt->SetRadius(0.0f);
	}
}


const char *CWaypointCutMenuItem::GetCaption(CClient *pClient, WptColor &color)
{
	sprintf(m_szCaption, "Cut Waypoint");
	color = WptColor::white;

	return m_szCaption;
}

void CWaypointCutMenuItem::Activate(CClient *pClient)
{
	pClient->UpdateCurrentWaypoint();

	CWaypoint *pwpt = CWaypoints::GetWaypoint(pClient->CurrentWaypoint());

	if (pwpt)
	{
		pClient->SetWaypointCut(pwpt);
		CWaypoints::DeleteWaypoint(pClient->CurrentWaypoint());
	}
}

const char *CWaypointCopyMenuItem::GetCaption(CClient *pClient, WptColor &color)
{
	sprintf(m_szCaption, "Copy Waypoint");
	color = WptColor::white;

	return m_szCaption;
}

void CWaypointCopyMenuItem::Activate(CClient *pClient)
{
	pClient->UpdateCurrentWaypoint();

	CWaypoint *pwpt = CWaypoints::GetWaypoint(pClient->CurrentWaypoint());

	if (pwpt)
	{
		pClient->SetWaypointCopy(pwpt);
	}
}

const char *CWaypointPasteMenuItem::GetCaption(CClient *pClient, WptColor &color)
{
	sprintf(m_szCaption, "Paste Waypoint");
	color = WptColor::white;

	return m_szCaption;
}

void CWaypointPasteMenuItem::Activate(CClient *pClient)
{
	CWaypoints::AddWaypoint(pClient, NULL, NULL, NULL, NULL, true);
}

void CBotMenu::Render(CClient *pClient)
{
	CBotMenuItem *item;
	WptColor color;
	unsigned int i;
	Vector vOrigin;
	Vector vForward;
	Vector vRight;
	QAngle angles = CBotGlobals::PlayerAngles(pClient->GetPlayer());
	const char *pszCaption;
	IPlayerInfo *pPlayerInfo = playerinfomanager->GetPlayerInfo(pClient->GetPlayer());
	extern ConVar bot_menu_update_time1;
	float fUpdateTime = bot_menu_update_time1.GetFloat();

	vOrigin = pPlayerInfo->GetAbsOrigin();

	AngleVectors(angles, &vForward);

	vForward = vForward / vForward.Length();

	vRight = vForward.Cross(Vector(0, 0, 1));

	vOrigin = vOrigin + (vForward * 100) - (vRight * 100);
	vOrigin.z += 72.0f;

	pszCaption = GetCaption(pClient, color);

	debugoverlay->AddTextOverlayRGB(vOrigin, 0, fUpdateTime, color.r, color.g, color.b, color.a, pszCaption);
	debugoverlay->AddTextOverlayRGB(vOrigin, 1, fUpdateTime, color.r, color.g, color.b, color.a, "----------------");
	/*
		Vector screen;
		Vector point = Vector(0,0,0);

		debugoverlay->ScreenPosition(0.5f, 0.5f, screen);
		debugoverlay->ScreenPosition(point,screen);*/

	for (i = 0; i < m_MenuItems.size(); i++)
	{
		item = m_MenuItems[i];

		pszCaption = item->GetCaption(pClient, color);

		debugoverlay->AddTextOverlayRGB(vOrigin, i + 2, fUpdateTime, color.r, color.g, color.b, color.a, "%d. %s", (i == 9) ? (0) : (i + 1), pszCaption);
	}
}

const char *CBotMenuItem::GetCaption(CClient *pClient, WptColor &color)
{
	color.r = 255;
	color.g = 255;
	color.b = 255;
	color.a = 255;

	return m_szCaption;
}

void CBotMenuList::Render(CClient *pClient) // render
{
	CBotMenu *pMenu = pClient->GetCurrentMenu();

	pMenu->Render(pClient);
	//m_MenuList[iMenu]->render(pClient);
}

void CBotMenuList::SelectedMenu(CClient *pClient, unsigned int iMenu)
{
	CBotMenu *pMenu = pClient->GetCurrentMenu();

	pMenu->SelectedMenu(pClient, iMenu);
}

void CBotMenu::Activate(CClient *pClient)
{
	pClient->SetCurrentMenu(this);
}

void CBotMenu::SelectedMenu(CClient *pClient, unsigned int iMenu)
{
	if (iMenu < m_MenuItems.size())
		m_MenuItems[iMenu]->Activate(pClient);
}

CWaypointFlagShowMenu::CWaypointFlagShowMenu(CBotMenu *pParent)
{
	int iMod = CBotGlobals::GetCurrentMod()->GetModId();
	// check the number of waypoint types available
	// caption
	// 1.
	// 2.
	// 3.
	// 4.
	// 5.
	// 6.
	// 7.
	// 8. More...
	// 9. Go Back

	int iNumTypes = CWaypointTypes::GetNumTypes();
	int iNumAdded = 0;
	CBotMenu *pCurrent;

	int i;

	pCurrent = this;

	for (i = 0; i < iNumTypes; i++)
	{
		if (!CWaypointTypes::GetTypeByIndex(i)->ForMod(iMod))
			continue;

		pCurrent->AddMenuItem(new CWaypointFlagShowMenuItem(i));
		iNumAdded++;

		if ((iNumAdded > 7) || (i == (iNumTypes - 1)))
		{
			CBotMenuItem *back = new CBotGotoMenuItem("Back...", pParent);
			//	make a new menu
			pParent = pCurrent;

			if ((iNumAdded > 7) && (i < (iNumTypes - 1)))
			{
				pCurrent = new CBotMenu();
				pCurrent->SetCaption("Show Waypoint Flags (More)");
				pParent->AddMenuItem(new CBotGotoMenuItem("More...", pCurrent));

			}

			pParent->AddMenuItem(back);

			iNumAdded = 0; // reset

		}
	}
}

const char *CWaypointFlagShowMenu::GetCaption(CClient *pClient, WptColor &color)
{
	if (pClient->IsShowingAllWaypoints())
	{
		sprintf(m_szCaption, "Showing All Waypoints (change)");
		color = WptColor::white;
	}
	else
	{
		sprintf(m_szCaption, "Showing Only Some Waypoints (change)");
		color = CWaypointTypes::GetColour(pClient->GetShowWaypointFlags());
	}

	return m_szCaption;
}


const char *CWaypointFlagShowMenuItem::GetCaption(CClient *pClient, WptColor &color)
{
	CWaypointType *type = CWaypointTypes::GetTypeByIndex(m_iFlag);

	color = type->GetColour();

	sprintf(m_szCaption, "[%s] %s", (pClient->IsShowingAllWaypoints() || pClient->IsShowingWaypoint(type->GetBits())) ? "showing" : "hiding", type->GetName());

	return m_szCaption;
}

void CWaypointFlagShowMenuItem::Activate(CClient *pClient)
{
	CWaypointType *type = CWaypointTypes::GetTypeByIndex(m_iFlag);

	// toggle
	if (pClient->IsShowingWaypoint(type->GetBits()))
		pClient->DontShowWaypoints(type->GetBits());
	else
		pClient->ShowWaypoints(type->GetBits());
}

void CBotMenuItem::FreeMemory()
{
	// do nothing
}

void CBotMenu::FreeMemory()
{
	for (unsigned int i = 0; i < m_MenuItems.size(); i++)
	{
		CBotMenuItem *temp = m_MenuItems[i];

		temp->FreeMemory();

		delete temp;
	}
}

void CBotMenuList::FreeMemory()
{
	for (unsigned int i = 0; i < BOT_MENU_MAX; i++)
	{
		CBotMenu *temp = m_MenuList[i];

		temp->FreeMemory();

		delete temp;
	}
}

const char *CPathWaypointDeleteToMenuItem::GetCaption(CClient *pClient, WptColor &color)
{
	int iWpt = pClient->CurrentWaypoint();

	color = WptColor::white;

	if (iWpt == -1)
	{
		strcpy(m_szCaption, "No Waypoint");
		return m_szCaption;
	}

	CWaypoint *pWaypoint = CWaypoints::GetWaypoint(iWpt);

	sprintf(m_szCaption, "Delete Paths To This Waypoint (%d)", pWaypoint->NumPathsToThisWaypoint());

	return m_szCaption;
}

void CPathWaypointDeleteToMenuItem::Activate(CClient *pClient)
{
	if (pClient->CurrentWaypoint() != -1)
		CWaypoints::DeletePathsTo(pClient->CurrentWaypoint());
}


const char *CPathWaypointDeleteFromMenuItem::GetCaption(CClient *pClient, WptColor &color)
{
	int iWpt = pClient->CurrentWaypoint();

	color = WptColor::white;

	if (iWpt == -1)
	{
		strcpy(m_szCaption, "No Waypoint");
		return m_szCaption;
	}

	CWaypoint *pWaypoint = CWaypoints::GetWaypoint(iWpt);

	sprintf(m_szCaption, "Delete Paths From This Waypoint (%d)", pWaypoint->NumPaths());

	return m_szCaption;
}

void CPathWaypointDeleteFromMenuItem::Activate(CClient *pClient)
{
	if (pClient->CurrentWaypoint() != -1)
		CWaypoints::DeletePathsFrom(pClient->CurrentWaypoint());
}

