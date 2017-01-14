#ifndef __RCBOT_PROFILE_H__
#define __RCBOT_PROFILE_H__

#include <vector>
using namespace std;

class CBotProfile
{
public:
	CBotProfile()
	{
		memset(this, 0, sizeof(CBotProfile));
	}

	CBotProfile operator =(CBotProfile &other)
	{
		*this = other;
	}

	// setup profile
	CBotProfile(
		int iVisionTicks,
		int iPathTicks,
		int iVisionTicksClients,
		int iSensitivity,
		float fBraveness,
		float fAimSkill);

	int m_iVisionTicks;			// speed of finding non players (npcs/teleporters etc)
	int m_iPathTicks;			// speed of finding a path
	int m_iVisionTicksClients;	// speed of finding other players and enemy players
	int m_iSensitivity;		// 1 to 20 sensitivity of bot's "mouse" (angle speed)
	float m_fBraveness;			// 0.0 to 1.0 sensitivity to danger (brave = less sensitive)
	float m_fAimSkill;			// 0.0 to 1.0 ability to predict players movements (aim skill)
};

class CBotProfiles
{
public:
	static void DeleteProfiles();

	static void SetupProfile();

	static CBotProfile *GetDefaultProfile();

private:
	static CBotProfile **m_Profiles;
	static CBotProfile *m_pDefaultProfile;
};

#endif