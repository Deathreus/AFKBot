/**
* =============================================================================
* CEntity Entity Handling Framework
* Copyright (C) 2011 Matt Woodrow.  All rights reserved.
* =============================================================================
*
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License, version 3.0, as published by the
* Free Software Foundation.
* 
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _INCLUDE_IENTITYFACTORY_H_
#define _INCLUDE_IENTITYFACTORY_H_

#include "CEntityManager.h"

#define LINK_ENTITY_TO_CLASS(mapClassName,DLLClassName) \
	static CEntityFactory<DLLClassName> mapClassName##Factory(#mapClassName);

#define LINK_ENTITY_TO_INTERNAL_CLASS(mapClassName,DLLClassName) \
	static CEntityFactory<DLLClassName> mapClassName##Factory(#mapClassName,true);

#define LINK_ENTITY_TO_CUSTOM_CLASS(mapClassName,replaceClassName,DLLClassName); \
	static CCustomEntityFactory<DLLClassName> mapClassName##CustomFactory(#mapClassName, #replaceClassName); \
	static CEntityFactory<DLLClassName> mapClassName##Factory(#mapClassName);

class CEntity;

class IEntityFactory
{
public:
	virtual CEntity *Create(edict_t *pEdict, CBaseEntity *pEnt) = 0;
};

class IEntityFactoryDictionary
{
public:
	virtual void InstallFactory(IEntityFactory *pFactory, const char *pClassName) = 0;
	virtual IServerNetworkable *Create(const char *pClassName) = 0;
	virtual void Destroy(const char *pClassName, IServerNetworkable *pNetworkable) = 0;
	virtual IEntityFactory *FindFactory(const char *pClassName) = 0;
	virtual const char *GetCannonicalName(const char *pClassName) = 0;
};

template <class T>
class CEntityFactory : public IEntityFactory
{
public:
	CEntityFactory(const char *pClassName, bool internalClass = false)
	{
		GetEntityManager()->LinkEntityToClass(this, pClassName, internalClass);
	}

	CEntity *Create(edict_t *pEdict, CBaseEntity *pEnt)
	{
		if (!pEdict || !pEnt)
		{
			return NULL;
		}

		T* pOurEnt = new T();
		pOurEnt->Init(pEdict, pEnt);

		return pOurEnt;
	}
};

abstract_class IEntityFactoryReal
{
public:
	IEntityFactoryReal()
	{
		m_Next = m_Head;
		m_Head = this;
	}
	virtual IServerNetworkable *Create( const char *pClassName ) = 0;
	virtual void Destroy( IServerNetworkable *pNetworkable ) = 0;
	virtual size_t GetEntitySize() = 0;
	virtual void AddToList() =0;

	static IEntityFactoryReal *m_Head;
	IEntityFactoryReal *m_Next;
};

template <class T>
class CCustomEntityFactory : public IEntityFactoryReal
{
public:
	CCustomEntityFactory(const char *pClassName, const char *pReplaceName)
	{
		this->pReplaceName = pReplaceName;
		this->pClassName = pClassName;
	}

	void AddToList()
	{
		assert(EntityFactoryDictionary);
		EntityFactoryDictionary()->InstallFactory((IEntityFactory *)this, pClassName );
	}

	IServerNetworkable *Create( const char *pClassName )
	{
		IEntityFactoryReal *pFactory = (IEntityFactoryReal *)EntityFactoryDictionary()->FindFactory(pReplaceName);
		assert(pFactory);

		return pFactory->Create(pReplaceName);
	}

	void Destroy( IServerNetworkable *pNetworkable )
	{
		IEntityFactoryReal *pFactory = (IEntityFactoryReal *)EntityFactoryDictionary()->FindFactory(pReplaceName);
		assert(pFactory);
		return pFactory->Destroy(pNetworkable);
	}

	virtual size_t GetEntitySize()
	{
		IEntityFactoryReal *pFactory = (IEntityFactoryReal *)EntityFactoryDictionary()->FindFactory(pReplaceName);
		assert(pFactory);
		return pFactory->GetEntitySize();
	}

	const char *pReplaceName;
	const char *pClassName;
};

#endif // _INCLUDE_IENTITYFACTORY_H_
