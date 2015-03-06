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

/// @file egolib/file_formats/wawalite_file.h
/// @details loading the environment definitions for a module

#pragma once

#include "egolib/_math.h"
#include "egolib/vec.h"

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

#define MAXWATERLAYER 2                             ///< Maximum water layers

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

/// A wrapper for the water layer data in "wawalite.txt"
    struct wawalite_water_layer_t
    {
        uint32_t frame_add;      ///< Speed

        float z;                 ///< Base height of water
        float amp;               ///< Amplitude of waves

        fvec2_t dist;            ///< For distant backgrounds
        uint32_t light_dir;      ///< direct  reflectivity 0 - 63
        uint32_t light_add;      ///< ambient reflectivity 0 - 63

        fvec2_t tx_add;          ///< Texture movement
        uint8_t alpha;           ///< Transparency

        wawalite_water_layer_t() :
            frame_add(0),
            z(0.0f),
            amp(0.0f),
            dist(0, 0),
            light_dir(0),
            light_add(0),
            tx_add(0, 0),
            alpha(0)
        {
            // default ctor
        }

        wawalite_water_layer_t(const wawalite_water_layer_t& other) :
            frame_add(other.frame_add),
            z(other.z),
            amp(other.amp),
            dist(other.dist),
            light_dir(other.light_dir),
            light_add(other.light_add),
            tx_add(other.tx_add),
            alpha(other.alpha)
        {
            // copy ctor
        }

        wawalite_water_layer_t& operator=(const wawalite_water_layer_t& other)
        {
            frame_add = other.frame_add;
            z = other.z;
            amp = other.amp;
            dist = other.dist;
            light_dir = other.light_dir;
            light_add = other.light_add;
            tx_add = other.tx_add;
            alpha = other.alpha;
            return *this;
        }

    };

//--------------------------------------------------------------------------------------------

/// A wrapper for the water data in "wawalite.txt"
    struct wawalite_water_t
    {
        int layer_count;
        wawalite_water_layer_t layer[MAXWATERLAYER];

        float surface_level;    ///< Surface level for water striders
        float douse_level;      ///< Surface level for torches
        uint8_t spek_start;     ///< Specular begins at which light value
        uint8_t spek_level;     ///< General specular amount (0-255)
        bool is_water;          ///< Is it water?  ( Or lava... )
        bool overlay_req;
        bool background_req;

        bool light;             ///< Is it light ( default is alpha )

        float foregroundrepeat;
        float backgroundrepeat;

        wawalite_water_t() :
            layer_count(0),
            layer(),
            surface_level(0.0f),
            douse_level(0.0f),
            spek_start(0),
            spek_level(0),
            is_water(true),
            overlay_req(false),
            background_req(false),
            light(false),
            foregroundrepeat(0.0f),
            backgroundrepeat(0.0f)
        {
            // default ctor            
        }

        wawalite_water_t(const wawalite_water_t& other) :
            layer_count(other.layer_count),
            /*layer(other.layer),*/ /* See below. */
            surface_level(other.surface_level),
            douse_level(other.douse_level),
            spek_start(other.spek_start),
            spek_level(other.spek_level),
            is_water(other.is_water),
            overlay_req(other.overlay_req),
            background_req(other.background_req),
            light(other.light),
            foregroundrepeat(other.foregroundrepeat),
            backgroundrepeat(other.backgroundrepeat)
        {
            for (int i = 0; i < layer_count; ++i)
            {
                layer[i] = other.layer[i];
            }        
        }

        wawalite_water_t& operator=(const wawalite_water_t& other)
        {
            layer_count = other.layer_count;
            /*layer(other.layer),*/ /* See below. */
            surface_level = other.surface_level;
            douse_level = other.douse_level;
            spek_start = other.spek_start;
            spek_level = other.spek_level;
            is_water = other.is_water;
            overlay_req = other.overlay_req;
            background_req = other.background_req;
            light = other.light;
            foregroundrepeat = other.foregroundrepeat;
            backgroundrepeat = other.backgroundrepeat;
            for (int i = 0; i < layer_count; ++i)
            {
                layer[i] = other.layer[i];
            }
            return *this;
        }
    };

//--------------------------------------------------------------------------------------------

/// A wrapper for the physics data in "wawalite.txt"
    struct wawalite_physics_t
    {
        float hillslide;
        float slippyfriction;
        float airfriction;
        float waterfriction;
        float noslipfriction;
        float gravity;

        wawalite_physics_t() :
            hillslide(0.0f),
            slippyfriction(0.0f),
            airfriction(0.0f),
            waterfriction(0.0f),
            noslipfriction(0.0f),
            gravity(0.0f)
        {
            // default ctor
        }

        wawalite_physics_t(const wawalite_physics_t& other) :
            hillslide(other.hillslide),
            slippyfriction(other.slippyfriction),
            airfriction(other.airfriction),
            waterfriction(other.waterfriction),
            noslipfriction(other.noslipfriction),
            gravity(other.gravity)
        {
            // copy ctor
        }

        wawalite_physics_t& operator=(const wawalite_physics_t& other)
        {
            hillslide = other.hillslide;
            slippyfriction = other.slippyfriction;
            airfriction = other.airfriction;
            waterfriction = other.waterfriction;
            noslipfriction = other.noslipfriction;
            gravity = other.gravity;
            return *this;
        }

    };

//--------------------------------------------------------------------------------------------

/// A wrapper for the animated tile data in "wawalite.txt"
    struct wawalite_animtile_t
    {

        uint32_t update_and;
        uint32_t frame_and;

        wawalite_animtile_t() :
            update_and(0),
            frame_and(0)
        {
            // default ctor
        }

        wawalite_animtile_t(const wawalite_animtile_t& other) :
            update_and(other.update_and),
            frame_and(other.frame_and)
        {
            // copy ctor
        }

        wawalite_animtile_t& operator=(const wawalite_animtile_t& other)
        {
            update_and = other.update_and;
            frame_and = other.frame_and;
            return *this;
        }

    };

//--------------------------------------------------------------------------------------------

/// A wrapper for the damagetile data in "wawalite.txt"
    struct wawalite_damagetile_t
    {
        uint32_t amount; /// The amount of damage dealt.
        int damagetype;  ///< The type of damage dealt.

        int part_gpip;
        uint32_t partand;
        int sound_index;

        wawalite_damagetile_t() :
            amount(0),
            damagetype(0),
            part_gpip(-1),
            partand(0),
            sound_index(-1)
        {
            // default ctor
        }

        wawalite_damagetile_t(const wawalite_damagetile_t& other) :
            amount(other.amount),
            damagetype(other.damagetype),
            part_gpip(other.part_gpip),
            partand(other.partand),
            sound_index(other.sound_index)
        {
            // default ctor
        }

        wawalite_damagetile_t& operator=(const wawalite_damagetile_t& other)
        {
            amount = other.amount;
            damagetype = other.damagetype;
            part_gpip = other.part_gpip;
            partand = other.partand;
            sound_index = other.sound_index;
            return *this;
        }
    };

//--------------------------------------------------------------------------------------------

/// A wrapper for the weather data in "wawalite.txt"
    struct wawalite_weather_t
    {
        bool over_water;
        int timer_reset;
        int part_gpip; ///< Which particle to spawn?
        STRING weather_name;

        wawalite_weather_t() :
            over_water(false),
            timer_reset(0),
            part_gpip(-1)
        {
			weather_name[0] = '\0';
        }

        wawalite_weather_t(const wawalite_weather_t& other) :
            over_water(other.over_water),
            timer_reset(other.timer_reset),
            part_gpip(other.part_gpip)
        {
            strcpy(weather_name, other.weather_name);
        }

        wawalite_weather_t& operator=(const wawalite_weather_t& other)
        {
            over_water = other.over_water;
            timer_reset = other.timer_reset;
            part_gpip = other.part_gpip;
            strcpy(weather_name, other.weather_name);
            return *this;
        }
    };

//--------------------------------------------------------------------------------------------

    /// A wrapper for the graphics data in "wawalite.txt"
    struct wawalite_graphics_t
    {

        bool exploremode;
        bool usefaredge;

        wawalite_graphics_t() :
            exploremode(false),
            usefaredge(false)
        {
            // default ctor   
        }

        wawalite_graphics_t(const wawalite_graphics_t& other) :
            exploremode(other.exploremode),
            usefaredge(other.usefaredge)
        {
            // copy ctor
        }

        wawalite_graphics_t& operator=(const wawalite_graphics_t& other)
        {
            exploremode = other.exploremode;
            usefaredge = other.usefaredge;
            return *this;
        }

    };

//--------------------------------------------------------------------------------------------

/// A wrapper for the camera data in "wawalite.txt"
    struct wawalite_camera_t
    {
        bool swing;
        float swing_rate;
        float swing_amp;

        wawalite_camera_t() :
            swing(false),
            swing_rate(0.0f),
            swing_amp(0.0f)
        {
            // default ctor
        }

        wawalite_camera_t(const wawalite_camera_t& other) :
            swing(other.swing),
            swing_rate(other.swing_rate),
            swing_amp(other.swing_amp)
        {
            // copy ctor
        }

        wawalite_camera_t& operator=(const wawalite_camera_t& other)
        {
            swing = other.swing;
            swing_rate = other.swing_rate;
            swing_amp = other.swing_amp;
            return *this;
        }

    };

//--------------------------------------------------------------------------------------------

/// A wrapper for the fog data in "wawalite.txt"
    struct wawalite_fog_t
    {
        bool found;
        float top;
        float bottom;
        float red, grn, blu; ///< @todo Should be Ego::Math::Colour3f.
        bool affects_water;

        wawalite_fog_t() :
            found(false),
            top(0),
            bottom(0),
            red(0),
            grn(0),
            blu(0),
            affects_water(false)
        {
            // default ctor
        }

        wawalite_fog_t(const wawalite_fog_t& other) :
            found(other.found),
            top(other.top),
            bottom(other.bottom),
            red(other.red),
            grn(other.grn),
            blu(other.blu),
            affects_water(other.affects_water)
        {
            // copy ctor
        }

        wawalite_fog_t& operator=(const wawalite_fog_t& other)
        {
            found = other.found;
            top = other.top;
            bottom = other.bottom;
            red = other.red;
            grn = other.grn;
            blu = other.blu;
            affects_water = other.affects_water;
            return *this;
        }
    };

//--------------------------------------------------------------------------------------------

/// An internal representation of the data in "wawalite.txt"
    struct wawalite_data_t
    {
        uint32_t seed;
        Sint8 version;

        wawalite_water_t      water;
        wawalite_physics_t    phys;
        wawalite_animtile_t   animtile;
        wawalite_damagetile_t damagetile;

        wawalite_weather_t    weather;
        wawalite_graphics_t   graphics;
        wawalite_camera_t     camera;
        wawalite_fog_t        fog;

        /// @brief Directional light vector.
        /// @todo  Egoboo does not allow for specifying a directional light colour.
        float light_x, light_y, light_z; ///< @todo Should be fvec3_t.
        /// @brief Ambient light brightniess.
        /// @todo Egoboo does not allow for specifying an ambient light colour,
        /// only its alpha value; the effective ambient light color is given by
        /// (1, 1, 1, light_a).        
        float light_a;

        wawalite_data_t() :
            seed(0),
            version(0),
            water(),
            phys(),
            animtile(),
            damagetile(),
            weather(),
            graphics(),
            camera(),
            fog(),
            light_x(0.0f),
            light_y(0.0f),
            light_z(0.0f),
            light_a(0.0f)
        {
            // default ctor
        }

        wawalite_data_t(const wawalite_data_t& other) :
            seed(other.seed),
            version(other.version),
            water(other.water),
            phys(other.phys),
            animtile(other.animtile),
            damagetile(other.damagetile),
            weather(other.weather),
            graphics(other.graphics),
            camera(other.camera),
            fog(other.fog),
            light_x(other.light_x),
            light_y(other.light_y),
            light_z(other.light_z),
            light_a(other.light_a)
        {
            // copy ctor
        }

        wawalite_data_t& operator=(const wawalite_data_t& other)
        {
            seed = other.seed;
            version = other.version;
            water = other.water;
            phys = other.phys;
            animtile = other.animtile;
            damagetile = other.damagetile;
            weather = other.weather;
            graphics = other.graphics;
            camera = other.camera;
            fog = other.fog;
            light_x = other.light_x;
            light_y = other.light_y;
            light_z = other.light_z;
            light_a = other.light_a;
            return *this;
        }
    };

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

    extern wawalite_data_t wawalite_data;

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

    bool write_wawalite_file_vfs(const wawalite_data_t *self);
    wawalite_data_t *read_wawalite_file_vfs(const char *filename, wawalite_data_t *profile);
    wawalite_data_t *wawalite_limit(wawalite_data_t *self);
