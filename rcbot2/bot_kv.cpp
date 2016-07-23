#include "bot.h"
#include "bot_kv.h"

void CAFKBotKeyValueList::ParseFile(FILE *fp)
{
	char buffer[2 * (RCBOT_MAX_KV_LEN)];
	char szKey[RCBOT_MAX_KV_LEN];
	char szValue[RCBOT_MAX_KV_LEN];

	int iKi;
	int iVi;
	int iLen;
	int iCi; // current character index		
	bool bHaveKey;
	int iLine;

	iLine = 0;

	// parse profile ini
	while (fgets(buffer, 255, fp) != NULL)
	{
		iLine++;

		if (buffer[0] == '#') // skip comment
			continue;

		iLen = strlen(buffer);

		if (iLen == 0)
			continue;

		if (buffer[iLen - 1] == '\n')
			buffer[--iLen] = 0;

		if (buffer[iLen - 1] == '\r')
			buffer[--iLen] = 0;

		bHaveKey = false;

		iKi = 0;
		iVi = 0;

		for (iCi = 0; iCi < iLen; iCi++)
		{
			// ignore spacing
			if (buffer[iCi] == ' ')
				continue;

			if (!bHaveKey)
			{
				if (buffer[iCi] == '=')
				{
					bHaveKey = true;
					continue;
				}

				// parse key

				if (iKi < RCBOT_MAX_KV_LEN)
					szKey[iKi++] = buffer[iCi];
			}
			else if (iVi < RCBOT_MAX_KV_LEN)
				szValue[iVi++] = buffer[iCi];
			else
				break;
		}

		szKey[iKi] = 0;
		szValue[iVi] = 0;

		m_KVs.push_back(new CAFKBotKeyValue(szKey, szValue));

	}

}

CAFKBotKeyValueList :: ~CAFKBotKeyValueList()
{
	for (unsigned int i = 0; i < m_KVs.size(); i++)
	{
		delete m_KVs[i];
		m_KVs[i] = NULL;
	}

	m_KVs.clear();
}

CAFKBotKeyValue *CAFKBotKeyValueList::GetKV(const char *key)
{
	for (unsigned int i = 0; i < m_KVs.size(); i++)
	{
		if (FStrEq(m_KVs[i]->GetKey(), key))
			return m_KVs[i];
	}

	return NULL;
}

bool CAFKBotKeyValueList::GetFloat(const char *key, float *val)
{
	CAFKBotKeyValue *pKV;

	pKV = GetKV(key);

	if (!pKV)
		return false;

	*val = atof(pKV->GetValue());

	return true;
}


bool CAFKBotKeyValueList::GetInt(const char *key, int *val)
{
	CAFKBotKeyValue *pKV;

	pKV = GetKV(key);

	if (!pKV)
		return false;

	*val = atoi(pKV->GetValue());

	return true;
}


bool CAFKBotKeyValueList::GetString(const char *key, char **val)
{
	CAFKBotKeyValue *pKV;

	pKV = GetKV(key);

	if (!pKV)
		return false;

	*val = pKV->GetValue();

	return true;
}

CAFKBotKeyValue::CAFKBotKeyValue(const char *szKey, char *szValue)
{
	strncpy(m_szKey, szKey, RCBOT_MAX_KV_LEN - 1);
	m_szKey[RCBOT_MAX_KV_LEN - 1] = 0;
	strncpy(m_szValue, szValue, RCBOT_MAX_KV_LEN - 1);
	m_szValue[RCBOT_MAX_KV_LEN - 1] = 0;
}