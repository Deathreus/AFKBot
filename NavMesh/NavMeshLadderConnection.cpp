#include "NavMeshLadderConnection.h"


CNavMeshLadderConnection::CNavMeshLadderConnection(unsigned int connectingLadderID, eNavLadderDir direction)
{
	this->connectingLadderID = connectingLadderID;
	this->direction = direction;
}

CNavMeshLadderConnection::~CNavMeshLadderConnection() {}

unsigned int CNavMeshLadderConnection::GetConnectingLadderID() { return this->connectingLadderID; }

eNavLadderDir CNavMeshLadderConnection::GetDirection() { return this->direction; }
