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

#include "datamap.h"
#include "util.h"
#include "CEntity.h"

const char *variant_t::ToString(void) const
{
	COMPILE_TIME_ASSERT(sizeof(string_t) == sizeof(int));

	static char szBuf[512];

	switch (fieldType)
	{
	case FIELD_STRING:
	{
		return(STRING(iszVal));
	}

	case FIELD_BOOLEAN:
	{
		if (bVal == 0)
		{
			Q_strncpy(szBuf, "false", sizeof(szBuf));
		}
		else
		{
			Q_strncpy(szBuf, "true", sizeof(szBuf));
		}
		return(szBuf);
	}

	case FIELD_INTEGER:
	{
		Q_snprintf(szBuf, sizeof(szBuf), "%i", iVal);
		return(szBuf);
	}

	case FIELD_FLOAT:
	{
		Q_snprintf(szBuf, sizeof(szBuf), "%g", flVal);
		return(szBuf);
	}

	case FIELD_COLOR32:
	{
		Q_snprintf(szBuf, sizeof(szBuf), "%d %d %d %d", (int)rgbaVal.r, (int)rgbaVal.g, (int)rgbaVal.b, (int)rgbaVal.a);
		return(szBuf);
	}

	case FIELD_VECTOR:
	{
		Q_snprintf(szBuf, sizeof(szBuf), "[%g %g %g]", (double)vecVal[0], (double)vecVal[1], (double)vecVal[2]);
		return(szBuf);
	}

	case FIELD_VOID:
	{
		szBuf[0] = '\0';
		return(szBuf);
	}

	case FIELD_EHANDLE:
	{
		const char *pszName = NULL;
		CEntity *pEnt = CEntity::Instance(eVal);
		if (pEnt)
		{
			pszName = pEnt->GetClassname();
		}
		else
		{
			pszName = "<<null entity>>";
		}

		Q_strncpy(szBuf, pszName, 512);
		return (szBuf);
	}

	default:
		break;
	}

	return("No conversion to string");
}

void variant_t::Set(fieldtype_t ftype, void *data)
{
	fieldType = ftype;

	switch (ftype)
	{
	case FIELD_BOOLEAN:		bVal = *((bool *)data);				break;
	case FIELD_CHARACTER:	iVal = *((char *)data);				break;
	case FIELD_SHORT:		iVal = *((short *)data);			break;
	case FIELD_INTEGER:		iVal = *((int *)data);				break;
	case FIELD_STRING:		iszVal = *((string_t *)data);		break;
	case FIELD_FLOAT:		flVal = *((float *)data);			break;
	case FIELD_COLOR32:		rgbaVal = *((color32 *)data);		break;

	case FIELD_VECTOR:
	case FIELD_POSITION_VECTOR:
	{
		vecVal[0] = ((float *)data)[0];
		vecVal[1] = ((float *)data)[1];
		vecVal[2] = ((float *)data)[2];
		break;
	}

	case FIELD_EHANDLE:		eVal = *((EHANDLE *)data);			break;
	case FIELD_CLASSPTR:	eVal = *((CBaseEntity **)data);		break;
	case FIELD_VOID:
	default:
		iVal = 0; fieldType = FIELD_VOID;
		break;
	}
}

/**
 * This is the worst util ever, incredibly specific usage.
 * Searches a datamap for output types and swaps the SaveRestoreOps pointer for the global eventFuncs one.
 * Reason for this is we didn't have the eventFuncs pointer available statically (when the datamap structure was generated)
 */
void UTIL_PatchOutputRestoreOps(datamap_t *pMap)
{
	for (int i = 0; i < pMap->dataNumFields; i++)
	{
		if (pMap->dataDesc[i].flags & FTYPEDESC_OUTPUT)
		{
			pMap->dataDesc[i].pSaveRestoreOps = eventFuncs;
		}

		if (pMap->dataDesc[i].td)
		{
			UTIL_PatchOutputRestoreOps(pMap->dataDesc[i].td);
		}
	}
}
