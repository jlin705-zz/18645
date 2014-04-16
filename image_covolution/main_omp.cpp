#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define NUM_THREADS 8
#define maskWidth 5
#define maskRadius maskWidth/2

typedef struct{
    int w;
    int h;
    unsigned char * img_r;
    unsigned char * img_g;
    unsigned char * img_b;
} PPM_IMG;


PPM_IMG read_ppm( char * path);
void write_ppm(PPM_IMG img, char * path);
void read_csv(char* path, float mask[25]);
void write_ppm(PPM_IMG img, char * path);

float clamp(double x, double start, double end){
    double max;
    if (x < start) {
        max = start;
    }
    else max = x;
    if (max < end) {
        return max;
    }
    else return end;
}


void convolution (float mask[25], PPM_IMG image, PPM_IMG * output){
    int height = image.h;
    int width = image.w;
    printf("%d,%d;\n", width,height);
    int imagePixel;
    float maskValue, accum_R, accum_G, accum_B;
//	#pragma unroll(3);
//#pragma omp parallel for num_threads(NUM_THREADS) private (imagePixel, maskValue, accum_R, accum_G, accum_B)
        for (int i = 0; i < height; i++){
		for (int j = 0; j < width; j++){
//#pragma omp parallel for
//#pragma omp parallel for 
                        accum_R = 0;
                        accum_G =0;
                        accum_B = 0;

				for (int y=0-maskRadius; y <= maskRadius; y++) {
                  for (int x=0-maskRadius; x <= maskRadius; x++) {
//#pragma omp single {
                        int xOffset = j + x;
                        int yOffset = i + y;
//}
                        if (xOffset >= 0 && xOffset < width &&
                            yOffset >= 0 && yOffset < height) {
                          
			 imagePixel = image.img_r[yOffset * width + xOffset];
                         maskValue = mask[(y+maskRadius)*maskWidth+x+maskRadius];
                         accum_R += imagePixel * maskValue; 
                         
                         imagePixel = image.img_g[yOffset * width + xOffset];
                         maskValue = mask[(y+maskRadius)*maskWidth+x+maskRadius];
                         accum_G += imagePixel*maskValue;

                         imagePixel = image.img_b[yOffset * width + xOffset];
                         maskValue = mask[(y+maskRadius)*maskWidth+x+maskRadius];
                         accum_B += imagePixel*maskValue;

          //   }
                        }		
                    }
                }
                output->img_r[i*width+j] = accum_R;
                output->img_g[i*width+j] = accum_G;
                output->img_b[i*width+j] = accum_B;

               // printf("ouput from thread %d\n", omp_get_thread_num());
            
		}
	}
}

int main (int argc, char * argv[]){
    char * path = "/afs/andrew.cmu.edu/usr24/jialianl/image/input0.ppm";
    char * path_csv = "/afs/andrew.cmu.edu/usr24/jialianl/image/input1.csv";
    char * path_out = "/afs/andrew.cmu.edu/usr24/jialianl/image/outTestRGB.ppm";
    clock_t begin, end;
    double time_spent;
    
    //begin = clock();
    
    PPM_IMG img = read_ppm (path);
    PPM_IMG output;
    output.h = img.h;
    output.w = img.w;
    output.img_r = (unsigned char *)malloc(output.w * output.h * sizeof(unsigned char));
    output.img_g = (unsigned char *)malloc(output.w * output.h * sizeof(unsigned char));
    output.img_b = (unsigned char *)malloc(output.w * output.h * sizeof(unsigned char));
    float mask[25];
    read_csv(path_csv, mask);
    begin = clock();
    convolution(mask, img, &output);
    end = clock();
    write_ppm(output, path_out);
    free (output.img_r);
    free (output.img_g);
    free (output.img_b);
   // end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("execution time: %lf.\n",time_spent);
    return 0;
    
}

void read_csv( char* path, float mask[25]){
        /*
         * Open the file.
         */
        FILE *file = fopen(path, "r");
        if ( file )
        {
//            float mask[5][5];
            size_t i;
            char buffer[1024];
            /*
             * Read each line from the file.
             */
            for ( i = 0; fgets(buffer, sizeof buffer, file); ++i )
            {
                /*
                 * Parse the comma-separated values from each line into 'mask'.
                 */
                sscanf(buffer, "%f,%f,%f,%f,%f", &mask[i*5+0],&mask[i*5+1],&mask[i*5+2],&mask[i*5+3],&mask[i*5+4]);
            }
            fclose(file);
        }
        else /* fopen() returned NULL */
        {
            perror(path);
        }
        return;
}

PPM_IMG read_ppm( char * path){
    FILE * in_file;
    char sbuf[256];
    char * ibuf;
    int i, c;
    
    PPM_IMG result;
    int v_max;
    in_file = fopen(path, "r");
    if (in_file == NULL){
        printf("Input file not found!\n");
        exit(1);
    }
    /*Skip the magic number*/
    fscanf(in_file, "%s", sbuf);
    printf("%s", sbuf);
    //fgets(sbuf, sizeof sbuf, in_file);
    c = fgetc(in_file);
    c = fgetc(in_file);
    while (c == '#') {
        while (fgetc(in_file) != '\n') ;
        c = fgetc(in_file);

    }
    
    ungetc(c, in_file);
    
    fscanf(in_file, "%d",&result.w);
    fscanf(in_file, "%d",&result.h);
    fscanf(in_file, "%d\n",&v_max);
    printf("Image size: %d x %d\n", result.w, result.h);
    
    result.img_r = (unsigned char *)malloc(result.w * result.h * sizeof(unsigned char));
    result.img_g = (unsigned char *)malloc(result.w * result.h * sizeof(unsigned char));
    result.img_b = (unsigned char *)malloc(result.w * result.h * sizeof(unsigned char));
    ibuf = (char *)malloc(3 * result.w * result.h * sizeof(char));

                    
    fread(ibuf,sizeof(unsigned char), 3 * result.w*result.h, in_file);

    for(i = 0; i < result.w*result.h; i ++){
        result.img_r[i] = ibuf[3*i + 0];
        result.img_g[i] = ibuf[3*i + 1];
        result.img_b[i] = ibuf[3*i + 2];
    }             
    fclose(in_file);
    free(ibuf);
    return result;
}

void write_ppm(PPM_IMG img, char * path){
  FILE * out_file;
  int i;
          
  char * obuf = (char *)malloc(3 * img.w * img.h * sizeof(char));

  for(i = 0; i < img.w*img.h; i ++){
    obuf[3*i + 0] = img.img_r[i];
    obuf[3*i + 1] = img.img_g[i];
    obuf[3*i + 2] = img.img_b[i];
                                            }
  out_file = fopen(path, "wb");
  fprintf(out_file, "P6\n");
  fprintf(out_file, "%d %d\n255\n",img.w, img.h);
  fwrite(obuf,sizeof(unsigned char), 3*img.w*img.h, out_file);
  fclose(out_file);
  free(obuf);
  
}
