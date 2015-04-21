#ifndef __GRIDREADER_H__
#define __GRIDREADER_H__

typedef struct GridReader_S GridReader_T;
struct GridReader_S
{
   size_t max_width;
   size_t height;
   size_t * width_list;
   char *** col_ptr;

   size_t data_size;
   size_t data_count;
   char ** data;
};

void GridReader_Init(GridReader_T * grid, const char * filename);
void GridReader_Destroy(GridReader_T * grid);

void GridReader_GetSize(GridReader_T * grid, int * max_width, int * height);
const char * GridReader_GetCell(GridReader_T * grid, int x, int y);
int GridReader_GetWidthOfY(GridReader_T * grid, int y);

#endif //__GRIDREADER_H__

