#include <stdio.h>
#include <stdlib.h>
#include "ArrayList.h"
#include "ConfigLoader.h"

typedef struct ConfigLoader_Pair_S ConfigLoader_Pair_T;

struct ConfigLoader_Pair_S
{
   char * str_key;
   char * str_value;

   union ConfigLoader_Value_U
   {
      int value_int;
      float value_float;
   } data;
};


static long Util_ParseLong(const char * long_str, int * error_flag);
static double Util_ParseDouble(const char * double_str, int * error_flag);

static void ConfigLoader_PopulateLists(ConfigLoader_T * loader);
static ConfigLoader_Pair_T * ConfigLoader_GetPair(ConfigLoader_T * loader, const char * key);
static void ConfigLoader_AddPair(ConfigLoader_T * loader, ArrayList_T * l_key, ArrayList_T * l_value);

void ConfigLoader_LoadFilename(ConfigLoader_T * loader, const char * filename)
{
   loader->file = fopen(filename, "r");
   loader->is_file_owned = 1;
   if(loader->file != NULL)
   {
      ConfigLoader_PopulateLists(loader);
   }

}

void ConfigLoader_LoadFile(ConfigLoader_T * loader, FILE * file)
{
   loader->file = file;
   loader->is_file_owned = 0;
   if(loader->file != NULL)
   {
      ConfigLoader_PopulateLists(loader);
   }
}

#define IS_WHITESPACE(x)  (((x) == ' ') || ((x) == '\t') || ((x) == '\r') || ((x) == '\n'))
#define IS_NEWLINE(x)     (((x) == '\r') || ((x) == '\n'))
#define STATE_BEFORE   0
#define STATE_KEY      1
#define STATE_BETWEEN  2
#define STATE_VALUE    3
#define STATE_COMMENT  4

static void ConfigLoader_PopulateLists(ConfigLoader_T * loader)
{
   ArrayList_T l_key, l_value;
   int c;
   int state;
   char * p;

   loader->pair_list = malloc(sizeof(ArrayList_T));
   ArrayList_Init(loader->pair_list, sizeof(ConfigLoader_Pair_T), 0);
   ArrayList_Init(&l_key,            sizeof(char),                0);
   ArrayList_Init(&l_value,          sizeof(char),                0);

   state = STATE_BEFORE;

   while((c = fgetc(loader->file)) != EOF)
   {
      if(state == STATE_BEFORE)
      {
         if(c == '#')
         {
            state = STATE_COMMENT;
         }
         else if(!IS_WHITESPACE(c))
         {
            state = STATE_KEY;
            ArrayList_Clear(&l_key);
            p = ArrayList_Add(&l_key, NULL);
            (*p) = (char)c;

         }
      }
      else if(state == STATE_KEY)
      {         
         if(!IS_WHITESPACE(c) && c != ':')
         {
            p = ArrayList_Add(&l_key, NULL);
            (*p) = (char)c;
         }
         else
         {
            state = STATE_BETWEEN;
            p = ArrayList_Add(&l_key, NULL);
            (*p) = '\0';
         }
      }
      else if(state == STATE_BETWEEN)
      {
         if(!IS_WHITESPACE(c) && c != ':')
         {
            state = STATE_VALUE;
            ArrayList_Clear(&l_value);
            p = ArrayList_Add(&l_value, NULL);
            (*p) = (char)c;
         }
      }
      else if(state == STATE_VALUE)
      {
         if(!IS_NEWLINE(c) && c != '#')
         {
            p = ArrayList_Add(&l_value, NULL);
            (*p) = (char)c;
         }
         else
         {
            if(c == '#')
            {
               state = STATE_COMMENT;
            }
            else
            {
               state = STATE_BEFORE;
            }
            p = ArrayList_Add(&l_value, NULL);
            (*p) = '\0';

            // Write Values to array
            ConfigLoader_AddPair(loader, &l_key, &l_value);
         }
      }
      else if(state == STATE_COMMENT)
      {
         if(IS_NEWLINE(c))
         {
            state = STATE_BEFORE;
         }
      }
      else
      {
         state = STATE_BEFORE;
      }
   }

   
   ArrayList_Destroy(&l_key);
   ArrayList_Destroy(&l_value);

}

static void ConfigLoader_AddPair(ConfigLoader_T * loader, ArrayList_T * l_key, ArrayList_T * l_value)
{
   size_t i, size;
   ConfigLoader_Pair_T * pair;
   pair            = ArrayList_Add(loader->pair_list,  NULL);
   pair->str_key   = ArrayList_GetCopy(l_key,   NULL,  NULL);
   pair->str_value = ArrayList_GetCopy(l_value, &size, NULL);

   // Trim the WhiteSpace Off the end
   for(i = size - 2; i < size; i--)
   {
      if(!IS_WHITESPACE(pair->str_value[i]))
      {
         pair->str_value[i + 1] = '\0';
         break;
      }
   }

   //printf("\"%s\" : \"%s\"\n", pair->str_key, pair->str_value);

}

void ConfigLoader_Destroy(ConfigLoader_T * loader)
{
   size_t i, size;
   ConfigLoader_Pair_T * pair;
   if(loader->file != NULL)
   {
      if( loader->is_file_owned == 1)
      {
         fclose(loader->file);
         loader->file = NULL;
      }

      pair = ArrayList_Get(loader->pair_list, &size, NULL);
      for(i = 0; i < size; i++)
      {
         free(pair[i].str_key);
         free(pair[i].str_value);
      }

      ArrayList_Destroy(loader->pair_list);
      free(loader->pair_list);
   }
   loader->pair_list = NULL;

}

static ConfigLoader_Pair_T * ConfigLoader_GetPair(ConfigLoader_T * loader, const char * key)
{
   size_t i, size;
   ConfigLoader_Pair_T * pairs, *result;

   if(loader->file != NULL)
   {
      pairs = ArrayList_Get(loader->pair_list, &size, NULL);
      result = NULL;
      for(i = 0; i < size; i++)
      {
         if(strcmp(key, pairs[i].str_key) == 0)
         {
            result = &pairs[i];
            break;
         }
      }
   }
   else
   {
      result = NULL;
   }
   return result;
}

int ConfigLoader_HasKey(ConfigLoader_T * loader, const char * key)
{

   ConfigLoader_Pair_T * pair;
   int result;
   pair = ConfigLoader_GetPair(loader, key);
   if(pair == NULL)
   {
      result = 0;
   }
   else
   {
      result = 1;
   }

   return result;
}


int ConfigLoader_GetInt(ConfigLoader_T * loader, const char * key, int default_value)
{
   ConfigLoader_Pair_T * pair;
   int                   result;
   int                   error_flag;   
   pair = ConfigLoader_GetPair(loader, key);
   if(pair != NULL)
   {
      error_flag = 0;
      result = (int)Util_ParseLong(pair->str_value, &error_flag);
      if(error_flag == 1)
      {
         result = default_value;
      }
   }
   else
   {
      result = default_value;
   }

   return result;
}

float ConfigLoader_GetFloat(ConfigLoader_T * loader, const char * key, float default_value)
{
   ConfigLoader_Pair_T * pair;
   float                 result;
   int                   error_flag;
   pair = ConfigLoader_GetPair(loader, key);
   if(pair != NULL)
   {
      error_flag = 0;
      result = (float)Util_ParseDouble(pair->str_value, &error_flag);
      if(error_flag == 1)
      {
         result = default_value;
      }
   }
   else
   {
      result = default_value;
   }

   return result;
}

const char * ConfigLoader_GetString(ConfigLoader_T * loader, const char * key, const char * default_value)
{
   ConfigLoader_Pair_T * pair;
   const char *          result;
   pair = ConfigLoader_GetPair(loader, key);
   if(pair != NULL)
   {
      result = pair->str_value;
   }
   else
   {
      result = default_value;
   }

   return result;

}


int ConfigLoader_GetBoolean(ConfigLoader_T * loader, const char * key, int default_value)
{
   ConfigLoader_Pair_T * pair;
   int                   result;
   int                   num_value;
   int                   error_flag;   
   pair = ConfigLoader_GetPair(loader, key);
   if(pair != NULL)
   {
      error_flag = 0;
      num_value = (int)Util_ParseLong(pair->str_value, &error_flag);

      if(error_flag == 1)
      {
         if(strcmp(pair->str_value, "true") == 0)
         {
            result = 1;
         }
         else if(strcmp(pair->str_value, "false") == 0)
         {
            result = 0;
         }
         else
         {
            result = default_value;
         }
      }
      else
      {
         if(num_value == 0 ||
            num_value == 1)
         {
            result = num_value;            
         }
         else
         {
            result = default_value;
         }
      }
   }
   else
   {
      result = default_value;
   }

   return result;

}

static long Util_ParseLong(const char * long_str, int * error_flag)
{
   long value;
   char * endptr;
   errno = 0;
   value = strtol(long_str, &endptr, 0);
   if(error_flag != NULL && (errno || *endptr != '\0'))
   {
      (*error_flag) = 1;
   }
   return value;
}

static double Util_ParseDouble(const char * double_str, int * error_flag)
{
   double value;
   char * endptr;
   errno = 0;
   value = strtod(double_str, &endptr);
   if(error_flag != NULL && (errno || *endptr != '\0'))
   {
      (*error_flag) = 1;
   }
   return value;
}


