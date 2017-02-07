#include <math.h>

#include "hge.h"
#include "hgeSprite.h"

#include "ship.h"
#include "energyball.h"

#include <iostream>
Energyball::Energyball(char* filename, float x, float y, float w, int shipid) :
angular_velocity(5)
#ifdef INTERPOLATEMOVEMENT
, server_w_(0)
, client_w_(0)
, server_velx_(0)
, server_vely_(0)
, ratio_(1)
#endif
{
	HGE* hge = hgeCreate(HGE_VERSION);
	tex_ = hge->Texture_Load(filename);
	hge->Release();
	sprite_.reset(new hgeSprite(tex_, 0, 0, 42, 42));
	sprite_->SetHotSpot(21, 21);
#ifdef INTERPOLATEMOVEMENT
	x_ = server_x_ = client_x_ = x;
	y_ = server_y_ = client_y_ = y;
#else
	x_ = x;
	y_ = y;
#endif
	server_w_ = w_ = w;
	ownerid = shipid;

	server_velx_ = velocity_x_ = 200.0f * cosf(w_);
	server_vely_ = velocity_y_ = 200.0f * sinf(w_);

	x_ += velocity_x_ * 0.5f;
	y_ += velocity_y_ * 0.5f;

	xflip = false;
	xflip = false;
	xmax = 50.f;
	ymax = 50.f;
}

Energyball::~Energyball()
{
	HGE* hge = hgeCreate(HGE_VERSION);
	hge->Texture_Free(tex_);
	hge->Release();
}

Ship* Energyball::Update(std::vector<Ship*> &shiplist, float timedelta)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	float pi = 3.141592654f * 2;
	
	if (!xflip) {
		server_velx_ -= 100 * timedelta;
		if (server_velx_ < -xmax)
			xflip = true;
	}
	else if (xflip) {
		server_velx_ += 100 * timedelta;
		if (server_velx_ > xmax)
			xflip = false;
	}
	if (!yflip) {
		server_vely_ -= 100 * timedelta;
		if (server_vely_ < -ymax)
			yflip = true;
	}
	else if (yflip) {
		server_vely_ += 100 * timedelta;
		if (server_vely_ > ymax)
			yflip = false;
	}

	server_w_ += angular_velocity * timedelta;

	if (server_w_ > pi)
		server_w_ -= pi;

	if (server_w_ < 0.0f)
		server_w_ += pi;

	client_w_ += angular_velocity * timedelta;

	if (client_w_ > pi)
		client_w_ -= pi;

	if (client_w_ < 0.0f)
		client_w_ += pi;

	w_ = ratio_ * server_w_ + (1 - ratio_) * client_w_;

	if (w_ > pi)
		w_ -= pi;

	if (w_ < 0.0f)
		w_ += pi;

	float screenwidth = static_cast<float>(hge->System_GetState(HGE_SCREENWIDTH));
	float screenheight = static_cast<float>(hge->System_GetState(HGE_SCREENHEIGHT));
	float spritewidth = sprite_->GetWidth();
	float spriteheight = sprite_->GetHeight();


	server_x_ += server_velx_ * timedelta;
	server_y_ += server_vely_ * timedelta;

	// Deadreckon
	if (server_x_ < -spritewidth / 2)
		server_x_ += screenwidth + spritewidth;
	else if (server_x_ > screenwidth + spritewidth / 2)
		server_x_ -= screenwidth + spritewidth;

	if (server_y_ < -spriteheight / 2)
		server_y_ += screenheight + spriteheight;
	else if (server_y_ > screenheight + spriteheight / 2)
		server_y_ -= screenheight + spriteheight;


	client_x_ += velocity_x_ * timedelta;
	client_y_ += velocity_y_ * timedelta;

	// Deadreckon
	if (client_x_ < -spritewidth / 2)
		client_x_ += screenwidth + spritewidth;
	else if (client_x_ > screenwidth + spritewidth / 2)
		client_x_ -= screenwidth + spritewidth;

	if (client_y_ < -spriteheight / 2)
		client_y_ += screenheight + spriteheight;
	else if (client_y_ > screenheight + spriteheight / 2)
		client_y_ -= screenheight + spriteheight;


	x_ = ratio_ * server_x_ + (1 - ratio_) * client_x_;
	y_ = ratio_ * server_y_ + (1 - ratio_) * client_y_;

	if (ratio_ < 1)
	{
		ratio_ += timedelta * 4;
		if (ratio_ > 1)
			ratio_ = 1;
	}

	if (x_ < -spritewidth / 2)
		x_ += screenwidth + spritewidth;
	else if (x_ > screenwidth + spritewidth / 2)
		x_ -= screenwidth + spritewidth;

	if (y_ < -spriteheight / 2)
		y_ += screenheight + spriteheight;
	else if (y_ > screenheight + spriteheight / 2)
		y_ -= screenheight + spriteheight;

	for (std::vector<Ship*>::iterator thisship = shiplist.begin();
		thisship != shiplist.end(); thisship++)
	{
		if (HasCollided((*(*thisship))))
		{
			// Assignment 2 
			if (ownerid == (*thisship)->GetID())
				continue;

			// if both are stuck
			return (*thisship);
		}
	}
	return nullptr;
}

void Energyball::Render()
{
	sprite_->RenderEx(x_, y_, w_);
}

bool Energyball::HasCollided(Ship &ship)
{
	sprite_->GetBoundingBox(x_, y_, &collidebox);

	return collidebox.Intersect(ship.GetBoundingBox());
}