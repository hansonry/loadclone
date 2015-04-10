#ifndef __CONFIGLOADER_H__
#define __CONFIGLOADER_H__

typedef struct ConfigLoader_S ConfigLoader_T;

typedef struct ArrayList_S ArrayList_T;



struct ConfigLoader_S
{
   FILE        * file;
   int           is_file_owned;
   ArrayList_T * pair_list;
};

void ConfigLoader_LoadFilename(ConfigLoader_T * loader, const char * filename);
void ConfigLoader_LoadFile(ConfigLoader_T * loader, FILE * file);

void ConfigLoader_Destroy(ConfigLoader_T * loader);


int ConfigLoader_HasKey(ConfigLoader_T * loader, const char * key);


int ConfigLoader_GetInt(ConfigLoader_T * loader, const char * key, int default_value);

float ConfigLoader_GetFloat(ConfigLoader_T * loader, const char * key, float default_value);

const char * ConfigLoader_GetString(ConfigLoader_T * loader, const char * key, const char * default_value);


int ConfigLoader_GetBoolean(ConfigLoader_T * loader, const char * key, int default_value);

#endif // __CONFIGLOADER_H__

