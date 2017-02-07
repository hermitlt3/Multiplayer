#include <math.h>

#include "hge.h"
#include "hgeSprite.h"

#include "ship.h"
#include "Exppack.h"
#include "PowerupBaseClass.h"
#include <iostream>
ExpPack::ExpPack(char* filename, float x, float y, float duration)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	tex_ = hge->Texture_Load(filename);
	hge->Release();
	sprite_.reset(new hgeSprite(tex_, 0, 0, 20, 20));
	sprite_->SetHotSpot(10, 10);
	this->duration = duration;
	timer = duration;
	type = PowerupBaseClass::EXP;
	x_ = x;
	y_ = y;
}

ExpPack::~ExpPack()
{
	HGE* hge = hgeCreate(HGE_VERSION);
	hge->Texture_Free(tex_);
	hge->Release();
}

Ship* ExpPack::Update(std::vector<Ship*> &shiplist, float timedelta)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	float pi = 3.141592654f * 2;

	float screenwidth = static_cast<float>(hge->System_GetState(HGE_SCREENWIDTH));
	float screenheight = static_cast<float>(hge->System_GetState(HGE_SCREENHEIGHT));
	float spritewidth = sprite_->GetWidth();
	float spriteheight = sprite_->GetHeight();

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

			// if both are stuck
			return (*thisship);
		}
	}
	return nullptr;
}

void ExpPack::Render()
{
	sprite_->RenderEx(x_, y_, 0);
}

bool ExpPack::HasCollided(Ship &ship)
{
	sprite_->GetBoundingBox(x_, y_, &collidebox);

	return collidebox.Intersect(ship.GetBoundingBox());
}