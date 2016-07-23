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
#include "bot_buttons.h"
#include "in_buttons.h"

void CBotButtons::Attack(float fFor, float fFrom)
{
	HoldButton(IN_ATTACK, fFrom, fFor, 0.1);
}

void CBotButtons::Jump(float fFor, float fFrom)
{
	HoldButton(IN_JUMP, fFrom, fFor, 0.25);
}

void CBotButtons::Duck(float fFor, float fFrom)
{
	HoldButton(IN_DUCK, fFrom, fFor);
}

void CBotButton::Hold(float fFrom, float fFor, float fLetGoTime)
{
	fFrom += engine->Time();
	m_fTimeStart = fFrom;
	m_fTimeEnd = fFrom + fFor;
	m_fLetGoTime = m_fTimeEnd + fLetGoTime;
}

CBotButtons::CBotButtons()
{
	Add(new CBotButton(IN_ATTACK));
	Add(new CBotButton(IN_ATTACK2));
	Add(new CBotButton(IN_DUCK));
	Add(new CBotButton(IN_JUMP));
	Add(new CBotButton(IN_RELOAD));
	Add(new CBotButton(IN_SPEED)); // for sprint
	Add(new CBotButton(IN_FORWARD)); // for ladders
	Add(new CBotButton(IN_USE)); // for chargers
	Add(new CBotButton(IN_ALT1)); // for proning
	Add(new CBotButton(IN_RUN)); // ????

	m_bLetGoAll = false;
}

void CBotButtons::HoldButton(int iButtonId, float fFrom, float fFor, float fLetGoTime)
{
	for (unsigned int i = 0; i < m_theButtons.size(); i++)
	{
		if (m_theButtons[i]->GetID() == iButtonId)
		{
			m_theButtons[i]->Hold(fFrom, fFor, fLetGoTime);
			return;
		}
	}
}

void CBotButtons::LetGo(int iButtonId)
{
	for (unsigned int i = 0; i < m_theButtons.size(); i++)
	{
		if (m_theButtons[i]->GetID() == iButtonId)
		{
			m_theButtons[i]->LetGo();
			return;
		}
	}
}

int CBotButtons::GetBitMask()
{
	if (m_bLetGoAll)
		return 0;
	else
	{

		int iBitMask = 0;

		float fTime = engine->Time();

		for (unsigned int i = 0; i < m_theButtons.size(); i++)
		{
			if (m_theButtons[i]->Held(fTime))
			{
				m_theButtons[i]->UnTap();
				iBitMask |= m_theButtons[i]->GetID();
			}
		}

		return iBitMask;

	}
}

bool CBotButtons::CanPressButton(int iButtonId)
{
	for (unsigned int i = 0; i < m_theButtons.size(); i++)
	{
		if (m_theButtons[i]->GetID() == iButtonId)
			return m_theButtons[i]->CanPress(engine->Time());
	}
	return false;
}

void CBotButtons::Add(CBotButton *theButton)
{
	m_theButtons.push_back(theButton);
}

bool CBotButtons::HoldingButton(int iButtonId)
{
	for (unsigned int i = 0; i < m_theButtons.size(); i++)
	{
		if (m_theButtons[i]->GetID() == iButtonId)
			return m_theButtons[i]->Held(engine->Time());
	}

	return false;
}

void CBotButtons::Tap(int iButtonId)
{
	for (unsigned int i = 0; i < m_theButtons.size(); i++)
	{
		if (m_theButtons[i]->GetID() == iButtonId)
		{
			m_theButtons[i]->Tap();

			return;
		}
	}
}
