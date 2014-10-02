#include <stdio.h>
#include <stdlib.h>
#include "lodepng.h"
#include "histogrammata_funcs.h"

int histogrammata(void) {

	unsigned ref_rows, ref_cols, imit_rows, imit_cols, rows, cols, error, select_func;
	char *ref_name, *imit_name;
	unsigned char *ref_image, *imit_image;
	int **ref_pixel_valuesR, **imit_pixel_valuesR, **ref_transformR, **imit_transformR;
	int **ref_pixel_valuesG, **imit_pixel_valuesG, **ref_transformG, **imit_transformG;
	int **ref_pixel_valuesB, **imit_pixel_valuesB, **ref_transformB, **imit_transformB;

    /*constant definition*/
    /*mporoun na dhlwthoun parallhla -> den eimai sigourh oti ginetai*/
	ref_name = "C:\\Users\\p_mel_000\\Dropbox\\WORKSPACE\\histogrammata\\diplo000000-L.png"; //"sunset1.png";
	ref_rows = 0;
	ref_cols = 0;

	imit_name ="C:\\Users\\p_mel_000\\Dropbox\\WORKSPACE\\histogrammata\\diplo000000-R.png"; //"sunset2.png";
	imit_rows = 0;
	imit_cols = 0;

	rows = 0;
	cols = 0;

	select_func = 1; //0 -> equalization 1 -> matching

	/*open image + handle error*/
	/*mporoun na anoiksoun parallhla*/
	error = lodepng_decode32_file(&ref_image, &ref_cols, &ref_rows, ref_name);
	if(error) {
		printf("error %u: %s\n", error, lodepng_error_text(error));}
	else{
		printf("Image successfully open!\n");}

    error = lodepng_decode32_file(&imit_image, &imit_cols, &imit_rows, imit_name);
	if(error) {
		printf("error %u: %s\n", error, lodepng_error_text(error));}
	else{
		printf("Image successfully open!\n");}

    /*test*/
    print_RGB_values(ref_rows, ref_cols, 4, ref_image);

    /*memory allocation*/
    /*mporoun na arxikopoiithoyn parallhla*/
    ref_pixel_valuesR = allocate_2d_array(ref_rows, ref_cols, "ref_pixel_values");
    ref_transformR = allocate_2d_array(ref_rows, ref_cols, "ref_transform");
    ref_pixel_valuesG = allocate_2d_array(ref_rows, ref_cols, "ref_pixel_values");
    ref_transformG = allocate_2d_array(ref_rows, ref_cols, "ref_transform");
    ref_pixel_valuesB = allocate_2d_array(ref_rows, ref_cols, "ref_pixel_values");
    ref_transformB = allocate_2d_array(ref_rows, ref_cols, "ref_transform");

    imit_pixel_valuesR = allocate_2d_array(imit_rows, imit_cols, "imit_pixel_values");
    imit_transformR = allocate_2d_array(imit_rows, imit_cols, "imit_transform");
    imit_pixel_valuesG = allocate_2d_array(imit_rows, imit_cols, "imit_pixel_values");
    imit_transformG = allocate_2d_array(imit_rows, imit_cols, "imit_transform");
    imit_pixel_valuesB = allocate_2d_array(imit_rows, imit_cols, "imit_pixel_values");
    imit_transformB = allocate_2d_array(imit_rows, imit_cols, "imit_transform");


    exact_matching(imit_image, ref_image, imit_pixel_valuesR, ref_pixel_valuesR, ref_rows, ref_cols, imit_rows, imit_cols, &rows, &cols, 0);
    exact_matching(imit_image, ref_image, imit_pixel_valuesG, ref_pixel_valuesG, ref_rows, ref_cols, imit_rows, imit_cols, &rows, &cols, 1);
    exact_matching(imit_image, ref_image, imit_pixel_valuesB, ref_pixel_valuesB, ref_rows, ref_cols, imit_rows, imit_cols, &rows, &cols, 2);

    change_RGB_values(ref_rows, ref_cols, 4, ref_image, ref_pixel_valuesR, ref_pixel_valuesG, ref_pixel_valuesB);
    change_RGB_values(imit_rows, imit_cols, 4, imit_image, imit_pixel_valuesR, imit_pixel_valuesG, imit_pixel_valuesB);

    //Encode the image + error handling
    //mporoun na kwdikopoihthoun parallhla
    error = lodepng_encode32_file("histogrammata_out_1.png", ref_image, ref_cols, ref_rows);
    if(error){
        printf("error %u: %s\n", error, lodepng_error_text(error));}
    else{
        printf("Image successfully saved!\n");}

    error = lodepng_encode32_file("histogrammata_out_2.png", imit_image, imit_cols, imit_rows);
    if(error){
        printf("error %u: %s\n", error, lodepng_error_text(error));}
    else{
        printf("Image successfully saved!\n");}




    free(ref_pixel_valuesR);
    free(ref_transformR);
    free(ref_pixel_valuesG);
    free(ref_transformG);
    free(ref_pixel_valuesB);
    free(ref_transformB);
	free(ref_image);

	free(imit_pixel_valuesR);
    free(imit_transformR);
    free(imit_pixel_valuesG);
    free(imit_transformG);
    free(imit_pixel_valuesB);
    free(imit_transformB);
	free(imit_image);

	return 0;
}
