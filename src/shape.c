#include <stdio.h>
#include <time.h>

#define NN_IMPLEMENTATION
#include "nn.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define OLIVEC_AA_RES 1
#define OLIVEC_IMPLEMENTATION
#include "olive.c"

#define WIDTH 28
#define HEIGHT WIDTH

enum {
  SHAPE_CIRCLE,
  SHAPE_RECT,
  SHAPES,
};

#define SAMPLES_PER_SHAPE 10
#define BACKGROUND_COLOR 0xFF000000
#define FOREGROUND_COLOR 0xFFFFFFFF

size_t arch[] = {WIDTH*HEIGHT, 14, SHAPES};

void random_boundary(size_t width, size_t height, int *x1, int *y1, int *w, int *h)
{
  int x2, y2, i = 0;
  do
  {
    *x1 = rand()%width;
    *y1 = rand()%height;
    x2 = rand()%width;
    y2 = rand()%height;
    if (*x1 > x2) OLIVEC_SWAP(int, *x1, x2);
    if (*y1 > y2) OLIVEC_SWAP(int, *y1, y2);
    *w = x2 - *x1;
    *h = y2 - *y1;
  }
  while((*w < 4 || *h < 4) && i++ < 100);
  assert(*w >= 4 && *h >= 4);
}

void random_circle(Olivec_Canvas oc)
{
  int x, y, w, h;
  random_boundary(oc.width, oc.height, &x, &y, &w, &h);
  olivec_fill(oc, BACKGROUND_COLOR);
  int r = ( w < h ? w : h)/2;
  olivec_circle(oc, x + w/2, y + h/2, r, FOREGROUND_COLOR);
}

void random_rect(Olivec_Canvas oc)
{
  int x, y, w, h;
  random_boundary(oc.width, oc.height, &x, &y, &w, &h);
  olivec_fill(oc, BACKGROUND_COLOR);
  olivec_rect(oc, x, y, w, h, FOREGROUND_COLOR);
}

void canvas_to_row(Olivec_Canvas oc, Row row)
{
  NN_ASSERT(oc.width*oc.height == row.cols);
  for (size_t y = 0; y < oc.height; ++y) {
    for (size_t x = 0; x < oc.width; ++x) {
      //convert into brightness
      ROW_AT(row, y*oc.width + x) = (float)(OLIVEC_PIXEL(oc, x, y)&0xFF)/255.f;
    }
  }
}

int main (void)
{
  srand(time(0));

  //256 MB big arena allocator
  Region temp = region_alloc_alloc(256*1024*1024);
  Olivec_Canvas oc = {0};
  //random_circle(oc);
  oc.pixels = region_alloc(&temp, WIDTH*HEIGHT*sizeof(*oc.pixels));
  oc.width = WIDTH;
  oc.height = HEIGHT;
  oc.stride = WIDTH;

  // training data matrix
  NN nn = nn_alloc(NULL, arch, ARRAY_LEN(arch));
  nn_rand(nn, -1, 1);

  // rows: 10 samples per shape * 2 shapes, cols = width*height,2
  Mat t = mat_alloc(NULL, SAMPLES_PER_SHAPE*SHAPES, NN_INPUT(nn).cols + NN_OUTPUT(nn).cols);
  for (size_t i = 0; i < SAMPLES_PER_SHAPE; ++i) {
    for (size_t j = 0; j < SHAPES; ++j) {
      Row row = mat_row(t, i*2 + j); //0, 1, 2, 3, 4, 5..20
      Row in = row_slice(row, 0, NN_INPUT(nn).cols);
      Row out = row_slice(row, NN_INPUT(nn).cols, NN_OUTPUT(nn).cols);
      switch(j) {
      case SHAPE_CIRCLE: random_circle(oc); break;
      case SHAPE_RECT: random_rect(oc); break;
      default: assert(0 && "unreachable");
      }
      canvas_to_row(oc, in);
      row_fill(out, 0);
      ROW_AT(out, j) = 1.0f;
    }
  }

  for (size_t i = 0; i < t.rows; ++i) {
    Row row = mat_row(t, i);
    Row in = row_slice(row, 0, NN_INPUT(nn).cols);
    Row out = row_slice(row, NN_INPUT(nn).cols, NN_OUTPUT(nn).cols);

    for (size_t y = 0; y < HEIGHT; ++y) {
      for (size_t x = 0; x < WIDTH; ++x) {
        printf("%f ", ROW_AT(in, y*WIDTH + x));
      }
      printf("\n");
    }
    MAT_PRINT(row_as_mat(out));
    }
  return 0;
}
