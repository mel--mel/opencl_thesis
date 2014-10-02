#include <stdio.h>
#include <stdlib.h>
#include "lodepng.h"

typedef struct pixel_struct{
    int pxl_value;
    float mo;
    int transform;
    unsigned row;
    unsigned col;
} pixel_struct;


void open_png(unsigned char* image, unsigned rows, unsigned cols, char *image_name){
	unsigned error;

	/*open image + handle error*/
	error = lodepng_decode32_file(&image, &cols, &rows, image_name);
	if(error) {
		printf("error %u: %s\n", error, lodepng_error_text(error));}
	else{
		printf("Image successfully open!\n");}

	/*print width + height*/
	//printf("\nwidth = %u\n", width);
	//printf("height = %u\n\n\n", height);

}

unsigned equal_size(unsigned a, unsigned b){
    if (a < b){
        return a;}
    else{
        return b;}
}

void print_RGB_values(unsigned rows, unsigned cols, int input_channels, unsigned char *image){

	int i, j, pos;

	/*print width + height*/
	printf("\ncols = %u\n", cols);
	printf("rows = %u\n\n\n", rows);

    /*print rgb values*/
    printf("RGB values are:\n");

	for (i = 0; i < 3; i++) { //rows
        for (j = 0; j < 3; j++) { //columns
            for (pos = 0; pos < 3; pos++) {
                printf("%d ", image[i*cols*input_channels + j*input_channels + pos]);
            }
        printf("| ");
        }
        printf("\n");
    }
    printf("\n");
}

void print_pixel_values(unsigned cols, unsigned rows, int **pixel_values){
	int x, y;

	for (x=0; x < 3; x++) {//rows
			for (y=0; y < 3; y++) { //cols
				printf("|%d|  ", pixel_values[x][y]);
				}
			printf("\n");
			}
}

int **allocate_2d_array(unsigned rows, unsigned cols, char *array_name){
	int i;
	int **new_array;

	new_array = (int **)malloc(rows * sizeof(int*));
	for (i = 0; i < rows; i++){
		new_array[i] = (int *)malloc((cols) * sizeof(int));
		}
    if ((*new_array == NULL) || (new_array == NULL)) {
		puts("malloc error!!\n\n");}
	else {
		printf("%s successfully allocated!! \n\n", array_name);}

	return new_array;
}

/*mporei na ftiaxtei parallhla gia kathe pixel*/
void pixel_values_2d_array(unsigned rows, unsigned cols, int input_channels, unsigned char *image, int **pixel_values, int color){
	int i, j;

	for (i=0; i < rows; i++) {
			for (j = 0; j < cols; j++) {
                pixel_values[i][j] = (int)image[i*cols*input_channels + j*input_channels + color];
				}
		    }
}

void convert_RGB_to_grayscale(unsigned rows, unsigned cols, int **pixel_valuesR, int **pixel_valuesG, int **pixel_valuesB){
	int i, j;

	for (i = 0; i < rows; i++){
		for (j = 0; j < cols; j++){
			pixel_valuesR[i][j] = 0.2989*pixel_valuesR[i][j] + 0.587*pixel_valuesG[i][j] + 0.114*pixel_valuesB[i][j];
			pixel_valuesG[i][j] = pixel_valuesR[i][j];
			pixel_valuesB[i][j] = pixel_valuesR[i][j];
		}
	}

}

/*mporei na ftiaxtei parallhla gia kathe i*/
void initialize_1d_array(int size, int* array_name){
	int i;

	for (i = 0; i < size; i++){
		array_name[i] = 0;
	}
}

void initialize_2d_array(unsigned rows, unsigned cols, int **disparity){

	int i, j;

	for (i = 0; i < rows; i++){
		for (j = 0; j < cols; j++){
			disparity[i][j] = 0;
		}
	}
}

void count_pixel_values(int rows, int cols, int **pixel_values, int *count_values){
	int x, y, temp;

	for (x = 0; x < rows; x++){
		for (y = 0; y < cols; y++){
			temp = pixel_values[x][y];
			count_values[temp]++;
		}
	}
}

pixel_struct **allocate_struct_array(unsigned rows, unsigned cols, char *array_name){
    int i;
	pixel_struct **new_array;

	new_array = (pixel_struct **)malloc(rows * sizeof(pixel_struct*));
	for (i = 0; i < rows; i++){
		new_array[i] = (pixel_struct *)malloc((cols) * sizeof(pixel_struct));
		}
    if ((*new_array == NULL) || (new_array == NULL)) {
		puts("malloc error!!\n\n");}
	else {
		printf("%s successfully allocated!! \n\n", array_name);}

	return new_array;
}

void make_pixel_struct_array(int **pixel_values, pixel_struct *pixel_struct_array, unsigned rows, unsigned cols){
    int i, j, k;

    k = 0;
    for (i = 0; i < rows; i++){
        for (j = 0; j < cols; j++){
            pixel_struct_array[k].pxl_value = pixel_values[i][j];
            if (i>0 && i<(rows-1) && j>0 && j<(cols-1)){
                pixel_struct_array[k].mo = (pixel_values[i-1][j] + pixel_values[i][j-1] + pixel_values[i][j+1] + pixel_values[i+1][j])/4;}
            else if (i==0){
                if  (j>0 && j<(cols-1)){
                    pixel_struct_array[k].mo = (pixel_values[i][j-1] + pixel_values[i][j+1] + pixel_values[i+1][j])/3.0;}
                else if (j==0){
                    pixel_struct_array[k].mo = (pixel_values[i][j+1] + pixel_values[i+1][j])/2.0;}
                else {
                    pixel_struct_array[k].mo = (pixel_values[i][j-1] + pixel_values[i+1][j])/2.0;}}
            else if (i==(rows-1)){
                if  (j>0 && j<(cols-1)){
                    pixel_struct_array[k].mo = (pixel_values[i][j-1] + pixel_values[i][j+1] + pixel_values[i-1][j])/3.0;}
                else if (j==0){
                    pixel_struct_array[k].mo = (pixel_values[i][j+1] + pixel_values[i-1][j])/2.0;}
                else {
                    pixel_struct_array[k].mo = (pixel_values[i][j-1] + pixel_values[i-1][j])/2.0;}}
            else if (j==0){
                pixel_struct_array[k].mo = (pixel_values[i-1][j] + pixel_values[i][j+1] + pixel_values[i+1][j])/4.0;}
             else if (j==(cols-1)){
                pixel_struct_array[k].mo = (pixel_values[i-1][j] + pixel_values[i][j-1] + pixel_values[i+1][j])/4.0;}
            pixel_struct_array[k].transform = 0;
            pixel_struct_array[k].row = i;
            pixel_struct_array[k].col = j;
            k++;
        }
    }

}

int compfunc(const void *pa, const void *pb){

    pixel_struct a, b;

    a = *(const pixel_struct*)pa;
    b = *(const pixel_struct*)pb;

    if (a.pxl_value == b.pxl_value && a.mo == b.mo){
        return 0;
    } else if ((a.pxl_value < b.pxl_value) || (a.pxl_value == b.pxl_value && a.mo < b.mo) ){
        return -1;
    } else {
        return 1;
    }
}

void histogram_equalization(pixel_struct *pixel_struct_array, unsigned *rows, unsigned *cols){

    int i, j, value, count, acc255;
    float mo;
    double acc;

    //equalize
    acc = 0;
    count = 1;
    value = pixel_struct_array[0].pxl_value;
    mo = pixel_struct_array[0].mo;
    for (i = 1; i < ((*rows) * (*cols)); i++){
        if ((pixel_struct_array[i].pxl_value == value) && (pixel_struct_array[i].mo == mo)){
            //count how many values are the same
            count++;
        }
        else {
            //find equalized value
            acc += (double)count/((double)((*rows) * (*cols)));
            acc255 = (int)(acc*255);

            //check
            //if ((i%200) == 0)printf("i = %d \n",i);

            //for every same pixel
            //save transformation + change pixel to equalized value
            for (j = (i - count); j < i; j++){
                pixel_struct_array[j].transform = acc255 - pixel_struct_array[j].pxl_value;
                if (acc255 < 255){
                    pixel_struct_array[j].pxl_value = acc255;
                }else{
                    pixel_struct_array[j].pxl_value = 255;
                }
            }

            //start counting again
            count = 1;
            value = pixel_struct_array[i].pxl_value;
            mo = pixel_struct_array[i].mo;
        }

    }
}

void histogram_matching(pixel_struct* refpixel_struct_array, pixel_struct* imitpixel_struct_array, unsigned *rows, unsigned *cols){

    int i;

    for (i = 0; i < ((*rows)*(*cols)); i++){
        //match imit to reference
        if ((imitpixel_struct_array[i].pxl_value - refpixel_struct_array[i].transform > 0)&& (imitpixel_struct_array[i].pxl_value - refpixel_struct_array[i].transform <256)){
            imitpixel_struct_array[i].pxl_value -= refpixel_struct_array[i].transform;
        }else if (imitpixel_struct_array[i].pxl_value - refpixel_struct_array[i].transform <= 0){
            imitpixel_struct_array[i].pxl_value = 1;
        }else{
            imitpixel_struct_array[i].pxl_value = 255;
        }
        //change reference back to normal
        refpixel_struct_array[i].pxl_value -= refpixel_struct_array[i].transform;
    }
}

void exact_matching(unsigned char* imit_image, unsigned char* ref_image, int **imit_pixel_values, int **ref_pixel_values, unsigned ref_rows, unsigned ref_cols, unsigned imit_rows, unsigned imit_cols, unsigned *rows, unsigned *cols, int color){
    pixel_struct *imitpixel_struct_array, *refpixel_struct_array;
    int i;

    //make rows-columns equal
    *rows = equal_size(ref_rows, imit_rows);
    *cols = equal_size(ref_cols, imit_cols);

    pixel_values_2d_array(*rows, *cols, 4, ref_image, ref_pixel_values, color);
    pixel_values_2d_array(*rows, *cols, 4, imit_image, imit_pixel_values, color);

    //memory allocation
    refpixel_struct_array = (pixel_struct *)malloc((*rows) * (*cols) * sizeof(pixel_struct));
    imitpixel_struct_array = (pixel_struct *)malloc((*rows) * (*cols) * sizeof(pixel_struct));

    //fill array - unsorted
    make_pixel_struct_array(ref_pixel_values, refpixel_struct_array, *rows, *cols);
    make_pixel_struct_array(imit_pixel_values, imitpixel_struct_array, *rows, *cols);

    //sort pixel struct array
    qsort(refpixel_struct_array, ((*rows)*(*cols)), sizeof(pixel_struct), compfunc);
    qsort(imitpixel_struct_array, ((*rows)*(*cols)), sizeof(pixel_struct), compfunc);

    //check as usual
    /*for (i = 500; i < 800; i++){
        printf("%d %f %d\n", refpixel_struct_array[i].pxl_value, refpixel_struct_array[i].mo, refpixel_struct_array[i].row);
    }*/

    //equalize
    histogram_equalization(refpixel_struct_array, rows, cols);
    histogram_equalization(imitpixel_struct_array, rows, cols);

    //match
    histogram_matching(refpixel_struct_array, imitpixel_struct_array, rows, cols);

    //change 2d array --> na to kanw function
    for (i = 0; i < ((*rows)*(*cols)); i++){
        ref_pixel_values[refpixel_struct_array[i].row][refpixel_struct_array[i].col] = refpixel_struct_array[i].pxl_value;
        imit_pixel_values[imitpixel_struct_array[i].row][imitpixel_struct_array[i].col] = imitpixel_struct_array[i].pxl_value;
    }

}

/*mporei na ginei parallhla gia kathe kanali*/
void change_RGB_values(unsigned rows, unsigned cols, int input_channels, unsigned char *image, int **pixel_valuesR, int **pixel_valuesG, int **pixel_valuesB){
	int x, y;

	for (x = 0; x < rows; x++) {
		for (y=0; y < cols; y++) {
            image[x*cols*input_channels + y*input_channels] = pixel_valuesR[x][y];
            image[x*cols*input_channels + y*input_channels + 1] = pixel_valuesG[x][y];
            image[x*cols*input_channels + y*input_channels + 2] = pixel_valuesB[x][y];
			}
		}
}



