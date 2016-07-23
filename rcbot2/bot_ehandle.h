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
#ifndef __RCBOT_EHANDLE_H__
#define __RCBOT_EHANDLE_H__

////// entity handling in network
class MyEHandle
{
public:
	MyEHandle()
	{
		m_pEnt = NULL;
		m_iSerialNumber = 0;
	}

	MyEHandle(edict_t *pent)
	{
		m_pEnt = pent;

		if (pent)
		{
			m_iSerialNumber = pent->m_NetworkSerialNumber;
		}
		else
			m_iSerialNumber = 0;
	}

	inline bool NotValid() { return Get() == NULL; }
	inline bool IsValid() { return Get() != NULL; }

	inline edict_t *Get()
	{
		if (m_iSerialNumber && m_pEnt)
		{
			if (!m_pEnt->IsFree() && (m_iSerialNumber == m_pEnt->m_NetworkSerialNumber))
				return m_pEnt;
		}
		else if (m_pEnt)
			m_pEnt = NULL;

		return NULL;
	}

	inline edict_t *Get_Old()
	{
		return m_pEnt;
	}

	inline operator edict_t * const ()
	{ // same as get function (inlined for speed)
		if (m_iSerialNumber && m_pEnt)
		{
			if (!m_pEnt->IsFree() && (m_iSerialNumber == m_pEnt->m_NetworkSerialNumber))
				return m_pEnt;
		}
		else if (m_pEnt)
			m_pEnt = NULL;

		return NULL;
	}

	inline bool operator == (int a)
	{
		return ((int)Get() == a);
	}

	inline bool operator == (edict_t *pent)
	{
		return (Get() == pent);
	}

	inline bool operator == (MyEHandle &other)
	{
		return (Get() == other.Get());
	}

	inline edict_t * operator = (edict_t *pent)
	{
		m_pEnt = pent;

		if (pent)
		{
			m_iSerialNumber = pent->m_NetworkSerialNumber;
		}
		else
			m_iSerialNumber = 0;

		return m_pEnt;
	}
private:
	int m_iSerialNumber;
	edict_t *m_pEnt;
};

#endif