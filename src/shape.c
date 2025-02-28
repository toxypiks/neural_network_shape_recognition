#include <stdio.h>
#include <time.h>

#define OLIVEC_AA_RES 1
#define OLIVEC_IMPLEMENTATION
#include "olive.c"

#define GYM_IMPLEMENTATION
#include "gym.h"

#define NN_IMPLEMENTATION
#include "nn.h"

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
size_t batch_size = 5;
size_t batches_per_frame = 4;
float rate = 1.f;
bool paused = true;

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

Mat generate_samples(Region *r, size_t samples)
{
  size_t input_size = WIDTH*HEIGHT;
  size_t output_size = SHAPES;
  // matrix of training data
  Mat t = mat_alloc(r, samples*SHAPES, input_size + output_size);
  size_t s = region_save(r);
  Olivec_Canvas oc = {0};
  oc.pixels = region_alloc(r, WIDTH*HEIGHT*sizeof(*oc.pixels));
  oc.width = WIDTH;
  oc.height = HEIGHT;
  oc.stride = WIDTH;
  for (size_t i = 0; i < samples; ++i) {
    for (size_t j = 0; j < SHAPES; ++j) {
      Row row = mat_row(t, i*2 + j); //0, 1, 2, 3, 4, 5..20
      Row in = row_slice(row, 0, input_size);
      Row out = row_slice(row, input_size, output_size);
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
  region_rewind(r, s);
  return t;
}

int main (void)
{
  srand(time(0));

  //256 MB big arena allocator
  Region temp = region_alloc_alloc(256*1024*1024);
  Region main = region_alloc_alloc(256*1024*1024);

  // training data matrix
  NN nn = nn_alloc(NULL, arch, ARRAY_LEN(arch));
  nn_rand(nn, -1, 1);
  Mat t = generate_samples(&main, SAMPLES_PER_SHAPE);

  Gym_Plot plot = {0};
  Batch batch = {0};

  int factor = 80;
  InitWindow(16*factor, 9*factor, "Shape");
  SetTargetFPS(60);

  while(!WindowShouldClose()) {
    if (IsKeyPressed(KEY_SPACE)) {
      paused = !paused;
    }
    for (size_t i = 0; i < batches_per_frame && !paused; ++i) {
      batch_process(&temp, &batch, batch_size, nn, t, rate);
      if (batch.finished) {
        da_append(&plot, batch.cost);
        mat_shuffle_rows(t);
      }
    }
    BeginDrawing();
    ClearBackground(GYM_BACKGROUND);
    Gym_Rect root = {0};
    root.w = GetRenderWidth();
    root.h = GetRenderHeight();
    gym_plot(plot, root);
    EndDrawing();
  }
  CloseWindow();

  return 0;
}
