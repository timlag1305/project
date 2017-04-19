#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <mpi.h>

#define NUM_IMAGES	17

static cv::Mat image;
static const int8_t g_x[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
static const int8_t g_y[] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
static const char *image_names[] = {
	"../standard_test_images/lena_gray_512.tif",
	"../standard_test_images/jetplane.tif",
	"../standard_test_images/lena_color_512.tif",
	"../standard_test_images/woman_blonde.tif",
	"../standard_test_images/lena_gray_256.tif",
	"../standard_test_images/peppers_gray.tif",
	"../standard_test_images/pirate.tif",
	"../standard_test_images/mandril_color.tif",
	"../standard_test_images/woman_darkhair.tif",
	"../standard_test_images/peppers_color.tif",
	"../standard_test_images/house.tif",
	"../standard_test_images/livingroom.tif",
	"../standard_test_images/lena_color_256.tif",
	"../standard_test_images/lake.tif",
	"../standard_test_images/cameraman.tif",
	"../standard_test_images/walkbridge.tif",
	"../standard_test_images/mandril_gray.tif"
};

static double timer() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return ((double) (tp.tv_sec) + 1e-6 * tp.tv_usec);

}

inline uint8_t apply_filt(int x, int y) {
	int16_t temp_x = 0;
	int16_t temp_y = 0;
	int16_t magnitude;

	temp_x = g_x[0] * image.data[(y - 1) * image.cols + x - 1] + g_x[1] *
		image.data[(y - 1) * image.cols + x] + g_x[2] * image.data[(y - 1) *
		image.cols + x + 1] + g_x[3] * image.data[y * image.cols + x - 1] +
		g_x[4] * image.data[y * image.cols + x] + g_x[5] * image.data[y *
		image.cols + x + 1] + g_x[6] * image.data[(y + 1) * image.cols + x - 1]
		+ g_x[7] * image.data[(y + 1) * image.cols + x] + g_x[8] *
		image.data[(y + 1) * image.cols + x + 1];
	temp_y = g_y[0] * image.data[(y - 1) * image.cols + x - 1] + g_y[1] *
		image.data[(y - 1) * image.cols + x] + g_y[2] * image.data[(y - 1) *
		image.cols + x + 1] + g_y[3] * image.data[y * image.cols + x - 1] +
		g_y[4] * image.data[y * image.cols + x] + g_y[5] * image.data[y *
		image.cols + x + 1] + g_y[6] * image.data[(y + 1) * image.cols + x - 1]
		+ g_y[7] * image.data[(y + 1) * image.cols + x] + g_y[8] *
		image.data[(y + 1) * image.cols + x + 1];

	// Normalize the gradient to 8 bits
	temp_x = abs(temp_x);
	temp_y = abs(temp_y);
	temp_x = temp_x > 255 ? 255 : temp_x;
	temp_y = temp_y > 255 ? 255 : temp_y;
	magnitude = ceil(temp_x * 0.5 + temp_y * 0.5);
	return magnitude;
}

int main(int argc, char **argv) {
	double total_time = 0;
    int rank, num_tasks;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Status stat;

	for (int i = 0; i < NUM_IMAGES; i++) {
		image = cv::imread(image_names[i], cv::IMREAD_GRAYSCALE);

		if (!image.data) {
			printf("No image data \n");
			return -1;
		}


		cv::Mat padded;
		cv::Mat output = cv::Mat(image.rows, image.cols, CV_8U);
		int padding = 1;
		copyMakeBorder(image, image, padding, padding, padding, padding, cv::BORDER_CONSTANT, cv::BORDER_DEFAULT);
		double elt = timer();

		for (int y = (image.rows - 2) * (float) rank / num_tasks + 1; y < (image.rows - 2) * (float) (rank + 1) / num_tasks + 1; y++) {
			for (int x = 1; x < image.cols - 1; x++) {
				output.data[(y - 1) * output.cols + x - 1] = apply_filt(x, y);
			}
		}

		elt = timer() - elt;
		total_time += elt;
	}

	fprintf(stderr, "Time taken: %3.3lf s.\n", total_time);
	MPI_Finalize();

	return 0;
}
