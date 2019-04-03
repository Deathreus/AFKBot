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

/**
  * Entities can and will be destroyed at any point,
  * this class dynamically verifies that any entity
  * actually still exists and is the same before we procede 
  * to modify or use the pointer
  */
class MyEHandle
{
public:
	MyEHandle()
	{
		m_pEnt = NULL;
		m_iSerialNumber = 0;
	}

	MyEHandle(edict_t *pEnt)
	{
		m_pEnt = pEnt;
		m_iSerialNumber = m_pEnt ? m_pEnt->m_NetworkSerialNumber : 0;
	}

	// Will invalidate itself if required during the call
	inline const bool IsValid() const { return Get() != NULL; }

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

	inline edict_t *const Get() const
	{
		if(m_iSerialNumber && m_pEnt)
		{
			if(!m_pEnt->IsFree() && (m_iSerialNumber == m_pEnt->m_NetworkSerialNumber))
				return m_pEnt;
		}

		return NULL;
	}

	inline edict_t *Get_Old() const
	{
		return m_pEnt;
	}

	// Same as Get() (inlined for speed)
	inline operator edict_t *()
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

	// Entity access
	inline edict_t *const operator->() const
	{
		return m_pEnt;
	}

	inline bool operator!() const
	{
		return !IsValid();
	}

	inline bool operator==(MyEHandle &other) const
	{
		return (m_pEnt == other.Get());
	}
	inline bool operator!=(MyEHandle &other) const
	{
		return (m_pEnt != other.Get());
	}

	// edict_t assigning
	inline MyEHandle& operator=(edict_t *pEnt)
	{
		m_pEnt = pEnt;

		if (m_pEnt)
			m_iSerialNumber = m_pEnt->m_NetworkSerialNumber;
		else
			m_iSerialNumber = 0;

		return *this;
	}

	// CBaseEntity assigning
	inline MyEHandle& operator=(CBaseEntity *pEnt)
	{
		extern IServerGameEnts *gameents;
		m_pEnt = gameents->BaseEntityToEdict(pEnt);

		if(m_pEnt)
			m_iSerialNumber = m_pEnt->m_NetworkSerialNumber;
		else
			m_iSerialNumber = 0;

		return *this;
	}

	// EHandle assigning
	inline MyEHandle& operator=(MyEHandle &other)
	{
		m_pEnt = other.Get();

		if (m_pEnt)
			m_iSerialNumber = m_pEnt->m_NetworkSerialNumber;
		else
			m_iSerialNumber = 0;

		return *this;
	}

private:
	short m_iSerialNumber;
	edict_t *m_pEnt;
};

#endif