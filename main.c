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
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"

#include "ArrayList.h"
#include "FontText.h"

#define TILE_WIDTH             32
#define TILE_HEIGHT            32

#define IMGID_BLOCK        0x0000
#define IMGID_BROKENBLOCK  0x0001
#define IMGID_LADDER       0x0002
#define IMGID_GOLD         0x0100
#define IMGID_GUY          0x0101
#define IMGID_BAR          0x0102
#define IMGID_DOORCLOSE    0x0200
#define IMGID_DOOROPEN     0x0201

#define MOVE_TIMEOUT             0.5f
#define FALL_TIMEOUT             0.3f
#define DIG_TIMEOUT              0.5f
#define HOLE_TIMEOUT             5.0f

#define PLAYER_STATE_NOT_MOVING  0
#define PLAYER_STATE_MOVING      1
#define PLAYER_STATE_FALLING     2
#define PLAYER_STATE_DIGGING     3
#define PLAYER_STATE_DEATH       4
#define PLAYER_STATE_WIN         5


#define TMAP_TILE_AIR    0
#define TMAP_TILE_DIRT   1
#define TMAP_TILE_LADDER 2
#define TMAP_TILE_BAR    3
#define TMAP_TILE_DOOR   4

typedef enum player_input_e player_input_t;
enum player_input_e
{
   e_pi_move_up,
   e_pi_move_down,
   e_pi_move_left,
   e_pi_move_right,
   e_pi_dig_left,
   e_pi_dig_right,
   e_pi_last
};

typedef struct pos_s pos_t;
struct pos_s
{
   int x;
   int y;
};

#define POS_SPLIT(p, dx, dy) (p).x + (dx), (p).y + (dy)

typedef struct player_data_s player_data_t;
struct player_data_s
{
   int input_flags[e_pi_last];
   pos_t grid_p;
   pos_t next_grid_p;
   int player_state;
   float move_timer;
   float move_timeout;
};

typedef struct TerrainMap_S TerrainMap_T;
struct TerrainMap_S
{
   int width;
   int height;
   int * data;
};

typedef struct Level_S Level_T;
struct Level_S
{
   TerrainMap_T tmap;
   ArrayList_T  dig_list;
   ArrayList_T  gold_list;
   ArrayList_T  gold_list_init;
   pos_t start_spot;
};


typedef struct Gold_S Gold_T;
struct Gold_S
{
   pos_t pos;
};

typedef struct DigSpot_S DigSpot_T;
struct DigSpot_S
{
   pos_t pos;
   float timer;
};

typedef struct LevelTile_S LevelTile_T;
struct LevelTile_S
{
   pos_t pos;
   int index;
   int terrain_type;
   int has_hole;
   int out_of_range;
   int gold_index;
};


static void Level_Init(Level_T * level);

static void Level_Destroy(Level_T * level);

static void Level_Load(Level_T * level, const char * filename);

static void Level_Restart(Level_T * level);

static void Level_Render(Level_T * level, SDL_Renderer * rend, SDL_Texture * t_palet);

static void Level_Update(Level_T * level, float seconds);

static void Level_AddDigSpot(Level_T * level, int x, int y);

static void Level_AddGold(Level_T * level, int x, int y);

static void Level_RemoveGold(Level_T * level, size_t gold_index);

static Gold_T * Level_GetGold(Level_T * level, int x, int y, size_t * out_index);

static DigSpot_T * Level_GetDigSpot(Level_T * level, int x, int y);

static void Level_QueryTile(Level_T * level, int x, int y, LevelTile_T * tile);

static void TerrainMap_Init(TerrainMap_T * map, int width, int height);

static void TerrainMap_Destroy(TerrainMap_T * map);

static int TerrainMap_GetTile(TerrainMap_T * map, int x, int y);

static int IsTerrainPassable(LevelTile_T * from, LevelTile_T * to);
static int IsTerrainFallable(LevelTile_T * from, LevelTile_T * to);
static int IsAllGoldColected(Level_T * level);
static void UpdateGoldCount(Level_T * level, FontText_T * gold_count_text);

static void CheckForExit(const SDL_Event *event, int * done);

static SDL_Texture * load_texture(SDL_Renderer * rend, const char * filename);

static void draw_at(SDL_Renderer * rend, SDL_Texture * text, int imgid, int x, int y);

static void handle_input(const SDL_Event * event, int * done, int * player_input_flags);

static void handle_update(float seconds, Level_T * level, player_data_t * player1_data, FontText_T * gold_count_text);

static void handle_render(SDL_Renderer * rend, SDL_Texture * t_palet, Level_T * level, player_data_t * player1_data);

int main(int args, char * argc[])
{
   SDL_Window  * window;
   SDL_Renderer * rend;   
   SDL_Event event;
   SDL_Texture * t_palet;
   int done;
   int prevTicks, diffTicks, nowTicks;
   float seconds;

   int i;

   // Font
   TTF_Font * font;
   FontText_T gold_count_text;
 
   player_data_t player1_data;
   Level_T level;
   

   for(i = 0; i < e_pi_last; i++)
   {
      player1_data.input_flags[i] = 0;
   }
   
   Level_Init(&level);
   Level_Load(&level, "testmap.txt");

   player1_data.grid_p.x = level.start_spot.x;
   player1_data.grid_p.y = level.start_spot.y;
   player1_data.player_state = PLAYER_STATE_NOT_MOVING;
   

   SDL_Init(SDL_INIT_EVERYTHING);   
   TTF_Init();
   window = SDL_CreateWindow("Load Clone", 
                             SDL_WINDOWPOS_CENTERED, 
                             SDL_WINDOWPOS_CENTERED, 
                             800, 600, SDL_WINDOW_SHOWN);
   
   rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
   
   t_palet = load_texture(rend, "load.png");
   
   font = TTF_OpenFont("cnr.otf", 28);
   if(font == NULL)
   {
      printf("Font Null\n");
   }
   FontText_Init(&gold_count_text, font, rend);
   UpdateGoldCount(&level, &gold_count_text);


   prevTicks = SDL_GetTicks();
   
   done = 0;
   while(done == 0)
   {
      while(SDL_PollEvent(&event))
      {
         handle_input(&event, &done, player1_data.input_flags);
      }
      
      nowTicks = SDL_GetTicks();
      diffTicks = nowTicks - prevTicks;
      seconds = (float)diffTicks / 1000.0f;
      prevTicks = nowTicks;
      
      handle_update(seconds, &level, &player1_data, &gold_count_text);
      
      SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0x00, 0xFF);
      SDL_RenderClear( rend );
      
      handle_render(rend, t_palet, &level, &player1_data);
      FontText_Render(&gold_count_text, 10, 10);
      SDL_RenderPresent(rend);
   }
   
   Level_Destroy(&level); 
   
   SDL_DestroyRenderer(rend);
   SDL_DestroyWindow(window);
   SDL_DestroyTexture(t_palet);

   FontText_Destroy(&gold_count_text);
   TTF_CloseFont(font);

   SDL_Quit();
   
   
   printf("End\n");
   return 0;
}

static void CheckForExit(const SDL_Event *event, int * done)
{
   if(event->type == SDL_QUIT)
   {
      (*done) = 1;
   }
   else if(event->type == SDL_KEYDOWN)
   {
      if(event->key.keysym.sym == SDLK_ESCAPE)
      {
         (*done) = 1;
      }
   }

}


static void handle_input(const SDL_Event * event, int * done, int * player_input_flags)
{
   player_input_t key;
   CheckForExit(event, done);

   if(event->key.keysym.sym == SDLK_KP_8)
   {
      key = e_pi_move_up;
   }
   else if(event->key.keysym.sym == SDLK_KP_5)
   {
      key = e_pi_move_down;
   }
   else if(event->key.keysym.sym == SDLK_KP_4)
   {
      key = e_pi_move_left;
   }
   else if(event->key.keysym.sym == SDLK_KP_6)
   {
      key = e_pi_move_right;
   }
   else if(event->key.keysym.sym == SDLK_KP_7)
   {
      key = e_pi_dig_left;
   }
   else if(event->key.keysym.sym == SDLK_KP_9)
   {
      key = e_pi_dig_right;
   }
   else
   {
      key = e_pi_last;
   }

   if(key != e_pi_last)
   {
      if(event->type == SDL_KEYDOWN) player_input_flags[key] = 1;
      if(event->type == SDL_KEYUP)   player_input_flags[key] = 0;
   }

}

static SDL_Texture * load_texture(SDL_Renderer * rend, const char * filename)
{
   SDL_Surface * surf;
   SDL_Texture * text;
   surf = IMG_Load(filename);
   if(surf == NULL)
   {
      printf("Error: could not load %s\n", filename);
      return NULL;
   }

   text = SDL_CreateTextureFromSurface(rend, surf);

   SDL_FreeSurface(surf);
   return text;

}

static void draw_at(SDL_Renderer * rend, SDL_Texture * text, int imgid, int x, int y)
{   
   SDL_Rect r_src;
   SDL_Rect r_dest;
   int ix, iy;
   
   r_dest.x = x;
   r_dest.y = y;
   r_dest.w = TILE_WIDTH;
   r_dest.h = TILE_HEIGHT;


   ix = imgid & 0xFF;
   iy = (imgid >> 8) & 0xFF;

   r_src.x = ix * TILE_WIDTH;
   r_src.y = iy * TILE_HEIGHT;
   r_src.w = TILE_WIDTH;
   r_src.h = TILE_HEIGHT;

   SDL_RenderCopy(rend, text, &r_src, &r_dest);
}

static void handle_update(float seconds, Level_T * level, player_data_t * player1_data, FontText_T * gold_count_text)
{
   int cmd_dig_left_valid, cmd_dig_right_valid;
   int cmd_move_left_valid, cmd_move_right_valid;
   int cmd_move_up_valid, cmd_move_down_valid, cmd_let_go_valid, cmd_fall_valid;
   LevelTile_T player_current_tile, player_desired_tile, dig_desired_tile;
   LevelTile_T dig_above_tile, player_below_tile;
   int all_gold_colected;
    
   Level_QueryTile(level, POS_SPLIT(player1_data->grid_p, 0,  0), &player_current_tile);
   all_gold_colected = IsAllGoldColected(level);
   if(all_gold_colected == 1 && player_current_tile.terrain_type == TMAP_TILE_DOOR)
   {
      // TODO: WIN!!
      player1_data->player_state = PLAYER_STATE_WIN;

   }
   else if(player1_data->player_state != PLAYER_STATE_DEATH)
   {
      if(player_current_tile.out_of_range == 1 || 
         (player_current_tile.terrain_type == TMAP_TILE_DIRT && player_current_tile.has_hole == 0))
      {
         player1_data->player_state = PLAYER_STATE_DEATH;
         // Dec Lifes Here?
      }
   }


   
   if(player1_data->player_state == PLAYER_STATE_NOT_MOVING)
   {
      // Check for valid commands/behavoirs
      
      cmd_dig_left_valid   = 0;
      cmd_dig_right_valid  = 0;
      cmd_move_left_valid  = 0;
      cmd_move_right_valid = 0;
      cmd_move_up_valid    = 0;
      cmd_move_down_valid  = 0;
      cmd_let_go_valid     = 0;
      cmd_fall_valid       = 0;
      

      Level_QueryTile(level, POS_SPLIT(player1_data->grid_p, 0,  1), &player_below_tile);


      if(player_current_tile.gold_index >= 0) // Remove Gold if on Gold
      {
         Level_RemoveGold(level, player_current_tile.gold_index);
         UpdateGoldCount(level, gold_count_text);
      }

      if(IsTerrainFallable(&player_current_tile, &player_below_tile) == 1) // Falling
      {
         cmd_fall_valid = 1;
      }
      if(player1_data->input_flags[e_pi_dig_left] == 1) // Dig Left
      {
         Level_QueryTile(level, POS_SPLIT(player1_data->grid_p, -1, 1), &dig_desired_tile);
         Level_QueryTile(level, POS_SPLIT(player1_data->grid_p, -1, 0), &dig_above_tile);
         if(dig_desired_tile.terrain_type == TMAP_TILE_DIRT && 
            dig_above_tile.terrain_type == TMAP_TILE_AIR)
         {
            cmd_dig_left_valid = 1;
         }
      }
      if(player1_data->input_flags[e_pi_dig_right] == 1) // Dig Right
      {
         Level_QueryTile(level, POS_SPLIT(player1_data->grid_p, 1, 1), &dig_desired_tile);
         Level_QueryTile(level, POS_SPLIT(player1_data->grid_p, 1, 0), &dig_above_tile);
         if(dig_desired_tile.terrain_type == TMAP_TILE_DIRT && 
            dig_above_tile.terrain_type == TMAP_TILE_AIR)
         {
            cmd_dig_right_valid = 1;
         }
      }
      if(player1_data->input_flags[e_pi_move_left] == 1) // Move Left
      {
         Level_QueryTile(level, POS_SPLIT(player1_data->grid_p, -1, 0), &player_desired_tile);
         if(IsTerrainPassable(&player_current_tile, &player_desired_tile) == 1)
         {
            cmd_move_left_valid = 1;
         }
      }
      if(player1_data->input_flags[e_pi_move_right] == 1) // Move Right
      {
         Level_QueryTile(level, POS_SPLIT(player1_data->grid_p, 1, 0), &player_desired_tile);
         if(IsTerrainPassable(&player_current_tile, &player_desired_tile) == 1)
         {
            cmd_move_right_valid = 1;
         }
      }
      if(player1_data->input_flags[e_pi_move_up] == 1) // Climb Up
      {
         Level_QueryTile(level, POS_SPLIT(player1_data->grid_p, 0, -1), &player_desired_tile);
         if(IsTerrainPassable(&player_current_tile, &player_desired_tile) == 1 &&
            player_current_tile.terrain_type == TMAP_TILE_LADDER)
         {
            cmd_move_up_valid = 1;
         }
      }
      if(player1_data->input_flags[e_pi_move_down] == 1) // Climb Down or Fall
      {
         Level_QueryTile(level, POS_SPLIT(player1_data->grid_p, 0, 1), &player_desired_tile);
         if(IsTerrainPassable(&player_current_tile, &player_desired_tile) == 1)
         {
            if(player_current_tile.terrain_type == TMAP_TILE_BAR)
            {
               cmd_let_go_valid = 1;
            }
            else
            {
               cmd_move_down_valid = 1;
            }
         }
      }

      // Compute Next Position and state based on commands
      // Order
      // 1. Falling
      // 2. Digging
      // 3. Climbing
      // 4. Letting Go
      // 5. Horisontal Movement

      if(cmd_fall_valid == 1)
      {
         player1_data->player_state = PLAYER_STATE_FALLING;
         player1_data->next_grid_p.x = player1_data->grid_p.x;
         player1_data->next_grid_p.y = player1_data->grid_p.y + 1;
      }
      else if(cmd_dig_left_valid == 1 && cmd_dig_right_valid == 0)
      {
         player1_data->player_state = PLAYER_STATE_DIGGING;
         player1_data->next_grid_p.x = player1_data->grid_p.x;
         player1_data->next_grid_p.y = player1_data->grid_p.y;

         Level_AddDigSpot(level, player1_data->grid_p.x - 1, 
                                 player1_data->grid_p.y + 1);
      }
      else if(cmd_dig_right_valid == 1 && cmd_dig_left_valid == 0)
      {
         player1_data->player_state = PLAYER_STATE_DIGGING;
         player1_data->next_grid_p.x = player1_data->grid_p.x;
         player1_data->next_grid_p.y = player1_data->grid_p.y;

         Level_AddDigSpot(level, player1_data->grid_p.x + 1, 
                                 player1_data->grid_p.y + 1);
      }
      else if(cmd_move_up_valid == 1 && cmd_move_down_valid == 0)
      {
         player1_data->player_state = PLAYER_STATE_MOVING;
         player1_data->next_grid_p.x = player1_data->grid_p.x;
         player1_data->next_grid_p.y = player1_data->grid_p.y - 1;
      }
      else if(cmd_move_down_valid == 1 && cmd_move_up_valid == 0)
      {
         player1_data->player_state = PLAYER_STATE_MOVING;
         player1_data->next_grid_p.x = player1_data->grid_p.x;
         player1_data->next_grid_p.y = player1_data->grid_p.y + 1;
      }
      else if(cmd_let_go_valid == 1)
      {
         player1_data->player_state = PLAYER_STATE_FALLING;
         player1_data->next_grid_p.x = player1_data->grid_p.x;
         player1_data->next_grid_p.y = player1_data->grid_p.y + 1;
      }
      else if(cmd_move_left_valid == 1 && cmd_move_right_valid == 0)
      {
         player1_data->player_state = PLAYER_STATE_MOVING;
         player1_data->next_grid_p.x = player1_data->grid_p.x - 1;
         player1_data->next_grid_p.y = player1_data->grid_p.y;
      }
      else if(cmd_move_right_valid == 1 && cmd_move_left_valid == 0)
      {
         player1_data->player_state = PLAYER_STATE_MOVING;
         player1_data->next_grid_p.x = player1_data->grid_p.x + 1;
         player1_data->next_grid_p.y = player1_data->grid_p.y;
      }

      if(player1_data->player_state != PLAYER_STATE_NOT_MOVING)
      {
         player1_data->move_timer = 0;


         switch(player1_data->player_state)
         {
            case PLAYER_STATE_DIGGING: player1_data->move_timeout = DIG_TIMEOUT;  break;
            case PLAYER_STATE_MOVING:  player1_data->move_timeout = MOVE_TIMEOUT; break;
            case PLAYER_STATE_FALLING: player1_data->move_timeout = FALL_TIMEOUT; break;
            default:                   player1_data->move_timeout = 0;            break;
         }
      }
   }
   else if(player1_data->player_state == PLAYER_STATE_MOVING ||
           player1_data->player_state == PLAYER_STATE_FALLING ||
           player1_data->player_state == PLAYER_STATE_DIGGING)
   {
      player1_data->move_timer += seconds;
      if(player1_data->move_timer >= player1_data->move_timeout)
      {
         player1_data->player_state = PLAYER_STATE_NOT_MOVING;
         player1_data->grid_p.x = player1_data->next_grid_p.x;
         player1_data->grid_p.y = player1_data->next_grid_p.y;
      }
   }
   else if(player1_data->player_state == PLAYER_STATE_DEATH)
   {
      if(player1_data->input_flags[e_pi_dig_left]   == 1 ||
         player1_data->input_flags[e_pi_dig_right]  == 1 ||
         player1_data->input_flags[e_pi_move_left]  == 1 ||
         player1_data->input_flags[e_pi_move_right] == 1 ||
         player1_data->input_flags[e_pi_move_up]    == 1 ||
         player1_data->input_flags[e_pi_move_down]  == 1)
      {
         player1_data->player_state = PLAYER_STATE_NOT_MOVING;
         player1_data->grid_p.x = level->start_spot.x;
         player1_data->grid_p.y = level->start_spot.y;
      }
   }
   Level_Update(level, seconds);
}

static void handle_render(SDL_Renderer * rend, SDL_Texture * t_palet, Level_T * level, player_data_t * player1_data)
{
   pos_t draw_loc;
   pos_t diff;
   float move_percent;
   int show;

   
   Level_Render(level, rend, t_palet);

   show = 1;
   switch(player1_data->player_state)
   {
   case PLAYER_STATE_NOT_MOVING:
  
      draw_loc.x = player1_data->grid_p.x * TILE_WIDTH;
      draw_loc.y = player1_data->grid_p.y * TILE_HEIGHT;
      break;
   case PLAYER_STATE_FALLING:
   case PLAYER_STATE_DIGGING:
   case PLAYER_STATE_MOVING:
   
      diff.x = (player1_data->next_grid_p.x - player1_data->grid_p.x) * TILE_WIDTH;
      diff.y = (player1_data->next_grid_p.y - player1_data->grid_p.y) * TILE_HEIGHT;
      move_percent = player1_data->move_timer / player1_data->move_timeout;
      draw_loc.x = (player1_data->grid_p.x * TILE_WIDTH) + 
                   (int)(diff.x * move_percent);
      draw_loc.y = (player1_data->grid_p.y * TILE_HEIGHT) + 
                   (int)(diff.y * move_percent);
      break;

   case PLAYER_STATE_DEATH:
      show = 0;
      break;
   case PLAYER_STATE_WIN:
      show = 0; // Sure why not?
      break;
   default:
      draw_loc.x = 0;
      draw_loc.y = 0;
      break;
   }

   
   if(show == 1)
   {
      draw_at(rend, t_palet, IMGID_GUY, draw_loc.x, 
                                        draw_loc.y); 
   }
 
}

static int IsTerrainPassable(LevelTile_T * from, LevelTile_T * to)
{
   int result;
   if(to->out_of_range == 0 && 
     (
       (to->terrain_type == TMAP_TILE_DIRT && to->has_hole == 1) || 
       (to->terrain_type != TMAP_TILE_DIRT)
     ))
   {
      result = 1;
   }
   else
   {
      result = 0;
   }
   return result;
}

static int IsTerrainFallable(LevelTile_T *  from, LevelTile_T *  to)
{
   int result;
   if((to->terrain_type == TMAP_TILE_AIR || to->terrain_type == TMAP_TILE_BAR || to->has_hole == 1) && 
      from->terrain_type != TMAP_TILE_LADDER && from->terrain_type != TMAP_TILE_BAR && to->out_of_range == 0)
   {
      result = 1;      
   }
   else
   {
      result = 0;
   }
   return result;
}

static int IsAllGoldColected(Level_T * level)
{
   int all_gold_colected;
   size_t size;
   (void)ArrayList_Get(&level->gold_list, &size, NULL);
   if(size > 0)
   {
      all_gold_colected = 0;
   }
   else
   {
      all_gold_colected = 1;
   }
   return all_gold_colected;
}

static void UpdateGoldCount(Level_T * level, FontText_T * gold_count_text)
{
   size_t gold_left, gold_total;
   char buffer[255];
   (void)ArrayList_Get(&level->gold_list,      &gold_left,  NULL);
   (void)ArrayList_Get(&level->gold_list_init, &gold_total, NULL);
   sprintf(buffer, "Gold %i/%i", gold_left, gold_total);
   FontText_SetString(gold_count_text, buffer);
}

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


static void Level_Init(Level_T * level)
{
   TerrainMap_Init(&level->tmap, 10, 10);
   ArrayList_Init(&level->dig_list,       sizeof(DigSpot_T), 0);
   ArrayList_Init(&level->gold_list,      sizeof(Gold_T),    0);
   ArrayList_Init(&level->gold_list_init, sizeof(Gold_T),    0);
   level->start_spot.x = 0;
   level->start_spot.y = 0;
}

static void Level_Destroy(Level_T * level)
{
   TerrainMap_Destroy(&level->tmap);
   ArrayList_Destroy(&level->dig_list);
   ArrayList_Destroy(&level->gold_list);
   ArrayList_Destroy(&level->gold_list_init);
}

static void Level_Load(Level_T * level, const char * filename)
{
   FILE * fp;
   int input, index;
   int w, h;
   TerrainMap_T * map;
   pos_t p;
   Gold_T * gold;

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
      index = 0;
      p.x = p.y = 0;
      while(!feof(fp))
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

static void Level_Restart(Level_T * level)
{
   size_t size;
   Gold_T * gold;
   gold = ArrayList_Get(&level->gold_list_init, &size, NULL);
   ArrayList_Clear(&level->gold_list);
   ArrayList_AddArray(&level->gold_list, gold, size);
}

static void Level_Render(Level_T * level, SDL_Renderer * rend, SDL_Texture * t_palet)
{
   DigSpot_T * dig_spot;
   int index;
   int tile_rend;
   pos_t p, c;
   TerrainMap_T * map;
   Gold_T * gold;
   size_t i, size;
   int all_gold_colected;


   all_gold_colected = IsAllGoldColected(level);

   //TerrainMap_Render(&level->tmap, rend, t_palet);

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
               draw_at(rend, t_palet, IMGID_BLOCK, c.x, c.y);
            }
            else
            {
               draw_at(rend, t_palet, IMGID_BROKENBLOCK, c.x, c.y);
            }
            break;
         case TMAP_TILE_LADDER:
            draw_at(rend, t_palet, IMGID_LADDER, c.x, c.y);
            break;
         case TMAP_TILE_BAR:
            draw_at(rend, t_palet, IMGID_BAR, c.x, c.y);
            break;
         case TMAP_TILE_DOOR:
            if(all_gold_colected == 1)
            {
               draw_at(rend, t_palet, IMGID_DOOROPEN, c.x, c.y);
            }
            else
            {
               draw_at(rend, t_palet, IMGID_DOORCLOSE, c.x, c.y);
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
      draw_at(rend, t_palet, IMGID_GOLD, c.x, c.y);
   }
   
}

static void Level_Update(Level_T * level, float seconds)
{
   // Update Dig Spots
   size_t size, i;
   DigSpot_T * dig_spot;

   dig_spot = ArrayList_Get(&level->dig_list, &size, NULL);

   // Update Dig Spots
   for(i = 0; i < size; i++)
   {
      dig_spot[i].timer += seconds;
   }

   // Remove spots

   // Iterate backward so that the removes don't effect the index
   for(i = size - 1; i < size; i--)
   {
      if(dig_spot[i].timer >= HOLE_TIMEOUT)
      {
         ArrayList_Remove(&level->dig_list, i);
      }
   }

}

static void Level_AddDigSpot(Level_T * level, int x, int y)
{
   DigSpot_T * dig_spot;
   dig_spot = ArrayList_Add(&level->dig_list, NULL);
   dig_spot->pos.x = x;
   dig_spot->pos.y = y;
   dig_spot->timer = 0;
}

static void Level_AddGold(Level_T * level, int x, int y)
{
   Gold_T * gold;
   gold = ArrayList_Add(&level->gold_list, NULL);
   gold->pos.x = x;
   gold->pos.y = y;
}

static void Level_RemoveGold(Level_T * level, size_t gold_index)
{
   ArrayList_Remove(&level->gold_list, gold_index);
}

static Gold_T * Level_GetGold(Level_T * level, int x, int y, size_t * out_index)
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

static DigSpot_T * Level_GetDigSpot(Level_T * level, int x, int y)
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

static void Level_QueryTile(Level_T * level, int x, int y, LevelTile_T * tile)
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


// E Level

