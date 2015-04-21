#include <stdio.h>
#include <stdlib.h>
#include "GridReader.h"

typedef struct GridReader_String_S GridReader_String_T;
struct GridReader_String_S
{
   size_t size;
   size_t count;
   char ** string;
};


#define GROW_BY 16

static void GridReader_String_Add(GridReader_String_T * str, int ic)
{
   size_t new_size;
   if((*str->string) == NULL)
   {
      new_size = GROW_BY;
      (*str->string) = malloc(sizeof(char) * new_size);
      str->count = 0;
      str->size = new_size;
   }
   else if(str->count >= str->size)
   {
      new_size = str->size + GROW_BY;
      (*str->string) = realloc(*str->string, sizeof(char) * new_size);
      str->size = new_size;
   }

   (*str->string)[str->count] = (char)ic;
   str->count ++;
}

static void GridReader_String_End(GridReader_String_T * str)
{
   if(str->string != NULL && 
      (*str->string) != NULL && 
      (*str->string)[str->count -1 ] != '\0')
   {
      GridReader_String_Add(str, '\0');
   }
}

static GridReader_String_T GridReader_GetSpot(GridReader_T * grid)
{
   size_t new_size;
   GridReader_String_T result;

   if(grid->data == NULL)
   {
      new_size         = GROW_BY;
      grid->data       = malloc(sizeof(char *) * new_size);
      grid->data_count = 0;
      grid->data_size  = new_size;
   }
   else if(grid->data_count >= grid->data_size)
   {
      new_size         = grid->data_size + GROW_BY;
      grid->data       = realloc(grid->data, sizeof(char *) * new_size);
      grid->data_size  = new_size;
   }
   result.string = &grid->data[grid->data_count];
   grid->data_count ++;
   (*result.string) = NULL;
   return result;
}

#define IS_WHITESPACE(x) ((x) == '\t' || (x) == ' ')
#define IS_NEWLINE(x)    ((x) == '\r' || (x) == '\n')

#define STATE_WHITESPACE 0
#define STATE_QUOTEVALUE 1
#define STATE_RAWVALUE   2
#define STATE_NEWLINE    3
#define STATE_COMMENT    4


void GridReader_Init(GridReader_T * grid, const char * filename)
{
   // The Parsing Data
   FILE * file;
   int ci, prev_ci;
   int state;
   int transition;
   GridReader_String_T string;
   int line_used_flag;
   // The Counting Data
   size_t i;
	size_t width;
	size_t index;

   // The Parsing Code
   prev_ci = 0;
   line_used_flag = 0;
   state = STATE_WHITESPACE;
   grid->data = NULL;
   string.string = NULL;

   file = fopen(filename, "r");
   
   while((ci = fgetc(file)) != EOF || state != STATE_NEWLINE)
   {
      transition = 0;
		if(ci == EOF) ci = '\n';
      switch(state)
      {
         case STATE_WHITESPACE:
            if(IS_NEWLINE(ci))
            {
               transition = 1;
               state = STATE_NEWLINE;
            }
            else if(ci == '#')
            {
               transition = 1;
               state = STATE_COMMENT;
            }
            else if(!IS_WHITESPACE(ci))
            {
               transition = 1;
               if(ci == '"')
               {
                  state = STATE_QUOTEVALUE;
               }
               else
               {
                  state = STATE_RAWVALUE;
               }
            }
            break;
         case STATE_QUOTEVALUE:
            if(ci == '"' && prev_ci != '\\')
            {
               transition = 1;
               state = STATE_WHITESPACE;
            }
            else if(prev_ci == '\\')
            {
               if(ci == '"')
               {
                  GridReader_String_Add(&string, '"');
               }
               else if(ci == '\\')
               {
                  GridReader_String_Add(&string, '\\');
               }
               else if(ci == 't')
               {
                  GridReader_String_Add(&string, '\t');
               }
               else if(ci == 'n')
               {
                  GridReader_String_Add(&string, '\n');
               }
            }
            else
            {   
               GridReader_String_Add(&string, ci);
            }
            break;
         case STATE_RAWVALUE:
            if(IS_WHITESPACE(ci))
            {
               transition = 1;
               state = STATE_WHITESPACE;
            }
            else if(IS_NEWLINE(ci))
            {
               transition = 1;
               state = STATE_NEWLINE;
            }
            else
            {
               GridReader_String_Add(&string, ci);
            }
            break;
         case STATE_NEWLINE:
            if(!IS_NEWLINE(ci))
            {
               transition = 1;
               if(ci == '"')
               {
                  state = STATE_QUOTEVALUE;
               }
               else if(IS_WHITESPACE(ci))
               {
                  state = STATE_WHITESPACE;
               }
               else if(ci == '#')
               {
                  state = STATE_COMMENT;
               }
               else
               {
                  state = STATE_RAWVALUE;
               }
            }
            break;
         case STATE_COMMENT:
            if(IS_NEWLINE(ci))
            {
               transition = 1;
               state = STATE_NEWLINE;
            }
            break;
         default:
            state = STATE_WHITESPACE;
            break;
      }
      if(transition == 1) // Transition To...
      {
         switch(state)
         {
            case STATE_WHITESPACE:
               GridReader_String_End(&string); 
               break;
            case STATE_QUOTEVALUE:
               string = GridReader_GetSpot(grid);
               line_used_flag = 1;
               break;
            case STATE_RAWVALUE:
               string = GridReader_GetSpot(grid);
               GridReader_String_Add(&string, ci);
               line_used_flag = 1;
               break;
            case STATE_NEWLINE:
               GridReader_String_End(&string); 
               if(line_used_flag == 1)
               {
                  string = GridReader_GetSpot(grid);
               }
               line_used_flag = 0;
               break;
            default:
               break;

         }
      }
      prev_ci = ci;
   }

   GridReader_String_End(&string); 
   fclose(file);
  
   // The Counting Code
	grid->height = 0;
   
   for(i = 0; i < grid->data_count; i++)
   {
      if(grid->data[i] == NULL)
      {
			grid->height ++;
      }
   }
	
	grid->width_list = malloc(sizeof(size_t)  * grid->height);
	grid->col_ptr    = malloc(sizeof(char **) * grid->height);
	grid->max_width  = 0;
	index = 0;
	width = 0;
	grid->col_ptr[0] = &grid->data[0];
   for(i = 0; i < grid->data_count; i++)
   {
      if(grid->data[i] == NULL)
      {
			grid->width_list[index] = width;
			if(grid->max_width < width)
			{
				grid->max_width = width;
			}
			index ++;
         if(index < grid->height)
         {
			   grid->col_ptr[index] = &grid->data[i + 1];
            //printf("s %s\n", grid->data[i + 1]);
         }
			
			width = 0;
      }
      else
      {
			width ++;
      }
   }
   
}

void GridReader_Destroy(GridReader_T * grid)
{
	size_t i;
   for(i = 0; i < grid->data_count; i++)
   {
      if(grid->data[i] != NULL)
      {
			free(grid->data[i]);
      }
   }

	free(grid->col_ptr);
	free(grid->width_list);
	free(grid->data);
}

void GridReader_GetSize(GridReader_T * grid, int * max_width, int * height)
{
	if(max_width != NULL)
	{
		(*max_width) = grid->max_width;
	}

	if(height != NULL)
	{
		(*height) = grid->height;
	}
}

const char * GridReader_GetCell(GridReader_T * grid, int x, int y)
{
	const char * result;
	if(y >= 0 && x >= 0 && y < grid->height && x < grid->width_list[y])
	{
		result = *(grid->col_ptr[y] + x);
	}
	else
	{
		result = NULL;
	}
	return result;

}


int GridReader_GetWidthOfY(GridReader_T * grid, int y)
{
	int result;
	if(y >= 0 && y < grid->height)
	{
		result = grid->width_list[y];		
	}
	else
	{
		result = 0;
	}
	return result;

}


