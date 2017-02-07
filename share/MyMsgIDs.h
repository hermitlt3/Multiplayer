#ifndef MYSMSGIDS_H_
#define MYSMSGIDS_H_

#include "MessageIdentifiers.h"

enum MyMsgIDs
{
	ID_WELCOME = ID_USER_PACKET_ENUM,
	ID_NEWSHIP,
	ID_LOSTSHIP,
	ID_INITIALPOS,
	ID_MOVEMENT,
    ID_COLLIDE,

    ID_NEWMISSILE,
	ID_UPDATEMISSILE,

	ID_NEWENERGYBALL,
	ID_UPDATEENERGYBALL,

	ID_NEWLAST,
	ID_UPDATELAST,

	// Assignment 2
	ID_NEWBOOM,
	ID_NEWBOMB,

	ID_MISSILEHIT,
	ID_BOMBHIT,
	ID_ENERGYBALLHIT,
	ID_LASTHIT,

	ID_NEWPOWERUPS,
	ID_COLLECTHEALTH,
	ID_COLLECTEXP,

	ID_MAX_PLAYERS
};

#endif