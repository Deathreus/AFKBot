#ifndef __war3source_navmeshladder_h__
#define __war3source_navmeshladder_h__

#include "public/INavMeshLadder.h"
#include "public/INavMeshArea.h"


class CNavMeshLadder : public INavMeshLadder
{
public:
	CNavMeshLadder(unsigned int id, float width, float length, float topX, float topY, float topZ,
		float bottomX, float bottomY, float bottomZ, eNavDir direction,
		unsigned int topForwardAreaID, unsigned int topLeftAreaID, unsigned int topRightAreaID, unsigned int topBehindAreaID, unsigned int bottomAreaID);
	~CNavMeshLadder() {}

	inline void Destroy() { delete this; }

	unsigned int GetID();
	float GetWidth();
	float GetLength();

	float GetTopX();
	float GetTopY();
	float GetTopZ();

	float GetBottomX();
	float GetBottomY();
	float GetBottomZ();

	const Vector GetTop();
	const Vector GetBottom();

	eNavDir GetDirection();

	unsigned int GetTopForwardAreaID();
	unsigned int GetTopLeftAreaID();
	unsigned int GetTopRightAreaID();
	unsigned int GetTopBehindAreaID();

	unsigned int GetBottomAreaID();

private:
	unsigned int id;
	float width;
	float length;
	Vector top;
	Vector bottom;
	eNavDir direction;
	unsigned int topForwardAreaID;
	unsigned int topLeftAreaID;
	unsigned int topRightAreaID;
	unsigned int topBehindAreaID;
	unsigned int bottomAreaID;
};

#endif