#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <vector>

#include "asteroid.h"
#include "ship.h"
#include "missile.h"
#include "boom.h"
#include "timebomb.h"
#include "energyball.h"
#include "textbox.h"
#include "PowerupBaseClass.h"
#include "Healthpack.h"
#include "Exppack.h"
#include "last.h"

class HGE;
class RakPeerInterface;

//! The default angular velocity of the ship when it is in motion
static const float DEFAULT_ANGULAR_VELOCITY = 3.0f; 
//! The default acceleration of the ship when powered
static const float DEFAULT_ACCELERATION = 50.0f;

/**
* The application class is the main body of the program. It will
* create an instance of the graphics engine and execute the
* Update/Render cycle.
*
*/

class Application
{
	HGE* hge_; //!< Instance of the internal graphics engine
	typedef std::vector<Ship*>					ShipList;  //!< A list of ships
	typedef std::vector<Missile*>				MissileList;
	typedef std::vector<Boom*>					BoomList;
	typedef std::vector<Timebomb*>				TimebombList;
	typedef std::vector<Energyball*>			EnergyballList;
	typedef std::vector<PowerupBaseClass*>		PowerupsList;
	typedef std::vector<Last*>					LastList;

	ShipList ships_; //!< List of all the ships in the universe

	Missile* mymissile;
	Energyball* myenergyball;
	Last* mylast;

	MissileList missiles_;
	BoomList booms_;
	TimebombList bombs_;
	EnergyballList energyballs_;
	PowerupsList powerupslist;
	LastList lastlist_;

	TextBox	*fpsbox;
	TextBox *databox;
	std::auto_ptr<hgeFont> font_;
	std::string rejectfont;

	RakPeerInterface* rakpeer_;
	unsigned int timer_;
	int totalsent_;
	int totalreceived_;
	
	bool have_missile;
	bool have_energyball;
	bool have_last;

	bool keydown_enter;
	bool keydown_q;
	bool keydown_w;

	bool rejected;

	bool Init();
	static bool Loop();
	void Shutdown();
	void ProcessWelcomePackage();
	bool SendInitialPosition();
	void CreateMissile( float x, float y, float w, int id );
	bool RemoveMissile( float x, float y, float w, int id );
	// Assignment 2
	void CreateBoom( float x, float y, float lifetime );
	void CreateBomb( float x, float y, float explosionRadius );
	void CreateEnergyBall(float x, float y, float w, int id);
	void CreateLastWeapon(float x, float y, float w, int id);

	void MissileHit( int id );
	void BombHit( int id );
	void EnergyBallHit( int id );
	void LastHit(int id);

	void CollectHealth( int id );
	void CollectEXP(int id);

	void SwitchPositions();

	void Reset();

public:
	Application();
	~Application() throw();

	void Start();
	bool Update();
	void Render();
	
};

#endif