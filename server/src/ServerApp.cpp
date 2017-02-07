#include <iostream>

#include "RakNetworkFactory.h"
#include "RakPeerInterface.h"
#include "Bitstream.h"
#include "GetTime.h"

#include "config.h"
#include "MyMsgIDs.h"

#include "ServerApp.h"

#define MAX_PLAYERS		2
#define MAX_POWERUP		2

ServerApp::ServerApp() : 
	rakpeer_(RakNetworkFactory::GetRakPeerInterface()),
	newID(0)
{
    rakpeer_->Startup( 100, 30, &SocketDescriptor( DFL_PORTNUMBER, 0 ), 1 );
    rakpeer_->SetMaximumIncomingConnections( DFL_MAX_CONNECTION );
	rakpeer_->SetOccasionalPing(true);
	std::cout << "Server Started" << std::endl;
	fixedTime = 0.f;
	timer = 0.f;
	randomTime = (float)(rand() % 5) + 10.f;
}

ServerApp::~ServerApp()
{
	rakpeer_->Shutdown(100);
	RakNetworkFactory::DestroyRakPeerInterface(rakpeer_);
}

void ServerApp::Loop()
{
	float dt = (RakNet::GetTime() - fixedTime) / 1000.f;
	fixedTime = RakNet::GetTime();
	timer += dt;
	if (timer > randomTime) {
		timer = 0.f;
		randomTime = (float)(rand() % 5) + 10.f;
		SpawnPowerUps();
	}
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
		case ID_NEW_INCOMING_CONNECTION:
			SendWelcomePackage(packet->systemAddress);
			break;

		case ID_DISCONNECTION_NOTIFICATION:
		case ID_CONNECTION_LOST:
			SendDisconnectionNotification(packet->systemAddress);
			break;

		case ID_INITIALPOS:
			{
				float x_, y_;
                int type_;
				std::cout << "ProcessInitialPosition" << std::endl;
				bs.Read( x_ );
				bs.Read( y_ );
                bs.Read( type_ );
				ProcessInitialPosition( packet->systemAddress, x_, y_, type_);
			}
			break;

		case ID_MOVEMENT:

		case ID_NEWMISSILE:
		case ID_UPDATEMISSILE:
						
		// Assignment 2
		case ID_MAX_PLAYERS:
		case ID_MISSILEHIT:
		case ID_BOMBHIT:
		case ID_ENERGYBALLHIT:
		case ID_LASTHIT:

		case ID_NEWBOOM:
		case ID_NEWBOMB:
		case ID_NEWENERGYBALL:
		case ID_UPDATEENERGYBALL:

		case ID_NEWLAST:
		case ID_UPDATELAST:
		case ID_COLLECTHEALTH:
		case ID_COLLECTEXP:
			bs.ResetReadPointer();
			rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);
			break;

		default:
			std::cout << "Unhandled Message Identifier: " << (int)msgid << std::endl;
		}

		rakpeer_->DeallocatePacket(packet);
	}
}

void ServerApp::SendWelcomePackage(SystemAddress& addr)
{
	++newID;
	unsigned int shipcount = static_cast<unsigned int>(clients_.size());

	if (shipcount < MAX_PLAYERS) {
		unsigned char msgid = ID_WELCOME;

		RakNet::BitStream bs;
		bs.Write(msgid);
		bs.Write(newID);
		bs.Write(shipcount);

		for (ClientMap::iterator itr = clients_.begin(); itr != clients_.end(); ++itr)
		{
			std::cout << "Ship " << itr->second.id << " pos" << itr->second.x_ << " " << itr->second.y_ << std::endl;
			bs.Write(itr->second.id);
			bs.Write(itr->second.x_);
			bs.Write(itr->second.y_);
			bs.Write(itr->second.type_);
		}

		rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, false);

		bs.Reset();

		GameObject newobject(newID);

		clients_.insert(std::make_pair(addr, newobject));

		std::cout << "New guy, assigned id " << newID << std::endl;
	}
	else
	{
		--newID;
		SendMaxPlayers(addr);
	}
}

void ServerApp::SendDisconnectionNotification(SystemAddress& addr)
{
	ClientMap::iterator itr = clients_.find(addr);
	if (itr == clients_.end())
		return;

	unsigned char msgid = ID_LOSTSHIP;
	RakNet::BitStream bs;
	bs.Write(msgid);
	bs.Write(itr->second.id);

	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, true);

	std::cout << itr->second.id << " has left the building" << std::endl;

	clients_.erase(itr);

}

void ServerApp::ProcessInitialPosition( SystemAddress& addr, float x_, float y_, int type_ ){
	unsigned char msgid;
	RakNet::BitStream bs;
	ClientMap::iterator itr = clients_.find(addr);
	if (itr == clients_.end())
		return;

	itr->second.x_ = x_;
	itr->second.y_ = y_;
    itr->second.type_ = type_;

	std::cout << "Received pos " << itr->second.x_ << " " << itr->second.y_ << std::endl;
    std::cout << "Received type " << itr->second.type_ << std::endl;

	msgid = ID_NEWSHIP;
    bs.Write( msgid );
    bs.Write( itr->second.id );
    bs.Write( itr->second.x_ );
    bs.Write( itr->second.y_ );
    bs.Write( itr->second.type_ );

	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, true);
}

void ServerApp::UpdatePosition( SystemAddress& addr, float x_, float y_ )
{
    ClientMap::iterator itr = clients_.find( addr );
    if( itr == clients_.end( ) )
        return;

    itr->second.x_ = x_;
    itr->second.y_ = y_;
}

// Assignment 2
void ServerApp::SendMaxPlayers(SystemAddress & addr)
{
	unsigned char msgid = ID_MAX_PLAYERS;

	RakNet::BitStream bs;
	bs.Write(msgid);

	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, false);

	bs.Reset();
}

void ServerApp::SpawnPowerUps()
{
	unsigned char msgid = ID_NEWPOWERUPS;

	RakNet::BitStream bs;

	float randomPosX = (float)(rand() % 500 + 100);
	float randomPosY = (float)(rand() % 400 + 100);
	int randomType = (int)(rand() % MAX_POWERUP);

	bs.Write(msgid);
	bs.Write(randomType);
	bs.Write(randomPosX);
	bs.Write(randomPosY);

	for (ClientMap::iterator itr = clients_.begin(); itr != clients_.end(); ++itr) {
		rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, itr->first, false);
	}
	bs.Reset();
}