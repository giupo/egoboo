//********************************************************************************************
//*
//*    This file is part of Egoboo.
//*
//*    Egoboo is free software: you can redistribute it and/or modify it
//*    under the terms of the GNU General Public License as published by
//*    the Free Software Foundation, either version 3 of the License, or
//*    (at your option) any later version.
//*
//*    Egoboo is distributed in the hope that it will be useful, but
//*    WITHOUT ANY WARRANTY; without even the implied warranty of
//*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//*    General Public License for more details.
//*
//*    You should have received a copy of the GNU General Public License
//*    along with Egoboo.  If not, see <http:// www.gnu.org/licenses/>.
//*
//********************************************************************************************

/* Egoboo - menu.c
* Implements the main menu tree, using the code in Ui.*
*/

#include "menu.h"

#include "ui.h"
#include "graphic.h"
#include "log.h"
#include "proto.h"
#include "sound.h"
#include "input.h"
#include "char.h"
#include "file_common.h"
#include "particle.h"
#include "link.h"
#include "mad.h"
#include "game.h"

#include "SDL_extensions.h"

#include "egoboo_fileutil.h"
#include "egoboo_setup.h"
#include "egoboo.h"

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

#define INVALID_PLA MAXPLAYER

enum MenuStates
{
    MM_Begin,
    MM_Entering,
    MM_Running,
    MM_Leaving,
    MM_Finish
};

#define MENU_STACK_COUNT 256
#define MAXWIDGET        100

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
// "Slidy" buttons used in some of the menus.  They're shiny.
struct
{
    float lerp;
    int top;
    int left;
    char **buttons;
} SlidyButtonState;

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
static int menu_stack_index = 0;
static int menu_stack[MENU_STACK_COUNT];

static int  mnu_whichMenu       = MainMenu;

bool_t mnu_draw_background = btrue;

//static int         mnu_widgetCount;
static ui_Widget_t mnu_widgetList[MAXWIDGET];

int              loadplayer_count = 0;
LOAD_PLAYER_INFO loadplayer[MAXLOADPLAYER];

int    mnu_selectedPlayerCount = 0;
int    mnu_selectedInput[MAXPLAYER] = {0};
Uint16 mnu_selectedPlayer[MAXPLAYER] = {0};

static int selectedModule = -1;

/* Copyright text variables.  Change these to change how the copyright text appears */
static char copyrightText[] = "Welcome to Egoboo!\nhttp:// egoboo.sourceforge.net\nVersion " VERSION "\n";
static int  copyrightLeft = 0;
static int  copyrightTop  = 0;

/* Options info text variables.  Change these to change how the options text appears */
const char optionsText[] = "Change your audio, input and video\nsettings here.";
static int optionsTextLeft = 0;
static int optionsTextTop  = 0;

/* Button labels.  Defined here for consistency's sake, rather than leaving them as constants */
const char *mainMenuButtons[] =
{
    "New Game",
    "Load Game",
    "Options",
    "Quit",
    ""
};

const char *singlePlayerButtons[] =
{
    "New Player",
    "Load Saved Player",
    "Back",
    ""
};

const char *optionsButtons[] =
{
    "Audio Options",
    "Input Controls",
    "Video Settings",
    "Back",
    ""
};

const char *audioOptionsButtons[] =
{
    "N/A",        // Enable sound
    "N/A",        // Sound volume
    "N/A",        // Enable music
    "N/A",        // Music volume
    "N/A",        // Sound channels
    "N/A",        // Sound buffer
    "Save Settings",
    ""
};

const char *videoOptionsButtons[] =
{
    "N/A",    // Antialaising
    "N/A",    // Color depth
    "N/A",    // Fast & ugly
    "N/A",    // Fullscreen
    "N/A",    // Reflections
    "N/A",    // Texture filtering
    "N/A",    // Shadows
    "N/A",    // Z bit
    "N/A",    // Fog
    "N/A",    //3D effects
    "N/A",    // Multi water layer
    "N/A",    // Max messages
    "N/A",    // Screen resolution
    "Save Settings",
    "N/A",    // Max particles
    ""
};

/* Button position for the "easy" menus, like the main one */
static int buttonLeft = 0;
static int buttonTop = 0;

static bool_t startNewPlayer = bfalse;

/* The font used for drawing text.  It's smaller than the button font */
Font *menuFont = NULL;

static int selectedPlayer = 0;           // Which player is currently selected to play

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
static int  get_skin( const char *filename );
static void load_all_menu_images();

static bool_t menu_stack_push( int menu );
static int    menu_stack_pop();
static void   menu_stack_clear();

static void initSlidyButtons( float lerp, const char *button_text[] );
static void updateSlidyButtons( float deltaTime );
static void drawSlidyButtons();


static bool_t mnu_checkSelectedPlayer( Uint16 player );
static Uint16 mnu_getSelectedPlayer( Uint16 player );
static bool_t mnu_addSelectedPlayer( Uint16 player );
static bool_t mnu_removeSelectedPlayer( Uint16 player );
static bool_t mnu_addSelectedPlayerInput( Uint16 player, Uint32 input );
static bool_t mnu_removeSelectedPlayerInput( Uint16 player, Uint32 input );

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void initSlidyButtons( float lerp, const char *button_text[] )
{
    int i;

    // Figure out where to draw the buttons
    buttonLeft = 40;
    buttonTop = sdl_scr.y - 20;

    for ( i = 0; button_text[i][0] != 0; i++ )
    {
        buttonTop -= 35;
    }

    SlidyButtonState.lerp = lerp;
    SlidyButtonState.buttons = ( char** )button_text;
}

//--------------------------------------------------------------------------------------------
void updateSlidyButtons( float deltaTime )
{
    SlidyButtonState.lerp += ( deltaTime * 1.5f );
}

//--------------------------------------------------------------------------------------------
void drawSlidyButtons()
{
    int i;

    for ( i = 0; SlidyButtonState.buttons[i][0] != 0; i++ )
    {
        int x = buttonLeft - ( 360 - i * 35 )  * SlidyButtonState.lerp;
        int y = buttonTop + ( i * 35 );

        ui_doButton( UI_Nothing, SlidyButtonState.buttons[i], x, y, 200, 30 );
    }
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/** initMenus
* Loads resources for the menus, and figures out where things should
* be positioned.  If we ever allow changing resolution on the fly, this
* function will have to be updated/called more than once.
*/

int initMenus()
{
    menuFont = fnt_loadFont( "basicdat" SLASH_STR "Negatori.ttf", 18 );
    if ( !menuFont )
    {
        log_error( "Could not load the menu font!\n" );
        return 0;
    }

    // intialize the buttons for the main menu
    initSlidyButtons( 0, mainMenuButtons );

    // Figure out where to draw the copyright text
    copyrightLeft = 0;
    copyrightLeft = 0;
    fnt_getTextBoxSize( menuFont, copyrightText, 20, &copyrightLeft, &copyrightTop );
    // Draw the copyright text to the right of the buttons
    copyrightLeft = 280;
    // And relative to the bottom of the screen
    copyrightTop = sdl_scr.y - copyrightTop - 20;

    // Figure out where to draw the options text
    optionsTextLeft = 0;
    optionsTextLeft = 0;
    fnt_getTextBoxSize( menuFont, optionsText, 20, &optionsTextLeft, &optionsTextTop );

    // Draw the options text to the right of the buttons
    optionsTextLeft = 280;
    // And relative to the bottom of the screen
    optionsTextTop = sdl_scr.y - optionsTextTop - 20;

    return 1;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
int doMainMenu( float deltaTime )
{
    static int menuState = MM_Begin;
    static oglx_texture background;
    static oglx_texture logo;

    // static float lerp;
    static int menuChoice = 0;
    float fminw = 1, fminh = 1, fmin = 1;
    static SDL_Rect bg_rect, logo_rect;

    int result = 0;

    switch ( menuState )
    {
        case MM_Begin:

            menuChoice = 0;
            menuState = MM_Entering;

            // set up menu variables
            oglx_texture_new( &background );
            ego_texture_load( &background, "basicdat" SLASH_STR "menu" SLASH_STR "menu_main", TRANSCOLOR );

            // load the menu image
            ego_texture_load( &background, "basicdat" SLASH_STR "menu" SLASH_STR "menu_main", INVALID_KEY );

            // load the logo image
            ego_texture_load( &logo,       "basicdat" SLASH_STR "menu" SLASH_STR "menu_logo", INVALID_KEY );

            // calculate the centered position of the background
            fminw = (float) MIN(sdl_scr.x, background.imgW) / (float) background.imgW;
            fminh = (float) MIN(sdl_scr.y, background.imgH) / (float) background.imgW;
            fmin  = MIN(fminw, fminh);

            bg_rect.w = background.imgW * fmin;
            bg_rect.h = background.imgH * fmin;
            bg_rect.x = (sdl_scr.x - bg_rect.w) * 0.5f;
            bg_rect.y = (sdl_scr.y - bg_rect.h) * 0.5f;

            // calculate the position of the logo
            fmin  = MIN(bg_rect.w * 0.5f / logo.imgW, bg_rect.h * 0.5f / logo.imgH);

            logo_rect.x = bg_rect.x;
            logo_rect.y = bg_rect.y;
            logo_rect.w = logo.imgW * fmin;
            logo_rect.h = logo.imgH * fmin;

            initSlidyButtons( 1.0f, mainMenuButtons );
            // let this fall through into MM_Entering

        case MM_Entering:
            // do buttons sliding in animation, and background fading in
            // background
            GL_DEBUG(glColor4f)(1, 1, 1, 1 - SlidyButtonState.lerp );

            if ( mnu_draw_background )
            {
                ui_drawImage( 0, &background, bg_rect.x,   bg_rect.y,   bg_rect.w,   bg_rect.h   );
                ui_drawImage( 0, &logo,       logo_rect.x, logo_rect.y, logo_rect.w, logo_rect.h );
            }

            // "Copyright" text
            fnt_drawTextBox( menuFont, copyrightText, copyrightLeft, copyrightTop, 0, 0, 20 );

            drawSlidyButtons();
            updateSlidyButtons( -deltaTime );

            // Let lerp wind down relative to the time elapsed
            if ( SlidyButtonState.lerp <= 0.0f )
            {
                menuState = MM_Running;
            }
            break;

        case MM_Running:
            // Do normal run
            // Background

            GL_DEBUG(glColor4f)(1, 1, 1, 1 );

            if ( mnu_draw_background )
            {
                ui_drawImage( 0, &background, bg_rect.x,   bg_rect.y,   bg_rect.w,   bg_rect.h   );
                ui_drawImage( 0, &logo,       logo_rect.x, logo_rect.y, logo_rect.w, logo_rect.h );
            }

            // "Copyright" text
            fnt_drawTextBox( menuFont, copyrightText, copyrightLeft, copyrightTop, 0, 0, 20 );

            // Buttons
            if ( BUTTON_UP == ui_doButton( 1, mainMenuButtons[0], buttonLeft, buttonTop, 200, 30 ) )
            {
                // begin single player stuff
                menuChoice = 1;
            }
            if ( BUTTON_UP == ui_doButton( 2, mainMenuButtons[1], buttonLeft, buttonTop + 35, 200, 30 ) )
            {
                // begin multi player stuff
                menuChoice = 2;
            }
            if ( BUTTON_UP == ui_doButton( 3, mainMenuButtons[2], buttonLeft, buttonTop + 35 * 2, 200, 30 ) )
            {
                // go to options menu
                menuChoice = 3;
            }
            if ( BUTTON_UP == ui_doButton( 4, mainMenuButtons[3], buttonLeft, buttonTop + 35 * 3, 200, 30 ) )
            {
                // quit game
                menuChoice = 4;
            }
            if ( menuChoice != 0 )
            {
                menuState = MM_Leaving;
                initSlidyButtons( 0.0f, mainMenuButtons );
            }
            break;

        case MM_Leaving:
            // Do buttons sliding out and background fading
            // Do the same stuff as in MM_Entering, but backwards
            GL_DEBUG(glColor4f)(1, 1, 1, 1 - SlidyButtonState.lerp );

            if ( mnu_draw_background )
            {
                ui_drawImage( 0, &background, bg_rect.x,   bg_rect.y,   bg_rect.w,   bg_rect.h   );
                ui_drawImage( 0, &logo,       logo_rect.x, logo_rect.y, logo_rect.w, logo_rect.h );
            }

            // "Copyright" text
            fnt_drawTextBox( menuFont, copyrightText, copyrightLeft, copyrightTop, 0, 0, 20 );

            // Buttons
            drawSlidyButtons();
            updateSlidyButtons( deltaTime );
            if ( SlidyButtonState.lerp >= 1.0f )
            {
                menuState = MM_Finish;
            }
            break;

        case MM_Finish:
            // Free the background texture; don't need to hold onto it
            oglx_texture_Release( &background );
            menuState = MM_Begin;  // Make sure this all resets next time doMainMenu is called

            // reset the ui
            ui_Reset();

            // Set the next menu to load
            result = menuChoice;
            break;
    };

    return result;
}

//--------------------------------------------------------------------------------------------
int doSinglePlayerMenu( float deltaTime )
{
    static int menuState = MM_Begin;
    static oglx_texture background;
    static int menuChoice;
    int result = 0;

    switch ( menuState )
    {
        case MM_Begin:
            // Load resources for this menu
            oglx_texture_new( &background );
            ego_texture_load( &background, "basicdat" SLASH_STR "menu" SLASH_STR "menu_advent", TRANSCOLOR );
            menuChoice = 0;

            menuState = MM_Entering;

            initSlidyButtons( 1.0f, singlePlayerButtons );

            // Let this fall through

        case MM_Entering:
            GL_DEBUG(glColor4f)(1, 1, 1, 1 - SlidyButtonState.lerp );

            // Draw the background image
            if ( mnu_draw_background )
            {
                ui_drawImage( 0, &background, sdl_scr.x - background.imgW, 0, 0, 0 );
            }

            // "Copyright" text
            fnt_drawTextBox( menuFont, copyrightText, copyrightLeft, copyrightTop, 0, 0, 20 );

            drawSlidyButtons();
            updateSlidyButtons( -deltaTime );
            if ( SlidyButtonState.lerp <= 0.0f )
                menuState = MM_Running;

            break;

        case MM_Running:

            // Draw the background image
            if ( mnu_draw_background )
            {
                ui_drawImage( 0, &background, sdl_scr.x - background.imgW, 0, 0, 0 );
            }

            // "Copyright" text
            fnt_drawTextBox( menuFont, copyrightText, copyrightLeft, copyrightTop, 0, 0, 20 );

            // Buttons
            if ( BUTTON_UP == ui_doButton( 1, singlePlayerButtons[0], buttonLeft, buttonTop, 200, 30 ) )
            {
                menuChoice = 1;
            }
            if ( BUTTON_UP == ui_doButton( 2, singlePlayerButtons[1], buttonLeft, buttonTop + 35, 200, 30 ) )
            {
                menuChoice = 2;
            }
            if ( BUTTON_UP == ui_doButton( 3, singlePlayerButtons[2], buttonLeft, buttonTop + 35 * 2, 200, 30 ) )
            {
                menuChoice = 3;
            }
            if ( menuChoice != 0 )
            {
                menuState = MM_Leaving;
                initSlidyButtons( 0.0f, singlePlayerButtons );
            }
            break;

        case MM_Leaving:
            // Do buttons sliding out and background fading
            // Do the same stuff as in MM_Entering, but backwards
            GL_DEBUG(glColor4f)(1, 1, 1, 1 - SlidyButtonState.lerp );

            if ( mnu_draw_background )
            {
                ui_drawImage( 0, &background, sdl_scr.x - background.imgW, 0, 0, 0 );
            }

            // "Copyright" text
            fnt_drawTextBox( menuFont, copyrightText, copyrightLeft, copyrightTop, 0, 0, 20 );

            drawSlidyButtons();
            updateSlidyButtons( deltaTime );
            if ( SlidyButtonState.lerp >= 1.0f )
            {
                menuState = MM_Finish;
            }
            break;

        case MM_Finish:
            // Release the background texture
            oglx_texture_Release( &background );

            // reset the ui
            ui_Reset();

            // Set the next menu to load
            result = menuChoice;

            // And make sure that if we come back to this menu, it resets
            // properly
            menuState = MM_Begin;
    }

    return result;
}

//--------------------------------------------------------------------------------------------
// TODO: I totally fudged the layout of this menu by adding an offset for when
// the game isn't in 640x480.  Needs to be fixed.
int doChooseModule( float deltaTime )
{
    static int menuState = MM_Begin;
    static int startIndex;
    static oglx_texture background;
    static int validModules[MAXMODULE];
    static int numValidModules;

    static int moduleMenuOffsetX;
    static int moduleMenuOffsetY;

    int result = 0;
    int i, x, y;
    char txtBuffer[128];

    switch ( menuState )
    {
        case MM_Begin:
            // Reload all modules, something might be unlocked
            load_all_menu_images();

            // Load font & background
            oglx_texture_new( &background );
            ego_texture_load( &background, "basicdat" SLASH_STR "menu" SLASH_STR "menu_sleepy", TRANSCOLOR );
            startIndex = 0;
            selectedModule = -1;

            // Find the module's that we want to allow loading for.  If startNewPlayer
            // is true, we want ones that don't allow imports (e.g. starter modules).
            // Otherwise, we want modules that allow imports
            memset( validModules, 0, sizeof( int ) * MAXMODULE );
            numValidModules = 0;
            for ( i = 0; i < ModList_count; i++ )
            {
                // if this module is not valid given the game options and the
                // selected players, skip it
                if ( !modlist_test_by_index(i) ) continue;

                if ( startNewPlayer && 0 == ModList[i].importamount )
                {
                    // starter module
                    validModules[numValidModules] = i;
                    numValidModules++;
                }
                else
                {
                    if ( mnu_selectedPlayerCount > ModList[i].importamount ) continue;
                    if ( mnu_selectedPlayerCount < ModList[i].minplayers   ) continue;
                    if ( mnu_selectedPlayerCount > ModList[i].maxplayers   ) continue;

                    // regular module
                    validModules[numValidModules] = i;
                    numValidModules++;
                }
            }

            // Figure out at what offset we want to draw the module menu.
            moduleMenuOffsetX = ( sdl_scr.x - 640 ) / 2;
            moduleMenuOffsetY = ( sdl_scr.y - 480 ) / 2;

            menuState = MM_Entering;

            // fall through...

        case MM_Entering:
            menuState = MM_Running;

            // fall through for now...

        case MM_Running:
            // Draw the background
            GL_DEBUG(glColor4f)(1, 1, 1, 1 );
            x = ( sdl_scr.x / 2 ) - ( background.imgW / 2 );
            y = sdl_scr.y - background.imgH;

            if ( mnu_draw_background )
            {
                ui_drawImage( 0, &background, x, y, 0, 0 );
            }

            // use the mouse wheel to scan the modules
            if ( mouse_wheel_event )
            {
                if (mous.z > 0)
                {
                    startIndex++;
                }
                else if (mous.z < 0)
                {
                    startIndex--;
                }

                mouse_wheel_event = bfalse;
                mous.z = 0;
            }

            // Draw the arrows to pick modules
            if ( BUTTON_UP == ui_doButton( 1051, "<-", moduleMenuOffsetX + 20, moduleMenuOffsetY + 74, 30, 30 ) )
            {
                startIndex--;
            }
            if ( BUTTON_UP == ui_doButton( 1052, "->", moduleMenuOffsetX + 590, moduleMenuOffsetY + 74, 30, 30 ) )
            {
                startIndex++;
            }

            // restrict the range to valid values
            startIndex = CLIP(startIndex, 0, numValidModules - 3);

            // Draw buttons for the modules that can be selected
            x = 93;
            y = 20;

            for ( i = startIndex; i < ( startIndex + 3 ) && i < numValidModules; i++ )
            {
                // fix the menu images in case one or more of them are undefined
                int         imod       = validModules[i];
                Uint32      tex_offset = ModList[imod].tex;
                oglx_texture * ptex       = ((Uint32)(~0) == tex_offset) ? NULL : TxTitleImage + tex_offset;

                if ( ui_doImageButton( i, ptex, moduleMenuOffsetX + x, moduleMenuOffsetY + y, 138, 138 ) )
                {
                    selectedModule = i;
                }

                x += 138 + 20;  // Width of the button, and the spacing between buttons
            }

            // Draw an unused button as the backdrop for the text for now
            ui_drawButton( UI_Nothing, moduleMenuOffsetX + 21, moduleMenuOffsetY + 173, 291, 230, NULL );

            // And draw the next & back buttons
            if ( BUTTON_UP == ui_doButton( 53, "Select Module",
                                           moduleMenuOffsetX + 327, moduleMenuOffsetY + 173, 200, 30 ) )
            {
                // go to the next menu with this module selected
                selectedModule = validModules[selectedModule];
                menuState = MM_Leaving;
            }
            if ( BUTTON_UP == ui_doButton( 54, "Back", moduleMenuOffsetX + 327, moduleMenuOffsetY + 208, 200, 30 ) )
            {
                // Signal doMenu to go back to the previous menu
                selectedModule = -1;
                menuState = MM_Leaving;
            }

            // Draw the text description of the selected module
            if ( selectedModule > -1 )
            {
                y = 173 + 5;
                x = 21 + 5;
                GL_DEBUG(glColor4f)(1, 1, 1, 1 );
                fnt_drawText( menuFont, moduleMenuOffsetX + x, moduleMenuOffsetY + y,
                              ModList[validModules[selectedModule]].longname );
                y += 20;

                snprintf( txtBuffer, 128, "Difficulty: %s", ModList[validModules[selectedModule]].rank );
                fnt_drawText( menuFont, moduleMenuOffsetX + x, moduleMenuOffsetY + y, txtBuffer );
                y += 20;
                if ( ModList[validModules[selectedModule]].maxplayers > 1 )
                {
                    if ( ModList[validModules[selectedModule]].minplayers == ModList[validModules[selectedModule]].maxplayers )
                    {
                        snprintf( txtBuffer, 128, "%d Players", ModList[validModules[selectedModule]].minplayers );
                    }
                    else
                    {
                        snprintf( txtBuffer, 128, "%d - %d Players", ModList[validModules[selectedModule]].minplayers, ModList[validModules[selectedModule]].maxplayers );
                    }
                }
                else
                {
                    if ( ModList[validModules[selectedModule]].importamount != 0 ) snprintf( txtBuffer, 128, "Single Player" );
                    else snprintf( txtBuffer, 128, "Starter Module" );
                }

                fnt_drawText( menuFont, moduleMenuOffsetX + x, moduleMenuOffsetY + y, txtBuffer );
                y += 20;

                // And finally, the summary
                snprintf( txtBuffer, 128, "modules" SLASH_STR "%s" SLASH_STR "gamedat" SLASH_STR "menu.txt", ModList[validModules[selectedModule]].loadname );

                for ( i = 0; i < SUMMARYLINES; i++ )
                {
                    fnt_drawText( menuFont, moduleMenuOffsetX + x, moduleMenuOffsetY + y, ModList[validModules[selectedModule]].summary[i] );
                    y += 20;
                }
            }
            break;

        case MM_Leaving:
            menuState = MM_Finish;
            // fall through for now

        case MM_Finish:
            oglx_texture_Release( &background );

            menuState = MM_Begin;
            if ( selectedModule == -1 )
            {
                result = -1;
            }
            else
            {
                // Save the name of the module that we've picked
                pickedmodule_index = selectedModule;
                strncpy( pickedmodule_name, ModList[selectedModule].loadname, SDL_arraysize(pickedmodule_name) );

                // If the module allows imports, return 1.  Else, return 2
                if ( ModList[selectedModule].importamount > 0 )
                {
                    importvalid = btrue;
                    result = 1;
                }
                else
                {
                    importvalid = bfalse;
                    result = 2;
                }

                importamount = ModList[selectedModule].importamount;
                exportvalid  = ModList[selectedModule].allowexport;
                playeramount = ModList[selectedModule].maxplayers;

                respawnvalid = bfalse;
                respawnanytime = bfalse;
                if ( ModList[selectedModule].respawnvalid ) respawnvalid = btrue;
                if ( ModList[selectedModule].respawnvalid == ANYTIME ) respawnanytime = btrue;

                rtscontrol = bfalse;
            }

            // reset the ui
            ui_Reset();

            break;
    }

    return result;
}

//--------------------------------------------------------------------------------------------
int doChoosePlayer( float deltaTime )
{
    static int menuState = MM_Begin;
    static oglx_texture background;
    int result = 0;
    int i, j, x, y;
    char srcDir[64], destDir[64];
    static int startIndex = 0;

    static int numVertical, numHorizontal;
    static oglx_texture TxInput[4];
    static Uint32 BitsInput[4];
    int cnt, menuChoice;

    static const char * button_text[] = { "Select Player", "Back", ""};

    switch ( menuState )
    {
        case MM_Begin:
            mnu_selectedPlayerCount = 0;

            oglx_texture_new( &background );

            for (i = 0; i < 4; i++)
            {
                oglx_texture_new(TxInput + i);
            };

            mnu_selectedPlayerCount = 0;
            mnu_selectedPlayer[0] = 0;

            ego_texture_load( TxInput + 0, "basicdat" SLASH_STR "keybicon", INVALID_KEY );
            BitsInput[0] = INPUT_BITS_KEYBOARD;

            ego_texture_load( TxInput + 1, "basicdat" SLASH_STR "mousicon", INVALID_KEY );
            BitsInput[1] = INPUT_BITS_MOUSE;

            ego_texture_load( TxInput + 2, "basicdat" SLASH_STR "joyaicon", INVALID_KEY );
            BitsInput[2] = INPUT_BITS_JOYA;

            ego_texture_load( TxInput + 3, "basicdat" SLASH_STR "joybicon", INVALID_KEY );
            BitsInput[3] = INPUT_BITS_JOYB;

            ego_texture_load( &background, "basicdat" SLASH_STR "menu" SLASH_STR "menu_sleepy", TRANSCOLOR );

            // load information for all the players that could be imported
            check_player_import( "players", btrue );

            initSlidyButtons( 1.0f, button_text );

            numVertical   = buttonTop / 47 - 2;
            numHorizontal = 1;

            x = 20;
            y = 20;
            for ( i = 0; i < numVertical; i++ )
            {
                int m = i * 5;

                ui_initWidget( mnu_widgetList + m, m, menuFont, NULL, NULL, x, y, 175, 42 );
                ui_widgetAddMask( mnu_widgetList + m, UI_BITS_CLICKED );

                for ( j = 0, m++; j < 4; j++, m++ )
                {
                    ui_initWidget( mnu_widgetList + m, m, menuFont, NULL, TxInput + j, x + 175 + j*42, y, 42, 42 );
                    ui_widgetAddMask( mnu_widgetList + m, UI_BITS_CLICKED );
                };

                y += 47;
            };

            menuState = MM_Entering;
            // fall through

        case MM_Entering:
            GL_DEBUG(glColor4f)(1, 1, 1, 1 - SlidyButtonState.lerp );

            drawSlidyButtons();

            updateSlidyButtons( -deltaTime );

            // Let lerp wind down relative to the time elapsed
            if ( SlidyButtonState.lerp <= 0.0f )
            {
                menuState = MM_Running;
            }
            break;

        case MM_Running:
            // Figure out how many players we can show without scrolling

            // Draw the background
            x = ( sdl_scr.x / 2 ) - ( background.imgW / 2 );
            y = sdl_scr.y - background.imgH;

            if ( mnu_draw_background )
            {
                ui_drawImage( 0, &background, x, y, 0, 0 );
            }

            // use the mouse wheel to scan the characters
            if ( mouse_wheel_event )
            {
                if (mous.z > 0)
                {
                    if ( startIndex + numVertical < loadplayer_count )
                    {
                        startIndex++;
                    }
                }
                else if (mous.z < 0)
                {
                    if ( startIndex > 0 )
                    {
                        startIndex--;
                    }
                }

                mouse_wheel_event = bfalse;
                mous.z = 0;
            }

            // Draw the player selection buttons
            x = 20;
            y = 20;
            for ( i = 0; i < numVertical; i++ )
            {
                Uint16 player;
                Uint16 splayer;
                int m = i * 5;

                player = i + startIndex;
                if ( player >= loadplayer_count ) continue;

                splayer = mnu_getSelectedPlayer( player );

                // do the character button
                mnu_widgetList[m].img  = TxIcon + player;
                mnu_widgetList[m].text = loadplayer[player].name;
                if ( INVALID_PLA != splayer )
                {
                    mnu_widgetList[m].state |= UI_BITS_CLICKED;
                }
                else
                {
                    mnu_widgetList[m].state &= ~UI_BITS_CLICKED;
                }

                if ( BUTTON_DOWN == ui_doWidget( mnu_widgetList + m ) )
                {
                    if ( 0 != ( mnu_widgetList[m].state & UI_BITS_CLICKED ) && !mnu_checkSelectedPlayer( player ) )
                    {
                        // button has become clicked
                        //mnu_addSelectedPlayer(player);
                    }
                    else if ( 0 == ( mnu_widgetList[m].state & UI_BITS_CLICKED ) && mnu_checkSelectedPlayer( player ) )
                    {
                        // button has become unclicked
                        mnu_removeSelectedPlayer( player );
                    };
                };

                // do each of the input buttons
                for ( j = 0, m++; j < 4; j++, m++ )
                {
                    // make the button states reflect the chosen input devices
                    if ( INVALID_PLA == splayer || 0 == ( mnu_selectedInput[ splayer ] & BitsInput[j] ) )
                    {
                        mnu_widgetList[m].state &= ~UI_BITS_CLICKED;
                    }
                    else if ( 0 != ( mnu_selectedInput[splayer] & BitsInput[j] ) )
                    {
                        mnu_widgetList[m].state |= UI_BITS_CLICKED;
                    }

                    if ( BUTTON_DOWN == ui_doWidget( mnu_widgetList + m ) )
                    {
                        if ( 0 != ( mnu_widgetList[m].state & UI_BITS_CLICKED ) )
                        {
                            // button has become clicked
                            if ( INVALID_PLA == splayer )
                            {
                                mnu_addSelectedPlayer( player );
                            }
                            mnu_addSelectedPlayerInput( player, BitsInput[j] );
                        }
                        else if ( INVALID_PLA != splayer && 0 == ( mnu_widgetList[m].state & UI_BITS_CLICKED ) )
                        {
                            // button has become unclicked
                            mnu_removeSelectedPlayerInput( player, BitsInput[j] );
                        };
                    };
                }
            }

            // Buttons for going ahead
            menuChoice = -1;
            for ( cnt = 0; cnt < 2; cnt ++ )
            {
                if ( BUTTON_UP == ui_doButton( cnt + 100, button_text[cnt], buttonLeft, buttonTop + ( cnt * 35 ), 200, 30 ) )
                {
                    // audio options
                    menuChoice = cnt + 1;
                    menuState = MM_Leaving;
                }
            }

            switch ( menuChoice )
            {
                case 1: menuState = MM_Leaving; break;
                case 2: mnu_selectedPlayerCount = 0; menuState = MM_Leaving; break;
            }
            break;

        case MM_Leaving:
            // Do buttons sliding out and background fading
            // Do the same stuff as in MM_Entering, but backwards
            GL_DEBUG(glColor4f)(1, 1, 1, 1 - SlidyButtonState.lerp );

            // Buttons
            drawSlidyButtons();
            updateSlidyButtons( deltaTime );
            if ( SlidyButtonState.lerp >= 1.0f )
            {
                menuState = MM_Finish;
            }
            break;

        case MM_Finish:

            for (i = 0; i < 4; i++)
            {
                oglx_texture_delete(TxInput + i);
            };

            oglx_texture_delete( &background );

            menuState = MM_Begin;
            if ( 0 == mnu_selectedPlayerCount )
            {
                result = -1;
            }
            else
            {
                // Build the import directory
                empty_import_directory();
                fs_createDirectory( "import" );

                // set up the slots and the import stuff for the selected players
                numimport = mnu_selectedPlayerCount;
                for ( i = 0; i < numimport; i++ )
                {
                    selectedPlayer = mnu_selectedPlayer[i];

                    local_control[i] = mnu_selectedInput[i];
                    local_slot[i]    = i * MAXIMPORTPERPLAYER;

                    // Copy the character to the import directory
                    sprintf( srcDir, "players" SLASH_STR "%s", loadplayer[selectedPlayer].dir );
                    sprintf( destDir, "import" SLASH_STR "temp%04d.obj", local_slot[i] );
                    fs_copyDirectory( srcDir, destDir );

                    // Copy all of the character's items to the import directory
                    for ( j = 0; j < 8; j++ )
                    {
                        sprintf( srcDir, "players" SLASH_STR "%s" SLASH_STR "%d.obj", loadplayer[selectedPlayer].dir, j );
                        sprintf( destDir, "import" SLASH_STR "temp%04d.obj", local_slot[i] + j + 1 );

                        fs_copyDirectory( srcDir, destDir );
                    }

                }

                result = 1;
            }

            // reset the ui
            ui_Reset();

            break;
    }

    return result;
}

//--------------------------------------------------------------------------------------------
int doOptions( float deltaTime )
{
    static int menuState = MM_Begin;
    static oglx_texture background;
    static int menuChoice = 0;

    int result = 0;

    switch ( menuState )
    {
        case MM_Begin:
            // set up menu variables
            oglx_texture_new( &background );
            ego_texture_load( &background, "basicdat" SLASH_STR "menu" SLASH_STR "menu_gnome", TRANSCOLOR );
            menuChoice = 0;
            menuState = MM_Entering;

            initSlidyButtons( 1.0f, optionsButtons );
            // let this fall through into MM_Entering

        case MM_Entering:
            // do buttons sliding in animation, and background fading in
            // background
            GL_DEBUG(glColor4f)(1, 1, 1, 1 - SlidyButtonState.lerp );

            // Draw the background
            if ( mnu_draw_background )
            {
                ui_drawImage( 0, &background, ( sdl_scr.x - background.imgW ), 0, 0, 0 );
            }

            // "Copyright" text
            fnt_drawTextBox( menuFont, optionsText, optionsTextLeft, optionsTextTop, 0, 0, 20 );

            drawSlidyButtons();
            updateSlidyButtons( -deltaTime );

            // Let lerp wind down relative to the time elapsed
            if ( SlidyButtonState.lerp <= 0.0f )
            {
                menuState = MM_Running;
            }
            break;

        case MM_Running:
            // Do normal run
            // Background
            GL_DEBUG(glColor4f)(1, 1, 1, 1 );

            if ( mnu_draw_background )
            {
                ui_drawImage( 0, &background, ( sdl_scr.x - background.imgW ), 0, 0, 0 );
            }

            // "Options" text
            fnt_drawTextBox( menuFont, optionsText, optionsTextLeft, optionsTextTop, 0, 0, 20 );

            // Buttons
            if ( BUTTON_UP == ui_doButton( 1, optionsButtons[0], buttonLeft, buttonTop, 200, 30 ) )
            {
                // audio options
                menuChoice = 1;
            }
            if ( BUTTON_UP == ui_doButton( 2, optionsButtons[1], buttonLeft, buttonTop + 35, 200, 30 ) )
            {
                // input options
                menuChoice = 2;
            }
            if ( BUTTON_UP == ui_doButton( 3, optionsButtons[2], buttonLeft, buttonTop + 35 * 2, 200, 30 ) )
            {
                // video options
                menuChoice = 3;
            }
            if ( BUTTON_UP == ui_doButton( 4, optionsButtons[3], buttonLeft, buttonTop + 35 * 3, 200, 30 ) )
            {
                // back to main menu
                menuChoice = 4;
            }
            if ( menuChoice != 0 )
            {
                menuState = MM_Leaving;
                initSlidyButtons( 0.0f, optionsButtons );
            }
            break;

        case MM_Leaving:
            // Do buttons sliding out and background fading
            // Do the same stuff as in MM_Entering, but backwards
            GL_DEBUG(glColor4f)(1, 1, 1, 1 - SlidyButtonState.lerp );

            if ( mnu_draw_background )
            {
                ui_drawImage( 0, &background, ( sdl_scr.x - background.imgW ), 0, 0, 0 );
            }

            // "Options" text
            fnt_drawTextBox( menuFont, optionsText, optionsTextLeft, optionsTextTop, 0, 0, 20 );

            // Buttons
            drawSlidyButtons();
            updateSlidyButtons( deltaTime );
            if ( SlidyButtonState.lerp >= 1.0f )
            {
                menuState = MM_Finish;
            }
            break;

        case MM_Finish:
            // Free the background texture; don't need to hold onto it
            oglx_texture_Release( &background );
            menuState = MM_Begin;  // Make sure this all resets next time doMainMenu is called

            // reset the ui
            ui_Reset();

            // Set the next menu to load
            result = menuChoice;
            break;
    }

    return result;
}

//--------------------------------------------------------------------------------------------
int doInputOptions( float deltaTime )
{
    static int menuState = MM_Begin;
    static int menuChoice = 0;
    static int waitingforinput = -1;

    static STRING inputOptionsButtons[CONTROL_COMMAND_COUNT + 2];

    Sint8  result = 0;
    static Sint32 player = 0;

    Uint32              i;
    Sint32              idevice, iicon;
    device_controls_t * pdevice;

    pdevice = NULL;
    if (player >= 0 && player < input_device_count)
    {
        pdevice = controls + player;
    };

    idevice = player;
    if (NULL == pdevice)
    {
        idevice = -1;
    }

    iicon = MIN(4, idevice);
    if ( idevice < 0 )
    {
        iicon = -1;
    }

    switch ( menuState )
    {
        case MM_Begin:
            // set up menu variables
            menuChoice = 0;
            menuState = MM_Entering;
            // let this fall through into MM_Entering

            waitingforinput = -1;

            for ( i = 0; i < CONTROL_COMMAND_COUNT; i++)
            {
                inputOptionsButtons[i][0] = '\0';
            }
            strncpy( inputOptionsButtons[i++], "Player 1", sizeof(STRING) );
            strncpy( inputOptionsButtons[i++], "Save Settings", sizeof(STRING) );

            //Load the global icons (keyboard, mouse, etc.)
            if ( !load_all_global_icons() ) log_warning( "Could not load all global icons!\n" );

            gamepaused = btrue;         // In case we are running as an in-game menu

        case MM_Entering:
            // do buttons sliding in animation, and background fading in
            // background
            GL_DEBUG(glColor4f)(1, 1, 1, 1 - SlidyButtonState.lerp );

            // Fall trough
            menuState = MM_Running;
            break;

        case MM_Running:
            // Do normal run
            // Background
            GL_DEBUG(glColor4f)(1, 1, 1, 1 );
            fnt_drawTextBox( menuFont, "LEFT HAND", buttonLeft, sdl_scr.y - 470, 0, 0, 20 );

            //Are we waiting for input?
            if (SDLKEYDOWN( SDLK_ESCAPE )) waitingforinput = -1;  //Someone pressed abort

            // Grab the key/button input from the selected device
            if (waitingforinput != -1)
            {
                if (NULL == pdevice || idevice < 0 || idevice >= input_device_count)
                {
                    waitingforinput = -1;
                }
                else
                {
                    if (waitingforinput >= pdevice->count)
                    {
                        // invalid input range for this device
                        waitingforinput = -1;
                    }
                    else
                    {
                        control_t * pcontrol;
                        int         tag;

                        // make sure to update the input
                        input_read();

                        pcontrol = pdevice->control + waitingforinput;
                        if ( idevice >= INPUT_DEVICE_JOY )
                        {
                            int ijoy = idevice - INPUT_DEVICE_JOY;
                            if (ijoy < MAXJOYSTICK)
                            {
                                for ( tag = 0; tag < scantag_count; tag++ )
                                {
                                    if ( 0 != scantag[tag].value && (Uint32)scantag[tag].value == joy[ijoy].b )
                                    {
                                        pcontrol->tag    = scantag[tag].value;
                                        pcontrol->is_key = bfalse;
                                        waitingforinput = -1;
                                    }
                                }

                                for ( tag = 0; tag < scantag_count; tag++ )
                                {
                                    if ( scantag[tag].value < SDLK_NUMLOCK && SDLKEYDOWN( scantag[tag].value ) )
                                    {
                                        pcontrol->tag    = scantag[tag].value;
                                        pcontrol->is_key = btrue;
                                        waitingforinput = -1;
                                    }
                                }
                            }
                        }
                        else
                        {
                            switch ( idevice )
                            {
                                case INPUT_DEVICE_KEYBOARD:
                                    {
                                        for ( tag = 0; tag < scantag_count; tag++ )
                                        {
                                            if ( scantag[tag].value < SDLK_NUMLOCK && SDLKEYDOWN( scantag[tag].value ) )
                                            {
                                                pcontrol->tag    = scantag[tag].value;
                                                pcontrol->is_key = btrue;
                                                waitingforinput = -1;
                                            }
                                        }
                                    }
                                    break;

                                case INPUT_DEVICE_MOUSE:
                                    {
                                        for ( tag = 0; tag < scantag_count; tag++ )
                                        {
                                            if ( 0 != scantag[tag].value && (Uint32)scantag[tag].value == mous.b )
                                            {
                                                pcontrol->tag    = scantag[tag].value;
                                                pcontrol->is_key = bfalse;
                                                waitingforinput = -1;
                                            }
                                        }

                                        for ( tag = 0; tag < scantag_count; tag++ )
                                        {
                                            if ( scantag[tag].value < SDLK_NUMLOCK && SDLKEYDOWN( scantag[tag].value ) )
                                            {
                                                pcontrol->tag    = scantag[tag].value;
                                                pcontrol->is_key = btrue;
                                                waitingforinput = -1;
                                            }
                                        }
                                    }
                                    break;
                            }
                        }

                    }
                }
            }
            if (NULL != pdevice && waitingforinput == -1)
            {
                // update the control names
                for ( i = CONTROL_BEGIN; i <= CONTROL_END && i < pdevice->count; i++)
                {
                    char * tag = scantag_get_string(pdevice->device, pdevice->control[i].tag, pdevice->control[i].is_key);

                    strncpy( inputOptionsButtons[i], tag, sizeof(STRING) );
                }
                for ( /* nothing */; i <= CONTROL_END ; i++)
                {
                    inputOptionsButtons[i][0] = '\0';
                }
            }

            //Left hand
            if ( '\0' != inputOptionsButtons[CONTROL_LEFT_USE][0] )
            {
                fnt_drawTextBox( menuFont, "Use:", buttonLeft, sdl_scr.y - 440, 0, 0, 20 );
                if ( BUTTON_UP == ui_doButton( 1, inputOptionsButtons[CONTROL_LEFT_USE], buttonLeft + 100, sdl_scr.y - 440, 140, 30 ) )
                {
                    waitingforinput = CONTROL_LEFT_USE;
                    strncpy( inputOptionsButtons[CONTROL_LEFT_USE], "...", sizeof(STRING) );
                }
            }
            if ( '\0' != inputOptionsButtons[CONTROL_LEFT_GET][0] )
            {
                fnt_drawTextBox( menuFont, "Get/Drop:", buttonLeft, sdl_scr.y - 410, 0, 0, 20 );
                if ( BUTTON_UP == ui_doButton( 2, inputOptionsButtons[CONTROL_LEFT_GET], buttonLeft + 100, sdl_scr.y - 410, 140, 30 ) )
                {
                    waitingforinput = CONTROL_LEFT_GET;
                    strncpy( inputOptionsButtons[CONTROL_LEFT_GET], "...", sizeof(STRING) );
                }
            }
            if ( '\0' != inputOptionsButtons[CONTROL_LEFT_PACK][0] )
            {
                fnt_drawTextBox( menuFont, "Inventory:", buttonLeft, sdl_scr.y - 380, 0, 0, 20 );
                if ( BUTTON_UP == ui_doButton( 3, inputOptionsButtons[CONTROL_LEFT_PACK], buttonLeft + 100, sdl_scr.y - 380, 140, 30 ) )
                {
                    waitingforinput = CONTROL_LEFT_PACK;
                    strncpy( inputOptionsButtons[CONTROL_LEFT_PACK], "...", sizeof(STRING) );
                }
            }

            //Right hand
            fnt_drawTextBox( menuFont, "RIGHT HAND", buttonLeft + 300, sdl_scr.y - 470, 0, 0, 20 );
            if ( '\0' != inputOptionsButtons[CONTROL_RIGHT_USE][0] )
            {
                fnt_drawTextBox( menuFont, "Use:", buttonLeft + 300, sdl_scr.y - 440, 0, 0, 20 );
                if ( BUTTON_UP == ui_doButton( 4, inputOptionsButtons[CONTROL_RIGHT_USE], buttonLeft + 400, sdl_scr.y - 440, 140, 30 ) )
                {
                    waitingforinput = CONTROL_RIGHT_USE;
                    strncpy( inputOptionsButtons[CONTROL_RIGHT_USE], "...", sizeof(STRING) );
                }
            }
            if ( '\0' != inputOptionsButtons[CONTROL_RIGHT_GET][0] )
            {
                fnt_drawTextBox( menuFont, "Get/Drop:", buttonLeft + 300, sdl_scr.y - 410, 0, 0, 20 );
                if ( BUTTON_UP == ui_doButton( 5, inputOptionsButtons[CONTROL_RIGHT_GET], buttonLeft + 400, sdl_scr.y - 410, 140, 30 ) )
                {
                    waitingforinput = CONTROL_RIGHT_GET;
                    strncpy( inputOptionsButtons[CONTROL_RIGHT_GET], "...", sizeof(STRING) );
                }
            }
            if ( '\0' != inputOptionsButtons[CONTROL_RIGHT_PACK][0] )
            {
                fnt_drawTextBox( menuFont, "Inventory:", buttonLeft + 300, sdl_scr.y - 380, 0, 0, 20 );
                if ( BUTTON_UP == ui_doButton( 6, inputOptionsButtons[CONTROL_RIGHT_PACK], buttonLeft + 400, sdl_scr.y - 380, 140, 30 ) )
                {
                    waitingforinput = CONTROL_RIGHT_PACK;
                    strncpy( inputOptionsButtons[CONTROL_RIGHT_PACK], "...", sizeof(STRING) );
                }
            }

            //Controls
            fnt_drawTextBox( menuFont, "CONTROLS", buttonLeft, sdl_scr.y - 320, 0, 0, 20 );
            if ( '\0' != inputOptionsButtons[CONTROL_JUMP][0] )
            {
                fnt_drawTextBox( menuFont, "Jump:", buttonLeft, sdl_scr.y - 290, 0, 0, 20 );
                if ( BUTTON_UP == ui_doButton( 7, inputOptionsButtons[CONTROL_JUMP], buttonLeft + 100, sdl_scr.y - 290, 140, 30 ) )
                {
                    waitingforinput = CONTROL_JUMP;
                    strncpy( inputOptionsButtons[CONTROL_JUMP], "...", sizeof(STRING) );
                }
            }
            if ( '\0' != inputOptionsButtons[CONTROL_UP][0] )
            {
                fnt_drawTextBox( menuFont, "Up:", buttonLeft, sdl_scr.y - 260, 0, 0, 20 );
                if ( BUTTON_UP == ui_doButton( 8, inputOptionsButtons[CONTROL_UP], buttonLeft + 100, sdl_scr.y - 260, 140, 30 ) )
                {
                    waitingforinput = CONTROL_UP;
                    strncpy( inputOptionsButtons[CONTROL_UP], "...", sizeof(STRING) );
                }
            }
            if ( '\0' != inputOptionsButtons[CONTROL_DOWN][0] )
            {
                fnt_drawTextBox( menuFont, "Down:", buttonLeft, sdl_scr.y - 230, 0, 0, 20 );
                if ( BUTTON_UP == ui_doButton( 9, inputOptionsButtons[CONTROL_DOWN], buttonLeft + 100, sdl_scr.y - 230, 140, 30 ) )
                {
                    waitingforinput = CONTROL_DOWN;
                    strncpy( inputOptionsButtons[CONTROL_DOWN], "...", sizeof(STRING) );
                }
            }
            if ( '\0' != inputOptionsButtons[CONTROL_LEFT][0] )
            {
                fnt_drawTextBox( menuFont, "Left:", buttonLeft, sdl_scr.y - 200, 0, 0, 20 );
                if ( BUTTON_UP == ui_doButton( 10, inputOptionsButtons[CONTROL_LEFT], buttonLeft + 100, sdl_scr.y - 200, 140, 30 ) )
                {
                    waitingforinput = CONTROL_LEFT;
                    strncpy( inputOptionsButtons[CONTROL_LEFT], "...", sizeof(STRING) );
                }
            }
            if ( '\0' != inputOptionsButtons[CONTROL_RIGHT][0] )
            {
                fnt_drawTextBox( menuFont, "Right:", buttonLeft, sdl_scr.y - 170, 0, 0, 20 );
                if ( BUTTON_UP == ui_doButton( 11, inputOptionsButtons[CONTROL_RIGHT], buttonLeft + 100, sdl_scr.y - 170, 140, 30 ) )
                {
                    waitingforinput = CONTROL_RIGHT;
                    strncpy( inputOptionsButtons[CONTROL_RIGHT], "...", sizeof(STRING) );
                }
            }

            //Controls
            fnt_drawTextBox( menuFont, "CAMERA CONTROL", buttonLeft + 300, sdl_scr.y - 320, 0, 0, 20 );
            if ( '\0' != inputOptionsButtons[CONTROL_CAMERA_IN][0] )
            {
                fnt_drawTextBox( menuFont, "Zoom In:", buttonLeft + 300, sdl_scr.y - 290, 0, 0, 20 );
                if ( BUTTON_UP == ui_doButton( 12, inputOptionsButtons[CONTROL_CAMERA_IN], buttonLeft + 450, sdl_scr.y - 290, 140, 30 ) )
                {
                    waitingforinput = CONTROL_CAMERA_IN;
                    strncpy( inputOptionsButtons[CONTROL_CAMERA_IN], "...", sizeof(STRING) );
                }
            }
            else
            {
                // single button camera control
                fnt_drawTextBox( menuFont, "Camera:", buttonLeft + 300, sdl_scr.y - 290, 0, 0, 20 );
                if ( BUTTON_UP == ui_doButton( 12, inputOptionsButtons[CONTROL_CAMERA], buttonLeft + 450, sdl_scr.y - 290, 140, 30 ) )
                {
                    waitingforinput = CONTROL_CAMERA;
                    strncpy( inputOptionsButtons[CONTROL_CAMERA], "...", sizeof(STRING) );
                }
            }
            if ( '\0' != inputOptionsButtons[CONTROL_CAMERA_OUT][0] )
            {
                fnt_drawTextBox( menuFont, "Zoom Out:", buttonLeft + 300, sdl_scr.y - 260, 0, 0, 20 );
                if ( BUTTON_UP == ui_doButton( 13, inputOptionsButtons[CONTROL_CAMERA_OUT], buttonLeft + 450, sdl_scr.y - 260, 140, 30 ) )
                {
                    waitingforinput = CONTROL_CAMERA_OUT;
                    strncpy( inputOptionsButtons[CONTROL_CAMERA_OUT], "...", sizeof(STRING) );
                }
            }
            if ( '\0' != inputOptionsButtons[CONTROL_CAMERA_LEFT][0] )
            {
                fnt_drawTextBox( menuFont, "Rotate Left:", buttonLeft + 300, sdl_scr.y - 230, 0, 0, 20 );
                if ( BUTTON_UP == ui_doButton( 14, inputOptionsButtons[CONTROL_CAMERA_LEFT], buttonLeft + 450, sdl_scr.y - 230, 140, 30 ) )
                {
                    waitingforinput = CONTROL_CAMERA_LEFT;
                    strncpy( inputOptionsButtons[CONTROL_CAMERA_LEFT], "...", sizeof(STRING) );
                }
            }
            if ( '\0' != inputOptionsButtons[CONTROL_CAMERA_RIGHT][0] )
            {
                fnt_drawTextBox( menuFont, "Rotate Right:", buttonLeft + 300, sdl_scr.y - 200, 0, 0, 20 );
                if ( BUTTON_UP == ui_doButton( 15, inputOptionsButtons[CONTROL_CAMERA_RIGHT], buttonLeft + 450, sdl_scr.y - 200, 140, 30 ) )
                {
                    waitingforinput = CONTROL_CAMERA_RIGHT;
                    strncpy( inputOptionsButtons[CONTROL_CAMERA_RIGHT], "...", sizeof(STRING) );
                }
            }

            //The select player button
            if ( iicon < 0 )
            {
                if ( BUTTON_UP == ui_doButton( 16, "Select Player...", buttonLeft + 300, sdl_scr.y - 90, 140, 50 ) )
                {
                    player = 0;
                }
            }
            else if ( BUTTON_UP ==  ui_doImageButtonWithText( 16, TxIcon + (keybicon + iicon), inputOptionsButtons[CONTROL_COMMAND_COUNT+0],  buttonLeft + 300, sdl_scr.y - 90, 140, 50 ))
            {
                if (input_device_count > 0)
                {
                    player++;
                    player %= input_device_count;
                }

                snprintf(inputOptionsButtons[CONTROL_COMMAND_COUNT+0], sizeof(STRING), "Player %i", player + 1);
            }

            //Save settings button
            if ( BUTTON_UP == ui_doButton( 17, inputOptionsButtons[CONTROL_COMMAND_COUNT+1], buttonLeft, sdl_scr.y - 60, 200, 30 ) )
            {
                // save settings and go back
                player = 0;
                input_settings_save("controls.txt");
                menuState = MM_Leaving;
            }
            break;

        case MM_Leaving:
            // Do buttons sliding out and background fading
            // Do the same stuff as in MM_Entering, but backwards
            GL_DEBUG(glColor4f)(1, 1, 1, 1 - SlidyButtonState.lerp );

            // Fall trough
            menuState = MM_Finish;
            break;

        case MM_Finish:
            menuState = MM_Begin;  // Make sure this all resets next time doMainMenu is called

            // reset the ui
            ui_Reset();

            // Set the next menu to load
            result = 1;
            gamepaused = bfalse;            // In case we are running as an in-game menu
            break;
    }

    return result;
}

//--------------------------------------------------------------------------------------------
//Audio options menu
int doAudioOptions( float deltaTime )
{
    static int menuState = MM_Begin;
    static oglx_texture background;
    static int menuChoice = 0;
    static char Cmaxsoundchannel[128];
    static char Cbuffersize[128];
    static char Csoundvolume[128];
    static char Cmusicvolume[128];

    int result = 0;

    switch ( menuState )
    {
        case MM_Begin:
            // set up menu variables
            oglx_texture_new( &background );
            ego_texture_load( &background, "basicdat" SLASH_STR "menu" SLASH_STR "menu_gnome", TRANSCOLOR );
            menuChoice = 0;
            menuState = MM_Entering;
            gamepaused = btrue;         // In case we are running as an in-game menu
            // let this fall through into MM_Entering

        case MM_Entering:
            // do buttons sliding in animation, and background fading in
            // background
            GL_DEBUG(glColor4f)(1, 1, 1, 1 - SlidyButtonState.lerp );

            // Draw the background
            if ( mnu_draw_background )
            {
                ui_drawImage( 0, &background, ( sdl_scr.x - background.imgW ), 0, 0, 0 );
            }

            // Load the current settings
            if ( cfg.sound_allowed ) audioOptionsButtons[0] = "On";
            else audioOptionsButtons[0] = "Off";

            sprintf( Csoundvolume, "%i", cfg.sound_volume );
            audioOptionsButtons[1] = Csoundvolume;
            if ( cfg.music_allowed ) audioOptionsButtons[2] = "On";
            else audioOptionsButtons[2] = "Off";

            sprintf( Cmusicvolume, "%i", cfg.music_volume );
            audioOptionsButtons[3] = Cmusicvolume;

            sprintf( Cmaxsoundchannel, "%i", cfg.sound_channel_count );
            audioOptionsButtons[4] = Cmaxsoundchannel;

            sprintf( Cbuffersize, "%i", cfg.sound_buffer_size );
            audioOptionsButtons[5] = Cbuffersize;

            // Fall trough
            menuState = MM_Running;
            break;

        case MM_Running:
            // Do normal run
            // Background
            GL_DEBUG(glColor4f)(1, 1, 1, 1 );

            if ( mnu_draw_background )
            {
                ui_drawImage( 0, &background, ( sdl_scr.x - background.imgW ), 0, 0, 0 );
            }

            fnt_drawTextBox( menuFont, "Sound:", buttonLeft, sdl_scr.y - 270, 0, 0, 20 );

            // Buttons
            if ( BUTTON_UP == ui_doButton( 1, audioOptionsButtons[0], buttonLeft + 150, sdl_scr.y - 270, 100, 30 ) )
            {
                cfg.sound_allowed = !cfg.sound_allowed;
                audioOptionsButtons[0] = cfg.sound_allowed ? "On" : "Off";
            }

            fnt_drawTextBox( menuFont, "Sound Volume:", buttonLeft, sdl_scr.y - 235, 0, 0, 20 );
            if ( BUTTON_UP == ui_doButton( 2, audioOptionsButtons[1], buttonLeft + 150, sdl_scr.y - 235, 100, 30 ) )
            {
                cfg.sound_volume += 5;
                if (cfg.sound_volume > 100) cfg.sound_volume = 0;

                sprintf( Csoundvolume, "%i", cfg.sound_volume );
                audioOptionsButtons[1] = Csoundvolume;
            }

            fnt_drawTextBox( menuFont, "Music:", buttonLeft, sdl_scr.y - 165, 0, 0, 20 );
            if ( BUTTON_UP == ui_doButton( 3, audioOptionsButtons[2], buttonLeft + 150, sdl_scr.y - 165, 100, 30 ) )
            {
                cfg.music_allowed = !cfg.music_allowed;
                audioOptionsButtons[2] = cfg.music_allowed ? "On" : "Off";
            }

            fnt_drawTextBox( menuFont, "Music Volume:", buttonLeft, sdl_scr.y - 130, 0, 0, 20 );
            if ( BUTTON_UP == ui_doButton( 4, audioOptionsButtons[3], buttonLeft + 150, sdl_scr.y - 130, 100, 30 ) )
            {
                if ( cfg.music_volume < 0 )
                {
                    cfg.music_volume = 0;
                }
                else
                {
                    cfg.music_volume += 5;
                }

                if (cfg.music_volume > 100) cfg.music_volume = 0;

                sprintf( Cmusicvolume, "%i", cfg.music_volume );
                audioOptionsButtons[3] = Cmusicvolume;
            }

            fnt_drawTextBox( menuFont, "Sound Channels:", buttonLeft + 300, sdl_scr.y - 200, 0, 0, 20 );
            if ( BUTTON_UP == ui_doButton( 5, audioOptionsButtons[4], buttonLeft + 450, sdl_scr.y - 200, 100, 30 ) )
            {
                if ( cfg.sound_channel_count < 8 )
                {
                    cfg.sound_channel_count = 8;
                }
                else
                {
                    cfg.sound_channel_count <<= 1;
                }

                if ( cfg.sound_channel_count > 128 )
                {
                    cfg.sound_channel_count = 8;
                }

                sprintf( Cmaxsoundchannel, "%i", cfg.sound_channel_count );
                audioOptionsButtons[4] = Cmaxsoundchannel;
            }

            fnt_drawTextBox( menuFont, "Buffer Size:", buttonLeft + 300, sdl_scr.y - 165, 0, 0, 20 );
            if ( BUTTON_UP == ui_doButton( 6, audioOptionsButtons[5], buttonLeft + 450, sdl_scr.y - 165, 100, 30 ) )
            {
                if ( cfg.sound_buffer_size < 512 )
                {
                    cfg.sound_buffer_size = 512;
                }
                else
                {
                    cfg.sound_buffer_size <<= 1;
                }

                if ( cfg.sound_buffer_size > 8196 )
                {
                    cfg.sound_buffer_size = 512;
                }

                sprintf( Cbuffersize, "%i", cfg.sound_buffer_size );
                audioOptionsButtons[5] = Cbuffersize;
            }

            if ( BUTTON_UP == ui_doButton( 7, audioOptionsButtons[6], buttonLeft, sdl_scr.y - 60, 200, 30 ) )
            {
                // synchronoze the config values with the various game subsystems
                setup_synch( &cfg );

                // save the setup file
                setup_upload( &cfg );
                setup_write();

                //Reload the sound system
                sound_restart();

                //Do we restart the music?
                if ( cfg.music_allowed )
                {
                    load_all_music_sounds();
                    fade_in_music( musictracksloaded[songplaying] );
                }

                menuState = MM_Leaving;
            }
            break;

        case MM_Leaving:
            // Do buttons sliding out and background fading
            // Do the same stuff as in MM_Entering, but backwards
            GL_DEBUG(glColor4f)(1, 1, 1, 1 - SlidyButtonState.lerp );

            if ( mnu_draw_background )
            {
                ui_drawImage( 0, &background, ( sdl_scr.x - background.imgW ), 0, 0, 0 );
            }

            // Fall trough
            menuState = MM_Finish;
            break;

        case MM_Finish:
            // Free the background texture; don't need to hold onto it
            oglx_texture_Release( &background );
            menuState = MM_Begin;  // Make sure this all resets next time doMainMenu is called

            // reset the ui
            ui_Reset();

            // Set the next menu to load
            result = 1;
            gamepaused = bfalse;            // In case we are running as an in-game menu
            break;
    }

    return result;
}

//--------------------------------------------------------------------------------------------
int doVideoOptions( float deltaTime )
{
    static int menuState = MM_Begin;
    static oglx_texture background;
    static int menuChoice = 0;
    int result = 0;
    static STRING Cantialiasing;
    static char Cmaxmessage[128];
    static char Cmaxlights[128];
    static char Cscrz[128];
    static char Cmaxparticles[128];
    static char Cmaxdyna[128];

    switch ( menuState )
    {
        case MM_Begin:
            // set up menu variables
            oglx_texture_new( &background );
            ego_texture_load( &background, "basicdat" SLASH_STR "menu" SLASH_STR "menu_gnome", TRANSCOLOR );
            menuChoice = 0;
            menuState = MM_Entering;    // let this fall through into MM_Entering
            gamepaused = btrue;         // In case we are running as an in-game menu

        case MM_Entering:
            // do buttons sliding in animation, and background fading in
            // background
            GL_DEBUG(glColor4f)(1, 1, 1, 1 - SlidyButtonState.lerp );

            // Draw the background
            if ( mnu_draw_background )
            {
                ui_drawImage( 0, &background, ( sdl_scr.x - background.imgW ), 0, 0, 0 );
            }

            // Load all the current video settings
            if (cfg.multisamples == 0) strcpy(Cantialiasing , "Off");
            else snprintf(Cantialiasing, sizeof(Cantialiasing), "X%i", cfg.multisamples);
            videoOptionsButtons[0] = Cantialiasing;

            //Message duration
            if ( cfg.message_duration <= 100 )
            {
                videoOptionsButtons[1] = "Short";
            }
            else if ( cfg.message_duration <= 150 )
            {
                videoOptionsButtons[1] = "Normal";
            }
            else
            {
                videoOptionsButtons[1] = "Long";
            }

            //Texture filtering
            switch ( cfg.texturefilter_req )
            {
                case TX_UNFILTERED:
                    videoOptionsButtons[5] = "Unfiltered";
                    break;
                case TX_LINEAR:
                    videoOptionsButtons[5] = "Linear";
                    break;
                case TX_MIPMAP:
                    videoOptionsButtons[5] = "Mipmap";
                    break;
                case TX_BILINEAR:
                    videoOptionsButtons[5] = "Bilinear";
                    break;
                case TX_TRILINEAR_1:
                    videoOptionsButtons[5] = "Trilinear 1";
                    break;
                case TX_TRILINEAR_2:
                    videoOptionsButtons[5] = "Trilinear 2";
                    break;
                case TX_ANISOTROPIC:
                    videoOptionsButtons[5] = "Ansiotropic";
                    break;
                default:                  // Set to defaults
                    videoOptionsButtons[5] = "Linear";
                    cfg.texturefilter_req = TX_LINEAR;
                    break;
            }

            videoOptionsButtons[2] = cfg.use_dither ? "Yes" : "No";

            videoOptionsButtons[3] = cfg.fullscreen_req ? "True" : "False";

            if ( cfg.reflect_allowed )
            {
                videoOptionsButtons[4] = "Low";
                if ( cfg.reflect_prt )
                {
                    videoOptionsButtons[4] = "Medium";
                    if ( cfg.reflect_fade == 0 )
                    {
                        videoOptionsButtons[4] = "High";
                    }
                }
            }
            else
            {
                videoOptionsButtons[4] = "Off";
            }

            if ( maxmessage > MAXMESSAGE || maxmessage < 0 ) maxmessage = MAXMESSAGE - 1;
            if ( maxmessage == 0 )
            {
                sprintf( Cmaxmessage, "None" );           // Set to default
            }
            else
            {
                sprintf( Cmaxmessage, "%i", maxmessage );
            }
            videoOptionsButtons[11] = Cmaxmessage;

            if ( cfg.shadow_allowed )
            {
                videoOptionsButtons[6] = "Normal";
                if ( !cfg.shadow_sprite )
                {
                    videoOptionsButtons[6] = "Best";
                }
            }
            else videoOptionsButtons[6] = "Off";

            if ( cfg.scrz_req != 32 && cfg.scrz_req != 16 && cfg.scrz_req != 24 )
            {
                cfg.scrz_req = 16;              // Set to default
            }
            sprintf( Cscrz, "%i", cfg.scrz_req );      // Convert the integer to a char we can use
            videoOptionsButtons[7] = Cscrz;

            sprintf( Cmaxlights, "%i", cfg.dyna_count_req );
            videoOptionsButtons[8] = Cmaxlights;

            if ( cfg.use_phong )
            {
                videoOptionsButtons[9] = "Okay";
                if ( cfg.overlay_allowed && cfg.background_allowed )
                {
                    videoOptionsButtons[9] = "Good";
                    if ( cfg.use_perspective )
                    {
                        videoOptionsButtons[9] = "Superb";
                    }
                }
                else                            // Set to defaults
                {
                    cfg.use_perspective    = bfalse;
                    cfg.background_allowed = bfalse;
                    cfg.overlay_allowed    = bfalse;
                    videoOptionsButtons[9] = "Off";
                }
            }
            else                              // Set to defaults
            {
                cfg.use_perspective    = bfalse;
                cfg.background_allowed = bfalse;
                cfg.overlay_allowed    = bfalse;
                videoOptionsButtons[9] = "Off";
            }

            if ( cfg.twolayerwater_allowed ) videoOptionsButtons[10] = "On";
            else videoOptionsButtons[10] = "Off";

            sprintf( Cmaxparticles, "%i", cfg.particle_count_req );      // Convert the integer to a char we can use
            videoOptionsButtons[14] = Cmaxparticles;

            switch ( cfg.scrx_req )
            {
                case 1024: videoOptionsButtons[12] = "1024X768";
                    break;

                case 640: videoOptionsButtons[12] = "640X480";
                    break;

                case 800: videoOptionsButtons[12] = "800X600";
                    break;

                default:
                    videoOptionsButtons[12] = "Custom";
                    break;
            }

            menuState = MM_Running;
            break;

        case MM_Running:
            // Do normal run
            // Background
            GL_DEBUG(glColor4f)(1, 1, 1, 1 );

            if ( mnu_draw_background )
            {
                ui_drawImage( 0, &background, ( sdl_scr.x - background.imgW ), 0, 0, 0 );
            }

            // Antialiasing Button
            fnt_drawTextBox( menuFont, "Antialiasing:", buttonLeft, sdl_scr.y - 215, 0, 0, 20 );
            if ( BUTTON_UP == ui_doButton( 1, videoOptionsButtons[0], buttonLeft + 150, sdl_scr.y - 215, 100, 30 ) )
            {
                // make the multi-sampling even

                if ( cfg.multisamples < 0 )
                {
                    cfg.multisamples = 0;
                }
                else
                {
                    cfg.multisamples += 1;
                }

                // set some arbitrary limit
                if ( cfg.multisamples > 4 ) cfg.multisamples = 0;

                if (cfg.multisamples == 0) strcpy(Cantialiasing , "Off");
                else snprintf(Cantialiasing, sizeof(Cantialiasing), "X%i", cfg.multisamples);

                videoOptionsButtons[0] = Cantialiasing;
            }

            // Message time
            fnt_drawTextBox( menuFont, "Message Duration:", buttonLeft, sdl_scr.y - 180, 0, 0, 20 );
            if ( BUTTON_UP == ui_doButton( 2, videoOptionsButtons[1], buttonLeft + 150, sdl_scr.y - 180, 100, 30 ) )
            {
                if ( cfg.message_duration < 0 )
                {
                    cfg.message_duration = 100;
                }
                else
                {
                    cfg.message_duration += 50;
                }

                if ( cfg.message_duration >= 250 )
                {
                    cfg.message_duration = 100;
                }

                if ( cfg.message_duration <= 100 )
                {
                    videoOptionsButtons[1] = "Short";
                }
                else if ( cfg.message_duration <= 150 )
                {
                    videoOptionsButtons[1] = "Normal";
                }
                else
                {
                    videoOptionsButtons[1] = "Long";
                }
            }

            // Dithering
            fnt_drawTextBox( menuFont, "Dithering:", buttonLeft, sdl_scr.y - 145, 0, 0, 20 );
            if ( BUTTON_UP == ui_doButton( 3, videoOptionsButtons[2], buttonLeft + 150, sdl_scr.y - 145, 100, 30 ) )
            {
                cfg.use_dither = !cfg.use_dither;
                videoOptionsButtons[2] = cfg.use_dither ? "Yes" : "No";
            }

            // Fullscreen
            fnt_drawTextBox( menuFont, "Fullscreen:", buttonLeft, sdl_scr.y - 110, 0, 0, 20 );
            if ( BUTTON_UP == ui_doButton( 4, videoOptionsButtons[3], buttonLeft + 150, sdl_scr.y - 110, 100, 30 ) )
            {
                cfg.fullscreen_req = !cfg.fullscreen_req;

                videoOptionsButtons[3] = cfg.fullscreen_req ? "True" : "False";
            }

            // Reflection
            fnt_drawTextBox( menuFont, "Reflections:", buttonLeft, sdl_scr.y - 250, 0, 0, 20 );
            if ( BUTTON_UP == ui_doButton( 5, videoOptionsButtons[4], buttonLeft + 150, sdl_scr.y - 250, 100, 30 ) )
            {

                if ( cfg.reflect_allowed && cfg.reflect_fade == 0 && cfg.reflect_prt )
                {
                    cfg.reflect_allowed = bfalse;
                    cfg.reflect_fade = 255;
                    cfg.reflect_prt = bfalse;
                    videoOptionsButtons[4] = "Off";
                }
                else
                {
                    if ( cfg.reflect_allowed && !cfg.reflect_prt )
                    {
                        videoOptionsButtons[4] = "Medium";
                        cfg.reflect_fade = 255;
                        cfg.reflect_prt = btrue;
                    }
                    else
                    {
                        if ( cfg.reflect_allowed && cfg.reflect_fade == 255 && cfg.reflect_prt )
                        {
                            videoOptionsButtons[4] = "High";
                            cfg.reflect_fade = 0;
                        }
                        else
                        {
                            cfg.reflect_allowed = btrue;
                            cfg.reflect_fade = 255;
                            videoOptionsButtons[4] = "Low";
                            cfg.reflect_prt = bfalse;
                        }
                    }
                }
            }

            // Texture Filtering
            fnt_drawTextBox( menuFont, "Texture Filtering:", buttonLeft, sdl_scr.y - 285, 0, 0, 20 );
            if ( BUTTON_UP == ui_doButton( 6, videoOptionsButtons[5], buttonLeft + 150, sdl_scr.y - 285, 130, 30 ) )
            {
                if ( cfg.texturefilter_req < TX_UNFILTERED )
                {
                    cfg.texturefilter_req = TX_UNFILTERED;
                }
                else
                {
                    cfg.texturefilter_req = (TX_FILTERS)( (int)cfg.texturefilter_req + 1 );
                }

                if ( cfg.texturefilter_req > TX_ANISOTROPIC )
                {
                    cfg.texturefilter_req = TX_UNFILTERED;
                }

                switch ( cfg.texturefilter_req )
                {

                    case TX_UNFILTERED:
                        videoOptionsButtons[5] = "Unfiltered";
                        break;

                    case TX_LINEAR:
                        videoOptionsButtons[5] = "Linear";
                        break;

                    case TX_MIPMAP:
                        videoOptionsButtons[5] = "Mipmap";
                        break;

                    case TX_BILINEAR:
                        videoOptionsButtons[5] = "Bilinear";
                        break;

                    case TX_TRILINEAR_1:
                        videoOptionsButtons[5] = "Trilinear 1";
                        break;

                    case TX_TRILINEAR_2:
                        videoOptionsButtons[5] = "Trilinear 2";
                        break;

                    case TX_ANISOTROPIC:
                        videoOptionsButtons[5] = "Anisotropic";
                        break;

                    default:
                        cfg.texturefilter_req = TX_UNFILTERED;
                        videoOptionsButtons[5] = "Unfiltered";
                        break;
                }
            }

            // Shadows
            fnt_drawTextBox( menuFont, "Shadows:", buttonLeft, sdl_scr.y - 320, 0, 0, 20 );
            if ( BUTTON_UP == ui_doButton( 7, videoOptionsButtons[6], buttonLeft + 150, sdl_scr.y - 320, 100, 30 ) )
            {
                if ( cfg.shadow_allowed && !cfg.shadow_sprite )
                {
                    cfg.shadow_allowed = bfalse;
                    cfg.shadow_sprite = bfalse;                // Just in case
                    videoOptionsButtons[6] = "Off";
                }
                else
                {
                    if ( cfg.shadow_allowed && cfg.shadow_sprite )
                    {
                        videoOptionsButtons[6] = "Best";
                        cfg.shadow_sprite = bfalse;
                    }
                    else
                    {
                        cfg.shadow_allowed = btrue;
                        cfg.shadow_sprite = btrue;
                        videoOptionsButtons[6] = "Normal";
                    }
                }
            }

            // Z bit
            fnt_drawTextBox( menuFont, "Z Bit:", buttonLeft + 300, sdl_scr.y - 320, 0, 0, 20 );
            if ( BUTTON_UP == ui_doButton( 8, videoOptionsButtons[7], buttonLeft + 450, sdl_scr.y - 320, 100, 30 ) )
            {
                if ( cfg.scrz_req < 0 )
                {
                    cfg.scrz_req = 8;
                }
                else
                {
                    cfg.scrz_req += 8;
                }

                if ( cfg.scrz_req > 32) cfg.scrz_req = 8;

                snprintf(Cscrz, SDL_arraysize(Cscrz), "%d", cfg.scrz_req );
                videoOptionsButtons[7] = Cscrz;
            }

            // Max dynamic lights
            fnt_drawTextBox( menuFont, "Max Lights:", buttonLeft + 300, sdl_scr.y - 285, 0, 0, 20 );
            if ( BUTTON_UP == ui_doButton( 9, videoOptionsButtons[8], buttonLeft + 450, sdl_scr.y - 285, 100, 30 ) )
            {
                if ( cfg.dyna_count_req < 8 )
                {
                    cfg.dyna_count_req = 8;
                }
                else
                {
                    cfg.dyna_count_req += 8;
                }

                if ( cfg.dyna_count_req > TOTAL_MAX_DYNA )
                {
                    cfg.dyna_count_req = 8;
                }

                snprintf(Cmaxdyna, SDL_arraysize(Cmaxdyna), "%d", cfg.dyna_count_req );
                videoOptionsButtons[8] = Cmaxdyna;
            }

            // Perspective correction and phong mapping
            fnt_drawTextBox( menuFont, "3D Effects:", buttonLeft + 300, sdl_scr.y - 250, 0, 0, 20 );
            if ( BUTTON_UP == ui_doButton( 10, videoOptionsButtons[9], buttonLeft + 450, sdl_scr.y - 250, 100, 30 ) )
            {
                if ( cfg.use_phong && cfg.use_perspective && cfg.overlay_allowed && cfg.background_allowed )
                {
                    cfg.use_phong          = bfalse;
                    cfg.use_perspective    = bfalse;
                    cfg.overlay_allowed    = bfalse;
                    cfg.background_allowed = bfalse;
                    videoOptionsButtons[9] = "Off";
                }
                else
                {
                    if ( !cfg.use_phong )
                    {
                        videoOptionsButtons[9] = "Okay";
                        cfg.use_phong = btrue;
                    }
                    else
                    {
                        if ( !cfg.use_perspective && cfg.overlay_allowed && cfg.background_allowed )
                        {
                            videoOptionsButtons[9] = "Superb";
                            cfg.use_perspective = btrue;
                        }
                        else
                        {
                            cfg.overlay_allowed = btrue;
                            cfg.background_allowed = btrue;
                            videoOptionsButtons[9] = "Good";
                        }
                    }
                }
            }

            // Water Quality
            fnt_drawTextBox( menuFont, "Good Water:", buttonLeft + 300, sdl_scr.y - 215, 0, 0, 20 );
            if ( BUTTON_UP == ui_doButton( 11, videoOptionsButtons[10], buttonLeft + 450, sdl_scr.y - 215, 100, 30 ) )
            {
                if ( cfg.twolayerwater_allowed )
                {
                    videoOptionsButtons[10] = "Off";
                    cfg.twolayerwater_allowed = bfalse;
                }
                else
                {
                    videoOptionsButtons[10] = "On";
                    cfg.twolayerwater_allowed = btrue;
                }
            }

            // Max particles
            fnt_drawTextBox( menuFont, "Max Particles:", buttonLeft + 300, sdl_scr.y - 180, 0, 0, 20 );
            if ( BUTTON_UP == ui_doButton( 15, videoOptionsButtons[14], buttonLeft + 450, sdl_scr.y - 180, 100, 30 ) )
            {
                if ( cfg.particle_count_req < 256 )
                {
                    cfg.particle_count_req = 256;
                }
                else
                {
                    cfg.particle_count_req += 128;
                }

                if (cfg.particle_count_req > TOTAL_MAX_PRT ) cfg.particle_count_req = 256;

                snprintf( Cmaxparticles, SDL_arraysize(Cmaxparticles), "%i", cfg.particle_count_req );    // Convert integer to a char we can use
                videoOptionsButtons[14] =  Cmaxparticles;
            }

            // Text messages
            fnt_drawTextBox( menuFont, "Max  Messages:", buttonLeft + 300, sdl_scr.y - 145, 0, 0, 20 );
            if ( BUTTON_UP == ui_doButton( 12, videoOptionsButtons[11], buttonLeft + 450, sdl_scr.y - 145, 75, 30 ) )
            {
                if ( cfg.message_count_req < 0 )
                {
                    cfg.message_count_req = 0;
                }
                else
                {
                    cfg.message_count_req++;
                }

                if ( cfg.message_count_req > MAXMESSAGE )
                {
                    cfg.message_count_req = 0;
                }

                if ( 0 == cfg.message_count_req )
                {
                    sprintf( Cmaxmessage, "None" );
                }
                else
                {
                    sprintf( Cmaxmessage, "%i", cfg.message_count_req );    // Convert integer to a char we can use
                }

                videoOptionsButtons[11] = Cmaxmessage;
            }

            // Screen Resolution
            fnt_drawTextBox( menuFont, "Resolution:", buttonLeft + 300, sdl_scr.y - 110, 0, 0, 20 );
            if ( BUTTON_UP == ui_doButton( 13, videoOptionsButtons[12], buttonLeft + 450, sdl_scr.y - 110, 125, 30 ) )
            {
                // TODO: add widescreen support
                switch ( cfg.scrx_req )
                {
                    case 1024:
                        cfg.scrx_req = 640;
                        cfg.scry_req = 480;
                        videoOptionsButtons[12] = "640x480";
                        break;

                    case 640:
                        cfg.scrx_req = 800;
                        cfg.scry_req = 600;
                        videoOptionsButtons[12] = "800x600";
                        break;

                    case 800:
                        cfg.scrx_req = 1024;
                        cfg.scry_req = 768;
                        videoOptionsButtons[12] = "1024x768";
                        break;

                    default:
                        cfg.scrx_req = 640;
                        cfg.scry_req = 480;
                        videoOptionsButtons[12] = "640x480";
                        break;
                }
            }

            // Save settings button
            if ( BUTTON_UP == ui_doButton( 14, videoOptionsButtons[13], buttonLeft, sdl_scr.y - 60, 200, 30 ) )
            {
                menuChoice = 1;

                // synchronoze the config values with the various game subsystems
                setup_synch( &cfg );

                // save the setup file
                setup_upload( &cfg );
                setup_write();

                // Reload some of the graphics
                load_graphics();
            }
            if ( menuChoice != 0 )
            {
                menuState = MM_Leaving;
                initSlidyButtons( 0.0f, videoOptionsButtons );
            }
            break;

        case MM_Leaving:
            // Do buttons sliding out and background fading
            // Do the same stuff as in MM_Entering, but backwards
            GL_DEBUG(glColor4f)(1, 1, 1, 1 - SlidyButtonState.lerp );

            if ( mnu_draw_background )
            {
                ui_drawImage( 0, &background, ( sdl_scr.x - background.imgW ), 0, 0, 0 );
            }

            // "Options" text
            fnt_drawTextBox( menuFont, optionsText, optionsTextLeft, optionsTextTop, 0, 0, 20 );

            // Fall trough
            menuState = MM_Finish;
            break;

        case MM_Finish:
            // Free the background texture; don't need to hold onto it
            oglx_texture_Release( &background );
            menuState = MM_Begin;  // Make sure this all resets next time doMainMenu is called

            // reset the ui
            ui_Reset();

            // Set the next menu to load
            result = menuChoice;
            gamepaused = bfalse;            // In case we are running as an in-game menu
            break;
    }

    return result;
}

//--------------------------------------------------------------------------------------------
int doShowMenuResults( float deltaTime )
{
    int x, y;
    STRING text;
    Font *font;
    Uint8 i;

    font = ui_getFont();

    ui_drawButton( UI_Nothing, 30, 30, sdl_scr.x - 60, sdl_scr.y - 65, NULL );

    x = 35;
    y = 35;
    GL_DEBUG(glColor4f)(1, 1, 1, 1 );
    snprintf( text, sizeof(text), "Module selected: %s", ModList[selectedModule].loadname );
    fnt_drawText( font, x, y, text );
    y += 35;
    if ( importvalid )
    {
        snprintf( text, sizeof(text), "Player selected: %s", loadplayer[selectedPlayer].name );
    }
    else
    {
        snprintf( text, sizeof(text), "Starting a new player." );
    }

    fnt_drawText( font, x, y, text );

    // And finally, the summary
    y += 60;
    snprintf( text, sizeof(text), "%s", ModList[selectedModule].longname );
    fnt_drawText( font, x, y, text );
    y += 30;
    snprintf( text, sizeof(text), "modules" SLASH_STR "%s" SLASH_STR "gamedat" SLASH_STR "menu.txt", ModList[selectedModule].loadname );

    for ( i = 0; i < SUMMARYLINES; i++ )
    {
        fnt_drawText( menuFont, x, y, ModList[selectedModule].summary[i] );
        y += 20;
    }

    return 1;
}

//--------------------------------------------------------------------------------------------
int doNotImplemented( float deltaTime )
{
    int x, y;
    int w, h;
    char notImplementedMessage[] = "Not implemented yet!  Check back soon!";

    fnt_getTextSize( ui_getFont(), notImplementedMessage, &w, &h );
    w += 50; // add some space on the sides

    x = sdl_scr.x / 2 - w / 2;
    y = sdl_scr.y / 2 - 17;
    if ( BUTTON_UP == ui_doButton( 1, notImplementedMessage, x, y, w, 30 ) )
    {
        return 1;
    }

    return 0;
}


//--------------------------------------------------------------------------------------------
int doGamePaused( float deltaTime )
{
    static int menuState = MM_Begin;
    static int menuChoice = 0;

    static const char * buttons[] =
    {
        "Quit Module",
        "Audio Options",
        "Input Controls",
        "Video Settings",
        "Return to Module",
        ""
    };

    int result = 0, cnt;

    switch ( menuState )
    {
        case MM_Begin:
            // set up menu variables
            menuChoice = 0;
            menuState = MM_Entering;

            initSlidyButtons( 1.0f, buttons );

            gamepaused = btrue;

        case MM_Entering:
            drawSlidyButtons();
            updateSlidyButtons( -deltaTime );

            // Let lerp wind down relative to the time elapsed
            if ( SlidyButtonState.lerp <= 0.0f )
            {
                menuState = MM_Running;
            }
            break;

        case MM_Running:
            // Do normal run
            // Background
            GL_DEBUG(glColor4f)(1, 1, 1, 1 );

            // Buttons
            for ( cnt = 0; cnt < 5; cnt ++ )
            {
                if ( BUTTON_UP == ui_doButton( cnt + 1, buttons[cnt], buttonLeft, buttonTop + ( cnt * 35 ), 200, 30 ) )
                {
                    // audio options
                    menuChoice = cnt + 1;
                }
            }

            //Quick return to game
            if (SDLKEYDOWN( SDLK_ESCAPE )) menuChoice = 5;

            if ( menuChoice != 0 )
            {
                menuState = MM_Leaving;
                initSlidyButtons( 0.0f, buttons );
            }
            break;

        case MM_Leaving:
            // Do buttons sliding out and background fading
            // Do the same stuff as in MM_Entering, but backwards
            GL_DEBUG(glColor4f)(1, 1, 1, 1 - SlidyButtonState.lerp );

            // Buttons
            drawSlidyButtons();
            updateSlidyButtons( deltaTime );
            if ( SlidyButtonState.lerp >= 1.0f )
            {
                menuState = MM_Finish;
            }
            break;

        case MM_Finish:
            // Free the background texture; don't need to hold onto it
            menuState = MM_Begin;  // Make sure this all resets next time doMainMenu is called

            // reset the ui
            ui_Reset();

            gamepaused = bfalse;

            // Set the next menu to load
            result = menuChoice;
            break;
    }

    return result;
}

//--------------------------------------------------------------------------------------------
int doShowEndgame( float deltaTime )
{
    static int menuState = MM_Begin;
    static int menuChoice = 0;
    static int x, y, w, h;
    static Font *font;

    static const char * buttons[] =
    {
        "BLAH",
        ""
    };

    int cnt, retval;

    retval = 0;
    switch ( menuState )
    {
        case MM_Begin:
            menuState = MM_Entering;
            font = ui_getFont();

            initSlidyButtons( 1.0f, buttons );

            if ( exportvalid )
            {
                buttons[0] = "Save and Exit";
            }
            else
            {
                buttons[0] = "Exit Game";
            }

            x = 70;
            y = 70;
            w = sdl_scr.x - 2 * x;
            h = sdl_scr.y - 2 * y;

            gamepaused = btrue;

        case MM_Entering:

            GL_DEBUG(glColor4f)(1, 1, 1, 1 - SlidyButtonState.lerp );

            ui_drawTextBox( endtext, x, y, w, h, 20 );
            drawSlidyButtons();

            updateSlidyButtons( -deltaTime );

            // Let lerp wind down relative to the time elapsed
            if ( SlidyButtonState.lerp <= 0.0f )
            {
                menuState = MM_Running;
            }
            break;

        case MM_Running:
            GL_DEBUG(glColor4f)(1, 1, 1, 1 );

            // Buttons
            for ( cnt = 0; cnt < 1; cnt ++ )
            {
                if ( BUTTON_UP == ui_doButton( cnt + 1, buttons[cnt], buttonLeft, buttonTop + ( cnt * 35 ), 200, 30 ) )
                {
                    // audio options
                    menuChoice = cnt + 1;
                    menuState = MM_Leaving;
                }
            }

            // escape also kills this menu
            if ( SDLKEYDOWN( SDLK_ESCAPE ) )
            {
                menuChoice = 1;
                menuState = MM_Leaving;
            }

            ui_drawTextBox( endtext, x, y, w, h, 20 );

            break;

        case MM_Leaving:
            // Do buttons sliding out and background fading
            // Do the same stuff as in MM_Entering, but backwards
            GL_DEBUG(glColor4f)(1, 1, 1, 1 - SlidyButtonState.lerp );

            ui_drawTextBox( endtext, x, y, w, h, 20 );

            // Buttons
            drawSlidyButtons();
            updateSlidyButtons( deltaTime );
            if ( SlidyButtonState.lerp >= 1.0f )
            {
                menuState = MM_Finish;
            }
            break;

        case MM_Finish:
            {
                bool_t reloaded = bfalse;

                // try to pop the last module off the module stack
                reloaded = link_pop_module();

                // try to go to the world map
                //if( !reloaded )
                //{
                //    reloaded = link_load_parent( ModList[pickedmodule_index].parent_modname, ModList[pickedmodule_index].parent_pos );
                //}

                // fix the menu that is returned when you break out of the game
                if ( beatmodule && startNewPlayer )
                {
                    int return_menu;

                    // we started with a new player and beat the module... yay!
                    // now we want to graduate to the ChoosePlayer menu to
                    // build our party

                    startNewPlayer = bfalse;

                    // what menu were we supposed to go to?
                    return_menu = MainMenu;
                    if ( menu_stack_index > 0 )
                    {
                        return_menu = menu_stack[menu_stack_index-1];
                    }

                    // if we beat a beginner module, we want to
                    // go to ChoosePlayer instead of ChooseModule.
                    if ( return_menu == ChooseModule )
                    {
                        menu_stack_pop();
                        menu_stack_push( ChoosePlayer );
                    }
                }

                // actually quit the module
                if ( !reloaded )
                {
                    game_finish_module();
                    pickedmodule_index = -1;
                    gameactive   = bfalse;
                    moduleactive = bfalse;
                    menuactive   = btrue;
                }

                menuState = MM_Begin;

                // Set the next menu to load
                retval = menuChoice;
            }
    }

    return retval;
}


//--------------------------------------------------------------------------------------------
int doMenu( float deltaTime )
{
    int retval, result = 0;

    if ( mnu_whichMenu == MainMenu )
    {
        menu_stack_clear();
    };

    retval = MENU_NOTHING;

    switch ( mnu_whichMenu )
    {
        case MainMenu:
            result = doMainMenu( deltaTime );
            if ( result != 0 )
            {
                if ( result == 1 )      { mnu_begin_menu( ChooseModule ); startNewPlayer = btrue; }
                else if ( result == 2 ) { mnu_begin_menu( ChoosePlayer ); startNewPlayer = bfalse; }
                else if ( result == 3 ) { mnu_begin_menu( Options ); }
                else if ( result == 4 ) retval = MENU_QUIT;  // need to request a quit somehow
            }
            break;

        case SinglePlayer:
            result = doSinglePlayerMenu( deltaTime );

            if ( result != 0 )
            {
                if ( result == 1 )
                {
                    mnu_begin_menu( ChooseModule );
                    startNewPlayer = btrue;
                }
                else if ( result == 2 )
                {
                    mnu_begin_menu( ChoosePlayer );
                    startNewPlayer = bfalse;
                }
                else if ( result == 3 )
                {
                    mnu_end_menu();
                    retval = MENU_END;
                }
                else
                {
                    mnu_begin_menu( NewPlayer );
                }
            }
            break;

        case ChooseModule:
            result = doChooseModule( deltaTime );

            if ( result == -1 )     { mnu_end_menu(); retval = MENU_END; }
            else if ( result == 1 ) mnu_begin_menu( ShowMenuResults );  // imports are not valid (starter module)
            else if ( result == 2 ) mnu_begin_menu( ShowMenuResults );  // imports are valid

            break;

        case ChoosePlayer:
            result = doChoosePlayer( deltaTime );

            if ( result == -1 )     { mnu_end_menu(); retval = MENU_END; }
            else if ( result == 1 ) mnu_begin_menu( ChooseModule );

            break;

        case Options:
            result = doOptions( deltaTime );
            if ( result != 0 )
            {
                if ( result == 1 )      mnu_begin_menu( AudioOptions );
                else if ( result == 2 ) mnu_begin_menu( InputOptions );
                else if ( result == 3 ) mnu_begin_menu( VideoOptions );
                else if ( result == 4 ) { mnu_end_menu(); retval = MENU_END; }
            }
            break;

        case AudioOptions:
            result = doAudioOptions( deltaTime );
            if ( result != 0 )
            {
                mnu_end_menu();
                retval = MENU_END;
            }
            break;

        case VideoOptions:
            result = doVideoOptions( deltaTime );
            if ( result != 0 )
            {
                mnu_end_menu();
                retval = MENU_END;
            }
            break;

        case InputOptions:
            result = doInputOptions( deltaTime );
            if ( result != 0 )
            {
                mnu_end_menu();
                retval = MENU_END;
            }
            break;

        case ShowMenuResults:
            result = doShowMenuResults( deltaTime );
            if ( result != 0 )
            {
                mnu_end_menu();
                retval = MENU_SELECT;
            }
            break;

        case GamePaused:
            result = doGamePaused( deltaTime );
            if ( result != 0 )
            {
                if ( result == 1 )
                {
                    bool_t reloaded = bfalse;

                    mnu_end_menu();

                    // try to pop the last module off the module stack
                    reloaded = link_pop_module();

                    // try to go to the world map
                    //if( !reloaded )
                    //{
                    //    reloaded = link_load_parent( ModList[pickedmodule_index].parent_modname, ModList[pickedmodule_index].parent_pos );
                    //}

                    if ( !reloaded )
                    {
                        game_finish_module();
                        gameactive = bfalse;
                        moduleactive = bfalse;
                        menuactive = btrue;
                    }

                    result = MENU_QUIT;
                }
                else if ( result == 2 ) mnu_begin_menu( AudioOptions );
                else if ( result == 3 ) mnu_begin_menu( InputOptions );
                else if ( result == 4 ) mnu_begin_menu( VideoOptions );
                else if ( result == 5 ) { mnu_end_menu(); retval = MENU_END; }
            }
            break;

        case ShowEndgame:
            result = doShowEndgame( deltaTime );
            if ( result == 1 )
            {
                mnu_end_menu();
                retval = MENU_END;
                menuactive = btrue;
            }
            break;

        case NotImplemented:
        default:
            result = doNotImplemented( deltaTime );
            if ( result != 0 )
            {
                mnu_end_menu();
                retval = MENU_END;
            }
    }

    return retval;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
bool_t mnu_checkSelectedPlayer( Uint16 player )
{
    int i;
    if ( player > loadplayer_count ) return bfalse;

    for ( i = 0; i < MAXPLAYER && i < mnu_selectedPlayerCount; i++ )
    {
        if ( mnu_selectedPlayer[i] == player ) return btrue;
    }

    return bfalse;
}

//--------------------------------------------------------------------------------------------
Uint16 mnu_getSelectedPlayer( Uint16 player )
{
    Uint16 ipla;
    if ( player > loadplayer_count ) return INVALID_PLA;

    for ( ipla = 0; ipla < MAXPLAYER && ipla < mnu_selectedPlayerCount; ipla++ )
    {
        if ( mnu_selectedPlayer[ ipla ] == player ) return ipla;
    }

    return INVALID_PLA;
}

//--------------------------------------------------------------------------------------------
bool_t mnu_addSelectedPlayer( Uint16 player )
{
    if ( player > loadplayer_count || mnu_selectedPlayerCount >= MAXPLAYER ) return bfalse;
    if ( mnu_checkSelectedPlayer( player ) ) return bfalse;

    mnu_selectedPlayer[mnu_selectedPlayerCount] = player;
    mnu_selectedInput[mnu_selectedPlayerCount]  = INPUT_BITS_NONE;
    mnu_selectedPlayerCount++;

    return btrue;
}

//--------------------------------------------------------------------------------------------
bool_t mnu_removeSelectedPlayer( Uint16 player )
{
    int i;
    bool_t found = bfalse;

    if ( player > loadplayer_count || mnu_selectedPlayerCount <= 0 ) return bfalse;

    if ( mnu_selectedPlayerCount == 1 )
    {
        if ( mnu_selectedPlayer[0] == player )
        {
            mnu_selectedPlayerCount = 0;
        };
    }
    else
    {
        for ( i = 0; i < MAXPLAYER && i < mnu_selectedPlayerCount; i++ )
        {
            if ( mnu_selectedPlayer[i] == player )
            {
                found = btrue;
                break;
            }
        }

        if ( found )
        {
            i++;
            for ( /* nothing */; i < MAXPLAYER && i < mnu_selectedPlayerCount; i++ )
            {
                mnu_selectedPlayer[i-1] = mnu_selectedPlayer[i];
                mnu_selectedInput[i-1]  = mnu_selectedInput[i];
            }

            mnu_selectedPlayerCount--;
        }
    };

    return found;
}

//--------------------------------------------------------------------------------------------
bool_t mnu_addSelectedPlayerInput( Uint16 player, Uint32 input )
{
    int i;
    bool_t done, retval = bfalse;

    int selected_index = -1;

    for ( i = 0; i < mnu_selectedPlayerCount; i++ )
    {
        if ( mnu_selectedPlayer[i] == player )
        {
            selected_index = i;
            break;
        }
    }

    if ( -1 == selected_index )
    {
        mnu_addSelectedPlayer( player );
    }

    if ( selected_index >= 0 && selected_index < mnu_selectedPlayerCount )
    {
        for ( i = 0; i < mnu_selectedPlayerCount; i++ )
        {
            if ( i == selected_index  )
            {
                // add in the selected bits for the selected player
                mnu_selectedInput[i] |= input;
                retval = btrue;
            }
            else
            {
                // remove the selectd bits from all other players
                mnu_selectedInput[i] &= ~input;
            }
        }
    }

    // Do the tricky part of removing all players with invalid inputs from the list
    // It is tricky because removing a player changes the value of the loop control
    // value mnu_selectedPlayerCount within the loop.
    done = bfalse;
    while ( !done )
    {
        // assume the best
        done = btrue;

        for ( i = 0; i < mnu_selectedPlayerCount; i++ )
        {
            if ( INPUT_BITS_NONE == mnu_selectedInput[i] )
            {
                // we found one
                done = bfalse;
                mnu_removeSelectedPlayer( mnu_selectedPlayer[i] );
            }
        }
    }

    return retval;
}

//--------------------------------------------------------------------------------------------
bool_t mnu_removeSelectedPlayerInput( Uint16 player, Uint32 input )
{
    int i;
    bool_t retval = bfalse;

    for ( i = 0; i < MAXPLAYER && i < mnu_selectedPlayerCount; i++ )
    {
        if ( mnu_selectedPlayer[i] == player )
        {
            mnu_selectedInput[i] &= ~input;

            // This part is not so tricky as in mnu_addSelectedPlayerInput.
            // Even though we are modding the loop control variable, it is never
            // tested in the loop because we are using the break command to
            // break out of the loop immediately

            if ( INPUT_BITS_NONE == mnu_selectedInput[i] )
            {
                mnu_removeSelectedPlayer( player );
            }

            retval = btrue;

            break;
        }
    }

    return retval;
}

//--------------------------------------------------------------------------------------------
void check_player_import( const char *dirname, bool_t initialize )
{
    // ZZ> This function figures out which players may be imported, and loads basic
    //     data for each

    char searchname[128];
    STRING filename;
    int skin;
    const char *foundfile;

    if ( initialize )
    {
        // restart from nothing
        loadplayer_count = 0;
        globalicon_count = 0;
    };

    // Search for all objects
    sprintf( searchname, "%s" SLASH_STR "*.obj", dirname );
    foundfile = fs_findFirstFile( dirname, "obj" );

    while ( NULL != foundfile && loadplayer_count < MAXLOADPLAYER )
    {
        prime_names();
        sprintf( loadplayer[loadplayer_count].dir, "%s", foundfile );

        sprintf( filename, "%s" SLASH_STR "%s" SLASH_STR "skin.txt", dirname, foundfile );
        skin = get_skin( filename );

        snprintf( filename, sizeof(filename), "%s" SLASH_STR "%s" SLASH_STR "tris.md2", dirname, foundfile );
        md2_load_one( filename, &(MadList[loadplayer_count].md2) );

        sprintf( filename, "%s" SLASH_STR "%s" SLASH_STR "icon%d", dirname, foundfile, skin );
        load_one_icon( filename );

        sprintf( filename, "%s" SLASH_STR "%s" SLASH_STR "naming.txt", dirname, foundfile );
        chop_load( 0, filename );

        sprintf( loadplayer[loadplayer_count].name, "%s", chop_create( 0 ) );

        loadplayer_count++;

        foundfile = fs_findNextFile();
    }

    fs_findClose();
}

//--------------------------------------------------------------------------------------------
int get_skin( const char *filename )
{
    // ZZ> This function reads the skin.txt file...
    FILE*   fileread;
    int skin;

    skin = 0;
    fileread = fopen( filename, "r" );
    if ( fileread )
    {
        goto_colon( NULL, fileread, btrue );
        fscanf( fileread, "%d", &skin );
        skin %= MAXSKIN;
        fclose( fileread );
    }

    return skin;
}

//--------------------------------------------------------------------------------------------
void load_all_menu_images()
{
    // ZZ> This function loads the title image for each module.  Modules without a
    //     title are marked as invalid

    char loadname[256];
    int cnt;
    FILE* filesave;

    // reset all the title images
    release_all_titleimages();
    TxTitleImage_count = 0;

    // Log a directory list
    filesave = fopen( "modules.txt", "w" );
    if ( filesave != NULL )
    {
        fprintf( filesave, "This file logs all of the modules found\n" );
        fprintf( filesave, "** Denotes an invalid module\n" );
        fprintf( filesave, "## Denotes an unlockable module\n\n" );
    }

    // load all the title images for modules that we are going to display
    for ( cnt = 0; cnt < ModList_count; cnt++ )
    {
        // set the texture to some invlid value
        ModList[cnt].tex = (Uint32)(~0);

        if ( !ModList[cnt].loaded )
        {
            fprintf( filesave, "**.  %s\n", ModList[cnt].loadname );
        }
        else if ( modlist_test_by_index( cnt ) )
        {
            // NOTE: just because we can't load the title image DOES NOT mean that we ignore the module
            sprintf( loadname, "modules" SLASH_STR "%s" SLASH_STR "gamedat" SLASH_STR "title", ModList[cnt].loadname );

            ModList[cnt].tex = load_one_title_image( loadname );

            fprintf( filesave, "%02d.  %s\n", cnt, ModList[cnt].longname );
        }
        else
        {
            fprintf( filesave, "##.  %s\n", ModList[cnt].longname );
        }
    }

    if ( filesave != NULL ) fclose( filesave );
}



//--------------------------------------------------------------------------------------------
bool_t mnu_begin_menu( int which )
{
    if ( !menu_stack_push( mnu_whichMenu ) ) return bfalse;
    mnu_whichMenu = which;

    return btrue;
}

//--------------------------------------------------------------------------------------------
void   mnu_end_menu()
{
    mnu_whichMenu = menu_stack_pop();
}

//--------------------------------------------------------------------------------------------
int mnu_get_menu_depth()
{
    return menu_stack_index;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
bool_t menu_stack_push( int menu )
{
    if ( menu_stack_index < 0 )
    {
        menu_stack_index = 0;
    }
    if (menu_stack_index > MENU_STACK_COUNT)
    {
        menu_stack_index = MENU_STACK_COUNT;
    }

    if ( menu_stack_index >= MENU_STACK_COUNT ) return bfalse;


    menu_stack[menu_stack_index] = menu;
    menu_stack_index++;

    return btrue;
}

//--------------------------------------------------------------------------------------------
int menu_stack_pop()
{
    if ( menu_stack_index < 0 )
    {
        menu_stack_index = 0;
        return MainMenu;
    }
    if (menu_stack_index > MENU_STACK_COUNT)
    {
        menu_stack_index = MENU_STACK_COUNT;
    }

    if ( menu_stack_index == 0 ) return MainMenu;

    menu_stack_index--;
    return menu_stack[menu_stack_index];
}

//--------------------------------------------------------------------------------------------
void menu_stack_clear()
{
    menu_stack_index = 0;
    menu_stack[0] = MainMenu;
}
