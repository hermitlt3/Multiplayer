#include <iostream>
#include <vector>

#include "hge.h"
#include "hgeFont.h"
#include "RakNetworkFactory.h"
#include "RakPeerInterface.h"
#include "Bitstream.h"
#include "GetTime.h"

#include "config.h"
#include "MyMsgIDs.h"

#include "Globals.h"
#include "ship.h"
#include "Application.h"

#define BUFFERSIZE 256
/** 
* Constuctor
*
* Creates an instance of the graphics engine and network engine
*/

Application::Application() : 
	hge_(hgeCreate(HGE_VERSION)),
	fpsbox(0),
	rakpeer_(RakNetworkFactory::GetRakPeerInterface()),
	timer_( 0 ),
	totalsent_(0),
	totalreceived_(0),
	mymissile(0),
	myenergyball(0),
	mylast(0),
	have_missile( false ),
	have_energyball(false),
	have_last(false),
	keydown_enter(false),
	keydown_q( false ),
	keydown_w( false ),
	rejectfont("Max players in server")
{
}

/**
* Destructor
*
* Does nothing in particular apart from calling Shutdown
*/

Application::~Application() throw()
{
	delete fpsbox;
	fpsbox = 0;
	delete databox;
	databox = 0;

	Shutdown();
	font_.release(); 
	rakpeer_->Shutdown(100);
	RakNetworkFactory::DestroyRakPeerInterface(rakpeer_);
}

/**
* Initialises the graphics system
* It should also initialise the network system
*/

bool Application::Init()
{
	srand( RakNet::GetTime() );

	hge_->System_SetState(HGE_FRAMEFUNC, Application::Loop);
	hge_->System_SetState(HGE_WINDOWED, true);
	hge_->System_SetState(HGE_USESOUND, false);
	hge_->System_SetState(HGE_TITLE, "DM2241 Multiplayer Game Programming");
//	hge_->System_SetState(HGE_LOGFILE, "movement.log");
	hge_->System_SetState(HGE_DONTSUSPEND, true);

	if(hge_->System_Initiate()) 
	{
        
        fpsbox = new TextBox( "font1.fnt" );
		fpsbox->SetPos( 5, 5 );

		databox = new TextBox("font1.fnt");
		databox->SetPos( 580, 5 );

        int ShipType = rand() % 4 + 1;
        float init_pos_x = (float)(rand() % 500 + 100);
        float init_pos_y = (float)(rand() % 400 + 100);
        ships_.push_back( new Ship( ShipType, init_pos_x, init_pos_y ) );
        std::cout << "My Ship: type[" << ShipType << "] x[" << init_pos_x << "] y[" << init_pos_y << "]" << std::endl;
		rejected = false;
		ships_.at(0)->SetName("My Ship");
		font_.reset(new hgeFont("font1.fnt"));
		font_->SetScale(0.5);
		if (rakpeer_->Startup(1, 30, &SocketDescriptor(), 1))
		{
			rakpeer_->SetOccasionalPing(true);
			return rakpeer_->Connect(DFL_SERVER_IP, DFL_PORTNUMBER, 0, 0);
		}
	}
	return false;
}

/**
* Update cycle
*
* Checks for keypresses:
*   - Esc - Quits the game
*   - Left - Rotates ship left
*   - Right - Rotates ship right
*   - Up - Accelerates the ship
*   - Down - Deccelerates the ship
*
* Also calls Update() on all the ships in the universe
*/
bool Application::Update()
{
    if( hge_->Input_GetKeyState( HGEK_ESCAPE ) )
        return true;

    float timedelta = hge_->Timer_GetDelta( );

    ships_.at( 0 )->SetAngularVelocity( 0.0f );

    if( hge_->Input_GetKeyState( HGEK_LEFT ) )
    {
        ships_.at( 0 )->SetAngularVelocity( ships_.at( 0 )->GetAngularVelocity( ) - DEFAULT_ANGULAR_VELOCITY );
    }

    if( hge_->Input_GetKeyState( HGEK_RIGHT ) )
    {
        ships_.at( 0 )->SetAngularVelocity( ships_.at( 0 )->GetAngularVelocity( ) + DEFAULT_ANGULAR_VELOCITY );
    }

    if( hge_->Input_GetKeyState( HGEK_UP ) )
    {
        ships_.at( 0 )->Accelerate( DEFAULT_ACCELERATION, timedelta );
    }

    if( hge_->Input_GetKeyState( HGEK_DOWN ) )
    {
        ships_.at( 0 )->Accelerate( -DEFAULT_ACCELERATION, timedelta );
    }


    if( hge_->Input_GetKeyState( HGEK_SPACE ) )
	{
		if( !keydown_enter )
		{
			if (ships_.at(0)->GetLevel() <= 1)
				CreateMissile(ships_.at(0)->GetX(), ships_.at(0)->GetY(), ships_.at(0)->GetW(), ships_.at(0)->GetID());  
			else if (ships_.at(0)->GetLevel() <= 3)
				CreateEnergyBall(ships_.at(0)->GetX(), ships_.at(0)->GetY(), ships_.at(0)->GetW(), ships_.at(0)->GetID());
			else
				CreateLastWeapon(ships_.at(0)->GetX(), ships_.at(0)->GetY(), ships_.at(0)->GetW(), ships_.at(0)->GetID());

			keydown_enter = true;
		}
	}
	else
	{
		if( keydown_enter )
		{
			keydown_enter = false;
		}
	}

	if (hge_->Input_GetKeyState(HGEK_Q))
	{
		if (!keydown_q)
		{
			CreateBomb(ships_.at(0)->GetX(), ships_.at(0)->GetY(), 3.f);
			keydown_q = true;
		}
	}
	else
	{
		if (keydown_q)
		{
			keydown_q = false;
		}
	}

	if (hge_->Input_GetKeyState(HGEK_W))
	{
		if (!keydown_w)
		{
			//SwitchPositions();
			keydown_w = true;
		}
	}
	else
	{
		if (keydown_w)
		{
			keydown_w = false;
		}
	}

    // update ships
	for (ShipList::iterator ship = ships_.begin();
		ship != ships_.end(); ship++)
	{
		(*ship)->Update(timedelta);
	}

	// update missiles
	if( mymissile )
	{
		if (Ship* shipHit = (mymissile->Update(ships_, timedelta)))
		{
			if (shipHit->GetActive()) {
				CreateBoom(mymissile->GetX(), mymissile->GetY(), 0.5f);
				MissileHit(shipHit->GetID());

				// have collision
				delete mymissile;
				mymissile = 0;
			}
		}
	}

	for (MissileList::iterator missile = missiles_.begin();
		missile != missiles_.end(); missile++)
	{
		if( Ship* shipHit = (*missile)->Update(ships_, timedelta) )
		{
			// have collision
			if (shipHit->GetActive()) {
				MissileHit(shipHit->GetID());

				delete *missile;
				missiles_.erase(missile);
				break;
			}
		}
	}

	if (myenergyball)
	{
		if (Ship* shipHit = myenergyball->Update(ships_, timedelta))
		{
			if (shipHit->GetActive()) {
				EnergyBallHit(shipHit->GetID());
				CreateBoom(myenergyball->GetX(), myenergyball->GetY(), 0.5f);
				// have collision
				delete myenergyball;
				myenergyball = 0;
			}
		}
	}

	// Assignment 2
	for (EnergyballList::iterator energyball = energyballs_.begin();
		energyball != energyballs_.end(); energyball++)
	{
		if (Ship* shipHit = (*energyball)->Update(ships_, timedelta))
		{
			// have collision
			if (shipHit->GetActive()) {
				EnergyBallHit(shipHit->GetID());

				delete *energyball;
				energyballs_.erase(energyball);
				break;
			}
		}
	}

	if (mylast)
	{
		if (Ship* shipHit = mylast->Update(ships_, timedelta))
		{
			if (shipHit->GetActive()) {
				LastHit(shipHit->GetID());
				CreateBoom(mylast->GetX(), mylast->GetY(), 0.5f);

				// have collision
				delete mylast;
				mylast = 0;
			}
		}
	}

	// Assignment 2
	for (LastList::iterator last = lastlist_.begin();
		last != lastlist_.end(); last++)
	{
		if (Ship* shipHit = (*last)->Update(ships_, timedelta))
		{
			// have collision
			if (shipHit->GetActive()) {
				LastHit(shipHit->GetID());

				delete *last;
				lastlist_.erase(last);
				break;
			}
		}
	}

	for (BoomList::iterator boom = booms_.begin();
		boom != booms_.end(); boom++)
	{
		if ((*boom)->Update(timedelta))
		{
			delete *boom;
			booms_.erase(boom);
			break;
		}
	}

	for (PowerupsList::iterator powerup = powerupslist.begin();
		powerup != powerupslist.end(); powerup++)
	{
		if (Ship* shipHit = (*powerup)->Update(ships_, timedelta))
		{
			if ((*powerup)->returnType() == PowerupBaseClass::HEALTH)
				CollectHealth(shipHit->GetID());
			else if ((*powerup)->returnType() == PowerupBaseClass::EXP)
				CollectEXP(shipHit->GetID());

			delete *powerup;
			powerupslist.erase(powerup);
			break;
		}
	}

	for (TimebombList::iterator bomb = bombs_.begin();
		bomb != bombs_.end(); bomb++)
	{
		std::vector<Ship*>temp;
		temp = (*bomb)->Update(ships_, timedelta);
		if (temp.size() > 0)
		{
			for (std::vector<Ship*>::iterator it = temp.begin(); it != temp.end(); ++it) {
				BombHit((*it)->GetID());
			}
			CreateBoom((*bomb)->GetX(), (*bomb)->GetY(), 0.5f);
			delete *bomb;
			bombs_.erase(bomb);
			break;
		}
		else if ((*bomb)->GetToDeleted()) 
		{
			CreateBoom((*bomb)->GetX(), (*bomb)->GetY(), 0.5f);
			delete *bomb;
			bombs_.erase(bomb);
			break;
		}
	}

	if (!rejected) 
	{
		if (Packet* packet = rakpeer_->Receive())
		{
			RakNet::BitStream bs(packet->data, packet->length, false);

			unsigned char msgid = 0;
			RakNetTime timestamp = 0;

			bs.Read(msgid);

			if (msgid == ID_TIMESTAMP)
			{
				bs.Read(timestamp);
				bs.Read(msgid);
			}

			switch (msgid)
			{
			case ID_CONNECTION_REQUEST_ACCEPTED:
				std::cout << "Connected to Server" << std::endl;
				break;

			case ID_NO_FREE_INCOMING_CONNECTIONS:
			case ID_CONNECTION_LOST:
			case ID_DISCONNECTION_NOTIFICATION:
				std::cout << "Lost Connection to Server" << std::endl;
				rakpeer_->DeallocatePacket(packet);
				return true;

			case ID_WELCOME:
			{
							   unsigned int shipcount, id;
							   float x_, y_;
							   int type_;
							   std::string temp;
							   char chartemp[5];

							   bs.Read(id);
							   ships_.at(0)->setID(id);
							   bs.Read(shipcount);

							   for (unsigned int i = 0; i < shipcount; ++i)
							   {
								   bs.Read(id);
								   bs.Read(x_);
								   bs.Read(y_);
								   bs.Read(type_);
								   std::cout << "Welcome Ship pos " << x_ << ", " << y_ << ", type " << type_ << std::endl;
								   Ship* ship = new Ship(type_, x_, y_);
								   temp = "Ship ";
								   temp += _itoa_s(id, chartemp, 10);
								   ship->SetName(temp.c_str());
								   ship->setID(id);
								   ships_.push_back(ship);
							   }

							   SendInitialPosition();
			}
				break;

			case ID_NEWSHIP:
			{
							   unsigned int id;
							   bs.Read(id);

							   if (id == ships_.at(0)->GetID())
							   {
								   // if it is me
								   break;
							   }
							   else
							   {
								   ships_.at(0)->Reset();
								   Reset();
								   float x_, y_;
								   int type_;
								   std::string temp;
								   char chartemp[5];

								   bs.Read(x_);
								   bs.Read(y_);
								   bs.Read(type_);
								   std::cout << "New Ship pos" << x_ << " " << y_ << std::endl;
								   Ship* ship = new Ship(type_, x_, y_);
								   temp = "Ship ";
								   temp += _itoa_s(id, chartemp, 10);
								   ship->SetName(temp.c_str());
								   ship->setID(id);
								   ships_.push_back(ship);
							   }
			}
				break;

			case ID_LOSTSHIP:
			{
								unsigned int shipid;
								bs.Read(shipid);
								for (ShipList::iterator itr = ships_.begin(); itr != ships_.end(); ++itr)
								{
									if ((*itr)->GetID() == shipid)
									{
										delete *itr;
										ships_.erase(itr);
										break;
									}
								}
			}
				break;

			case ID_INITIALPOS:
				break;

			case ID_MOVEMENT:
			{
								totalreceived_ += bs.GetNumberOfBytesUsed();

								unsigned int shipid;
								float server_x, server_y, server_w;
								float server_vel_x, server_vel_y, server_vel_angular;
								bool active;
								int health;
								bs.Read(shipid);
								for (ShipList::iterator itr = ships_.begin(); itr != ships_.end(); ++itr)
								{
									if ((*itr)->GetID() == shipid)
									{
										bs.Read(server_x);
										bs.Read(server_y);
										bs.Read(server_w);
										bs.Read(server_vel_x);
										bs.Read(server_vel_y);
										bs.Read(server_vel_angular);
										bs.Read(active);
										bs.Read(health);

										(*itr)->SetHealth(health);
										(*itr)->SetActive(active);
										(*itr)->SetServerLocation(server_x, server_y, server_w);
										(*itr)->SetServerVelocity(server_vel_x, server_vel_y, server_vel_angular);
										(*itr)->DoInterpolateUpdate();
										break;
									}
								}
			}
				break;

			case ID_COLLIDE:
			{
							   unsigned int shipid;
							   float x, y;
							   bs.Read(shipid);

							   if (shipid == ships_.at(0)->GetID())
							   {
								   std::cout << "collided with someone!" << std::endl;
								   bs.Read(x);
								   bs.Read(y);
								   ships_.at(0)->SetX(x);
								   ships_.at(0)->SetY(y);
								   bs.Read(x);
								   bs.Read(y);
								   ships_.at(0)->SetVelocityX(x);
								   ships_.at(0)->SetVelocityY(y);
#ifdef INTERPOLATEMOVEMENT
								   bs.Read(x);
								   bs.Read(y);
								   ships_.at(0)->SetServerVelocityX(x);
								   ships_.at(0)->SetServerVelocityY(y);
#endif	
							   }
			}
				break;


			case ID_NEWMISSILE:
			{
								  float x, y, w;
								  int id;

								  bs.Read(id);
								  bs.Read(x);
								  bs.Read(y);
								  bs.Read(w);

								  missiles_.push_back(new Missile("missile.png", x, y, w, id));
			}
				break;

			case ID_UPDATEMISSILE:
			{
									 float server_x, server_y, server_w;
									 float server_vel_x, server_vel_y, server_vel_angular;
									 int id;
									 char deleted;

									 bs.Read(id);
									 bs.Read(deleted);

									 for (MissileList::iterator itr = missiles_.begin(); itr != missiles_.end(); ++itr)
									 {
										 if ((*itr)->GetOwnerID() == id)
										 {
											 if (deleted)
											 {
												 delete *itr;
												 missiles_.erase(itr);
											 }
											 else
											 {
												 bs.Read(server_x);
												 bs.Read(server_y);
												 bs.Read(server_w);
												 bs.Read(server_vel_x);
												 bs.Read(server_vel_y);
												 bs.Read(server_vel_angular);

												 (*itr)->SetServerLocation(server_x, server_y, server_w);
												 (*itr)->SetServerVelocity(server_vel_x, server_vel_y, server_vel_angular);
												 (*itr)->DoInterpolateUpdate();
											 }
											 break;
										 }
									 }

			}
				break;

				// Assignment 2
			case ID_SWITCH:
			{
							  int shipid;
							  float x, y;
							  float server_x, server_y, server_w;
							  float server_vel_x, server_vel_y, server_vel_angular;
							  float x2, y2;
							  float server_x2, server_y2, server_w2;
							  float server_vel_x2, server_vel_y2, server_vel_angular2;
							  bs.Read(shipid);

							  for (ShipList::iterator itr = ships_.begin(); itr != ships_.end(); ++itr)
							  {
								  if ((*itr)->GetID() == shipid)
								  {
									  std::cout << shipid << ", " << (*itr)->GetID() << ", " << ships_.at(0)->GetID() << std::endl;
									  std::cout << (*itr)->GetX() << ", " << (*itr)->GetY() << std::endl;
									  std::cout << ships_.at(0)->GetX() << ", " << ships_.at(0)->GetY() << std::endl;
									  bs.Read(x);
									  bs.Read(y);
									  bs.Read(server_x);
									  bs.Read(server_y);
									  bs.Read(server_w);
									  bs.Read(server_vel_x);
									  bs.Read(server_vel_y);
									  bs.Read(server_vel_angular);

									  (*itr)->SetX(x);
									  (*itr)->SetY(y);
									  (*itr)->SetServerLocation(server_x, server_y, server_w);
									  (*itr)->SetServerVelocity(server_vel_x, server_vel_y, server_vel_angular);
									  (*itr)->DoInterpolateUpdate();

									  bs.Read(x2);
									  bs.Read(y2);
									  bs.Read(server_x2);
									  bs.Read(server_y2);
									  bs.Read(server_w2);
									  bs.Read(server_vel_x2);
									  bs.Read(server_vel_y2);
									  bs.Read(server_vel_angular2);

									  ships_.at(0)->SetX(x2);
									  ships_.at(0)->SetY(y2);
									  ships_.at(0)->SetServerLocation(server_x2, server_y2, server_w2);
									  ships_.at(0)->SetServerVelocity(server_vel_x2, server_vel_y2, server_vel_angular2);
									  ships_.at(0)->DoInterpolateUpdate();

									  std::cout << (*itr)->GetX() << ", " << (*itr)->GetY() << std::endl;
									  std::cout << ships_.at(0)->GetX() << ", " << ships_.at(0)->GetY() << std::endl;
									  break;
								  }
							  }

			}
				break;

			case ID_MAX_PLAYERS:
			{
								   std::cout << "hit the maximum player limit" << std::endl;
								   rejected = true;
			}
				break;

			case ID_BOMBHIT:
			case ID_ENERGYBALLHIT:
			case ID_MISSILEHIT:
			case ID_LASTHIT:
			case ID_COLLECTHEALTH:
			{
								  int shipID;
								  int health;
								  bs.Read(shipID);
								  bs.Read(health);
								  for (ShipList::iterator itr = ships_.begin(); itr != ships_.end(); ++itr)
								  {
									  if ((*itr)->GetID() == shipID)
									  { 
										  (*itr)->SetHealth(health);
										  break;
									  }
								  }
			}
				break;

			case ID_COLLECTEXP:
			{
								  int shipID;
								  int level;
								  bs.Read(shipID);
								  bs.Read(level);
								  for (ShipList::iterator itr = ships_.begin(); itr != ships_.end(); ++itr)
								  {
									  if ((*itr)->GetID() == shipID)
									  {
										  (*itr)->SetLevel(level);
										  break;
									  }
								  }
			}
				break;
			case ID_NEWBOMB:
			{
							   float x, y, explosionRadius;

							   bs.Read(x);
							   bs.Read(y);
							   bs.Read(explosionRadius);

							   bombs_.push_back(new Timebomb("bomb.png", x, y, explosionRadius));

			}
				break;
			case ID_NEWENERGYBALL:
			{
								  float x, y, w;
								  int id;

								  bs.Read(id);
								  bs.Read(x);
								  bs.Read(y);
								  bs.Read(w);

								  energyballs_.push_back(new Energyball("energyball.png", x, y, w, id));
			}
				break;

			case ID_UPDATEENERGYBALL:
			{
									 float server_x, server_y, server_w;
									 float server_vel_x, server_vel_y, server_vel_angular;
									 int id;
									 char deleted;

									 bs.Read(id);
									 bs.Read(deleted);

									 for (EnergyballList::iterator itr = energyballs_.begin(); itr != energyballs_.end(); ++itr)
									 {
										 if ((*itr)->GetOwnerID() == id)
										 {
											 if (deleted)
											 {
												 delete *itr;
												 energyballs_.erase(itr);
											 }
											 else
											 {
												 bs.Read(server_x);
												 bs.Read(server_y);
												 bs.Read(server_w);
												 bs.Read(server_vel_x);
												 bs.Read(server_vel_y);
												 bs.Read(server_vel_angular);

												 (*itr)->SetServerLocation(server_x, server_y, server_w);
												 (*itr)->SetServerVelocity(server_vel_x, server_vel_y, server_vel_angular);
												 (*itr)->DoInterpolateUpdate();
											 }
											 break;
										 }
									 }

			}
				break;

			case ID_NEWLAST :
			{
									 float x, y, w;
									 int id;

									 bs.Read(id);
									 bs.Read(x);
									 bs.Read(y);
									 bs.Read(w);

									 lastlist_.push_back(new Last("last.png", x, y, w, id));
			}
				break;

			case ID_UPDATELAST:
			{
										float server_x, server_y, server_w;
										float server_vel_x, server_vel_y, server_vel_angular;
										int id;
										char deleted;

										bs.Read(id);
										bs.Read(deleted);

										for (LastList::iterator itr = lastlist_.begin(); itr != lastlist_.end(); ++itr)
										{
											if ((*itr)->GetOwnerID() == id)
											{
												if (deleted)
												{
													delete *itr;
													lastlist_.erase(itr);
												}
												else
												{
													bs.Read(server_x);
													bs.Read(server_y);
													bs.Read(server_w);
													bs.Read(server_vel_x);
													bs.Read(server_vel_y);
													bs.Read(server_vel_angular);

													(*itr)->SetServerLocation(server_x, server_y, server_w);
													(*itr)->SetServerVelocity(server_vel_x, server_vel_y, server_vel_angular);
													(*itr)->DoInterpolateUpdate();
												}
												break;
											}
										}

			}
				break;

			case ID_NEWBOOM:
			{
							   float x, y, lifetime;

							   bs.Read(x);
							   bs.Read(y);
							   bs.Read(lifetime);

							   booms_.push_back(new Boom("boom.png", x, y, lifetime));
			}
				break;

			case ID_NEWPOWERUPS:
			{
								   int a;
								   float posX, posY;
								   bs.Read(a);
								   bs.Read(posX);
								   bs.Read(posY);

								   if (static_cast<PowerupBaseClass::POWERUP_TYPE>(a) == PowerupBaseClass::HEALTH) {
									   powerupslist.push_back(new HealthPack("healthpack.png", posX, posY, 1.f));
								   }
								   else if (static_cast<PowerupBaseClass::POWERUP_TYPE>(a) == PowerupBaseClass::EXP) {
									   powerupslist.push_back(new ExpPack("exp.png", posX, posY, 1.f));
								   }
			}
				break;

			default:
				std::cout << "Unhandled Message Identifier: " << (int)msgid << std::endl;

			}
			rakpeer_->DeallocatePacket(packet);
		}
		
		if (RakNet::GetTime() - timer_ > 100)
		{
			timer_ = RakNet::GetTime();

			RakNet::BitStream bs2;
			unsigned char msgid = ID_MOVEMENT;
			bs2.Write(msgid);
			bs2.Write(ships_.at(0)->GetID());
			bs2.Write(ships_.at(0)->GetServerX());
			bs2.Write(ships_.at(0)->GetServerY());
			bs2.Write(ships_.at(0)->GetServerW());
			bs2.Write(ships_.at(0)->GetServerVelocityX());
			bs2.Write(ships_.at(0)->GetServerVelocityY());
			bs2.Write(ships_.at(0)->GetAngularVelocity());
			bs2.Write(ships_.at(0)->GetActive());
			bs2.Write(ships_.at(0)->GetHealth());

			rakpeer_->Send(&bs2, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
			totalsent_ += bs2.GetNumberOfBytesUsed();

			// send missile update to server
			if (mymissile)
			{
				RakNet::BitStream bs3;
				unsigned char msgid2 = ID_UPDATEMISSILE;
				unsigned char deleted = 0;
				bs3.Write(msgid2);
				bs3.Write(mymissile->GetOwnerID());
				bs3.Write(deleted);
				bs3.Write(mymissile->GetServerX());
				bs3.Write(mymissile->GetServerY());
				bs3.Write(mymissile->GetServerW());
				bs3.Write(mymissile->GetServerVelocityX());
				bs3.Write(mymissile->GetServerVelocityY());
				bs3.Write(mymissile->GetAngularVelocity());

				rakpeer_->Send(&bs3, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
			}

			if (myenergyball)
			{
				RakNet::BitStream bs4;
				unsigned char msgid3 = ID_UPDATELAST;
				unsigned char deleted = 0;
				bs4.Write(msgid3);
				bs4.Write(myenergyball->GetOwnerID());
				bs4.Write(deleted);
				bs4.Write(myenergyball->GetServerX());
				bs4.Write(myenergyball->GetServerY());
				bs4.Write(myenergyball->GetServerW());
				bs4.Write(myenergyball->GetServerVelocityX());
				bs4.Write(myenergyball->GetServerVelocityY());
				bs4.Write(myenergyball->GetAngularVelocity());

				rakpeer_->Send(&bs4, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
			}

			if (mylast)
			{
				RakNet::BitStream bs5;
				unsigned char msgid4 = ID_UPDATELAST;
				unsigned char deleted = 0;
				bs5.Write(msgid4);
				bs5.Write(mylast->GetOwnerID());
				bs5.Write(deleted);
				bs5.Write(mylast->GetServerX());
				bs5.Write(mylast->GetServerY());
				bs5.Write(mylast->GetServerW());
				bs5.Write(mylast->GetServerVelocityX());
				bs5.Write(mylast->GetServerVelocityY());
				bs5.Write(mylast->GetAngularVelocity());

				rakpeer_->Send(&bs5, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
			}
		}
	}
	return false;
}


/**
* Render Cycle
*
* Clear the screen and render all the ships
*/
void Application::Render()
{
	std::string fps;
	//char temp[BUFFERSIZE];

	hge_->Gfx_BeginScene();
	hge_->Gfx_Clear(0);
	if (!rejected) {
		// render spaceships
		ShipList::iterator itr;
		for (itr = ships_.begin(); itr != ships_.end(); itr++)
		{
			(*itr)->Render();
		}

		// render missiles
		if (mymissile)
		{
			mymissile->Render();
		}
		if (myenergyball)
		{
			myenergyball->Render();
		}
		if (mylast)
		{
			mylast->Render();
		}
		MissileList::iterator itr2;
		for (itr2 = missiles_.begin(); itr2 != missiles_.end(); itr2++)
		{
			(*itr2)->Render();
		}

		BoomList::iterator itr3;
		for (itr3 = booms_.begin(); itr3 != booms_.end(); itr3++)
		{
			(*itr3)->Render();
		}

		TimebombList::iterator itr4;
		for (itr4 = bombs_.begin(); itr4 != bombs_.end(); itr4++)
		{
			(*itr4)->Render();
		}

		EnergyballList::iterator itr5;
		for (itr5 = energyballs_.begin(); itr5 != energyballs_.end(); itr5++)
		{
			(*itr5)->Render();
		}

		PowerupsList::iterator itr6;
		for (itr6 = powerupslist.begin(); itr6 != powerupslist.end(); itr6++)
		{
			(*itr6)->Render();
		}

		LastList::iterator itr7;
		for (itr7 = lastlist_.begin(); itr7 != lastlist_.end(); itr7++)
		{
			(*itr7)->Render();
		}
	}
	if (rejected)
	{
		font_->printf(400, 240, HGETEXT_LEFT, "%s", rejectfont.c_str());
	}

	hge_->Gfx_EndScene();
}


/** 
* Main game loop
*
* Processes user input events
* Supposed to process network events
* Renders the ships
*
* This is a static function that is called by the graphics
* engine every frame, hence the need to loop through the
* global namespace to find itself.
*/
bool Application::Loop()
{
	Global::application->Render();
	return Global::application->Update();
}

/**
* Shuts down the graphics and network system
*/

void Application::Shutdown()
{
	ShipList::iterator itr = ships_.begin();

	while( itr != ships_.end() )
	{
		delete *itr;
		ships_.erase(itr);
		itr = ships_.begin();
		if( itr == ships_.end() ) break;
//		itr++;
	//	if( itr == ships_.end()

	}
/*
	for (ShipList::iterator itr = ships_.begin(); itr != ships_.end(); itr++)
	{			
		delete *itr;
		ships_.erase(itr);
		itr = ships_.begin();
	}
*/
	hge_->System_Shutdown();
	hge_->Release();
}

/** 
* Kick starts the everything, called from main.
*/
void Application::Start()
{
	if (Init())
	{
		hge_->System_Start();
	}
}

bool Application::SendInitialPosition()
{
	RakNet::BitStream bs;
	unsigned char msgid = ID_INITIALPOS;
	bs.Write(msgid);
	bs.Write(ships_.at(0)->GetX());
	bs.Write(ships_.at(0)->GetY());
    bs.Write( ships_.at( 0 )->GetType( ) );
	
	std::cout << "Sending pos " << ships_.at(0)->GetX() << ", " << ships_.at(0)->GetY() << ", type " << ships_.at(0)->GetType() << std::endl;

	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

	return true;
}

void Application::CreateMissile(float x, float y, float w, int id)
{
	RakNet::BitStream bs;
	unsigned char msgid;
	unsigned char deleted=0;

	if( id != ships_.at(0)->GetID() )
	{
		// not my ship
		missiles_.push_back( new Missile("missile.png", x, y, w, id ) );
	}
	else
	{		
		if( have_missile )
		{
			//locate existing missile

			// send network command to delete across all clients
				deleted=1;
				msgid = ID_UPDATEMISSILE;
				bs.Write(msgid);
				bs.Write(id);
				bs.Write(deleted);
				bs.Write(x);
				bs.Write(y);
				bs.Write(w);
				
				rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

				have_missile = false;
		}
		
		// add new missile to list
		mymissile = new Missile("missile.png", x, y, w, id );
			
		// send network command to add new missile
		bs.Reset();
		msgid = ID_NEWMISSILE;
		bs.Write(msgid);
		bs.Write(id);
		bs.Write(x);
		bs.Write(y);
		bs.Write(w);
		
		rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

		have_missile = true;
	}
}

// Assignment 2
void Application::CreateBoom(float x, float y, float lifetime)
{
	RakNet::BitStream bs;
	unsigned char msgid;
	
	// add new boom to list
	booms_.push_back(new Boom("boom.png", x, y, lifetime));

	// send network command to add new missile
	bs.Reset();
	msgid = ID_NEWBOOM;
	bs.Write(msgid);
	bs.Write(x);
	bs.Write(y);
	bs.Write(lifetime);

	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
}

void Application::CreateBomb(float x, float y, float explosionRadius)
{
	RakNet::BitStream bs;
	unsigned char msgid;

	// add new boom to list
	bombs_.push_back(new Timebomb("bomb.png", x, y, explosionRadius));

	// send network command to add new missile
	bs.Reset();
	msgid = ID_NEWBOMB;
	bs.Write(msgid);
	bs.Write(x);
	bs.Write(y);
	bs.Write(explosionRadius);

	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

}

void Application::CreateEnergyBall(float x, float y, float w, int id)
{
	RakNet::BitStream bs;
	unsigned char msgid;
	unsigned char deleted = 0;

	if (id != ships_.at(0)->GetID())
	{
		// not my ship
		energyballs_.push_back(new Energyball("energyball.png", x, y, w, id));
	}
	else
	{
		if (have_energyball)
		{
			//locate existing missile

			// send network command to delete across all clients
			deleted = 1;
			msgid = ID_UPDATEENERGYBALL;
			bs.Write(msgid);
			bs.Write(id);
			bs.Write(deleted);
			bs.Write(x);
			bs.Write(y);
			bs.Write(w);

			rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

			have_energyball = false;
		}

		// add new missile to list
		myenergyball = new Energyball("energyball.png", x, y, w, id);

		// send network command to add new missile
		bs.Reset();
		msgid = ID_NEWENERGYBALL;
		bs.Write(msgid);
		bs.Write(id);
		bs.Write(x);
		bs.Write(y);
		bs.Write(w);

		rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

		have_energyball = true;
	}
}

void Application::CreateLastWeapon(float x, float y, float w, int id)
{
	RakNet::BitStream bs;
	unsigned char msgid;
	unsigned char deleted = 0;

	if (id != ships_.at(0)->GetID())
	{
		// not my ship
		lastlist_.push_back(new Last("last.png", x, y, w, id));
	}
	else
	{
		if (have_last)
		{
			//locate existing missile

			// send network command to delete across all clients
			deleted = 1;
			msgid = ID_UPDATELAST;
			bs.Write(msgid);
			bs.Write(id);
			bs.Write(deleted);
			bs.Write(x);
			bs.Write(y);
			bs.Write(w);

			rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

			have_last = false;
		}

		// add new missile to list
		mylast = new Last("last.png", x, y, w, id);

		// send network command to add new missile
		bs.Reset();
		msgid = ID_NEWLAST;
		bs.Write(msgid);
		bs.Write(id);
		bs.Write(x);
		bs.Write(y);
		bs.Write(w);

		rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

		have_last = true;
	}
}

void Application::MissileHit(int shipID)
{
	RakNet::BitStream bs;
	unsigned char msgid;

	int health;
	for (ShipList::iterator itr = ships_.begin(); itr != ships_.end(); ++itr) {
		if ((*itr)->GetID() == shipID && 
			shipID != ships_.at(0)->GetID()) {

			if ((*itr)->GetHealth() > 0) {
				health = (*itr)->GetHealth() - 1;
				(*itr)->SetHealth(health);
				// send network command to add new missile
				bs.Reset();
				msgid = ID_MISSILEHIT;
				bs.Write(msgid);
				bs.Write(shipID);
				bs.Write(health);
				rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
			}
			break;
		}
	}

}

void Application::BombHit(int shipID)
{
	RakNet::BitStream bs;
	unsigned char msgid;
	int health;
	for (ShipList::iterator itr = ships_.begin(); itr != ships_.end(); ++itr) {
		if ((*itr)->GetID() == shipID) {

			if ((*itr)->GetHealth() > 0) {
				if ((*itr)->GetID() != ships_.at(0)->GetID()) {
					health = (*itr)->GetHealth() - 2;
					(*itr)->SetHealth(health);
					// send network command to add new missile
					bs.Reset();
					msgid = ID_MISSILEHIT;
					bs.Write(msgid);
					bs.Write(shipID);
					bs.Write(health);
					rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
				}

			}
			break;
		}
	}
}

void Application::EnergyBallHit(int shipID)
{
	RakNet::BitStream bs;
	unsigned char msgid;
	int health;
	for (ShipList::iterator itr = ships_.begin(); itr != ships_.end(); ++itr) {
		if ((*itr)->GetID() == shipID &&
			shipID != ships_.at(0)->GetID()) {

			if ((*itr)->GetHealth() > 0) {
				health = (*itr)->GetHealth() - 1;
				(*itr)->SetHealth(health);
				// send network command to add new missile
				bs.Reset();
				msgid = ID_MISSILEHIT;
				bs.Write(msgid);
				bs.Write(shipID);
				bs.Write(health);
				rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
			}
			break;
		}
	}
}

void Application::LastHit(int shipID)
{
	RakNet::BitStream bs;
	unsigned char msgid;

	int health;
	for (ShipList::iterator itr = ships_.begin(); itr != ships_.end(); ++itr) {
		if ((*itr)->GetID() == shipID &&
			shipID != ships_.at(0)->GetID()) {

			if ((*itr)->GetHealth() > 0) {
				health = (*itr)->GetHealth() - 2;
				(*itr)->SetHealth(health);
				// send network command to add new missile
				bs.Reset();
				msgid = ID_LASTHIT;
				bs.Write(msgid);
				bs.Write(shipID);
				bs.Write(health);
				rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
			}
			break;
		}
	}
}

void Application::CollectHealth(int shipID)
{
	RakNet::BitStream bs;
	unsigned char msgid;
	int health;

	for (ShipList::iterator itr = ships_.begin(); itr != ships_.end(); ++itr) {
		if ((*itr)->GetID() == shipID &&
			shipID == ships_.at(0)->GetID()) {

				health = (*itr)->GetHealth() + 1;
				(*itr)->SetHealth(health);
				// send network command to add new missile
				bs.Reset();
				msgid = ID_COLLECTHEALTH;
				bs.Write(msgid);
				bs.Write(shipID);
				bs.Write(health);
				rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
			break;
		}
	}
}

void Application::CollectEXP(int shipID)
{
	RakNet::BitStream bs;
	unsigned char msgid;
	int level;

	for (ShipList::iterator itr = ships_.begin(); itr != ships_.end(); ++itr) {
		if ((*itr)->GetID() == shipID &&
			shipID == ships_.at(0)->GetID()) {

			level = (*itr)->GetLevel() + 1;
			(*itr)->SetLevel(level);
			// send network command to add new missile
			bs.Reset();
			msgid = ID_COLLECTEXP;
			bs.Write(msgid);
			bs.Write(shipID);
			bs.Write(level);
			rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
			break;
		}
	}
}

void Application::Reset()
{
	while (missiles_.size() > 0)
	{
		Missile *m = missiles_.back();
		delete m;
		missiles_.pop_back();
	}
	while (booms_.size() > 0)
	{
		Boom *m = booms_.back();
		delete m;
		booms_.pop_back();
	}
	while (bombs_.size() > 0)
	{
		Timebomb *m = bombs_.back();
		delete m;
		bombs_.pop_back();
	}
	while (energyballs_.size() > 0)
	{
		Energyball *m = energyballs_.back();
		delete m;
		energyballs_.pop_back();
	}
	while (powerupslist.size() > 0)
	{
		PowerupBaseClass* m = powerupslist.back();
		delete m;
		powerupslist.pop_back();
	}
	while (lastlist_.size() > 0)
	{
		Last* m = lastlist_.back();
		delete m;
		lastlist_.pop_back();
	}

	mymissile = nullptr;
	myenergyball = nullptr;
	mylast = nullptr;
	have_missile = false;
	have_energyball = false;
	have_last = false;
}

void Application::SwitchPositions()
{
	RakNet::BitStream bs;
	unsigned char msgid;

	for (ShipList::iterator itr = ships_.begin(); itr != ships_.end(); ++itr) {
		if ((*itr)->GetID() == ships_.at(0)->GetID())
			continue;
		else
		{
			bs.Reset();
			msgid = ID_SWITCH;
			bs.Write(msgid);
			bs.Write((*itr)->GetID());
			bs.Write(ships_.at(0)->GetX());
			bs.Write(ships_.at(0)->GetY());
			bs.Write(ships_.at(0)->GetServerX());
			bs.Write(ships_.at(0)->GetServerY());
			bs.Write(ships_.at(0)->GetServerW());
			bs.Write(ships_.at(0)->GetServerVelocityX());
			bs.Write(ships_.at(0)->GetServerVelocityY());
			bs.Write(ships_.at(0)->GetAngularVelocity());

			bs.Write((*itr)->GetX());
			bs.Write((*itr)->GetY());
			bs.Write((*itr)->GetServerX());
			bs.Write((*itr)->GetServerY());
			bs.Write((*itr)->GetServerW());
			bs.Write((*itr)->GetServerVelocityX());
			bs.Write((*itr)->GetServerVelocityY());
			bs.Write((*itr)->GetAngularVelocity());

			rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
			break;
		}
	}
}