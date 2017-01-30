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

// Writen by Brett "Brutal" Powell for the TF2RPG project.

#include "CHelpers.h"
#include <IEngineTrace.h>
#include <worldsize.h>

CHelpers *pHelpers;

#define HUD_PRINTNOTIFY		1
#define HUD_PRINTCONSOLE	2
#define HUD_PRINTTALK		3
#define HUD_PRINTCENTER		4

extern INetworkStringTableContainer *netstringtables;
extern IPlayerInfoManager *playerinfomanager;
extern IEngineSound *engsound;
extern IEngineTrace *engtrace;

CPlayer* CHelpers::UTIL_PlayerByIndex(int playerIndex)
{
	CPlayer *pPlayer = NULL;

	if (playerIndex > 0 && playerIndex <= gpGlobals->maxClients)
	{
		edict_t *pPlayerEdict = engine->PEntityOfEntIndex(playerIndex);
		if (pPlayerEdict && !pPlayerEdict->IsFree())
		{
			CEntity *pEntity = CEntity::Instance(playerIndex);
			if (pEntity)
			{
				pPlayer = dynamic_cast<CPlayer *>(pEntity);
				assert(pPlayer);
			}
		}
	}

	return pPlayer;
}

bool CHelpers::AddFileToDownloadTable(const char *path)
{
	int table = pHelpers->FindStringTable("downloadables");

	if (table == -1)
	{
		g_pSM->LogError(myself, "Downloads Table was not found!");
		return false;
	}
	else
	{
		bool save = pHelpers->LockStringTables(false);
		pHelpers->AddToStringTable(table, path);
		pHelpers->LockStringTables(save);
		return true;
	}

	return false;
}

#if 0
void CHelpers::AddFolderToDownloadTable(const char *path)
{
	char newpath[256];
	g_pSM->BuildPath(Path_Game, newpath, sizeof(newpath)-1, "%s/*.*", path);

	HANDLE hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA FindFileData;

	hFind = FindFirstFileA(newpath, &FindFileData);
	if(hFind == INVALID_HANDLE_VALUE)
	{
		g_pSM->LogError(myself, "FindFirstFile Failed (%d)\n", GetLastError());
		return;
	}

	do
	{
		if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(strcmp(FindFileData.cFileName, ".") == 0 || strcmp(FindFileData.cFileName, "..") == 0)
			{
				continue;
			}

			//Recursively search this subdirectory for files as well
			char dirpath[256];
			snprintf(dirpath, sizeof(dirpath)-1, "%s/%s", path, FindFileData.cFileName);
			pHelpers->AddFolderToDownloadTable(dirpath);
		}
		else
		{
			//Prepend the file with the local path and Add it to the Downloads Table
			char filepath[256];
			snprintf(filepath, sizeof(filepath)-1, "%s/%s", path, FindFileData.cFileName);
			pHelpers->AddFileToDownloadTable(filepath);
		}
	}
	while(FindNextFile(hFind, &FindFileData) != NULL);

	FindClose(hFind);
}
#endif

//==========================================================================================
// Purpose: StringTable Functions
//==========================================================================================
int CHelpers::FindStringTable(const char *table)
{
	INetworkStringTable *pTable = netstringtables->FindTable(table);

	if (!pTable)
	{
		return INVALID_STRING_TABLE;
	}

	return pTable->GetTableId();
}

bool CHelpers::AddToStringTable(int tableidx, const char *str)
{
	TABLEID idx = static_cast<TABLEID>(tableidx);
	INetworkStringTable *pTable = netstringtables->GetTable(idx);

	if (!pTable)
	{
		g_pSM->LogError(myself, "Invalid string table index %d", idx);
		return false;
	}

	pTable->AddString(true, str);

	return true;
}

bool CHelpers::LockStringTables(bool lock)
{
	return engine->LockNetworkStringTables(lock) ? 1 : 0;
}

//==========================================================================================
// Purpose: Sound Related Functions
//==========================================================================================
void CHelpers::EmitAmbientSound(const char *szPath, const Vector pos, int entity, int level, int flags,
	float volume, int pitch, float delay)
{
	engine->EmitAmbientSound(entity, pos, szPath, volume, (soundlevel_t)level, SND_NOFLAGS, pitch, delay);
}

void CHelpers::EmitSoundToClient(CPlayer *pPlayer,
	const char *szPath,
	int entity,
	int channel,
	int level,
	int flags,
	float volume,
	int pitch,
	int speakerentity,
	const Vector *vecOrigin,
	const Vector *vecDirection,
	bool updatePos,
	float soundtime,
	int specialdsp)
{
	CRecipientFilter filter;
	filter.AddRecipient(pPlayer);

	if (entity == SOUND_FROM_LOCAL_PLAYER)
	{
		int client = pPlayer->entindex();
		engsound->EmitSound(filter, client, channel, szPath, volume, (soundlevel_t)level, flags, pitch, specialdsp, vecOrigin, vecDirection, NULL, updatePos, soundtime, speakerentity);
	}
	else
	{
		engsound->EmitSound(filter, entity, channel, szPath, volume, (soundlevel_t)level, flags, pitch, specialdsp, vecOrigin, vecDirection, NULL, updatePos, soundtime, speakerentity);
	}
}

void CHelpers::EmitSoundToAll(const char *szPath,
	int entity,
	int channel,
	int level,
	int flags,
	float volume,
	int pitch,
	int speakerentity,
	const Vector *vecOrigin,
	const Vector *vecDirection,
	bool updatePos,
	float soundtime,
	int specialdsp)
{
	CRecipientFilter filter;
	filter.AddAllPlayers();

	engsound->EmitSound(filter, entity, channel, szPath, volume, (soundlevel_t)level, flags, pitch, specialdsp, vecOrigin, vecDirection, NULL, updatePos, soundtime, speakerentity);
}

//==========================================================================================
// Purpose: UserMessage Functions
//==========================================================================================
void CHelpers::PrintCenterText(CPlayer *pPlayer, const char *szText, ...)
{
	if (!pPlayer)
		return;

	static int msgindex = NULL;
	cell_t pClient[] = { pPlayer->entindex() };
	if (!msgindex) msgindex = g_SMAPI->FindUserMessage("TextMsg");

	char buffer;
	g_SMAPI->Format(&buffer, 256, szText);

	bf_write *pBuffer = usermsgs->StartBitBufMessage(msgindex, pClient, 1, USERMSG_RELIABLE);;
	pBuffer->WriteByte(HUD_PRINTCENTER);
	pBuffer->WriteString(&buffer);
	usermsgs->EndMessage();
}

void CHelpers::PrintCenterTextAll(const char *szText, ...)
{
	static int msgindex = NULL;
	if (!msgindex) msgindex = g_SMAPI->FindUserMessage("TextMsg");

	cell_t *pClients;
	for (short int i = 1; i < MAX_PLAYERS; i++)
	{
		IPlayerInfo *pPL = playerinfomanager->GetPlayerInfo(engine->PEntityOfEntIndex(i));
		CPlayer *pPlayer = UTIL_PlayerByIndex(i);

		if (pPlayer != NULL && pPL != NULL && pPL->IsPlayer() && pPL->IsConnected())
			pClients[i] = pPlayer->entindex();
	}

	char buffer;
	g_SMAPI->Format(&buffer, 256, szText);

	bf_write *pBuffer = usermsgs->StartBitBufMessage(msgindex, pClients, sizeof(pClients), USERMSG_RELIABLE);
	pBuffer->WriteByte(HUD_PRINTCENTER);
	pBuffer->WriteString(&buffer);
	usermsgs->EndMessage();
}

void CHelpers::PrintHudText(CPlayer *pPlayer, const hudtextparms_t &textparms, const char *szText)
{
	if (!pPlayer)
		return;
	
	static int msgindex = NULL;
	cell_t pClient[] = { pPlayer->entindex() };
	if (!msgindex) msgindex = g_SMAPI->FindUserMessage("HudMsg");

	bf_write *pBuffer = usermsgs->StartBitBufMessage(msgindex, pClient, 1, USERMSG_RELIABLE);
	pBuffer->WriteByte(textparms.channel & 0xFF);
	pBuffer->WriteFloat(textparms.x);
	pBuffer->WriteFloat(textparms.y);
	pBuffer->WriteByte(textparms.r1);
	pBuffer->WriteByte(textparms.g1);
	pBuffer->WriteByte(textparms.b1);
	pBuffer->WriteByte(textparms.a1);
	pBuffer->WriteByte(textparms.r2);
	pBuffer->WriteByte(textparms.g2);
	pBuffer->WriteByte(textparms.b2);
	pBuffer->WriteByte(textparms.a2);
	pBuffer->WriteByte(textparms.effect);
	pBuffer->WriteFloat(textparms.fadeinTime);
	pBuffer->WriteFloat(textparms.fadeoutTime);
	pBuffer->WriteFloat(textparms.holdTime);
	pBuffer->WriteFloat(textparms.fxTime);
	pBuffer->WriteString(szText);
	usermsgs->EndMessage();
}

void CHelpers::PrintHudTextAll(const hudtextparms_t &textparms, const char *szText)
{
	static int msgindex = NULL;
	if (!msgindex) msgindex = g_SMAPI->FindUserMessage("HudMsg");

	cell_t *pClients;
	for (short int i = 1; i < MAX_PLAYERS; i++)
	{
		IPlayerInfo *pPL = playerinfomanager->GetPlayerInfo(engine->PEntityOfEntIndex(i));
		CPlayer *pPlayer = UTIL_PlayerByIndex(i);

		if (pPlayer != NULL && pPL != NULL && pPL->IsPlayer() && pPL->IsConnected())
			pClients[i] = pPlayer->entindex();
	}

	bf_write *pBuffer = usermsgs->StartBitBufMessage(msgindex, pClients, sizeof(pClients), USERMSG_RELIABLE);
	pBuffer->WriteByte(textparms.channel & 0xFF);
	pBuffer->WriteFloat(textparms.x);
	pBuffer->WriteFloat(textparms.y);
	pBuffer->WriteByte(textparms.r1);
	pBuffer->WriteByte(textparms.g1);
	pBuffer->WriteByte(textparms.b1);
	pBuffer->WriteByte(textparms.a1);
	pBuffer->WriteByte(textparms.r2);
	pBuffer->WriteByte(textparms.g2);
	pBuffer->WriteByte(textparms.b2);
	pBuffer->WriteByte(textparms.a2);
	pBuffer->WriteByte(textparms.effect);
	pBuffer->WriteFloat(textparms.fadeinTime);
	pBuffer->WriteFloat(textparms.fadeoutTime);
	pBuffer->WriteFloat(textparms.holdTime);
	pBuffer->WriteFloat(textparms.fxTime);
	pBuffer->WriteString(szText);
	usermsgs->EndMessage();
}

void CHelpers::PrintToChat(CPlayer *pPlayer, CPlayer *pAuthor, const char *szText, ...)
{
	if (!pPlayer)
		return;

	static int msgindex = NULL;
	cell_t pClient[] = { pPlayer->entindex() };
	if (!msgindex) msgindex = g_SMAPI->FindUserMessage("SayText2");

	char buffer;
	g_SMAPI->Format(&buffer, 256, szText);

	bf_write *pBuffer = usermsgs->StartBitBufMessage(msgindex, pClient, 1, USERMSG_RELIABLE);
	pBuffer->WriteByte(pAuthor->entindex()); //author of message
	pBuffer->WriteString(&buffer);	//message content
	pBuffer->WriteByte(true);	//chat boolean
	usermsgs->EndMessage();
}

void CHelpers::PrintToChatAll(CPlayer *pPlayer, const char *szText, ...)
{
	static int msgindex = NULL;
	if (!msgindex) msgindex = g_SMAPI->FindUserMessage("SayText2");

	cell_t *pClients;
	for (short int i = 1; i < MAX_PLAYERS; i++)
	{
		IPlayerInfo *pPL = playerinfomanager->GetPlayerInfo(engine->PEntityOfEntIndex(i));
		CPlayer *pPlayer = UTIL_PlayerByIndex(i);

		if (pPlayer != NULL && pPL != NULL && pPL->IsPlayer() && pPL->IsConnected())
			pClients[i] = pPlayer->entindex();
	}

	char buffer;
	g_SMAPI->Format(&buffer, 256, szText);

	bf_write *pBuffer = usermsgs->StartBitBufMessage(msgindex, pClients, sizeof(pClients), USERMSG_RELIABLE);
	pBuffer->WriteByte(pPlayer->entindex()); //author of message
	pBuffer->WriteString(&buffer);	//message content
	pBuffer->WriteByte(true);	//chat boolean
	usermsgs->EndMessage();
}


//==========================================================================================
// Purpose: TraceRay Functions
//==========================================================================================
/** Custom TraceFilter to skip a specific Player **/
class CTraceFilterSkipPlayer : public CTraceFilter
{
public:
	CTraceFilterSkipPlayer(CBaseEntity *pPlayerIgnore){ m_pPlayerIgnore = pPlayerIgnore; };

	bool ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask)
	{
		//Get the entity were checking for permission to hit
		IServerUnknown *pUnk = (IServerUnknown*)pServerEntity;
		CBaseEntity *pEntity = pUnk->GetBaseEntity();

		if (!pEntity || !m_pPlayerIgnore)
			return true;

		if (pEntity == m_pPlayerIgnore)
			return false;

		return true;
	}
	virtual TraceType_t	GetTraceType() const
	{
		return TRACE_EVERYTHING;
	}

private:
	CBaseEntity *m_pPlayerIgnore;
};

tr_contents *CHelpers::TR_TraceRayFilter(const Vector origin, const QAngle angles, int flags, RayType rtype, CPlayer *pPlayer)
{
	CBaseEntity *pPlayerIgnore = pPlayer->BaseEntity();

	Vector vecStartPos, vecEndPos;
	CTraceFilterSkipPlayer filter(pPlayerIgnore);
	Ray_t ray;

	vecStartPos.Init(origin.x, origin.y, origin.z);

	switch (rtype)
	{
		case RayType_EndPoint:
		{
			//The angles param is our endpoint if we use this RayType
			vecEndPos.Init(angles.x, angles.y, angles.z);
			break;
		}
		case RayType_Infinite:
		{
			QAngle DirAngles;
			DirAngles.Init(angles.x, angles.y, angles.z);
			AngleVectors(DirAngles, &vecEndPos);

			/* Make it unitary and get the ending point */
			vecEndPos.NormalizeInPlace();
			vecEndPos = vecStartPos + vecEndPos * MAX_TRACE_LENGTH;
			break;
		}
	}

	trace_t trace;
	ray.Init(vecStartPos, vecEndPos);
	engtrace->TraceRay(ray, flags, &filter, &trace);

	if (trace.DidHit()) //did the trace collide with anything?
	{
		trcontents_t *contents = new trcontents_t;
		contents->base = trace;
		contents->entity = trace.m_pEnt;
		contents->endpos = trace.endpos;
		contents->fraction = trace.fraction;

		return contents;
	}

	return NULL;
}
