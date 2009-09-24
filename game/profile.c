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
//*    along with Egoboo.  If not, see <http://www.gnu.org/licenses/>.
//*
//********************************************************************************************

/* Egoboo - profile.c
 */

#include "profile.h"

#include "char.h"
#include "particle.h"
#include "mad.h"
#include "enchant.h"

#include "texture.h"
#include "log.h"
#include "script_compile.h"
#include "game.h"

#include "egoboo_setup.h"
#include "egoboo_strutil.h"
#include "egoboo_fileutil.h"
#include "egoboo_vfs.h"

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
static _profile_initialized = bfalse;

chop_data_t  chop = {0, 0};
pro_import_t import_data;

Uint16  bookicon_count   = 0;
Uint16  bookicon_ref[MAX_SKIN];                      // The first book icon

DECLARE_LIST ( ACCESS_TYPE_NONE, pro_t,  ProList  );

DECLARE_STACK( ACCESS_TYPE_NONE, int, MessageOffset );

Uint32  message_buffer_carat = 0;                           // Where to put letter
char    message_buffer[MESSAGEBUFFERSIZE];                  // The text buffer

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
static void get_message( vfs_FILE* fileread );

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void init_all_profiles()
{
    // ZZ> This function initializes all of the model profiles

    int tnc;

    // initialize all the sub-profile lists
    init_all_pip();
    init_all_eve();
    init_all_cap();
    init_all_mad();
    init_all_ai_scripts();

    // initialize the profile list
    ProList_init();

    // fix the book icon list
    for( tnc = 0; tnc < MAX_SKIN; tnc++ )
    {
        bookicon_ref[tnc] = INVALID_TEXTURE;
    }
    bookicon_count = 0;
}

//---------------------------------------------------------------------------------------------
void release_all_profiles()
{
    // ZZ> This function clears out all of the model data

    // release the allocated data in all profiles (sounds, textures, etc.)
    release_all_pro_data();

    // relese every type of sub-profile and re-initalize the lists
    release_all_pip();
    release_all_eve();
    release_all_cap();
    release_all_mad();
    release_all_ai_scripts();

    // re-initialize the profile list
    ProList_init();
}

//---------------------------------------------------------------------------------------------
void profile_init()
{
    // BB> initialize the profile list and load up some intialization files
    //     necessary for the the profile loading code to work

    if( _profile_initialized )
    {
        // release all profile data and reinitialize the profile list
        release_all_profiles();

        _profile_initialized = bfalse;
    }

    // initialize all the profile lists
    init_all_profiles();

    // necessary for loading up the ai script
    init_all_ai_scripts();
    load_ai_codes( "basicdat" SLASH_STR "aicodes.txt" );

    // necessary for loading up the copy.txt file
    load_action_names( "basicdat" SLASH_STR "actions.txt" );

    // necessary for properly reading the "message.txt"
    reset_messages();

    // necessary for reading "naming.txt" properly
    prime_names();

    // something that is used in the game that is somewhat related to the profile stuff
    init_slot_idsz();

    // let the code know that everything is initialized
    _profile_initialized = btrue;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
bool_t pro_init( pro_t * pobj )
{
    int cnt;

    if( NULL == pobj ) return bfalse;

    if( pobj->loaded )
    {
        log_warning( "pro_init() - trying to init an object in use" );
    }

    //---- reset everything to safe values
    memset( pobj, 0, sizeof(pro_t) );

    pobj->iai  = 0;
    pobj->icap = MAX_CAP;
    pobj->imad = MAX_MAD;
    pobj->ieve = MAX_EVE;

    for( cnt = 0; cnt < MAX_PIP_PER_PROFILE; cnt++ )
    {
        pobj->prtpip[cnt] = MAX_PIP;
    }

    for ( cnt = 0; cnt < MAXSECTION; cnt++ )
    {
        pobj->chop_sectionstart[cnt] = MAXCHOP;
        pobj->chop_sectionsize[cnt]  = 0;
    }

    // do the final invalidation
    pobj->loaded   = bfalse;
    strncpy( pobj->name, "*NONE*", SDL_arraysize(pobj->name) );

    // clear out the textures
    for ( cnt = 0; cnt < MAX_SKIN; cnt++)
    {
        pobj->tex_ref[cnt] = INVALID_TEXTURE;
        pobj->ico_ref[cnt] = INVALID_TEXTURE;
    }

    return btrue;
}

//--------------------------------------------------------------------------------------------
// The "private" ProList management functions
//--------------------------------------------------------------------------------------------
int ProList_search_free( Uint16 iobj )
{
    // BB> if an object of index iobj exists on the free list, return the free list index
    //     otherwise return -1

    int cnt, retval;

    // determine whether this character is already in the list of free textures
    // that is an error
    retval = -1;
    for ( cnt = 0; cnt < ProList.free_count; cnt++ )
    {
        if ( iobj == ProList.free_ref[cnt] )
        {
            retval = cnt;
            break;
        }
    }

    return retval;
}

//--------------------------------------------------------------------------------------------
int ProList_pop_free( int idx )
{
    // BB> pop off whatever object exists at the free list index idx

    int retval;

    if( idx >= 0 && idx < ProList.free_count )
    {
        // move the index idx to the top
        int idx_top, idx_bottom;

        idx_bottom = idx;
        idx_top    = ProList.free_count-1;

        // make sure this is a valid case
        if( idx_top > idx_bottom && idx_top > 1 )
        {
            SWAP(int, ProList.free_ref[idx_top], ProList.free_ref[idx_bottom] );
        }
    }

    // just pop off the top index
    retval = -1;
    if( ProList.free_count > 0 )
    {
        ProList.free_count--;
        retval = ProList.free_ref[ProList.free_count];
    }

    return retval;
}

//--------------------------------------------------------------------------------------------
bool_t ProList_push_free( Uint16 iobj )
{
    // BB> push an object onto the free stack

    int retval;

#if defined(USE_DEBUG)
    // determine whether this character is already in the list of free objects
    // that is an error
    if( -1 != ProList_search_free(iobj) ) return bfalse;
#endif

    // push it on the free stack
    retval = bfalse;
    if ( ProList.free_count < MAX_PROFILE )
    {
        ProList.free_ref[ProList.free_count] = iobj;
        ProList.free_count++;

        retval = btrue;
    }

    return retval;
}

//--------------------------------------------------------------------------------------------
// The "public" ProList management functions
//--------------------------------------------------------------------------------------------
void ProList_init()
{
    // BB> initialize all the objects and the object free list.
    //     call before ever using the object list.

    int cnt;

    ProList.free_count = 0;
    for ( cnt = 0; cnt < MAX_PROFILE; cnt++ )
    {
        // make sure we don't get a stupid warning
        ProList.lst[cnt].loaded = bfalse;
        pro_init( ProList.lst + cnt );

        ProList_push_free( cnt );
    }
}

//--------------------------------------------------------------------------------------------
Uint16 ProList_get_free( Uint16 override )
{
    // ZZ> This function returns the next free character or MAX_PROFILE if there are none

    Uint16 retval = MAX_PROFILE;

    if( VALID_PRO_RANGE(override) )
    {
        // grab the object that is specified.
        int free_idx;

        // if the object is in use, make sure to free everything associated with it
        if( VALID_PRO(override) )
        {
            release_one_pro( override );
        }

        // grab the specified index
        free_idx = ProList_search_free(override);
        retval   = ProList_pop_free(free_idx);
    }
    else
    {
        // grab the next free index
        retval = ProList_pop_free(-1);
    }

    return retval;
}

//--------------------------------------------------------------------------------------------
bool_t ProList_free_one( Uint16 iobj )
{
    // ZZ> This function sticks an object back on the free object stack

    if ( !VALID_PRO_RANGE(iobj) ) return bfalse;

    // object "destructor"
    // inilializes an object to safe values
    pro_init( ProList.lst + iobj );

    return ProList_push_free( iobj );
}

//--------------------------------------------------------------------------------------------
// object functions
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
bool_t release_one_profile_textures( Uint16 iobj )
{
    int tnc;
    pro_t  * pobj;

    if ( !VALID_PRO(iobj) ) return bfalse;
    pobj = ProList.lst + iobj;

    for ( tnc = 0; tnc < MAX_SKIN; tnc++ )
    {
        int itex;

        itex = pobj->tex_ref[tnc] ;
        if ( itex > TX_LAST )
        {
            TxTexture_free_one( itex );
        }

        itex = pobj->ico_ref[tnc] ;
        if ( itex > TX_LAST )
        {
            TxTexture_free_one( itex );
        }
    }

    // reset the bookicon stuff if this object is a book
    if ( SPELLBOOK == iobj )
    {
        for( tnc = 0; tnc < MAX_SKIN; tnc++ )
        {
            bookicon_ref[tnc] = INVALID_TEXTURE;
        }
        bookicon_count = 0;
    }

    return btrue;
}

//--------------------------------------------------------------------------------------------
void release_all_profile_textures()
{
    int cnt;

    for ( cnt = 0; cnt < MAX_PROFILE; cnt++ )
    {
        release_one_profile_textures(cnt);
    }
}

//--------------------------------------------------------------------------------------------
bool_t release_one_pro_data( Uint16 iobj )
{
    int cnt;
    pro_t * pobj;

    if( !VALID_PRO(iobj) ) return bfalse;
    pobj = ProList.lst + iobj;

    // free all sounds
    for ( cnt = 0; cnt < MAX_WAVE; cnt++ )
    {
        sound_free_chunk( pobj->wavelist[cnt] );
        pobj->wavelist[cnt] = NULL;
    }

    // release whatever textures are being used
    release_one_profile_textures( iobj );

    return btrue;
}

//--------------------------------------------------------------------------------------------
bool_t release_one_pro( Uint16 iobj )
{
    pro_t * pobj;

    if( !VALID_PRO_RANGE(iobj) ) return bfalse;

    if( !VALID_PRO(iobj) ) return btrue;
    pobj = ProList.lst + iobj;

    // release all of the sub-profiles
    //release_one_ai ( pobj->iai  );
    release_one_cap( pobj->icap );
    release_one_mad( pobj->imad );
    //release_one_eve( pobj->ieve );

    release_one_local_pips( iobj );

    // release the allocated data
    release_one_pro_data( iobj );

    pro_init( pobj );

    return btrue;
}

//--------------------------------------------------------------------------------------------
void release_all_pro()
{
    // BB> release the allocated data for all objects
    int cnt;

    for ( cnt = 0; cnt < MAX_PROFILE; cnt++ )
    {
        release_one_pro( cnt );
    }
}

//--------------------------------------------------------------------------------------------
void release_all_pro_data()
{
    // BB> release the allocated data for all objects
    int cnt;

    for ( cnt = 0; cnt < MAX_PROFILE; cnt++ )
    {
        release_one_pro_data( cnt );
    }
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
Uint16 pro_get_icap(Uint16 iobj)
{
    pro_t * pobj;

    if( INVALID_PRO(iobj) ) return MAX_CAP;
    pobj = ProList.lst + iobj;

    return VALID_CAP(pobj->icap) ? pobj->icap : MAX_CAP;
}

//--------------------------------------------------------------------------------------------
Uint16 pro_get_imad(Uint16 iobj)
{
    pro_t * pobj;

    if( INVALID_PRO(iobj) ) return MAX_MAD;
    pobj = ProList.lst + iobj;

    return VALID_MAD(pobj->imad) ? pobj->imad : MAX_MAD;
}

//--------------------------------------------------------------------------------------------
Uint16 pro_get_ieve(Uint16 iobj)
{
    pro_t * pobj;

    if( INVALID_PRO(iobj) ) return MAX_EVE;
    pobj = ProList.lst + iobj;

    return VALID_EVE(pobj->ieve) ? pobj->ieve : MAX_EVE;
}

//--------------------------------------------------------------------------------------------
Uint16 pro_get_ipip(Uint16 iobj, Uint16 ipip)
{
    pro_t * pobj;

    if( INVALID_PRO(iobj) ) return MAX_PIP;
    pobj = ProList.lst + iobj;

    // find the local pip if it exists
    if( ipip < MAX_PIP_PER_PROFILE )
    {
        ipip = pobj->prtpip[ipip];
    }

    return VALID_PIP(ipip) ? ipip : MAX_PIP;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
cap_t * pro_get_pcap(Uint16 iobj)
{
    pro_t * pobj;

    if( INVALID_PRO(iobj) ) return NULL;
    pobj = ProList.lst + iobj;

    if( INVALID_CAP(pobj->icap) ) return NULL;

    return CapList + pobj->icap;
}

//--------------------------------------------------------------------------------------------
mad_t * pro_get_pmad(Uint16 iobj)
{
    pro_t * pobj;

    if( INVALID_PRO(iobj) ) return NULL;
    pobj = ProList.lst + iobj;

    if( INVALID_MAD(pobj->imad) ) return NULL;

    return MadList + pobj->imad;
}

//--------------------------------------------------------------------------------------------
eve_t * pro_get_peve(Uint16 iobj)
{
    pro_t * pobj;

    if( INVALID_PRO(iobj) ) return NULL;
    pobj = ProList.lst + iobj;

    if( INVALID_EVE(pobj->ieve) ) return NULL;

    return EveStack.lst + pobj->ieve;
}

//--------------------------------------------------------------------------------------------
pip_t * pro_get_ppip(Uint16 iobj, Uint16 ipip)
{
    pro_t * pobj;

    if( INVALID_PRO(iobj) ) return NULL;
    pobj = ProList.lst + iobj;

    // find the local pip if it exists
    if( ipip < MAX_PIP_PER_PROFILE )
    {
        ipip = pobj->prtpip[ipip];
    }

    if( INVALID_PIP(ipip) ) return NULL;

    return PipStack.lst + ipip;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
IDSZ pro_get_idsz( Uint16 iobj, int type )
{
    cap_t * pcap;

    if(type >= IDSZ_COUNT) return IDSZ_NONE;

    pcap = pro_get_pcap( iobj );
    if( NULL == pcap) return IDSZ_NONE;

    return pcap->idsz[type];
}

//--------------------------------------------------------------------------------------------
Mix_Chunk * pro_get_chunk(Uint16 iobj, int index)
{
    pro_t * pobj;

    if( index < 0 || index >= MAX_WAVE ) return NULL;

    if( INVALID_PRO(iobj) ) return NULL;
    pobj = ProList.lst + iobj;

    return pobj->wavelist[index];
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void prime_names()
{
    // ZZ> This function prepares the name chopper for use

    chop.count = 0;
    chop.carat = 0;
}

//--------------------------------------------------------------------------------------------
int load_profile_skins( const char * tmploadname, Uint16 object )
{
    int min_skin_tx, min_icon_tx;
    int max_skin, max_icon, max_tex;
    int iskin, iicon;
    int cnt;

    STRING newloadname;

    pro_t * pobj;

    if ( !VALID_PRO_RANGE(object) ) return 0;
    pobj = ProList.lst + object;

    // Load the skins and icons
    max_skin    = max_icon    = -1;
    min_skin_tx = min_icon_tx = INVALID_TEXTURE;
    for ( cnt = 0; cnt < MAX_SKIN; cnt++)
    {
        snprintf( newloadname, SDL_arraysize(newloadname), "%s" SLASH_STR "tris%d", tmploadname, cnt );

        pobj->tex_ref[cnt] = TxTexture_load_one( newloadname, INVALID_TEXTURE, TRANSCOLOR );
        if ( INVALID_TEXTURE != pobj->tex_ref[cnt] )
        {
            max_skin = cnt;
            if ( INVALID_TEXTURE == min_skin_tx )
            {
                min_skin_tx = pobj->tex_ref[cnt];
            }
        }

        snprintf( newloadname, SDL_arraysize(newloadname), "%s" SLASH_STR "icon%d", tmploadname, cnt );
        pobj->ico_ref[cnt] = TxTexture_load_one( newloadname, INVALID_TEXTURE, INVALID_KEY );

        if ( INVALID_TEXTURE != pobj->ico_ref[cnt] )
        {
            max_icon = cnt;

            if ( INVALID_TEXTURE == min_icon_tx )
            {
                min_icon_tx = pobj->ico_ref[cnt];
            }

            if ( SPELLBOOK == object )
            {
                if ( bookicon_count < MAX_SKIN )
                {
                    bookicon_ref[bookicon_count] = pobj->ico_ref[cnt];
                    bookicon_count++;
                }
            }
        }
    }

    if ( max_skin < 0 )
    {
        // If we didn't get a skin, set it to the water texture
        max_skin = 0;
        pobj->tex_ref[cnt] = TX_WATER_TOP;

        if (cfg.dev_mode)
        {
            log_message( "NOTE: Object is missing a skin (%s)!\n", tmploadname );
        }
    }

    max_tex = MAX(max_skin, max_icon);

    // fill in any missing textures
    iskin = min_skin_tx;
    iicon = min_icon_tx;
    for ( cnt = 0; cnt <= max_tex; cnt++ )
    {
        if ( INVALID_TEXTURE != pobj->tex_ref[cnt] && iskin != pobj->tex_ref[cnt] )
        {
            iskin = pobj->tex_ref[cnt];
        }

        if ( INVALID_TEXTURE != pobj->ico_ref[cnt] && iicon != pobj->ico_ref[cnt] )
        {
            iicon = pobj->ico_ref[cnt];
        }

        pobj->tex_ref[cnt] = iskin;
        pobj->ico_ref[cnt] = iicon;
    }

    return max_tex + 1;
}

//--------------------------------------------------------------------------------------------
void get_message( vfs_FILE* fileread )
{
    // ZZ> This function loads a string into the message buffer, making sure it
    //    is null terminated.
    int cnt;
    char cTmp;
    STRING szTmp;

    if ( message_buffer_carat >= MESSAGEBUFFERSIZE )
    {
        message_buffer_carat = MESSAGEBUFFERSIZE - 1;
        message_buffer[message_buffer_carat] = '\0';
        return;
    }

    if ( MessageOffset.count >= MAXTOTALMESSAGE )
    {
        return;
    }

    MessageOffset.lst[MessageOffset.count] = message_buffer_carat;
    fget_string( fileread, szTmp, SDL_arraysize(szTmp) );
    szTmp[255] = '\0';

    cTmp = szTmp[0];
    cnt = 1;
    while ( '\0' != cTmp && message_buffer_carat < MESSAGEBUFFERSIZE - 1 )
    {
        if ( '_' == cTmp )  cTmp = ' ';

        message_buffer[message_buffer_carat] = cTmp;
        message_buffer_carat++;
        cTmp = szTmp[cnt];
        cnt++;
    }

    message_buffer[message_buffer_carat] = '\0';
    message_buffer_carat++;
    MessageOffset.count++;
}

//--------------------------------------------------------------------------------------------
void load_all_messages( const char *loadname, Uint16 object )
{
    // ZZ> This function loads all of an objects messages
    vfs_FILE *fileread;

    ProList.lst[object].message_start = 0;
    fileread = vfs_openRead( loadname );
    if ( fileread )
    {
        ProList.lst[object].message_start = MessageOffset.count;

        while ( goto_colon( NULL, fileread, btrue ) )
        {
            get_message( fileread );
        }

        vfs_close( fileread );
    }
}

//--------------------------------------------------------------------------------------------
bool_t release_one_local_pips( Uint16 iobj )
{
    int cnt;
    pro_t * pobj;

    if( !VALID_PRO_RANGE(iobj) ) return bfalse;

    if( INVALID_PRO(iobj) ) return btrue;
    pobj = ProList.lst + iobj;

    for ( cnt = 0; cnt < MAX_PIP_PER_PROFILE; cnt++ )
    {
        release_one_pip( pobj->prtpip[cnt] );
        pobj->prtpip[cnt] = MAX_PIP;
    }

    return btrue;
}

//--------------------------------------------------------------------------------------------
void release_all_local_pips()
{
    // clear out the local pips

    int object, cnt;

    for( object = 0; object < MAX_PROFILE; object++ )
    {
        pro_t * pobj;

        if( !ProList.lst[object].loaded) continue;
        pobj = ProList.lst + object;

        for ( cnt = 0; cnt < MAX_PIP_PER_PROFILE; cnt++ )
        {
            release_one_pip( pobj->prtpip[cnt] );
            pobj->prtpip[cnt] = MAX_PIP;
        }
    }
}

//--------------------------------------------------------------------------------------------
int obj_read_slot( const char * tmploadname )
{
    vfs_FILE* fileread;
    int slot;
    STRING szLoadName;

    make_newloadname( tmploadname, SLASH_STR "data.txt", szLoadName );

    // Open the file
    fileread = vfs_openRead( szLoadName );
    if ( NULL == fileread ) return -1;

    // load the slot's slot no matter what
    slot = fget_next_int( fileread );

    vfs_close( fileread );

    return slot;
}

//--------------------------------------------------------------------------------------------
bool_t obj_verify_file( const char * tmploadname )
{
    STRING szLoadName;

    make_newloadname( tmploadname, SLASH_STR "data.txt", szLoadName );

    // Open the file
    return vfs_exists( szLoadName );
}

//--------------------------------------------------------------------------------------------
int pro_get_slot( const char * tmploadname, int slot_override )
{
    int slot;

    slot = -1;
    if( VALID_PRO_RANGE(slot_override) )
    {
        // just use the slot that was provided
        slot = slot_override;
    }
    else
    {
        // grab the slot from the file
        int tmp_slot = obj_read_slot( tmploadname );

        // set the slot slot
        if ( tmp_slot >= 0 )
        {
            slot = tmp_slot;
        }
        else if ( import_data.slot >= 0 )
        {
            slot = import_data.slot;
        }
    }

    // return an error value if the file does not exist
    if( !obj_verify_file(tmploadname) )
    {
        slot = -1;
    }

    return slot;
}

//--------------------------------------------------------------------------------------------
int load_one_profile( const char* tmploadname, int slot_override )
{
    // ZZ> This function loads one object and returns the object slot

    int cnt;
    STRING newloadname;
    bool_t required;

    int     iobj;
    pro_t * pobj;

    required = !VALID_CAP_RANGE(slot_override);

    // get a slot value
    iobj = pro_get_slot( tmploadname, slot_override );

    // throw an error code if the slot is invalid of if the file doesn't exist
    if( !VALID_PRO_RANGE(iobj) )
    {
        // The data file wasn't found

        if ( required )
        {
            log_error( "load_one_profile() - \"%s\" was not found!\n", tmploadname );
        }
        else if( VALID_CAP_RANGE(slot_override) && slot_override > PMod->importamount * MAXIMPORTPERPLAYER )
        {
            log_warning( "load_one_profile() - Not able to open file \"%s\"\n", tmploadname );
        }

        return MAX_PROFILE;
    }

    // throw an error code if we are trying to load over an existing profile
    // without permission
    if( VALID_PRO(iobj) )
    {
        pro_t * pobj = ProList.lst + iobj;

        // Make sure global objects don't load over existing models
        if ( required && SPELLBOOK == iobj )
        {
            log_error( "load_one_profile() - object slot %i is a special reserved slot number (cannot be used by %s).\n", SPELLBOOK, tmploadname );
        }
        else if ( required && overrideslots )
        {
            log_error( "load_one_profile() - object slot %i used twice (%s, %s)\n", iobj, pobj->name, tmploadname );
        }
        else
        {
            // Stop, we don't want to override it
            return MAX_PROFILE;
        }
    }

    // allocate/reallocate this slot
    iobj = ProList_get_free( iobj );
    if( !VALID_PRO_RANGE(iobj) )
    {
        log_warning( "load_one_profile() - Cannot allocate object %d (\"%s\")\n", iobj, tmploadname );
        return MAX_PROFILE;
    }

    // grab a pointer to the object
    pobj = ProList.lst + iobj;

    // load the character profile
    pobj->icap = load_one_character_profile( tmploadname, iobj, bfalse );

    // Load the model for this iobj
    pobj->imad = load_one_model_profile( tmploadname, iobj );

    // Load the enchantment for this iobj
    make_newloadname( tmploadname, SLASH_STR "enchant.txt", newloadname );
    pobj->ieve = load_one_enchant_profile( newloadname, iobj );

    // Load the AI script for this iobj
    make_newloadname( tmploadname, SLASH_STR "script.txt", newloadname );
    pobj->iai = load_ai_script( newloadname );

    // Load the messages for this iobj
    make_newloadname( tmploadname, SLASH_STR "message.txt", newloadname );
    load_all_messages( newloadname, iobj );

    // Load the particles for this iobj
    for ( cnt = 0; cnt < MAX_PIP_PER_PROFILE; cnt++ )
    {
        snprintf( newloadname, SDL_arraysize( newloadname), "%s" SLASH_STR "part%d.txt", tmploadname, cnt );

        // Make sure it's referenced properly
        pobj->prtpip[cnt] = load_one_particle_profile( newloadname, MAX_PIP );
    }

    pobj->skins = load_profile_skins( tmploadname, iobj );

    // Load the waves for this iobj
    for ( cnt = 0; cnt < MAX_WAVE; cnt++ )
    {
        STRING  szLoadName, wavename;

        snprintf( wavename, SDL_arraysize( wavename), SLASH_STR "sound%d", cnt );
        make_newloadname( tmploadname, wavename, szLoadName );
        pobj->wavelist[cnt] = sound_load_chunk( szLoadName );
    }

    // Load the random naming table for this icap
    make_newloadname( tmploadname, SLASH_STR "naming.txt", newloadname );
    chop_load( iobj, newloadname );

    // Fix lighting if need be
    if ( CapList[pobj->icap].uniformlit )
    {
        mad_make_equally_lit( pobj->imad );
    }

    // mark the profile as loaded
    strncpy(pobj->name, tmploadname, SDL_arraysize(pobj->name) );
    pobj->loaded = btrue;

    return iobj;
}

//--------------------------------------------------------------------------------------------
void chop_load( Uint16 profile, const char *szLoadname )
{
    // ZZ> This function reads a naming file
    vfs_FILE *fileread;
    int   section, chopinsection;
    char  tmp_chop[32];

    fileread = vfs_openRead( szLoadname );
    if ( NULL == fileread ) return;

    section = 0;
    chopinsection = 0;
    while ( section < MAXSECTION && chop.carat < CHOPDATACHUNK && goto_colon( NULL, fileread, btrue ) )
    {
        fget_string( fileread, tmp_chop, SDL_arraysize(tmp_chop) );

        // convert all the '_' and junk in the string
        str_decode( tmp_chop, SDL_arraysize(tmp_chop), tmp_chop);

        if ( 0 == strcmp(tmp_chop, "STOP") )
        {
            if ( section < MAXSECTION )
            {
                ProList.lst[profile].chop_sectionsize[section]  = chopinsection;
                ProList.lst[profile].chop_sectionstart[section] = chop.count - chopinsection;
            }

            section++;
            chopinsection = 0;
            tmp_chop[0] = '\0';
        }
        else
        {
            int chop_len;

            // fill in the chop data
            chop.start[chop.count] = chop.carat;
            chop_len = snprintf( chop.buffer + chop.carat, CHOPDATACHUNK - chop.carat - 1, "%s", tmp_chop );

            chop.carat += chop_len + 1;
            chop.count++;
            chopinsection++;
            tmp_chop[0] = '\0';
        }
    }

    // handle the case where the chop buffer has overflowed
    // pretend the last command was "STOP"
    if ( '\0' != tmp_chop[0] && section < MAXSECTION )
    {
        ProList.lst[profile].chop_sectionsize[section]  = chopinsection;
        ProList.lst[profile].chop_sectionstart[section] = chop.count - chopinsection;
    }

    vfs_close( fileread );
}

//--------------------------------------------------------------------------------------------
void reset_messages()
{
    // ZZ> This makes messages safe to use
    int cnt;

    MessageOffset.count = 0;
    message_buffer_carat = 0;
    msgtimechange = 0;
    DisplayMsg.count = 0;

    for ( cnt = 0; cnt < MAX_MESSAGE; cnt++ )
    {
        DisplayMsg.lst[cnt].time = 0;
    }

    for ( cnt = 0; cnt < MAXTOTALMESSAGE; cnt++ )
    {
        MessageOffset.lst[cnt] = 0;
    }

    message_buffer[0] = '\0';
}