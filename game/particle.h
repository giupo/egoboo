#pragma once

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

#include "egoboo_typedef.h"
#include "egoboo_math.h"

#include "pip_file.h"

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

#define PRTLEVELFIX         20                      ///< Fix for shooting over cliffs

/// Particles
#define MAXPARTICLEIMAGE                256         ///< Number of particle images ( frames )
#define DYNAFANS  12

/// Physics
#define STOPBOUNCINGPART                5.0f         ///< To make particles stop bouncing

DEFINE_STACK_EXTERN(pip_t, PipStack, MAX_PIP );

#define VALID_PIP_RANGE( IPIP ) ( ((IPIP) >= 0) && ((IPIP) < MAX_PIP) )
#define LOADED_PIP( IPIP )       ( VALID_PIP_RANGE( IPIP ) && PipStack.lst[IPIP].loaded )

//------------------------------------
// Particle graphic data
//------------------------------------

/// All the data necessary to diaplay a partile
struct s_prt_instance
{
    bool_t valid;

    // basic info
    Uint8    type;               ///< particle type
    Uint16   image;              ///< which image
    float    alpha;              ///< base alpha
    Uint8    light;              ///< base self lighting

    // position info
    fvec3_t   pos;
    float     size;

    // billboard info
    prt_ori_t orientation;
    fvec3_t   up;
    fvec3_t   right;
    fvec3_t   nrm;

    // lighting info
    float    famb;               ///< cached ambient light
    float    fdir;               ///< cached directional light

    float    fintens;            ///< current brightness
    float    falpha;             ///< current alpha

    // graphical optimizations
    bool_t         indolist;        ///< Has it been added yet?

    // pre-compute some values for the reflected particle posisions
    fvec3_t ref_up;
    fvec3_t ref_right;
    fvec3_t ref_pos;
};
typedef struct s_prt_instance prt_instance_t;

//--------------------------------------------------------------------------------------------
/// Everything that is necessary to compute the character's interaction with the environment
struct s_prt_environment
{
    // floor stuff
    Uint8  twist;
    float  floor_level;           ///< Height of tile
    float  level;                 ///< Height of a tile or a platform
    float  zlerp;
    float  hlerp;

    // friction stuff
    bool_t is_slipping;
    bool_t is_slippy,    is_watery;
    float  air_friction, ice_friction;
    float  fluid_friction_xy, fluid_friction_z;
    float  friction_xy;
    float  traction;

    // misc states
    bool_t   inwater;
    fvec3_t  acc;
};
typedef struct s_prt_environment prt_environment_t;

//--------------------------------------------------------------------------------------------
// Particle variables
//--------------------------------------------------------------------------------------------

/// The definition of the particle object
/// This "inherits" for ego_object_base_t
struct s_prt
{
    ego_object_base_t obj_base;

    // profiles
    Uint16  pip_ref;                         ///< The part template
    Uint16  profile_ref;                     ///< the profile related to the spawned particle

    // links
    Uint16  attachedto_ref;                  ///< For torch flame
    Uint16  owner_ref;                       ///< The character that is attacking
    Uint16  target_ref;                      ///< Who it's chasing
    Uint16  parent_ref;                      ///< Did a another particle spawn this one?
    Uint32  parent_guid;                     ///< Just in case, the parent particle was despawned and a differnt particle now has the parent_ref

    Uint16  vrt_off;                         ///< It's vertex offset
    Uint8   type;                            ///< Transparency mode, 0-2
    Uint16  facing;                          ///< Direction of the part
    Uint8   team;                            ///< Team

    fvec3_t pos, pos_old, pos_stt;       ///< Position
    fvec3_t vel, vel_old, vel_stt;       ///< Velocity

    Uint32  onwhichfan;                      ///< Where the part is
    Uint32  onwhichblock;                    ///< The particle's collision block
    Uint16  onwhichplatform;                 ///< Is the particle on a platform?
    bool_t  is_hidden;                       ///< Is the particle related to a hidden character?

    Uint16  rotate;                          ///< Rotation direction
    Sint16  rotateadd;                       ///< Rotation rate

    int  size;                            ///< Size of particle (8.8-bit fixed point)
    Uint16  size_stt;                        ///< the starting size of the particle (8.8-bit fixed point)
    Sint16  size_add;                        ///< Change in size

    bool_t  inview;                          ///< Render this one?
    Uint16  image;                           ///< Which image (8.8-bit fixed point)
    Uint16  imageadd;                        ///< Animation rate
    Uint16  imagemax;                        ///< End of image loop
    Uint16  imagestt;                        ///< Start of image loop

    int     time_update;                     ///< Duration of particle
    int     time_frame;                      ///< Duration of particle
    Uint16  spawntime;                       ///< Time until spawn

    Uint32  bumpsize;                        ///< Size of bumpers
    Uint32  bumpsizebig;
    Uint8   bumpheight;                      ///< Bounding box height
    Uint16  bumplist_next;                   ///< Next particle on fanblock
    IPair   damage;                          ///< For strength
    Uint8   damagetype;                      ///< Damage type

    bool_t  is_eternal;

    bool_t is_bumpspawn;                      ///< this particle is like a flame, burning something
    bool_t inwater;

    bool_t   spawncharacterstate;              ///< if != SPAWNNOCHARACTER, then a character is spawned on end

    bool_t         safe_valid;
    fvec3_t        pos_safe;                      ///< Character's last safe position

    bool_t         is_homing;                 ///< Is the particle in control of its motion?

    dynalight_info_t  dynalight;              ///< Dynamic lighting...
    prt_instance_t    inst;                   ///< Everything needed for rendering
    prt_environment_t enviro;                 ///< the particle's environment
    phys_data_t       phys;                   ///< the particle's physics data
};
typedef struct s_prt prt_t;

extern float            sprite_list_u[MAXPARTICLEIMAGE][2];        ///< Texture coordinates
extern float            sprite_list_v[MAXPARTICLEIMAGE][2];

extern Uint16           maxparticles;                              ///< max number of particles

DEFINE_LIST_EXTERN(prt_t, PrtList, TOTAL_MAX_PRT);

#define VALID_PRT_RANGE( IPRT ) ( ((IPRT) >= 0) && ((IPRT) < maxparticles) && ((IPRT) < TOTAL_MAX_PRT) )
#define ALLOCATED_PRT( IPRT )   ( VALID_PRT_RANGE( IPRT ) && ALLOCATED_OBJ ( &(PrtList.lst[IPRT].obj_base) ) )
#define ACTIVE_PRT( IPRT )      ( VALID_PRT_RANGE( IPRT ) && ACTIVE_OBJ    ( &(PrtList.lst[IPRT].obj_base) ) )
#define WAITING_PRT( IPRT )     ( VALID_PRT_RANGE( IPRT ) && WAITING_OBJ   ( &(PrtList.lst[IPRT].obj_base) ) )
#define TERMINATED_PRT( IPRT )  ( VALID_PRT_RANGE( IPRT ) && TERMINATED_OBJ( &(PrtList.lst[IPRT].obj_base) ) )

#define DISPLAY_PRT( IPRT )     ( ACTIVE_PRT(IPRT) || WAITING_PRT( IPRT ) )

#define ACTIVE_PPRT( PPRT )      ( (NULL != (PPRT)) && VALID_PRT_RANGE( GET_INDEX( PPRT, TOTAL_MAX_PRT) ) && ACTIVE_OBJ( OBJ_GET_PBASE( (PPRT) ) ) )
#define GET_INDEX_PPRT( PPRT )   GET_INDEX( PPRT, TOTAL_MAX_PRT )

#define DISPLAY_PPRT( PPRT )     ( (NULL != (PPRT)) && VALID_PRT_RANGE(GET_INDEX(PPRT, TOTAL_MAX_PRT)) && (ACTIVE_OBJ(OBJ_GET_PBASE( (PPRT) )) || WAITING_OBJ( OBJ_GET_PBASE( (PPRT) ) )) )

//--------------------------------------------------------------------------------------------
/// function prototypes

void   init_all_pip();
void   release_all_pip();
bool_t release_one_pip( Uint16 ipip );

bool_t PrtList_free_one( Uint16 particle );
void   PrtList_free_all();
void   PrtList_update_used();

void   free_one_particle_in_game( Uint16 particle );

void particle_system_init();

void update_all_particles( void );
void move_all_particles( void );
void cleanup_all_particles( void );

void play_particle_sound( Uint16 particle, Sint8 sound );
int prt_get_free( int force );
Uint16 spawn_one_particle( fvec3_t   pos, Uint16 facing, Uint16 iprofile, Uint16 ipip,
                           Uint16 chr_attach, Uint16 vrt_offset, Uint8 team,
                           Uint16 chr_origin, Uint16 prt_origin, Uint16 multispawn, Uint16 oldtarget );

int prt_count_free();

int load_one_particle_profile( const char *szLoadName, Uint16 pip_override );
void reset_particles();

Uint32 __prthitawall(  prt_t * pprt, float nrm[] );

int    prt_is_over_water( Uint16 cnt );

bool_t release_one_pip( Uint16 ipip );

Uint16  prt_get_ipip( Uint16 cnt );
pip_t * prt_get_ppip( Uint16 cnt );

bool_t prt_request_terminate( Uint16 iprt );