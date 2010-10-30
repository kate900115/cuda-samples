#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <cuda.h>
#include <cuda_runtime_api.h>

#include "png_helper.h"

extern void convert_to_hsv_wrapper(uchar4 *rgb, float4 *hsv, int width, int height);
extern void convert_to_rgb_wrapper(float4 *hsv, uchar4 *rgb, int width, int height);

int main (int argc, char* argv[]) {
	uchar4 *host_in, *host_out, *host_golden;
	
	uchar4 *rgb_pixels;
	float4 *hsv_pixels;
	
	int *host_histogram;
	png_t *info;
	
	if(argc < 2) {
		printf("Must include file name to process. `%s <file_name>`\n", argv[0]);
		return -1;
	}
	
	if(read_png(argv[1], &info, &host_in) == PNG_FAILURE) {
		printf("Error reading file (%s).\n", argv[1]);
		return -1;
	}
		
	size_t number_of_bytes_rgb = sizeof(uchar4) * info->width * info->height;
	size_t number_of_bytes_hsv = sizeof(float4) * info->width * info->height;
	
	host_out = malloc(number_of_bytes_rgb);
	host_golden = malloc(number_of_bytes_rgb);
	
	cudaMalloc((void **) &rgb_pixels, number_of_bytes_rgb);
	assert(cudaGetLastError() == cudaSuccess);
	cudaMalloc((void **) &hsv_pixels, number_of_bytes_hsv);	
	assert(cudaGetLastError() == cudaSuccess);
	cudaMemcpy(rgb_pixels, host_in, number_of_bytes_rgb, cudaMemcpyHostToDevice);
	assert(cudaGetLastError() == cudaSuccess);
	
	convert_to_hsv_wrapper(rgb_pixels, hsv_pixels, info->width, info->height);
	
	convert_to_rgb_wrapper(hsv_pixels, rgb_pixels, info->width, info->height);
	
	cudaMemcpy(host_out, rgb_pixels, number_of_bytes_rgb, cudaMemcpyDeviceToHost);
	assert(cudaGetLastError() == cudaSuccess);
	
	char *output_file;	
	if(argc > 3) { output_file = argv[2]; } 
	else { output_file = "data/out.png"; }
	
	if(write_png(output_file, info, host_out) == PNG_FAILURE) {
		printf("Error writing to file (%s)\n", "data/out.png");
	}
	
	free(host_golden);
	free(host_in);
	free(host_out);

	return 0;
}