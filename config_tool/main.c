#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GridReader.h"

// Structure Headder File
// Example Config File
// Include file that populates a structure

#define CMD_NONE    0
#define CMD_INT     1
#define CMD_FLOAT   2
#define CMD_STRING  3
#define CMD_BOOLEAN 4
#define CMD_COMMENT 5
#define CMD_EMPTY   6

static char * GetVariableString(const char * str)
{
   char * out;
   size_t len, i;
   len = strlen(str) + 1;
   out = malloc(sizeof(char) * len);

   for(i = 0; i < len; i++)
   {
      if(str[i] == '.')
      {
         out[i] = '_';
      }
      else
      {
         out[i] = str[i];
      }
   }
   return out;

}

static void CreateStructHeader(GridReader_T * reader,
                               const char * filename, 
                               const char * struct_name,
                               const char * ifdef_block_name)
{
   FILE * file;

   int height;
   int y;
   const char * str;
   const char * cmd, * name, * description, * comment;
   char * var_name;
   int cmd_type;
   int is_param;

   file = fopen(filename, "w");
   fprintf(file, "// This code is AUTO-GENERATED!!!\n");
   fprintf(file, "// Do not manualy edit!\n");
   fprintf(file, "#ifndef %s\n", ifdef_block_name);
   fprintf(file, "#define %s\n", ifdef_block_name);
   fprintf(file, "\n");
   fprintf(file, "typedef struct %s_S %s_T;\n", struct_name, struct_name);
   fprintf(file, "struct %s_S\n", struct_name);
   fprintf(file, "{\n");
   
   GridReader_GetSize(reader, NULL, &height);
   for(y = 0; y < height; y++ )
   {
      // Write DataType
      cmd = GridReader_GetCell(reader, 0, y);
      if(cmd == NULL)
      {
         cmd_type = CMD_NONE;
      }
      else
      {
         switch(cmd[0])
         {
            case 'i':
            case 'b':
               cmd_type = CMD_INT;
               is_param = 1;
               break;
            case 'f':
               cmd_type = CMD_FLOAT;
               is_param = 1;
               break;
            case 's':
               cmd_type = CMD_STRING;
               is_param = 1;
               break;
            case 'c':
               cmd_type = CMD_COMMENT;
               is_param = 0;
               break;
            case 'e':
               cmd_type = CMD_EMPTY;
               is_param = 0;
               break;
            default:
               cmd_type = CMD_NONE;
               is_param = 0;
               break;
         }
      }

      if(cmd_type == CMD_COMMENT)
      {
         comment = GridReader_GetCell(reader, 1, y);     
      }
      else if(cmd_type != CMD_NONE)
      {
         name        = GridReader_GetCell(reader, 1, y);
         description = GridReader_GetCell(reader, 3, y);
      }
      else
      {
         name        = NULL;
         description = NULL;
         comment     = NULL;
      }

     
      if(is_param == 1 && name != NULL)
      {
         switch(cmd_type)
         {
            case CMD_INT:    str = "int         "; break;
            case CMD_FLOAT:  str = "float       "; break;
            case CMD_STRING: str = "const char *"; break;
         }
         var_name = GetVariableString(name);
         fprintf(file, "   %s %s;", str, var_name);
         free(var_name);
         if(description != NULL)
         {
            fprintf(file, " /* %s */", description);
         }
         fprintf(file, "\n");

      }
      else if(cmd_type == CMD_COMMENT && comment != NULL)
      {
         fprintf(file, "   /* %s */\n", comment);
      }
      else if(cmd_type == CMD_EMPTY)
      {
         fprintf(file, "\n");
      }

   }

   fprintf(file, "};\n");
   fprintf(file, "\n");
   fprintf(file, "#endif // %s\n", ifdef_block_name);
   fprintf(file, "\n");
   fclose(file);
}

static void CreateLoaderFunction(GridReader_T * reader,
                                 const char * filename, 
                                 const char * function_name,
                                 const char * struct_name,
                                 const char * struct_variable)
{
   FILE * file;

   int height;
   int y;
   const char * fname;
   const char * cmd, *name, * default_val;
   const char * dvq; // Default value quote
   char * var_name;
   int cmd_type;

   file = fopen(filename, "w");
   fprintf(file, "// This code is AUTO-GENERATED!!!\n");
   fprintf(file, "// Do not manualy edit!\n");
   fprintf(file, "\n");
   fprintf(file, 
           "static void %s(ConfigLoader_T * loader, %s_T * %s)\n", 
           function_name, 
           struct_name, 
           struct_variable);
   fprintf(file, "{\n");
   
   GridReader_GetSize(reader, NULL, &height);
   for(y = 0; y < height; y++ )
   {
      // Write DataType
      cmd = GridReader_GetCell(reader, 0, y);
      if(cmd == NULL)
      {
         cmd_type = CMD_NONE;
      }
      else
      {
         switch(cmd[0])
         {
            case 'b':
               cmd_type = CMD_BOOLEAN;
               break;
            case 'i':
               cmd_type = CMD_INT;
               break;
            case 'f':
               cmd_type = CMD_FLOAT;
               break;
            case 's':
               cmd_type = CMD_STRING;
               break;
            default:
               cmd_type = CMD_NONE;
               break;
         }
      }

      if(cmd_type != CMD_NONE)
      {
         name        = GridReader_GetCell(reader, 1, y);
         default_val = GridReader_GetCell(reader, 2, y);
      }
      else
      {
         name        = NULL;
         default_val = NULL;
      }

      if(name != NULL & default_val != NULL)
      {
         switch(cmd_type)
         {
            case CMD_INT:    
               dvq = "";   
               fname = "ConfigLoader_GetInt"; 
               break;
            case CMD_FLOAT:  
               dvq = "";   
               fname = "ConfigLoader_GetFloat"; 
               break;
            case CMD_STRING: 
               dvq = "\""; 
               fname = "ConfigLoader_GetString"; 
               break;
            case CMD_BOOLEAN:
               dvq = "";
               fname = "ConfigLoader_GetBoolean";
               break;
         }
         var_name = GetVariableString(name);

         fprintf(file, 
                 "   %s->%s = %s(loader, \"%s\", %s%s%s);\n", 
                 struct_variable, var_name, fname, 
                 name, dvq, default_val, dvq);
         free(var_name);

      }

   }

   fprintf(file, "}\n");
   fprintf(file, "\n");
   fclose(file);
}


static void CreateConfigTemplate(GridReader_T * reader,
                                 const char * filename)
{
   FILE * file;

   int height;
   int y;
   const char * str;
   const char * cmd, * name, * description, * comment, *default_val, * dvq;
   char * var_name;
   int cmd_type;
   int is_param;

   file = fopen(filename, "w");
   fprintf(file, "# This is a template file for config.txt\n");
   fprintf(file, "# config.txt hold game configurations.\n");
   fprintf(file, "# This file lists all the default values.\n");
   fprintf(file, "\n");
   
   GridReader_GetSize(reader, NULL, &height);
   for(y = 0; y < height; y++ )
   {
      // Write DataType
      cmd = GridReader_GetCell(reader, 0, y);
      if(cmd == NULL)
      {
         cmd_type = CMD_NONE;
      }
      else
      {
         switch(cmd[0])
         {
            case 'i':
            case 'b':
               cmd_type = CMD_INT;
               is_param = 1;
               break;
            case 'f':
               cmd_type = CMD_FLOAT;
               is_param = 1;
               break;
            case 's':
               cmd_type = CMD_STRING;
               is_param = 1;
               break;
            case 'c':
               cmd_type = CMD_COMMENT;
               is_param = 0;
               break;
            case 'e':
               cmd_type = CMD_EMPTY;
               is_param = 0;
               break;
            default:
               cmd_type = CMD_NONE;
               is_param = 0;
               break;
         }
      }

      if(cmd_type == CMD_COMMENT)
      {
         comment = GridReader_GetCell(reader, 1, y);     
      }
      else if(cmd_type != CMD_NONE)
      {
         name        = GridReader_GetCell(reader, 1, y);
         default_val = GridReader_GetCell(reader, 2, y);
         description = GridReader_GetCell(reader, 3, y);
      }
      else
      {
         name        = NULL;
         description = NULL;
         default_val = NULL;
         comment     = NULL;
      }

     
      if(is_param == 1 && name != NULL && default_val != NULL)
      {
         if(description != NULL)
         {
            fprintf(file, "# %s\n", description);
         }
         if(cmd_type == CMD_STRING)
         {
            dvq = "\"";
         }
         else
         {
            dvq = "";
         }
         fprintf(file, "%s : %s%s%s", name, dvq, default_val, dvq);
         fprintf(file, "\n");

      }
      else if(cmd_type == CMD_COMMENT && comment != NULL)
      {
         fprintf(file, "# %s\n", comment);
      }
      else if(cmd_type == CMD_EMPTY)
      {
         fprintf(file, "\n");
      }

   }

   fprintf(file, "\n");
   fclose(file);
}
int main(int argc, char * args[])
{
   const char * source_filename;
   const char * struct_header_filename;
   const char * example_config_filename;
   const char * populate_function_filename;
   const char * function_name;
   const char * struct_name;
   const char * struct_variable;
   const char * ifdef_block_name;

   GridReader_T reader;


   source_filename            = "config_source.txt";
   struct_header_filename     = "GameConfigData.h";
   example_config_filename    = "config_template.txt";
   populate_function_filename = "GameConfigData.inl";
   function_name              = "PopulateData";
   struct_name                = "GameConfigData";
   ifdef_block_name           = "__GAMECONFIGDATA_H__";
   struct_variable            = "config";

   GridReader_Init(&reader, source_filename);

   CreateStructHeader(&reader, 
                      struct_header_filename, 
                      struct_name, 
                      ifdef_block_name);

   CreateLoaderFunction(&reader,
                        populate_function_filename,
                        function_name,
                        struct_name,
                        struct_variable);

   CreateConfigTemplate(&reader,
                        example_config_filename);



   GridReader_Destroy(&reader);
   // printf("End\n"); // For testing only
   return 0;
}

