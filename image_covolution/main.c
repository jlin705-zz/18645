#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


#define maskWidth 5
#define maskRadius maskWidth/2
#define CHANNELS 3

struct Arg
{
    int argc;
    char ** argv;
};

typedef struct{
    int w;
    int h;
    unsigned char * data;
} PPM_IMG;


PPM_IMG read_ppm(const char * path);
void write_ppm(PPM_IMG img, const char * path);
void read_csv(const char* path, float mask[25]);
void write_ppm(PPM_IMG img, const char * path);

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


void convolution (int channels, int width, int height, float mask[25], PPM_IMG image, PPM_IMG * output){
    printf("%d,%d;\n", width,height);
    int imagePixel;
    float maskValue;
	int i, j, k, x, y, xOffset, yOffset;
	for (i = 0; i < height; i++){
		for (j = 0; j < width; j++){
			for (k = 0; k < channels; k++){
				float accum = 0;
				for (y=0-maskRadius; y <= maskRadius; y++)
                {
                    for (x=0-maskRadius; x <= maskRadius; x++) {
                        xOffset = j + x;
                        yOffset = i + y;
                        if (xOffset >= 0 && xOffset < width &&
                            yOffset >= 0 && yOffset < height) {
                            imagePixel = image.data[(yOffset * width + xOffset) * channels + k];
                            maskValue = mask[(y+maskRadius)*maskWidth+x+maskRadius];
                            accum += imagePixel*maskValue;
                        }
                    }
                }
                output->data[(i*width+j)*channels+k] = accum;
            }
            
		}
	}
}

int main (int argc, char * argv[]){
    char * path = "/Users/neo_cupid/Desktop/18645/final_proj/data-2/1/input0.ppm";
    char * path_csv = "/Users/neo_cupid/Desktop/18645/final_proj/data-2/1/input1.csv";
    char * path_out = "/Users/neo_cupid/Desktop/18645/final_proj/data-2/1/outTest.ppm";
    clock_t begin, end;
    double time_spent;
    
    begin = clock();
    
    PPM_IMG img = read_ppm (path);
    PPM_IMG output;
    output.h = img.h;
    output.w = img.w;
    output.data = (unsigned char *)malloc(3 * img.w * img.h * sizeof(unsigned char));
    float mask[25];
    read_csv(path_csv, mask);
    convolution(CHANNELS, img.w, img.h, mask, img, &output);
    write_ppm(output, path_out);
    free (output.data);
    free(img.data);
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("execution time: %lf.\n",time_spent);
    return 0;
    
}

void read_csv(const char* path, float mask[25]){
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

PPM_IMG read_ppm(const char * path){
    FILE * in_file;
    char sbuf[256];
    int c;
    
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
    
    result.data         = (unsigned char *)malloc(3 * result.w * result.h * sizeof(char));
    
    fread(result.data,sizeof(unsigned char), 3 * result.w*result.h, in_file);
    
    fclose(in_file);
    
    return result;
}
// Write back to generate the image
void write_ppm(PPM_IMG img, const char * path){
    FILE * out_file;
    int i;
    
    char * obuf = (char *)malloc(3 * img.w * img.h * sizeof(char));
    
    for(i = 0; i < 3*img.w*img.h; i ++){
        obuf[i] = img.data[i];
    }
    out_file = fopen(path, "wb");
    fprintf(out_file, "P6\n");
    fprintf(out_file, "%d %d\n255\n",img.w, img.h);
    fwrite(obuf,sizeof(unsigned char), 3*img.w*img.h, out_file);
    fclose(out_file);
    free(obuf);
}