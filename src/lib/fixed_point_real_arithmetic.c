#include "fixed_point_real_arithmetic.h"

/* Convert n to fixed point x. */
int32_t
conv_int_to_fp(int32_t n)
{
	return n * f;
}

/* Convert x to integer. */
int32_t
conv_fp_to_int(int32_t x, enum round_types r_type)
{
	if (r_type == TOWARD_ZERO)
		return x / f;
	return x >= 0 ? (x + f / 2) / f : (x - f / 2) / f;
}

/* Add x and y. */
int32_t
add_fp_fp(int32_t x, int32_t y)
{
	return x + y;
}

/* Subtract y from x. */
int32_t
sub_fp_fp(int32_t x, int32_t y)
{
	return x - y;
}

/* Add x and n. */
int32_t
add_fp_int(int32_t x, int32_t n)
{
	return x + n * f;
}

/* Subtract n from x. */
int32_t
sub_fp_int(int32_t x, int32_t n)
{
	return x - n * f;
}

/* Multiply x by y. */
int32_t
mul_fp_fp(int32_t x, int32_t y)
{
	return ((int64_t)x) * y / f;
}

/* Multiply x by n. */
int32_t
mul_fp_int(int32_t x, int32_t n)
{
	return x * n;
}

/* Divide x by y. */
int32_t
div_fp_fp(int32_t x, int32_t y)
{
	return ((int64_t)x) * f / y; 
}

/* Divide x by n. */
int32_t
div_fp_int(int32_t x, int32_t n)
{
	return x / n;
}