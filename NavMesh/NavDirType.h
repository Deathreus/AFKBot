#ifndef __war3source_navdirtype_h__
#define __war3source_navdirtype_h__


enum eNavDir
{
	NAV_DIR_NORTH	= 0,
	NAV_DIR_EAST	= 1,
	NAV_DIR_SOUTH	= 2,
	NAV_DIR_WEST	= 3,

	NAV_DIR_COUNT
};

enum eNavLadderDir
{
	NAV_LADDER_DIR_UP	= 0,
	NAV_LADDER_DIR_DOWN = 1,

	NAV_LADDER_DIR_COUNT
};

enum eNavTraverse
{
	GO_NORTH,
	GO_EAST,
	GO_SOUTH,
	GO_WEST,
	GO_LADDER_UP,
	GO_LADDER_DOWN,
	GO_JUMP,

	NAV_TRAVERSE_COUNT
};

#endif