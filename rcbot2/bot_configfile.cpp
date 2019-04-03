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
 *///=================================================================================//

#include "bot_base.h"
#include "bot_globals.h"
#include "bot_strings.h"
#include "bot_weapons.h"
#include "bot_configfile.h"

#include <convar.h>

#include <memory>

bot_util_t CRCBotTF2UtilFile::m_fUtils[UTIL_TYPE_MAX][BOT_UTIL_MAX][9];

void CBotConfigFile::Load()
{
	char filePath[PLATFORM_MAX_PATH];
	smutils->BuildPath(Path_SM, filePath, sizeof(filePath), "data\\afkbot\\config\\config.cfg");

	char error[128]; auto config = std::make_unique<CBotConfigSMC>();
	if (textparsers->ParseSMCFile(filePath, config.get(), nullptr, error, sizeof(error)) != SMCError_Okay)
	{
		smutils->LogError(myself, "Error reading '%s': %s", filePath, error);
	}
}

SMCResult CBotConfigSMC::ReadSMC_NewSection(const SMCStates *states, const char *name)
{
	if (!FStrEq(name, "botcfg"))
	{
		smutils->LogError(myself, "Config Error: Expected to find section 'botcfg', found '%s'", name);
		return SMCResult_HaltFail;
	}

	return SMCResult_Continue;
}

SMCResult CBotConfigSMC::ReadSMC_KeyValue(const SMCStates *states, const char *key, const char *value)
{
	ConVarRef cvar(key);
	if (cvar.IsValid())
	{
		cvar.SetValue(value);
#if defined _DEBUG
		smutils->LogMessage(myself, "Setting %s to %s", key, value);
#endif
	}
	else
	{
		smutils->LogError(myself, "Config Error: '%s' is not a cvar.", key);
		return SMCResult_HaltFail;
	}

	return SMCResult_Continue;
}

void CRCBotTF2UtilFile::Init()
{
	short unsigned int i, j, k;

	for (i = 0; i < UTIL_TYPE_MAX; i++)
	{
		for (j = 0; j < BOT_UTIL_MAX; j++)
		{
			for (k = 0; k < 9; k++)
			{
				m_fUtils[i][j][k].min = 0;
				m_fUtils[i][j][k].max = 0;
			}
		}
	}
}

void CRCBotTF2UtilFile::AddUtilPerturbation(eBotAction iAction, eTF2UtilType iUtil, float fUtility[9][2])
{
	short unsigned int i;

	for (i = 0; i < 9; i++)
	{
		m_fUtils[iUtil][iAction][i].min = fUtility[i][0];
		m_fUtils[iUtil][iAction][i].max = fUtility[i][1];
	}
}

void CRCBotTF2UtilFile::LoadConfig()
{
	eTF2UtilType iFile;
	char szFullFilename[512];
	char szFilename[64];
	char line[256];
	FILE *fp;

	Init();

	for (iFile = BOT_ATT_UTIL; iFile < UTIL_TYPE_MAX; iFile = (eTF2UtilType)((int)iFile + 1))
	{
		if (iFile == BOT_ATT_UTIL)
		{
			sprintf(szFilename, "attack_util.csv");
		}
		else
		{
			sprintf(szFilename, "normal_util.csv");
		}

		smutils->BuildPath(Path_SM, szFullFilename, sizeof(szFullFilename), "data\\afkbot\\config\\%s", szFilename);
		fp = CBotGlobals::OpenFile(szFullFilename, "r");

		if (fp)
		{
			eBotAction iUtil = (eBotAction)0;

			while (fgets(line, 255, fp) != nullptr)
			{
				float iClassList[TF_CLASS_MAX][2];
				char utiltype[64];

				if (line[0] == 'B' && line[1] == 'O' &&
					line[2] == 'T' && line[3] == '_') // OK
				{

					// Format:    U, 1, 2, 3, 4, 5, 6, 7, 8, 9
					//                
					//               s  s  s  d  m  h  p  s  e
					//               c  n  o  e  e  w  y  p  n
					//               o  i  l  m  d  g  r  y  g
					// 

					if (sscanf(line, "%[^,],%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\r\n", utiltype,
						&(iClassList[0][0]), &(iClassList[0][1]),
						&(iClassList[1][0]), &(iClassList[1][1]),
						&(iClassList[2][0]), &(iClassList[2][1]),
						&(iClassList[3][0]), &(iClassList[3][1]),
						&(iClassList[4][0]), &(iClassList[4][1]),
						&(iClassList[5][0]), &(iClassList[5][1]),
						&(iClassList[6][0]), &(iClassList[6][1]),
						&(iClassList[7][0]), &(iClassList[7][1]),
						&(iClassList[8][0]), &(iClassList[8][1])))
					{

						AddUtilPerturbation(iUtil, iFile, iClassList);

						iUtil = (eBotAction)((int)iUtil + 1);

						if (iUtil >= BOT_UTIL_MAX)
							break;

					}

				}
			}

			fclose(fp);
		}
	}

}
