#include <stdint.h>
#define f (1 << 14)

/* Arithmetic utils for fixed-point real numbers(using 17.14 format). 
   x: Fixed point number.
   n: Integer.
   f: 2**q in p.q fixed-point format. Here, f = 2**14.
*/

enum round_types
{
	TOWARD_ZERO,
	NEAREST
};

/* Convert n to fixed point x. */
int32_t conv_int_to_fp(int32_t n);

/* Convert x to integer. */
int32_t conv_fp_to_int(int32_t x, enum round_types r_type);

/* Add x and y. */
int32_t add_fp_fp(int32_t x, int32_t y);

/* Subtract y from x. */
int32_t sub_fp_fp(int32_t x, int32_t y);

/* Add x and n. */
int32_t add_fp_int(int32_t x, int32_t n);

/* Subtract n from x. */
int32_t sub_fp_int(int32_t x, int32_t n);

/* Multiply x by y. */
int32_t mul_fp_fp(int32_t x, int32_t y);

/* Multiply x by n. */
int32_t mul_fp_int(int32_t x, int32_t n);

/* Divide x by y. */
int32_t div_fp_fp(int32_t x, int32_t y);

/* Divide x by n. */
int32_t div_fp_int(int32_t x, int32_t n);