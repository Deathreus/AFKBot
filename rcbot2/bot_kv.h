#ifndef __RCBOT_KEY_VAL__
#define __RCBOT_KEY_VAL__

#define RCBOT_MAX_KV_LEN 256

#include <vector>
using namespace std;

class CAFKBotKeyValue
{
public:
	CAFKBotKeyValue(const char *szKey, char *szValue);

	char *GetKey()
	{
		return m_szKey;
	}

	char *GetValue()
	{
		return m_szValue;
	}

private:
	char m_szKey[RCBOT_MAX_KV_LEN];
	char m_szValue[RCBOT_MAX_KV_LEN];
};

class CAFKBotKeyValueList
{
public:
	~CAFKBotKeyValueList();

	void ParseFile(FILE *fp);

	//unsigned int Size ();

	//CAFKBotKeyValue *GetKV ( unsigned int iIndex );

	bool GetInt(const char *key, int *val);

	bool GetString(const char *key, char **val);

	bool GetFloat(const char *key, float *val);

private:

	CAFKBotKeyValue *GetKV(const char *key);

	vector <CAFKBotKeyValue*> m_KVs;
};

#endif