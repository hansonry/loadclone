#ifndef __LEVELSET_H__
#define __LEVELSET_H__

typedef struct LevelSet_S LevelSet_T;

struct LevelSet_S
{
   ArrayList_T level_list;
};


void LevelSet_Init(LevelSet_T * levelset);
void LevelSet_Destroy(LevelSet_T * levelset);


void LevelSet_Load(LevelSet_T * levelset, const char * filename);

Level_T * LevelSet_GetAll(LevelSet_T * levelset, size_t * size);

Level_T * LevelSet_GetLevel(LevelSet_T * levelset, size_t index);



#endif // __LEVELSET_H__

