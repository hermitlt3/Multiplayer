#include "hge.h"
#include "hgeFont.h"

#include "textbox.h"

TextBox::TextBox(char* filename)
:posx_(0), posy_(0)
{
	font_.reset(new hgeFont(filename));
	font_->SetScale( 0.5 );
}

TextBox::~TextBox()
{
	font_.release();
}

void TextBox::Render()
{
	font_->printf(posx_, posy_, HGETEXT_LEFT, "%s",
              mytext_.c_str());
}

void TextBox::SetText(const char * text)
{
	mytext_.clear();
	mytext_ = text;
}

void TextBox::SetPos( float x, float y )
{
	posx_ = x;
	posy_ = y;
}