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


#include "GlobalData.h"
#include "SDLTools.h"

#include "ArrayList.h"
#include "Pos2D.h"
#include "Level.h"
#include "FontText.h"

#include "ConfigLoader.h"



#define PLAYER_STATE_NOT_MOVING  0
#define PLAYER_STATE_MOVING      1
#define PLAYER_STATE_FALLING     2
#define PLAYER_STATE_DIGGING     3
#define PLAYER_STATE_DEATH       4
#define PLAYER_STATE_WIN         5



typedef struct GameSetting_S GameSettings_T;
struct GameSetting_S
{
   int window_width;
   int window_height;
   int window_fullscreen; // boolean
   int background_color_r;
   int background_color_g;
   int background_color_b;
   int foreground_color_r;
   int foreground_color_g;
   int foreground_color_b;
};


typedef enum PlayerInput_E PlayerInput_T;
enum PlayerInput_E
{
   e_pi_move_up,
   e_pi_move_down,
   e_pi_move_left,
   e_pi_move_right,
   e_pi_dig_left,
   e_pi_dig_right,
   e_pi_last
};

typedef enum GameInput_E GameInput_T;
enum GameInput_E
{
   e_gi_restart_level,
   e_gi_last
};


typedef struct player_data_s player_data_t;
struct player_data_s
{
   int input_flags[e_pi_last];
   Pos2D_T grid_p;
   Pos2D_T next_grid_p;
   int player_state;
   float move_timer;
   float move_timeout;
};


static int IsTerrainPassable(LevelTile_T * from, LevelTile_T * to);
static int IsTerrainFallable(LevelTile_T * from, LevelTile_T * to);
static int IsAllGoldColected(Level_T * level);
static void UpdateGoldCount(Level_T * level, FontText_T * gold_count_text);

static void CheckForExit(const SDL_Event *event, int * done);


static void handle_input(const SDL_Event * event, int * done, int * game_input_flags, int * player_input_flags);

static void handle_update(float seconds, Level_T * level, int * game_input_flags,  player_data_t * player1_data, FontText_T * gold_count_text);

static void handle_render(SDL_Renderer * rend, 
                          SDL_Texture * t_terrain, 
                          SDL_Texture * t_character,
                          Level_T * level, 
                          player_data_t * player1_data);

int main(int args, char * argc[])
{
   SDL_Window  * window;
   SDL_Renderer * rend;   
   SDL_Event event;
   SDL_Texture * t_terrain;
   SDL_Texture * t_character;
   int done;
   int prevTicks, diffTicks, nowTicks;
   float seconds;

   int i;
   int game_input_flags[e_gi_last];

   // Font
   TTF_Font * font;
   FontText_T gold_count_text;
 
   player_data_t player1_data;
   Level_T level;

   ConfigLoader_T loader;
   GameSettings_T game_settings;
   
   ConfigLoader_LoadFilename(&loader, "config.txt");
   game_settings.window_width       = ConfigLoader_GetInt(&loader,     "window.width",           800);
   game_settings.window_height      = ConfigLoader_GetInt(&loader,     "window.height",          600);
   game_settings.window_fullscreen  = ConfigLoader_GetBoolean(&loader, "window.fullscreen",      0);
   game_settings.background_color_r = ConfigLoader_GetInt(&loader,     "background.color.red",   0);
   game_settings.background_color_g = ConfigLoader_GetInt(&loader,     "background.color.green", 0);
   game_settings.background_color_b = ConfigLoader_GetInt(&loader,     "background.color.blue",  0);
   game_settings.foreground_color_r = ConfigLoader_GetInt(&loader,     "foreground.color.red",   255);
   game_settings.foreground_color_g = ConfigLoader_GetInt(&loader,     "foreground.color.green", 255);
   game_settings.foreground_color_b = ConfigLoader_GetInt(&loader,     "foreground.color.blue",  255);


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
                             game_settings.window_width,
                             game_settings.window_height,
                             SDL_WINDOW_SHOWN | ((game_settings.window_fullscreen == 1) ? SDL_WINDOW_FULLSCREEN : 0) );
   
   rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
   
   t_terrain   = SDLTools_LoadTexture(rend, "terrain.png");
   t_character = SDLTools_LoadTexture(rend, "character.png");
   
   font = TTF_OpenFont("cnr.otf", 28);
   if(font == NULL)
   {
      printf("Font Null\n");
   }
   FontText_Init(&gold_count_text, font, rend);
   FontText_SetColor(&gold_count_text,
                     game_settings.foreground_color_r,
                     game_settings.foreground_color_g,
                     game_settings.foreground_color_b, 0xFF);

   UpdateGoldCount(&level, &gold_count_text);


   prevTicks = SDL_GetTicks();
   
   done = 0;
   while(done == 0)
   {
      while(SDL_PollEvent(&event))
      {
         handle_input(&event, &done, game_input_flags, player1_data.input_flags);
      }
      
      nowTicks = SDL_GetTicks();
      diffTicks = nowTicks - prevTicks;
      seconds = (float)diffTicks / 1000.0f;
      prevTicks = nowTicks;
      
      handle_update(seconds, &level, game_input_flags, &player1_data, &gold_count_text);
      
      SDL_SetRenderDrawColor(rend, 
                             game_settings.background_color_r, 
                             game_settings.background_color_g,
                             game_settings.background_color_b, 0xFF);
      SDL_RenderClear( rend );
      
      handle_render(rend, t_terrain, t_character, &level, &player1_data);
      FontText_Render(&gold_count_text, 10, 10);
      SDL_RenderPresent(rend);
   }
   
   ConfigLoader_Destroy(&loader);

   Level_Destroy(&level); 
   
   SDL_DestroyRenderer(rend);
   SDL_DestroyWindow(window);
   SDL_DestroyTexture(t_terrain);
   SDL_DestroyTexture(t_character);

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


static void handle_input(const SDL_Event * event, int * done, int * game_input_flags, int * player_input_flags)
{
   PlayerInput_T pkey;
   GameInput_T gkey;
   CheckForExit(event, done);

   if(event->key.keysym.sym == SDLK_KP_8)
   {
      pkey = e_pi_move_up;
   }
   else if(event->key.keysym.sym == SDLK_KP_5)
   {
      pkey = e_pi_move_down;
   }
   else if(event->key.keysym.sym == SDLK_KP_4)
   {
      pkey = e_pi_move_left;
   }
   else if(event->key.keysym.sym == SDLK_KP_6)
   {
      pkey = e_pi_move_right;
   }
   else if(event->key.keysym.sym == SDLK_KP_7)
   {
      pkey = e_pi_dig_left;
   }
   else if(event->key.keysym.sym == SDLK_KP_9)
   {
      pkey = e_pi_dig_right;
   }
   else
   {
      pkey = e_pi_last;
   }

   if(pkey != e_pi_last)
   {
      if(event->type == SDL_KEYDOWN) player_input_flags[pkey] = 1;
      if(event->type == SDL_KEYUP)   player_input_flags[pkey] = 0;
   }

   if(event->key.keysym.sym == SDLK_r)
   {
      gkey = e_gi_restart_level;
   }
   else
   {
      gkey = e_gi_last;
   }

   if(gkey != e_gi_last)
   {
      if(event->type == SDL_KEYDOWN) game_input_flags[gkey] = 1;
      if(event->type == SDL_KEYUP)   game_input_flags[gkey] = 0;
   }



}


static void handle_update(float seconds, 
                          Level_T * level, 
                          int * game_input_flags, 
                          player_data_t * player1_data, 
                          FontText_T * gold_count_text)
{
   int cmd_dig_left_valid, cmd_dig_right_valid;
   int cmd_move_left_valid, cmd_move_right_valid;
   int cmd_move_up_valid, cmd_move_down_valid, cmd_let_go_valid, cmd_fall_valid;
   LevelTile_T player_current_tile, player_desired_tile, dig_desired_tile;
   LevelTile_T dig_above_tile, player_below_tile;
   int all_gold_colected;
   static int restart_key_prev = 0;
    
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
            (dig_above_tile.terrain_type == TMAP_TILE_AIR || dig_above_tile.has_hole == 1))
         {
            cmd_dig_left_valid = 1;
         }
      }
      if(player1_data->input_flags[e_pi_dig_right] == 1) // Dig Right
      {
         Level_QueryTile(level, POS_SPLIT(player1_data->grid_p, 1, 1), &dig_desired_tile);
         Level_QueryTile(level, POS_SPLIT(player1_data->grid_p, 1, 0), &dig_above_tile);
         if(dig_desired_tile.terrain_type == TMAP_TILE_DIRT && 
            (dig_above_tile.terrain_type == TMAP_TILE_AIR || dig_above_tile.has_hole == 1))
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
         //printf("ComeAlive!\n");
      }
   }

   if(game_input_flags[e_gi_restart_level] == 0 && restart_key_prev == 1)
   {
      Level_Restart(level);
      player1_data->player_state = PLAYER_STATE_DEATH;
      UpdateGoldCount(level, gold_count_text);
   }
   restart_key_prev = game_input_flags[e_gi_restart_level];
   Level_Update(level, seconds);
}

static void handle_render(SDL_Renderer * rend, 
                          SDL_Texture * t_terrain, 
                          SDL_Texture * t_character,
                          Level_T * level, 
                          player_data_t * player1_data)
{
   Pos2D_T draw_loc;
   Pos2D_T diff;
   float move_percent;
   int show;

   
   Level_Render(level, rend, t_terrain);

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
      SDLTools_DrawSubimage(rend, t_character, IMGID_GUY, draw_loc.x, 
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
   if((to->terrain_type == TMAP_TILE_AIR || to->terrain_type == TMAP_TILE_BAR || to->has_hole == 1 || to->terrain_type == TMAP_TILE_DOOR) && 
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
   int gold_count;
 
   gold_count = Level_GetGoldCount(level, NULL);
   if(gold_count > 0)
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
   int gold_left, gold_total;
   char buffer[255];
   gold_left = Level_GetGoldCount(level, &gold_total);

   sprintf(buffer, "Gold %i/%i", gold_left, gold_total);
   FontText_SetString(gold_count_text, buffer);
}


