#include <stdio.h>
#include "GameInput.h"
#include "GameSettings.h"
#include "ConfigLoader.h"
#include "SDLInclude.h"

static ConfigLoader_T loader;
static GameSettings_T settings;
static int settings_loaded = 0;

static void GameSettings_ParseFile(void);

void GameSettings_Load(const char * filename)
{
   ConfigLoader_LoadFilename(&loader, filename);
   settings_loaded = 1;
   GameSettings_ParseFile();
}

GameSettings_T * GameSettings_Get(void)
{
   GameSettings_T * result;
   if(settings_loaded == 1)
   {
      result = &settings;
   }
   else
   {
      result = NULL;
   }
   return result;
}


void GameSettings_Cleanup(void)
{
   if(settings_loaded == 1)
   {
      settings_loaded = 0;
      ConfigLoader_Destroy(&loader);
   }
}

static void GameSettings_ParseFile(void)
{
   settings.window_width       = ConfigLoader_GetInt(&loader,     "window.width",           800);
   settings.window_height      = ConfigLoader_GetInt(&loader,     "window.height",          600);
   settings.window_fullscreen  = ConfigLoader_GetBoolean(&loader, "window.fullscreen",      0);
   settings.background_color_r = ConfigLoader_GetInt(&loader,     "background.color.red",   0);
   settings.background_color_g = ConfigLoader_GetInt(&loader,     "background.color.green", 0);
   settings.background_color_b = ConfigLoader_GetInt(&loader,     "background.color.blue",  0);
   settings.foreground_color_r = ConfigLoader_GetInt(&loader,     "foreground.color.red",   255);
   settings.foreground_color_g = ConfigLoader_GetInt(&loader,     "foreground.color.green", 255);
   settings.foreground_color_b = ConfigLoader_GetInt(&loader,     "foreground.color.blue",  255);
   settings.levelset_filename  = ConfigLoader_GetString(&loader,  "game.levelset", "main_levelset.txt");

   settings.game_keys[e_gigk_restart_level]             = ConfigLoader_GetString(&loader,  "controls.game.restart_level", "R");
   settings.player1_keys.key_string[e_gipk_move_up]     = ConfigLoader_GetString(&loader,  "controls.player1.move_up", "Keypad 8");
   settings.player1_keys.key_string[e_gipk_move_down]   = ConfigLoader_GetString(&loader,  "controls.player1.move_down", "Keypad 5");
   settings.player1_keys.key_string[e_gipk_move_left]   = ConfigLoader_GetString(&loader,  "controls.player1.move_left", "Keypad 4");
   settings.player1_keys.key_string[e_gipk_move_right]  = ConfigLoader_GetString(&loader,  "controls.player1.move_right", "Keypad 6");
   settings.player1_keys.key_string[e_gipk_dig_left]    = ConfigLoader_GetString(&loader,  "controls.player1.dig_left", "Keypad 7");
   settings.player1_keys.key_string[e_gipk_dig_right]   = ConfigLoader_GetString(&loader,  "controls.player1.dig_right", "Keypad 9");

   settings.volume_master  = ConfigLoader_GetFloat(&loader, "volume.master",  100.0f);
   settings.volume_music   = ConfigLoader_GetFloat(&loader, "volume.music",   100.0f);
   settings.volume_effects = ConfigLoader_GetFloat(&loader, "volume.effects", 100.0f);

   // Compute actual music volumes based on master
   settings.raw_volume_music = (int)(MIX_MAX_VOLUME * (
                               (settings.volume_master / 100.0f) *
                               (settings.volume_music  / 100.0f)
                               ));

   settings.raw_volume_effects = (int)(MIX_MAX_VOLUME * (
                                 (settings.volume_master  / 100.0f) *
                                 (settings.volume_effects / 100.0f)
                                 ));

   settings.music_background = ConfigLoader_GetString(&loader, "music.background", "01-TimurIzhbulatov-Revz.ogg");
}



