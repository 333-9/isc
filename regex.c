#include <stdio.h>
#include <stdlib.h>

#include "regex.h"


char *
compile(const char *str)
{
	size_t re_sz = 32;
	char *re, *p, *ccp;
	if ((p = re = malloc(re_sz)) == NULL) return NULL;
	for (;; str++) {
		switch (*str) {
		case '\0': *p = P_END; return re;
		default:  *p++ = P_CHAR; *p++ = *str; break;
		case '?': *p++ = P_ANY;   break;
		case '*': *p++ = P_STAR;  break;
		case '[':
			*p++ = P_CC + (*(++str) == '^');
			str += *str == '^';
			*(ccp = p++) = 0;
			if (*str == ']') {
				*p++ = *str++;
				*ccp += 1;
			};
			for (;; str++) {
				if (*str == '-' &&
				*(str +1) != ']' && *(str +1) != '\0') {
					/* ignore if first char */
					if ((p -1) == ccp) continue;
					*p = *(p -1);
					*(p - 1) = '-';
					str++;
					p++;
					*ccp += 1;
				} else if (*str == ']') {
					break;
				} else if (*str == '\0') {
					if (*ccp == 0) *(ccp -1) = P_END;
					*p = P_END;
					return re;
				};
				*p++ = *str;
				*ccp += 1;
				if ((p - re) >= (re_sz - 4)) {
					ccp -= (size_t) re;
					p   -= (size_t) re;
					re = realloc(re, re_sz += 32);
					if (re == NULL) return NULL;
					ccp += (size_t) re;
					p   += (size_t) re;
				};
			};
		};
		if ((p - re) >= (re_sz - 4)) {
			p -= (size_t) re;
			if ((re = realloc(re, re_sz += 32)) == NULL) return NULL;
			p += (size_t) re;
		};
	};
	return re;
}


static inline int
cclass(unsigned char sz, const char *cc, char c)
{
	while (--sz) { /* cclass of size 0 will cause error */
		if (*cc == '-') { /* '-' is only alowed at the end of cc */
			if ((*(cc +1) <= c) && (c <= *(cc +2))) return P_CC;
			cc += 3;
		} else {
			if (*(cc++) == c) return P_CC;
		};
	};
	return P_CC + (*cc != c);
}


int
advance(const char *re, const char *str)
{
	do {
		switch (*re++) {
		case P_END: return 1;
		case P_CHAR:  if (*re++ != *str)  return 0;
		case P_ANY:   continue;
		case P_STAR:
			do {
				if (advance(re, str)) return 1;
			} while (*str++);
			return 0;
		case P_CC:
		case P_NCC:
			if (*(re -1) != cclass(*re, re +1, *str))  return 0;
			re += *re + 1;
			continue;
		};
	} while (*str++);
	return (*re == P_END);
}
