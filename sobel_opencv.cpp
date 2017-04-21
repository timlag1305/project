#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <string.h>
#include "sobel_upc.h"

#define	PADDING		1

static cv::Mat image;
static cv::Mat output;

void get_input_mat(uint8_t *out, const char *src) {
	image = cv::imread(src, cv::IMREAD_GRAYSCALE);
	copyMakeBorder(image, image, PADDING, PADDING, PADDING, PADDING, cv::BORDER_CONSTANT, cv::BORDER_DEFAULT);
	memcpy((void *) out, (const void *) image.data, image.cols * image.rows * sizeof(*out));
	image.release();
}

// We don't need output, so just free the memory
void set_output_mat(uint8_t *data) {
	output = cv::Mat(image.rows, image.cols, CV_8U);
	memcpy((void *) output.data, (const void *) data, output.cols * output.rows * sizeof(*data));
	output.release();
}
