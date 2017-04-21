/* Variables/Routines exported to C/C++ routines */
#ifndef MY_UPC_HEADER_H
#define MY_UPC_HEADER_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
	extern void get_input_mat(uint8_t *dest, const char *src);
	extern void set_output_mat(uint8_t *data);
#ifdef __cplusplus
} /* end "extern" */
#endif
#endif
