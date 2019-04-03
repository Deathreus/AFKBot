#ifndef __RCBOT_TF2_POINTS_H__
#define __RCBOT_TF2_POINTS_H__

#include "utlmap.h"
#include "shareddefs.h"

class variant_t
{
public:
	union
	{
		bool bVal;
		string_t iszVal;
		int iVal;
		float flVal;
		float vecVal[3];
		color32 rgbaVal;
	};
	CBaseHandle eVal; // this can't be in the union because it has a constructor. 
	fieldtype_t fieldType;
};

class CEventAction;
class CSoundPatch;

class CBaseEntityOutput
{
public:
	variant_t m_Value;
	CEventAction *m_ActionList;
	DECLARE_SIMPLE_DATADESC()
};
typedef CBaseEntityOutput COutputEvent;

class CTeamControlPoint
{
public:

	int			m_iTeam;
	int			m_iDefaultOwner;			// Team that initially owns the cap point
	int			m_iIndex;					// The index of this point in the controlpointArray
	int			m_iWarnOnCap;				// Warn the team that owns the control point when the opposing team starts to capture it.
	string_t	m_iszPrintName;
	string_t	m_iszWarnSound;				// Sound played if the team needs to be warned about this point being captured
	bool		m_bRandomOwnerOnRestart;	// Do we want to randomize the owner after a restart?
	bool		m_bLocked;
	float		m_flUnlockTime;				// Time to unlock

											// We store a copy of this data for each team, +1 for the un-owned state.
	struct perteamdata_t
	{
		perteamdata_t()
		{
			iszCapSound = NULL_STRING;
			iszModel = NULL_STRING;
			iModelBodygroup = -1;
			iIcon = 0;
			iszIcon = NULL_STRING;
			iOverlay = 0;
			iszOverlay = NULL_STRING;
			iPlayersRequired = 0;
			iTimedPoints = 0;
			for (int i = 0; i < MAX_PREVIOUS_POINTS; i++)
			{
				iszPreviousPoint[i] = NULL_STRING;
			}
			iTeamPoseParam = 0;
		}

		string_t	iszCapSound;
		string_t	iszModel;
		int			iModelBodygroup;
		int			iTeamPoseParam;
		int			iIcon;
		string_t	iszIcon;
		int			iOverlay;
		string_t	iszOverlay;
		int			iPlayersRequired;
		int			iTimedPoints;
		string_t	iszPreviousPoint[MAX_PREVIOUS_POINTS];
	};
	CUtlVector<perteamdata_t>	m_TeamData;

	COutputEvent	m_OnCapReset;

	COutputEvent	m_OnCapTeam1;
	COutputEvent	m_OnCapTeam2;

	COutputEvent	m_OnOwnerChangedToTeam1;
	COutputEvent	m_OnOwnerChangedToTeam2;

	COutputEvent	m_OnRoundStartOwnedByTeam1;
	COutputEvent	m_OnRoundStartOwnedByTeam2;

	COutputEvent	m_OnUnlocked;

	int			m_bPointVisible;		//should this capture point be visible on the hud?
	int			m_iPointIndex;			//the mapper set index value of this control point

	int			m_iCPGroup;			//the group that this control point belongs to
	bool		m_bActive;			//

	string_t	m_iszName;				//Name used in cap messages

	bool		m_bStartDisabled;

	float		m_flLastContestedAt;

	CSoundPatch *m_pCaptureInProgressSound;
	string_t	m_iszCaptureStartSound;
	string_t	m_iszCaptureEndSound;
	string_t	m_iszCaptureInProgress;
	string_t	m_iszCaptureInterrupted;
};

class CTeamControlPointRound
{
public:

	bool IsPointInRound(edict_t *point_pent);

	CUtlVector< CBaseHandle > m_ControlPoints;

	bool m_bDisabled;

	string_t	m_iszCPNames;
	int			m_nPriority;
	int			m_iInvalidCapWinner;
	string_t	m_iszPrintName;
};

class CTeamControlPointMaster
{
public:

	CUtlMap<int, CBaseEntity *> m_ControlPoints;

	bool m_bFoundPoints;

	CTeamControlPointRound *GetCurrentRound();

	CUtlVector<CBaseEntity *> m_ControlPointRounds;
	int m_iCurrentRoundIndex;

	bool m_bDisabled;

	string_t m_iszTeamBaseIcons[MAX_TEAMS];
	int m_iTeamBaseIcons[MAX_TEAMS];
	string_t m_iszCapLayoutInHUD;

	float m_flCustomPositionX;
	float m_flCustomPositionY;

	int m_iInvalidCapWinner;
	bool m_bSwitchTeamsOnWin;
	bool m_bScorePerCapture;
	bool m_bPlayAllRounds;

	bool m_bFirstRoundAfterRestart;

	COutputEvent m_OnWonByTeam1;
	COutputEvent m_OnWonByTeam2;

	float m_flPartialCapturePointsRate;
	float m_flLastOwnershipChangeTime;

};

#define TEAM_ARRAY( index, team )		(index + (team * MAX_CONTROL_POINTS))

typedef enum ePointAttackDefend
{
	TF2_POINT_DEFEND = 0,
	TF2_POINT_ATTACK
}ePointAttackDefend_t;

typedef struct
{
	float fProb;
	float fProbMultiplier;
	bool bValid;
	bool bNextPoint;
	bool bPrev;
	int iPrev[MAX_PREVIOUS_POINTS];
}TF2PointProb_t;

class CTFObjectiveResource
{
public:
	CTFObjectiveResource()
	{
		Q_memset(this, 0, sizeof(CTFObjectiveResource));
	}

	void Reset();

	const bool IsWaypointAreaValid(int wptarea, int waypointflags = 0) const;

	const bool TestProbWptArea(int iWptArea, int iTeam) const;

	void UpdatePoints();

	void Think();

	int GetControlPointArea(edict_t *pPoint) const;

	int NearestArea(Vector vOrigin);

	float GetLastCaptureTime(int index) const;

	void UpdateCaptureTime(int index);

	void Setup();

	inline const bool IsInitialised() const { return !!m_bInitialised; }

	int GetRandomValidPointForTeam(int team, ePointAttackDefend type);

	const bool IsCPValidWptArea(int iWptArea, int iTeam, ePointAttackDefend type) const;
	const bool IsCPValid(int iCPIndex, int iTeam, ePointAttackDefend type) const;

	inline int GetControlPointWaypoint(int index) const
	{
		return m_iControlPointWpt[index];
	}

	inline int GetNumControlPoints(void) const
	{
		return m_iNumControlPoints;
	}

	const Vector GetCPPosition(int index);

	bool TeamCanCapPoint(int index, int team);

	int GetOwningTeam(int index);

	bool IsCPVisible(int index);

	bool IsCPBlocked(int index);

	bool IsCPLocked(int index);

	int GetCappingTeam(int index);

	int GetTeamInZone(int index);

	int GetNumPlayersInArea(int index, int team);

	int GetRequiredCappers(int index, int team);

	int GetBaseControlPointForTeam(int iTeam);

	float GetCPUnlockTime(int index);

	int GetPreviousPointForPoint(int index, int team, int iPrevIndex);

	bool PlayingMiniRounds(void);
	bool IsInMiniRound(int index);

	void AssertValidIndex(int index) const
	{
		Assert( (0 <= index) && (index <= MAX_CONTROL_POINTS) && (index < m_iNumControlPoints) );
	}

	MyEHandle m_Resource;
	int m_IndexToWaypointAreaTranslation[MAX_CONTROL_POINTS];
	int m_WaypointAreaToIndexTranslation[MAX_CONTROL_POINTS + 1];
	int m_iMonitorPoint[2];
	float m_fNextCheckMonitoredPoint;
	float m_fUpdatePointTime;
	int m_iNumControlPoints;

private:
	bool m_bInitialised;

	MyEHandle m_pControlPoints[MAX_CONTROL_POINTS];
	int m_iControlPointWpt[MAX_CONTROL_POINTS];
	bool m_iControlPointWptReachable[MAX_CONTROL_POINTS];
	TF2PointProb_t m_ValidPoints[2][2][MAX_CONTROL_POINTS];
	int m_PointSignature[2][2];
	bool m_ValidAreas[MAX_CONTROL_POINTS];
	float m_fLastCaptureTime[MAX_CONTROL_POINTS];

	bool UpdateAttackPoints(int team);
	bool UpdateDefendPoints(int team);

	inline void ResetValidWaypointAreas()
	{
		Q_memset(m_ValidAreas, 0, sizeof(bool)*MAX_CONTROL_POINTS);
	}
	void UpdateValidWaypointAreas(void)
	{
		ResetValidWaypointAreas();

		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				for (int k = 0; k < MAX_CONTROL_POINTS; k++)
				{
					// OR
					m_ValidAreas[k] = (m_ValidAreas[k] || m_ValidPoints[i][j][k].bValid);
				}
			}
		}
	}
};

class CTeamRoundTimer
{
public:
	CTeamRoundTimer()
	{
		Q_memset(this, 0, sizeof(CTeamRoundTimer));
	}

	float GetSetupTime();

	float GetEndTime();

	const bool IsInSetup();

	void Reset();

	MyEHandle m_Resource;
};

#endif