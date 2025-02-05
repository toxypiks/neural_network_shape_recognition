#include <stdio.h>

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
  //256 MB big arena allocator
  Region temp = region_alloc_alloc(256*1024*1024);

  Olivec_Canvas oc = {0};
  oc.pixels = region_alloc(&temp, WIDTH*HEIGHT*sizeof(*oc.pixels));
  oc.width = WIDTH;
  oc.height = HEIGHT;
  oc.stride = WIDTH;

  olivec_fill(oc, 0xFF000000);
  olivec_circle(oc, WIDTH/2, HEIGHT/2, WIDTH/3, 0xFFFFFFFF);

  const char *file_path = "output.png";
  if(!stbi_write_png(file_path, WIDTH, HEIGHT, 4, oc.pixels, oc.stride*sizeof(*oc.pixels))) {
    fprintf(stderr, "Could not generate %s\n", file_path);
    return 1;
  }
  printf("Generated %s\n", file_path);
  return 0;
}
