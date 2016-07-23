#ifndef __BOT_SIGSCAN_H__
#define __BOT_SIGSCAN_H__

#include "bot_const.h"
#include "KeyValues.h"

struct DynLibInfo
{
	void *baseAddress;
	size_t memorySize;
};

class CAFKBotKeyValueList;

class CSignatureFunction
{
public:
	CSignatureFunction() { m_func = 0x0; }
private:
	size_t DecodeHexString(unsigned char *buffer, size_t maxlength, const char *hexstr);

	bool GetLibraryInfo(const void *libPtr, DynLibInfo &lib);

	void *FindPattern(const void *libPtr, const char *pattern, size_t len);

	void *FindSignature(void *addrInBase, const char *signature);
protected:
	void FindFunc(CAFKBotKeyValueList *kv, const char *pKey, void *pAddrBase, const char *defaultsig);

	void *m_func;
};

class CGameRulesObject : public CSignatureFunction
{
public:
	CGameRulesObject(CAFKBotKeyValueList *list, void *pAddrBase);

	bool Found() { return m_func != NULL; }

	void *GetGameRules() { return reinterpret_cast<void *>(m_func); }
};

class CCreateGameRulesObject : public CSignatureFunction
{
public:
	CCreateGameRulesObject(CAFKBotKeyValueList *list, void *pAddrBase);

	bool Found() { return m_func != NULL; }

	void *GetGameRules();
};

extern CGameRulesObject *g_pGameRules_Obj;
extern CCreateGameRulesObject *g_pGameRules_Create_Obj;

void *GetGameRules();

#endif