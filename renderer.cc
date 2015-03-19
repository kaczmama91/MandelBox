/*
This file is part of the Mandelbox program developed for the course
CS/SE  Distributed Computer Systems taught by N. Nedialkov in the
Winter of 2015 at McMaster University.

Copyright (C) 2015 T. Gwosdz and N. Nedialkov

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>

#include "color.h"
#include "mandelbox.h"
#include "camera.h"
#include "vector3d.h"
#include "3d.h"
#include "mpi.h"    

extern double getTime();
extern void   printProgress( double perc, double time );

extern void rayMarch (const RenderParams &render_params, const vec3 &from, const vec3  &to, pixelData &pix_data);
extern vec3 getColour(const pixelData &pixData, const RenderParams &render_params,
    const vec3 &from, const vec3  &direction);

void createRow(int j, int width, const CameraParams &camera_params, const RenderParams &renderer_params,vec3 &to, vec3 &from,  double *farPoint, pixelData &pix_data, unsigned char * image) {

//for each column pixel in the row
  for(int i = 0; i <width; i++)
  {
      vec3 color;
      if( renderer_params.super_sampling == 1 )
      {
          vec3 samples[9];
          int idx = 0;
          for(int ssj = -1; ssj < 2; ssj++){
            for(int ssi = -1; ssi< 2; ssi++){
              UnProject(i+ssi*0.5, j+ssj*0.5, camera_params, farPoint);

          // to = farPoint - camera_params.camPos
              to = SubtractDoubleDouble(farPoint,camera_params.camPos);
              to.Normalize();

          //render the pixel
              rayMarch(renderer_params, from, to, pix_data);

          //get the colour at this pixel
              samples[idx] = getColour(pix_data, renderer_params, from, to);
              idx++;
          }
      }
      color = (samples[0]*0.05 + samples[1]*0.1 + samples[2]*0.05 + 
       samples[3]*0.1  + samples[4]*0.4 + samples[5]*0.1  + 
       samples[6]*0.05 + samples[7]*0.1 + samples[8]*0.05);

  }
  else
  {
          // get point on the 'far' plane
          // since we render one frame only, we can use the more specialized method
      UnProject(i, j, camera_params, farPoint);

          // to = farPoint - camera_params.camPos
      to = SubtractDoubleDouble(farPoint,camera_params.camPos);
      to.Normalize();

          //render the pixel
      rayMarch(renderer_params, from, to, pix_data);

          //get the colour at this pixel
      color = getColour(pix_data, renderer_params, from, to);
  }

      //save colour into texture
  int k = (j * width + i)*3;
  image[k+2] = (unsigned char)(color.x * 255);
  image[k+1] = (unsigned char)(color.y * 255);
  image[k]   = (unsigned char)(color.z * 255);
}
}

int my_rank;            /* rank of process */
int p;                  /* number of processes */
int tag = 0;            /* tag for messages */
MPI_Status status;      /* status for receive */  


void renderFractal(int argc, char** argv, const CameraParams &camera_params, const RenderParams &renderer_params, 
    unsigned char* image)
{

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    printf("rendering fractal...\n");

    double farPoint[3];
    vec3 to, from;

    from.SetDoublePoint(camera_params.camPos);

    int height = renderer_params.height;
    int width  = renderer_params.width;

    pixelData pix_data;


    int block = height / p;
    int remainder = height % p;
    double time = getTime();

    printf("remainder: %d rank: %d\n", remainder, my_rank);

    int image_size = 3 * (renderer_params.width * renderer_params.height);

    MPI_Bcast(image, image_size, MPI_UNSIGNED_CHAR, 0, 
               MPI_COMM_WORLD);

    for(int j = 0; j < height; j++)
    {
        //printf("%d %d %d\n",  j, my_rank, block);
        //for each column pixel in the row
        if ( j >= ((my_rank)*block) && j <= ((my_rank+1)*block) ) {
            createRow(j, width, camera_params, renderer_params, to, from, farPoint, pix_data, image);
        }

        printProgress((j+1)/(double)height,getTime()-time);
    }
    printf("\n rendering done:\n");

    MPI_Finalize();

}
