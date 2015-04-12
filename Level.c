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
#include <stdio.h>
#include <stdlib.h>

#include "SDLInclude.h"

#include "GlobalData.h"
#include "SDLTools.h"

#include "ArrayList.h"
#include "Pos2D.h"
#include "Level.h"


static void TerrainMap_Init(TerrainMap_T * map, int width, int height);

static void TerrainMap_Destroy(TerrainMap_T * map);

static int TerrainMap_GetTile(TerrainMap_T * map, int x, int y);




static int Level_Render_DigSpot(SDL_Renderer * rend, SDL_Texture * t_terrain, DigSpot_T * dig_spot, int x, int y);

// S TerrainMap


static void TerrainMap_Init(TerrainMap_T * map, int width, int height)
{
   size_t i;
   size_t size;
   map->width  = width; 
   map->height = height;
   size        = width * height;
   map->data   = malloc(sizeof(int) * size);
   for(i = 0; i < size; i ++)
   {
      map->data[i] = TMAP_TILE_AIR;
   }
}

static void TerrainMap_Destroy(TerrainMap_T * map)
{
   free(map->data);
   map->data = NULL;
}


static int TerrainMap_GetTile(TerrainMap_T * map, int x, int y)
{
   int index;
   int result;
   if(x >= 0 && x < map->width && y >= 0 && y < map->height)
   {
      index = x + (map->width * y);
      result = map->data[index];
   }
   else
   {
      printf("Error: (%i, %i) is Out of Map Range\n", x, y);
      result = TMAP_TILE_AIR;
   }
   return result;
}

// E TerrainMap
// S Level


void Level_Init(Level_T * level)
{
   TerrainMap_Init(&level->tmap, 10, 10);
   ArrayList_Init(&level->dig_list,       sizeof(DigSpot_T), 0);
   ArrayList_Init(&level->gold_list,      sizeof(Gold_T),    0);
   ArrayList_Init(&level->gold_list_init, sizeof(Gold_T),    0);
   level->start_spot.x = 0;
   level->start_spot.y = 0;
}

void Level_Destroy(Level_T * level)
{
   TerrainMap_Destroy(&level->tmap);
   ArrayList_Destroy(&level->dig_list);
   ArrayList_Destroy(&level->gold_list);
   ArrayList_Destroy(&level->gold_list_init);
}

void Level_Load(Level_T * level, const char * filename)
{
   FILE * fp;
   int input, index;
   int w, h;
   TerrainMap_T * map;
   Pos2D_T p;
   Gold_T * gold;
   size_t size;

   map = &level->tmap;
   fp = fopen(filename, "r");

   if(fp == NULL)
   {
      printf("Error: Could not open \"%s\"\n", filename);
   }
   else
   {
      fscanf(fp, "%i", &w);
      fscanf(fp, "%i", &h);
      TerrainMap_Destroy(map);

      TerrainMap_Init(map, w, h);
      size = w * h;
      index = 0;
      p.x = p.y = 0;
      while(!feof(fp) && index < size)
      {
         fscanf(fp, "%i", &input);
         switch(input)
         {
            case 0:  map->data[index] = TMAP_TILE_AIR;    break;
            case 1:  map->data[index] = TMAP_TILE_DIRT;   break;
            case 2:  
               level->start_spot.x = p.x;
               level->start_spot.y = p.y;
               break;
            case 3:  map->data[index] = TMAP_TILE_LADDER; break;
            case 4:  map->data[index] = TMAP_TILE_BAR;    break;
            case 5:
               map->data[index] = TMAP_TILE_AIR;
               gold = ArrayList_Add(&level->gold_list_init, NULL);
               gold->pos.x = p.x;
               gold->pos.y = p.y;
               break;
            case 6: map->data[index] = TMAP_TILE_DOOR;    break;
            default: map->data[index] = TMAP_TILE_AIR;    break;
         }
         index ++;
         p.x ++;
         if(p.x >= w)
         {
            p.x = 0;
            p.y ++;
         }
         
      }

      fclose(fp);
      Level_Restart(level);
   }
}

void Level_Restart(Level_T * level)
{
   size_t size;
   Gold_T * gold;
   gold = ArrayList_Get(&level->gold_list_init, &size, NULL);
   ArrayList_Clear(&level->gold_list);
   ArrayList_AddArray(&level->gold_list, gold, size);
   ArrayList_Clear(&level->dig_list);

}

static int Level_Render_DigSpot(SDL_Renderer * rend, SDL_Texture * t_terrain, DigSpot_T * dig_spot, int x, int y)
{
   int show;
   int tile;
   if(dig_spot->state == e_dss_opening)
   {
      switch(dig_spot->frame)
      {
         case 0:  show = 1; tile = IMGID_BROKENBLOCK_0; break;
         case 1:  show = 1; tile = IMGID_BROKENBLOCK_1; break;
         case 2:  show = 1; tile = IMGID_BROKENBLOCK_2; break;
         default: show = 0; tile = 0xFFFF;              break;
      }
   }
   else if(dig_spot->state == e_dss_open)
   {
      show = 0;
      tile = 0xFFF;
   }
   else if(dig_spot->state == e_dss_closing)
   {
      switch(dig_spot->frame)
      {
         case 0:  show = 1; tile = IMGID_BROKENBLOCK_2; break;
         case 1:  show = 1; tile = IMGID_BROKENBLOCK_1; break;
         case 2:  show = 1; tile = IMGID_BROKENBLOCK_0; break;
         default: show = 0; tile = 0xFFFF;              break;
      }
   }

   if(show == 1)
   {
      SDLTools_DrawSubimage(rend, t_terrain, tile, x, y);
   }

}

void Level_Render(Level_T * level, SDL_Renderer * rend, SDL_Texture * t_terrain)
{
   DigSpot_T * dig_spot;
   int index;
   int tile_rend;
   Pos2D_T p, c;
   TerrainMap_T * map;
   Gold_T * gold;
   size_t i, size;
   int gold_count;


   gold_count = Level_GetGoldCount(level, NULL);

   map = &level->tmap;
   p.x = 0;
   p.y = 0;
   index = 0;
   c.x = 0;
   c.y = 0; 
   while(p.y < map->height)
   {
      switch(map->data[index])
      {
         case TMAP_TILE_DIRT:
            dig_spot = Level_GetDigSpot(level, p.x, p.y);
            if(dig_spot == NULL)
            {
               SDLTools_DrawSubimage(rend, t_terrain, IMGID_BLOCK, c.x, c.y);
            }
            else
            {
               Level_Render_DigSpot(rend, t_terrain, dig_spot, c.x, c.y);
            }
            break;
         case TMAP_TILE_LADDER:
            SDLTools_DrawSubimage(rend, t_terrain, IMGID_LADDER, c.x, c.y);
            break;
         case TMAP_TILE_BAR:
            SDLTools_DrawSubimage(rend, t_terrain, IMGID_BAR, c.x, c.y);
            break;
         case TMAP_TILE_DOOR:
            if(gold_count == 0)
            {
               SDLTools_DrawSubimage(rend, t_terrain, IMGID_DOOROPEN, c.x, c.y);
            }
            else
            {
               SDLTools_DrawSubimage(rend, t_terrain, IMGID_DOORCLOSE, c.x, c.y);
            }
            break;
      }

      index ++;
      p.x ++;
      c.x += TILE_WIDTH;
      if(p.x >= map->width)
      {
         p.x = 0;
         c.x = 0;
         p.y ++;
         c.y += TILE_HEIGHT;
      }
   }
   
   gold = ArrayList_Get(&level->gold_list, &size, NULL);
   for(i = 0; i < size; i++)
   {
      c.x = gold[i].pos.x * TILE_WIDTH;
      c.y = gold[i].pos.y * TILE_HEIGHT;
      SDLTools_DrawSubimage(rend, t_terrain, IMGID_GOLD, c.x, c.y);
   }
   
}

void Level_Update(Level_T * level, float seconds)
{
   // Update Dig Spots
   size_t size, i;
   DigSpot_T * dig_spot;

   dig_spot = ArrayList_Get(&level->dig_list, &size, NULL);

   // Update Dig Spots
   for(i = 0; i < size; i++)
   {
      dig_spot[i].timer += seconds;
      if(dig_spot[i].state == e_dss_opening)
      {
         if(dig_spot[i].timer >= DIG_SPOT_DELATA_FRAME_TIMEOUT)
         {
            dig_spot[i].frame ++;
            dig_spot[i].timer -= DIG_SPOT_DELATA_FRAME_TIMEOUT;
         }

         if(dig_spot[i].frame >= DIG_SPOT_FRAME_COUNT)
         {
            dig_spot[i].state = e_dss_open;
         }
      }
      else if(dig_spot[i].state == e_dss_open)
      {
         if(dig_spot[i].timer >= HOLE_TIMEOUT)
         {
            dig_spot[i].frame = 0;
            dig_spot[i].timer -= HOLE_TIMEOUT;
            dig_spot[i].state = e_dss_closing;
         }
      }
      else if(dig_spot[i].state == e_dss_closing)
      {
         if(dig_spot[i].timer >= DIG_SPOT_DELATA_FRAME_TIMEOUT)
         {
            dig_spot[i].frame ++;
            dig_spot[i].timer -= DIG_SPOT_DELATA_FRAME_TIMEOUT;
         }

         if(dig_spot[i].frame >= DIG_SPOT_FRAME_COUNT)
         {
            dig_spot[i].state = e_dss_close;
         }
      }

   }

   // Remove spots

   // Iterate backward so that the removes don't effect the index
   for(i = size - 1; i < size; i--)
   {
      if(dig_spot[i].state == e_dss_close)
      {
         ArrayList_Remove(&level->dig_list, i);
      }
   }

}

void Level_AddDigSpot(Level_T * level, int x, int y)
{
   DigSpot_T * dig_spot;
   dig_spot = ArrayList_Add(&level->dig_list, NULL);
   dig_spot->pos.x = x;
   dig_spot->pos.y = y;
   dig_spot->timer = 0;
   dig_spot->state = e_dss_opening;
   dig_spot->frame = 0;
}

void Level_AddGold(Level_T * level, int x, int y)
{
   Gold_T * gold;
   gold = ArrayList_Add(&level->gold_list, NULL);
   gold->pos.x = x;
   gold->pos.y = y;
}

void Level_RemoveGold(Level_T * level, size_t gold_index)
{
   ArrayList_Remove(&level->gold_list, gold_index);
}

Gold_T * Level_GetGold(Level_T * level, int x, int y, size_t * out_index)
{
   size_t i, size;
   Gold_T * gold, * result;
   result = NULL;
   gold = ArrayList_Get(&level->gold_list, &size, NULL);
   for(i = 0; i < size; i++)
   {
      if(x == gold[i].pos.x && y == gold[i].pos.y)
      {

         result = &gold[i];
         if(out_index != NULL)
         {
            (*out_index) = i;
         }
         break;
      }
   }
   return result;
}

DigSpot_T * Level_GetDigSpot(Level_T * level, int x, int y)
{
   size_t i, size;
   DigSpot_T * dig_spot, * result;

   result = NULL;
   dig_spot = ArrayList_Get(&level->dig_list, &size, NULL);
   for(i = 0; i < size; i ++)
   {
      if(x == dig_spot[i].pos.x && y == dig_spot[i].pos.y)
      {
         result = &dig_spot[i];
         break;
      }
   }

   return result;
}

void Level_QueryTile(Level_T * level, int x, int y, LevelTile_T * tile)
{
   DigSpot_T * dig_spot;
   Gold_T * gold;
   size_t index;
   tile->pos.x = x;
   tile->pos.y = y;
   if(x >= 0 && x < level->tmap.width && y >= 0 && y < level->tmap.height)
   {
      tile->index = x + (y * level->tmap.width);
      tile->out_of_range = 0;
      tile->terrain_type = level->tmap.data[tile->index];
      tile->has_hole = 0;

      // Check for hole
      dig_spot = Level_GetDigSpot(level, x, y);
      if(dig_spot == NULL)
      {
         tile->has_hole = 0;
      }
      else
      {
         tile->has_hole = 1;
      }

      // Check for gold
      gold = Level_GetGold(level, x, y, &index);
      if(gold == NULL)
      {
         tile->gold_index = -1;
      }
      else
      {
         tile->gold_index = (int)index;
      }
   }
   else
   {
      tile->out_of_range = 1;
   }
}


int Level_GetGoldCount(Level_T * level, int * level_total)
{
   size_t gold_left, gold_total;
   (void)ArrayList_Get(&level->gold_list,      &gold_left,  NULL);
   (void)ArrayList_Get(&level->gold_list_init, &gold_total, NULL);

   if(level_total != NULL)
   {
      (*level_total) = (int)gold_total;
   }
   return (int)gold_left;
}

void Level_GetStartSpot(Level_T * level, int * x, int * y)
{
   if(x != NULL)
   {
      (*x) = level->start_spot.x;
   }

   if(y != NULL)
   {
      (*y) = level->start_spot.y;
   }
}

// E Level



