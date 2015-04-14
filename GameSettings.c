#include <stdio.h>
#include "GameSettings.h"
#include "ConfigLoader.h"

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
}


