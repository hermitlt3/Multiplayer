#include <math.h>

#include "hge.h"
#include "hgeSprite.h"

#include "boom.h"

#include <iostream>
Boom::Boom(char* filename, float x, float y)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	tex_ = hge->Texture_Load(filename);
	hge->Release();
	sprite_.reset(new hgeSprite(tex_, 0, 0, 40, 20));
	sprite_->SetHotSpot(20, 10);
	timer_ = 0.f;
	x_ = x;
	y_ = y;
}

Boom::~Boom()
{
	HGE* hge = hgeCreate(HGE_VERSION);
	hge->Texture_Free(tex_);
	hge->Release();
}

bool Boom::Update(std::vector<Boom*> &shiplist, float timedelta)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	timer_ += timedelta;
	if (timer_ > 1.0) {
		return true;
	}
	return false;
}

void Boom::Render()
{
	sprite_->RenderEx(x_, y_, 0);
}