/*
 *  Copyright (C) 2015 Ryan Hanson <hansonry@gmail.com>
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty.  In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *  3. This notice may not be removed or altered from any source distribution.
 *
 */
#ifndef __LEVEL_H__
#define __LEVEL_H__

// Map Tile Type
#define TMAP_TILE_AIR    0
#define TMAP_TILE_DIRT   1
#define TMAP_TILE_LADDER 2
#define TMAP_TILE_BAR    3
#define TMAP_TILE_DOOR   4


typedef struct Level_S          Level_T;
typedef struct TerrainMap_S     TerrainMap_T;
typedef struct Gold_S           Gold_T;
typedef enum   DigSpot_State_E  DigSpot_State_T;
typedef struct DigSpot_S        DigSpot_T;
typedef struct LevelTile_S      LevelTile_T;



struct TerrainMap_S
{
   int width;
   int height;
   int * data;
};

struct Level_S
{
   TerrainMap_T tmap;
   ArrayList_T  dig_list;
   ArrayList_T  gold_list;
   ArrayList_T  gold_list_init;
   Pos2D_T start_spot;
};


struct Gold_S
{
   Pos2D_T pos;
};

enum DigSpot_State_E
{
   e_dss_close,
   e_dss_opening,
   e_dss_open,
   e_dss_closing
};

struct DigSpot_S
{
   Pos2D_T pos;
   float timer;
   DigSpot_State_T state;
   int frame;
};

struct LevelTile_S
{
   Pos2D_T pos;
   int index;
   int terrain_type;
   int has_hole;
   int out_of_range;
   int gold_index;
};


void Level_Init(Level_T * level);

void Level_Destroy(Level_T * level);

void Level_Load(Level_T * level, const char * filename);

void Level_Restart(Level_T * level);


#ifdef SDL_LIB_INCLUDED
void Level_Render(Level_T * level, SDL_Renderer * rend, SDL_Texture * t_terrain);
#endif // SDL_LIB_INCLUDED

void Level_Update(Level_T * level, float seconds);

void Level_AddDigSpot(Level_T * level, int x, int y);

void Level_AddGold(Level_T * level, int x, int y);

void Level_RemoveGold(Level_T * level, size_t gold_index);

Gold_T * Level_GetGold(Level_T * level, int x, int y, size_t * out_index);

DigSpot_T * Level_GetDigSpot(Level_T * level, int x, int y);

void Level_QueryTile(Level_T * level, int x, int y, LevelTile_T * tile);

int Level_GetGoldCount(Level_T * level, int * level_total);

void Level_GetStartSpot(Level_T * level, int * x, int * y);

#endif // __LEVEL_H__

