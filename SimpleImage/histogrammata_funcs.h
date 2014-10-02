#ifndef HISTOGRAMMATA_FUNCS

typedef struct pixel_struct{
    unsigned pxl_value;
    float mo;
    unsigned row;
    unsigned col;
} pixel_struct;

int histogrammata(void);

void open_png(unsigned char* image, unsigned rows, unsigned cols, char *image_name);

unsigned equal_size(unsigned a, unsigned b);

void print_RGB_values(unsigned rows, unsigned cols, int input_channels, unsigned char *image);

void print_pixel_values(unsigned cols, unsigned rows, int **pixel_values);

int **allocate_2d_array(unsigned rows, unsigned cols, char *array_name);

void pixel_values_2d_array(unsigned rows, unsigned cols, int input_channels, unsigned char *image, int **pixel_values, int color);

void convert_RGB_to_grayscale(unsigned rows, unsigned cols, int **pixel_valuesR, int **pixel_valuesG, int **pixel_valuesB);

void initialize_1d_array(int size, int* array_name);

void initialize_2d_array(unsigned rows, unsigned cols, int **disparity);

void count_pixel_values(int rows, int cols, int **pixel_values, int *count_values);

pixel_struct **allocate_struct_array(unsigned rows, unsigned cols, char *array_name);

void make_pixel_struct_array(int **pixel_values, pixel_struct *pixel_struct_array, unsigned rows, unsigned cols);

void histogram_equalization(pixel_struct *pixel_struct_array, unsigned *rows, unsigned *cols);

void histogram_matching(pixel_struct* refpixel_struct_array, pixel_struct* imitpixel_struct_array, unsigned *rows, unsigned *cols);

int compfunc(const void *a, const void *b);

void exact_matching(unsigned char* imit_image, unsigned char* ref_image, int **imit_pixel_values, int **ref_pixel_values, unsigned ref_rows, unsigned ref_cols, unsigned imit_rows, unsigned imit_cols, unsigned *rows, unsigned *cols, int color);

void change_RGB_values(unsigned rows, unsigned cols, int input_channels, unsigned char *image, int **pixel_valuesR, int **pixel_valuesG, int **pixel_valuesB);

#endif // HISTOGRAMMATA_FUNCS
