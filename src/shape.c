#include <stdio.h>
#include <time.h>

#define NN_IMPLEMENTATION
#include "nn.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define OLIVEC_IMPLEMENTATION
#include "olive.c"

#define WIDTH 28
#define HEIGHT WIDTH

int main (void)
{
  srand(time(0));
  //256 MB big arena allocator
  Region temp = region_alloc_alloc(256*1024*1024);

  Olivec_Canvas oc = {0};
  oc.pixels = region_alloc(&temp, WIDTH*HEIGHT*sizeof(*oc.pixels));
  oc.width = WIDTH;
  oc.height = HEIGHT;
  oc.stride = WIDTH;

  olivec_fill(oc, 0xFF000000);
  int x1, y1, x2, y2, w, h;
  do
  {
    x1 = rand()%WIDTH;
    y1 = rand()%HEIGHT;
    x2 = rand()%WIDTH;
    y2 = rand()%HEIGHT;
    if (x1 > x2) OLIVEC_SWAP(int, x1, x2);
    if (y1 > y2) OLIVEC_SWAP(int, y1, y2);
    w = x2 - x1;
    h = y2 - y1;
  }
  while(w < 4 || h < 4);
  olivec_frame(oc, x1, y1, w, h, 1, 0xFF0000FF);

  const char *file_path = "output.png";
  if(!stbi_write_png(file_path, WIDTH, HEIGHT, 4, oc.pixels, oc.stride*sizeof(*oc.pixels))) {
    fprintf(stderr, "Could not generate %s\n", file_path);
    return 1;
  }
  printf("Generated %s\n", file_path);
  return 0;
}
