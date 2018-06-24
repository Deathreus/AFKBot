#include "NavMeshLadder.h"


CNavMeshLadder::CNavMeshLadder(unsigned int id, float width, float length, float topX, float topY, float topZ,
	float bottomX, float bottomY, float bottomZ, eNavDir direction,
	unsigned int topForwardAreaID, unsigned int topLeftAreaID, unsigned int topRightAreaID, unsigned int topBehindAreaID, unsigned int bottomAreaID)
{
	this->id = id;
	this->width = width;
	this->length = length;
	this->topX = topX;
	this->topY = topY;
	this->topZ = topZ;
	this->bottomX = bottomX;
	this->bottomY = bottomY;
	this->bottomZ = bottomZ;
	this->direction = direction;
	this->topForwardAreaID = topForwardAreaID;
	this->topLeftAreaID = topLeftAreaID;
	this->topRightAreaID = topRightAreaID;
	this->topBehindAreaID = topBehindAreaID;
	this->bottomAreaID = bottomAreaID;
}

unsigned int CNavMeshLadder::GetID() { return this->id; }

float CNavMeshLadder::GetWidth() { return this->width; }

float CNavMeshLadder::GetLength() { return this->length; }

float CNavMeshLadder::GetTopX() { return this->topX; }

float CNavMeshLadder::GetTopY() { return this->topY; }

float CNavMeshLadder::GetTopZ() { return this->topZ; }

float CNavMeshLadder::GetBottomX() { return this->bottomX; }

float CNavMeshLadder::GetBottomY() { return this->bottomY; }

float CNavMeshLadder::GetBottomZ() { return this->bottomZ; }

eNavDir CNavMeshLadder::GetDirection() { return this->direction; }

unsigned int CNavMeshLadder::GetTopForwardAreaID() { return this->topForwardAreaID; }

unsigned int CNavMeshLadder::GetTopLeftAreaID() { return this->topLeftAreaID; }

unsigned int CNavMeshLadder::GetTopRightAreaID() { return this->topRightAreaID; }

unsigned int CNavMeshLadder::GetTopBehindAreaID() { return this->topBehindAreaID; }

unsigned int CNavMeshLadder::GetBottomAreaID() { return this->bottomAreaID; }
