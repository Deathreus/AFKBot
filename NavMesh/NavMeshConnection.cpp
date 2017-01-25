#include "NavMeshConnection.h"


CNavMeshConnection::CNavMeshConnection(unsigned int connectingAreaID, eNavDir direction)
{
	this->connectingAreaID = connectingAreaID;
	this->direction = direction;
}

CNavMeshConnection::~CNavMeshConnection() {}

unsigned int CNavMeshConnection::GetConnectingAreaID() { return this->connectingAreaID; }

eNavDir CNavMeshConnection::GetDirection() { return this->direction; }
