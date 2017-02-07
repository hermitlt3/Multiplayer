#include <math.h>

#include "hge.h"
#include "hgeSprite.h"

#include "ship.h"
#include "timebomb.h"

#include <iostream>
#include <sstream>

Timebomb::Timebomb(char* filename, float x, float y, float explosionRadius)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	tex_ = hge->Texture_Load(filename);
	hge->Release();
	sprite_.reset(new hgeSprite(tex_, 0, 0, 32, 27));
	sprite_->SetHotSpot(16, 13.5f);
	font_.reset(new hgeFont("font1.fnt"));
	font_->SetScale(0.5);

	x_ = x;
	y_ = y;
	explosionRadius_ = explosionRadius;
	destructTimer = 5.f;
	toDelete = false;
}

Timebomb::~Timebomb()
{
	HGE* hge = hgeCreate(HGE_VERSION);
	hge->Texture_Free(tex_);
	hge->Release();
}

std::vector<Ship*> Timebomb::Update(std::vector<Ship*> &shiplist, float timedelta)
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

	std::vector<Ship*> results;
	destructTimer -= timedelta;
	if (destructTimer <= 0.f) {
		toDelete = true;
		for (std::vector<Ship*>::iterator thisship = shiplist.begin();
			thisship != shiplist.end(); thisship++)
		{
			if (InExplosionRadius((*(*thisship))))
			{
				results.push_back(*thisship);
			}
		}
		return results;
	}
	return results;
}

void Timebomb::Render()
{
	sprite_->RenderEx(x_, y_, 0);
	std::ostringstream ss;
	ss.precision(1);
	ss << (int)destructTimer + 1;
	font_->printf(x_ + 5, y_ + 5, HGETEXT_LEFT, "%s", ss.str().c_str());

}

bool Timebomb::InExplosionRadius(Ship &ship)
{
	sprite_->GetSelfDefinedBoundingBox(x_, y_, explosionRadius_ / 2, explosionRadius_ / 2, & explosionRect);
	return explosionRect.Intersect(ship.GetBoundingBox());
}