#ifndef __GAMEINPUT_H__
#define __GAMEINPUT_H__



typedef enum GameInput_PlayerKeys_E GameInput_PlayerKeys_T;
typedef enum GameInput_GameKeys_E   GameInput_GameKeys_T;
enum GameInput_PlayerKeys_E
{
   e_gipk_move_up,
   e_gipk_move_down,
   e_gipk_move_left,
   e_gipk_move_right,
   e_gipk_dig_left,
   e_gipk_dig_right,
   e_gipk_last
};

enum GameInput_GameKeys_E
{
   e_gigk_restart_level,
   e_gigk_last
};

#ifdef SDL_LIB_INCLUDED

void GameInput_PopulateSDLScancodes(SDL_Scancode * scancode, const char ** text, size_t count);

#endif // SDL_LIB_INCLUDED



#endif // __GAMEINPUT_H__

