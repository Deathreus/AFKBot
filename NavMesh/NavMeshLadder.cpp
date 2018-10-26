#include "NavMeshLadder.h"
#include <vector.h>


CNavMeshLadder::CNavMeshLadder(unsigned int id, float width, float length, float topX, float topY, float topZ,
	float bottomX, float bottomY, float bottomZ, eNavDir direction,
	unsigned int topForwardAreaID, unsigned int topLeftAreaID, unsigned int topRightAreaID, unsigned int topBehindAreaID, unsigned int bottomAreaID)
{
	this->id = id;
	this->width = width;
	this->length = length;
	this->top = Vector(topX, topY, topZ);
	this->bottom = Vector(bottomX, bottomY, bottomZ);
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

float CNavMeshLadder::GetTopX() { return this->top.x; }

float CNavMeshLadder::GetTopY() { return this->top.y; }

float CNavMeshLadder::GetTopZ() { return this->top.z; }

float CNavMeshLadder::GetBottomX() { return this->bottom.x; }

float CNavMeshLadder::GetBottomY() { return this->bottom.y; }

float CNavMeshLadder::GetBottomZ() { return this->bottom.z; }

const Vector CNavMeshLadder::GetTop() { return this->top; }

const Vector CNavMeshLadder::GetBottom() { return this->bottom; }

eNavDir CNavMeshLadder::GetDirection() { return this->direction; }

unsigned int CNavMeshLadder::GetTopForwardAreaID() { return this->topForwardAreaID; }

unsigned int CNavMeshLadder::GetTopLeftAreaID() { return this->topLeftAreaID; }

unsigned int CNavMeshLadder::GetTopRightAreaID() { return this->topRightAreaID; }

unsigned int CNavMeshLadder::GetTopBehindAreaID() { return this->topBehindAreaID; }

unsigned int CNavMeshLadder::GetBottomAreaID() { return this->bottomAreaID; }