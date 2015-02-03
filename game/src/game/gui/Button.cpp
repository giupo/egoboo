#include "game/gui/Button.hpp"
#include "game/ui.h"

#include "game/menu.h"

Button::Button(int hotkey) :
    _buttonText(),
    _clickSoundID("button.wav"),
    _mouseOver(false),
    _onClickFunction(nullptr),
    _hotkey(hotkey),
    _slidyButtonTargetX(0.0f)
{
}

Button::Button(const std::string &buttonText, int hotkey) : Button(hotkey)
{
    _buttonText = buttonText;
}

void Button::setText(const std::string &text)
{
    _buttonText = text;
}

void Button::draw()
{
    GLXvector4f buttonColour;

    if(!_onClickFunction)
    {
        buttonColour[0] = 0.25;
        buttonColour[1] = 0.25;
        buttonColour[2] = 0.25;
        buttonColour[3] = 0.6f;
    }

    else if(_mouseOver)
    {
        buttonColour[0] = 0.54;
        buttonColour[1] = 0.00;
        buttonColour[2] = 0.00;
        buttonColour[3] = 1.0f;
    }
    else
    {
        buttonColour[0] = 0.66f;
        buttonColour[1] = 0.00;
        buttonColour[2] = 0.00;
        buttonColour[3] = 0.6f;
    }

    //Update slidy button effect
    if(_slidyButtonTargetX > 0.0f) {
        const float SLIDY_LERP = getWidth() / 10.0f;
        _slidyButtonTargetX -= SLIDY_LERP;
        setX(getX() + SLIDY_LERP);
    }

    // Draw the button
    GL_DEBUG( glDisable )( GL_TEXTURE_2D );
    
    // convert the virtual coordinates to screen coordinates
    //ui_virtual_to_screen( vx, vy, &x1, &y1 );
    //ui_virtual_to_screen( vx + vwidth, vy + vheight, &x2, &y2 );

    GL_DEBUG( glColor4fv )( buttonColour );
    GL_DEBUG( glBegin )( GL_QUADS );
    {
        GL_DEBUG( glVertex2f )( getX(), getY() );
        GL_DEBUG( glVertex2f )( getX(), getY()+getHeight() );
        GL_DEBUG( glVertex2f )( getX()+getWidth(), getY()+getHeight() );
        GL_DEBUG( glVertex2f )( getX()+getWidth(), getY() );
    }
    GL_DEBUG_END();

    GL_DEBUG( glEnable )( GL_TEXTURE_2D );

    //Draw centered text in button
    if(!_buttonText.empty())
    {
        int textWidth, textHeight;
        fnt_getTextSize(ui_getFont(), _buttonText.c_str(), &textWidth, &textHeight);

        GL_DEBUG( glColor3f )(1, 1, 1);
        fnt_drawText_OGL_immediate(ui_getFont(), {0xFF, 0xFF, 0xFF, 0x00}, getX() + (getWidth()-textWidth)/2, getY() + (getHeight()-textHeight)/2, "%s", _buttonText.c_str());        
    }
}

bool Button::notifyMouseMoved(const int x, const int y)
{
    _mouseOver = contains(x, y);

    return false;
}

bool Button::notifyMouseClicked(const int button, const int x, const int y)
{
    if(_mouseOver && button == SDL_BUTTON_LEFT)
    {
        doClick();
        return true;
    }   

    return false;
}

void Button::doClick()
{
    if(!_onClickFunction || !isEnabled()) return;

    if(!_clickSoundID.empty()) 
    {
        //_audioSystem.playSound(_clickSoundID);
    }

    _mouseOver = true;
    _onClickFunction();
}

void Button::setOnClickFunction(const std::function<void()> onClick)
{
    _onClickFunction = onClick;
}

bool Button::notifyKeyDown(const int keyCode)
{
    //No hotkey assigned to this button
    if(_hotkey == SDLK_UNKNOWN) return false;

    //Hotkey pressed?
    if(keyCode == _hotkey)
    {
        doClick();
        return true;
    }

    return false;
}

void Button::setClickSound(const std::string &soundID)
{
    _clickSoundID = soundID;
}

void Button::beginSlidyButtonEffect()
{
    _slidyButtonTargetX = getWidth();
    setX(getX() - getWidth());
}