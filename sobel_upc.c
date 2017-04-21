/* Variables/Routines exported to C/C++ routines */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <upc.h>
#include <upc_strict.h>
#include "sobel_upc.h"

#define NUM_IMAGES		17
#define INPUT_IMAGE_DIM	512
//#define CHUNKS_PER_THREAD (INPUT_IMAGE_DIM / THREADS)
#define PAD_IMAGE_DIM	(INPUT_IMAGE_DIM + 2)

//shared [PAD_IMAGE_DIM * (CHUNKS_PER_THREAD * THREADS + 2)] uint8_t input_pix_mat[PAD_IMAGE_DIM * (CHUNKS_PER_THREAD * THREADS + 2)];
//shared [INPUT_IMAGE_DIM  * CHUNKS_PER_THREAD * THREADS] uint8_t output_pix_mat[INPUT_IMAGE_DIM * CHUNKS_PER_THREAD * THREADS];
//uint8_t input_pix_mat[PAD_IMAGE_DIM * PAD_IMAGE_DIM];

static const int8_t g_x[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
static const int8_t g_y[] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
uint8_t *input_pix_mat;
uint8_t *output_pix_mat;
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

uint8_t apply_filt(int x, int y) {
	int16_t temp_x = 0;
	int16_t temp_y = 0;
	int16_t magnitude;

	temp_x = g_x[0] * input_pix_mat[(y - 1) * PAD_IMAGE_DIM + x - 1] + g_x[1] *
		input_pix_mat[(y - 1) * PAD_IMAGE_DIM + x] + g_x[2] * input_pix_mat[(y - 1) *
		PAD_IMAGE_DIM + x + 1] + g_x[3] * input_pix_mat[y * PAD_IMAGE_DIM + x - 1] +
		g_x[4] * input_pix_mat[y * PAD_IMAGE_DIM + x] + g_x[5] * input_pix_mat[y *
		PAD_IMAGE_DIM + x + 1] + g_x[6] * input_pix_mat[(y + 1) * PAD_IMAGE_DIM + x - 1]
		+ g_x[7] * input_pix_mat[(y + 1) * PAD_IMAGE_DIM + x] + g_x[8] *
		input_pix_mat[(y + 1) * PAD_IMAGE_DIM + x + 1];
	temp_y = g_y[0] * input_pix_mat[(y - 1) * PAD_IMAGE_DIM + x - 1] + g_y[1] *
		input_pix_mat[(y - 1) * PAD_IMAGE_DIM + x] + g_y[2] * input_pix_mat[(y - 1) *
		PAD_IMAGE_DIM + x + 1] + g_y[3] * input_pix_mat[y * PAD_IMAGE_DIM + x - 1] +
		g_y[4] * input_pix_mat[y * PAD_IMAGE_DIM + x] + g_y[5] * input_pix_mat[y *
		PAD_IMAGE_DIM + x + 1] + g_y[6] * input_pix_mat[(y + 1) * PAD_IMAGE_DIM + x - 1]
		+ g_y[7] * input_pix_mat[(y + 1) * PAD_IMAGE_DIM + x] + g_y[8] *
		input_pix_mat[(y + 1) * PAD_IMAGE_DIM + x + 1];

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

	input_pix_mat = calloc(PAD_IMAGE_DIM * PAD_IMAGE_DIM, sizeof(*input_pix_mat));
	output_pix_mat = calloc(INPUT_IMAGE_DIM * INPUT_IMAGE_DIM, sizeof(*output_pix_mat));

	for (int i = 0; i < NUM_IMAGES; i++) {
		get_input_mat(input_pix_mat, image_names[i]);

		if (input_pix_mat == NULL) {
			printf("No image data \n");
			return -1;
		}

		upc_barrier;

		double elt;
		if (MYTHREAD == 0) {
			elt = timer();
		}

		upc_forall(int y = 1; y < PAD_IMAGE_DIM - 1; y++; &input_pix_mat[y * PAD_IMAGE_DIM]) {
			for (int x = 1; x < PAD_IMAGE_DIM - 1; x++) {
				output_pix_mat[(y - 1) * INPUT_IMAGE_DIM + x - 1] = apply_filt(x, y);
			}
		}

		upc_barrier;
		if (MYTHREAD == 0) {
			set_output_mat(output_pix_mat);
			elt = timer() - elt;
			total_time += elt;
		}
	}

	if (MYTHREAD == 0) {
		fprintf(stderr, "Time taken: %3.3lf s.\n", total_time);
	}

	return 0;
}
