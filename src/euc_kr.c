/* euc_kr.c
 * EUC-KR handling functions
 */

/* C99 */
#include <stdint.h>
#include <stdbool.h>

#include <stdlib.h>

#include "unicode_table_uhc.h"
#include <grf.h>

static uint32_t euc_kr_strlen(const uint8_t *euckr) {
	uint32_t c=0;
	const uint8_t *t=euckr;
	while(*t) {
		uint8_t x=*t; t++;
		c++;
		if (x<0x80) continue; /* latin */
		if (x>0xa0 && x<0xff && x!=0xc9) {
			if (*t == 0) return 0; /* ERG */
			t++;
			c+=2; /* total size of a korean UTF8 char is 3 bytes */
			continue;
		}
		c+=5; // max length, as we don't know what will happen
		// This case wasn't supposed to happen as we were expecting to get a 100% EUC-KR
		// file, however we got it anyway. Life is unpredictable.
		//return 0; /* should not happen -> throw fatal */
	}
	return c;
}

static uint32_t utf8_strlen(const uint8_t *utf8) {
	uint32_t c=0;
	const uint8_t *t = utf8;
	while(*t) {
		uint8_t x=*t; t++; c++;
		uint8_t l=0;
		if ((x & 0x80) == 0) continue; // ASCII range
		if ((x & 0xe0) == 0xc0) { // 2chars utf8
			l=1;
		} else if ((x & 0xf0) == 0xe0) { // 3chars utf8
			l=2;
		} else if ((x & 0xf8) == 0xf0) { // 4chars utf8
			l=3;
		} else if ((x & 0xfc) == 0xf8) { // 5chars utf8 (illegal)
			l=4;
		} else if ((x & 0xfe) == 0xfc) { // 6chars utf8 (illegal)
			l=5;
		}
		if (l==0) return 0; /* ILLEGAL */
		for (int i=0;i<l;i++) {
			x=*t; t++; c++;
			if ((x & 0x80) != 0x80) return 0; /* ILLEGAL */
		}
	}
	return c;
}

static void utf8_append_from_wchar(uint8_t **r, int c) {
	if ( (c >= 0x70000000) || (c <= 0)) { /* MBFL_WCSGROUP_UCS4MAX */
		// illegal character
		return;
	}
	if (c < 0x80) { /* 1 char */
		**r = c;
		(*r)++;
		return;
	}
	if (c < 0x800) { /* 2 chars */
		**r = ((c >> 6) & 0x1f) | 0xc0; (*r)++;
		**r = (c & 0x3f) | 0x80; (*r)++;
		return;
	}
	if (c < 0x10000) { /* 3 chars */
		**r = ((c >> 12) & 0x0f) | 0xe0; (*r)++;
		**r = ((c >> 6) & 0x3f) | 0x80; (*r)++;
		**r = (c & 0x3f) | 0x80; (*r)++;
		return;
	}
	if (c < 0x200000) { /* 4 chars */
		**r = ((c >> 18) & 0x07) | 0xf0; (*r)++;
		**r = ((c >> 12) & 0x3f) | 0x80; (*r)++;
		**r = ((c >> 6) & 0x3f) | 0x80; (*r)++;
		**r = (c & 0x3f) | 0x80; (*r)++;
		return;
	}
	if (c < 0x4000000) { /* 5 chars */
		**r = ((c >> 24) & 0x03) | 0xf8; (*r)++;
		**r = ((c >> 18) & 0x3f) | 0x80; (*r)++;
		**r = ((c >> 12) & 0x3f) | 0x80; (*r)++;
		**r = ((c >> 6) & 0x3f) | 0x80; (*r)++;
		**r = (c & 0x3f) | 0x80; (*r)++;
		return;
	}
	/* 6 chars */
	**r = ((c >> 30) & 0x01) | 0xfc; (*r)++;
	**r = ((c >> 24) & 0x3f) | 0x80; (*r)++;
	**r = ((c >> 18) & 0x3f) | 0x80; (*r)++;
	**r = ((c >> 12) & 0x3f) | 0x80; (*r)++;
	**r = ((c >> 6) & 0x3f) | 0x80; (*r)++;
	**r = (c & 0x3f) | 0x80; (*r)++;
}

static bool euc_kr_append_from_wchar(uint8_t **r, int c) {
	int s=0, c1, c2;
	if (c >= ucs_a1_uhc_table_min && c < ucs_a1_uhc_table_max) {
		s = ucs_a1_uhc_table[c - ucs_a1_uhc_table_min];
	} else if (c >= ucs_a2_uhc_table_min && c < ucs_a2_uhc_table_max) {
		s = ucs_a2_uhc_table[c - ucs_a2_uhc_table_min];
	} else if (c >= ucs_a3_uhc_table_min && c < ucs_a3_uhc_table_max) {
		s = ucs_a3_uhc_table[c - ucs_a3_uhc_table_min];
	} else if (c >= ucs_i_uhc_table_min && c < ucs_i_uhc_table_max) {
		s = ucs_i_uhc_table[c - ucs_i_uhc_table_min];
	} else if (c >= ucs_s_uhc_table_min && c < ucs_s_uhc_table_max) {
		s = ucs_s_uhc_table[c - ucs_s_uhc_table_min];
	} else if (c >= ucs_r1_uhc_table_min && c < ucs_r1_uhc_table_max) {
		s = ucs_r1_uhc_table[c - ucs_r1_uhc_table_min];
	} else if (c >= ucs_r2_uhc_table_min && c < ucs_r2_uhc_table_max) {
		s = ucs_r2_uhc_table[c - ucs_r2_uhc_table_min];
	}

	c1 = (s >> 8) & 0xff;
	c2 = s & 0xff;
	/* exclude UHC extension area */
	if (c1 < 0xa1 || c2 < 0xa1){  
		s = c;
	}

	if (s <= 0) {
		return false; /* error */
	}
	if (s < 0x80) { /* latin */
		**r = s; (*r)++;
		return true;
	}
	**r = ((s>>8)&0xff); (*r)++;
	**r = (s&0xff); (*r)++;
	return true;
}

GRFEXPORT char *utf8_to_euc_kr_r(const char *orig, uint8_t *res) {
	int c;
	uint32_t flen = utf8_strlen((const uint8_t *)orig);
	uint8_t *t = (uint8_t *)orig;
	uint8_t *r = res;
	if (flen == 0) return NULL;
	while (*t) {
		uint8_t x = *t;
		uint8_t l = 0, p;
		t++;
		if (x<0x80) {
			// easy one, that's the same char in UTF-8
			*r = x; r++;
			continue;
		}
		if ((x & 0xe0) == 0xc0) { // 2chars utf8
			l=1; p=0x1f;
		} else if ((x & 0xf0) == 0xe0) { // 3chars utf8
			l=2; p=0xf;
		} else if ((x & 0xf8) == 0xf0) { // 4chars utf8
			l=3; p=0x7;
		} else if ((x & 0xfc) == 0xf8) { // 5chars utf8 (illegal)
			l=4; p=0x3;
		} else if ((x & 0xfe) == 0xfc) { // 6chars utf8 (illegal)
			l=5; p=0x1;
		}
		if (l==0) return NULL; /* ILLEGAL */
		c = x & p;
		for(int i=0;i<l;i++) c = (c << 6) | (*(t++) & 0x3f);
		if (!euc_kr_append_from_wchar(&r, c)) return NULL;
	}
	*r = 0;
	return (char *)res;
}

GRFEXPORT char *euc_kr_to_utf8_r(const char *orig, uint8_t *res) {
	int c;
	uint32_t flen = euc_kr_strlen((const uint8_t *)orig); /* final len */
	uint8_t *t = (uint8_t *)orig;
	uint8_t *r = res;
	if (flen == 0) return NULL;
	while (*t) {
		uint8_t x = *t;
		uint8_t x2;
		int flag;
		if (x<0x80) {
			// easy one, that's the same char in UTF-8
			*r = *t;
			r++; t++;
			continue;
		}
		if (x <= 0xa0 || x >= 0xff || x == 0xc9) {
			/* unexpected: skip character and hope tomorrow will be a better day */
			t++; continue;
//			return NULL;
		}
		t++;
		x2 = *t; t++;
		flag = 0;
		if (x >= 0xa1 && x <= 0xc6) {
			flag = 1;
		} else if (x >= 0xc7 && x <= 0xfe && x != 0xc9) {
			flag = 2;
		}
		if (flag > 0 && x2 >= 0xa1 && x2 <= 0xfe) {
			if (flag == 1) { /* 1st: 0xa1..0xc6, 2nd: 0x41..0x7a, 0x81..0xfe */
				c = (x - 0xa1)*190 + (x2 - 0x41);
				if (c >= 0 && c < uhc2_ucs_table_size) {
					c = uhc2_ucs_table[c];
				} else {
					c = 0;
				}
			} else { /* 1st: 0xc7..0xc8,0xca..0xfe, 2nd: 0xa1..0xfe */
				c = (x - 0xc7)*94 + (x2 - 0xa1);
				if (c >= 0 && c < uhc3_ucs_table_size) {
					c = uhc3_ucs_table[c];
				} else {
					c = 0;
				}
			}
			if (c <= 0) {
				c = (x << 8) | x2;
				c &= 0xffff; /* MBFL_WCSPLANE_MASK */
				c |= 0x70f10000; /* MBFL_WCSPLANE_KSC5601: 2121h - 7E7Eh */
			}
		} else if ((x2 < 0x21) || x2 == 0x7f) { /* CTLs */
			continue;
		} else {
			c = (x << 8) | x2;
			c &= 0xffffff; /* MBFL_WCSGROUP_MASK */
			c |= 0x78000000; /* MBFL_WCSGROUP_THROUGH: 000000h - FFFFFFh */
		}
		utf8_append_from_wchar(&r, c);
	}
	*r = 0;
	return (char *)res;
}

GRFEXPORT char *euc_kr_to_utf8(const char *orig) {
	static uint8_t *res[4096]; // should be enough
	return euc_kr_to_utf8_r(orig, (uint8_t *)&res);
}

GRFEXPORT char *utf8_to_euc_kr(const char *orig) {
	static uint8_t *res[4096]; // should be enough
	return utf8_to_euc_kr_r(orig, (uint8_t *)&res);
}

