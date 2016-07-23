// EXCLUDED
#ifndef __RCBOT_CONFIG_H__
#define __RCBOT_CONFIG_H__

class CRCBotConfig : public CConfigFile
{
public:
	void LoadConfig ();
private:
	int m_iMaxBots;
	int m_iMinBots;
};

#endif