#include "authenc_ac_gcm.h"

#include <string.h>

#define GCM_DIGS (AC_GCM_BLOCK_LEN / sizeof(dig_t))
#define DIGIT (sizeof(dig_t) * 8)
#define MASK(B)				(((dig_t)1 << (B)) - 1)

/**
 * Multiply two digit vectors in the GCM finite field.
 * @param[out] c	- the product.
 * @param[in] a		- the first operand.
 * @param[in] b		- the second operand.
 */
void ac_gcm_mul_low(dig_t *c, dig_t *a, dig_t *b);

static dig_t lshb_low(dig_t *c, dig_t *a, int bits) {
	size_t i;
	dig_t r, carry, mask, shift;

	/* Prepare the bit mask. */
	shift = DIGIT - bits;
	carry = 0;
	mask = MASK(bits);
	for (i = 0; i < GCM_DIGS; i++, a++, c++) {
		/* Get the needed least significant bits. */
		r = ((*a) >> shift) & mask;
		/* Shift left the operand. */
		*c = ((*a) << bits) | carry;
		/* Update the carry. */
		carry = r;
	}
	return carry;
}

static void gcm_lsh_low(dig_t *t)
{
	dig_t d;
	d = lshb_low(t, t, 1);
	t[GCM_DIGS - 1] ^= (d << (DIGIT - 1)) ^ (d << (DIGIT - 2)) ^ (d << (DIGIT - 7));
	t[0] ^= d;
}

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

void ac_gcm_tab_low(dig_t *t, unsigned char *h) {
	authenc_align dig_t ah[AC_GCM_BLOCK_LEN / sizeof(dig_t)];

	memcpy(ah, h, AC_GCM_BLOCK_LEN);
	memcpy(t, h, AC_GCM_BLOCK_LEN);
	gcm_lsh_low(t);

	ac_gcm_mul_low(t + AC_GCM_BLOCK_LEN / sizeof(dig_t), ah, t);
	gcm_lsh_low(t + AC_GCM_BLOCK_LEN / sizeof(dig_t));
}
