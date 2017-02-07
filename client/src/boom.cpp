#include <math.h>

#include "hge.h"
#include "hgeSprite.h"

#include "boom.h"

#include <iostream>
Boom::Boom(char* filename, float x, float y, float lifetime)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	tex_ = hge->Texture_Load(filename);
	hge->Release();
	sprite_.reset(new hgeSprite(tex_, 0, 0, 40, 40));
	sprite_->SetHotSpot(20, 20);
	timer_ = 0.f;
	x_ = x;
	y_ = y;
	lifetime_ = lifetime;
}

Boom::~Boom()
{
	HGE* hge = hgeCreate(HGE_VERSION);
	hge->Texture_Free(tex_);
	hge->Release();
}

bool Boom::Update(float timedelta)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	timer_ += timedelta;
	if (timer_ > lifetime_) {
		return true;
	}
	return false;
}

void Boom::Render()
{
	sprite_->RenderEx(x_, y_, 0);
}