#ifndef __war3source__navdirtype_h__
#define __war3source__navdirtype_h__


enum eNavDir
{
	NAV_DIR_NORTH = 0,
	NAV_DIR_EAST = 1,
	NAV_DIR_SOUTH = 2,
	NAV_DIR_WEST = 3,

	NAV_DIR_COUNT
};

enum eNavTraverse
{
	// THE FIRST 4 MUST MATCH eNavDir
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