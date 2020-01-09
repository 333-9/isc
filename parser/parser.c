/* * date:10.12. 2019
 */


#include <stdlib.h>
#include <stdio.h>


enum Instructions {
	I_null = 0,
	I_nop  = 1,
	I_call = 2,
	I_if   = 4,
	I_jump = 6,
	I_loop = 8,
	IL__args  = 9,
	I_cmp  = 10,
	I_move = 12,
	IL__math  = 13,
	I_add = 14,
	I_sub = 16,
	I_lsh = 18,
	I_rsh = 20,
	I_and = 22,
	I_or  = 24,
	I_xor = 26,
	I_mul = 28,
	I_div = 30,
	I_mod = 32,
	I_not = 34,
};


enum Sections { /* not used for now */
	S_info,
	S_data,
	S_text,
};


enum Arguments {
	A_reg_char  = 1,
	A_reg_short = 2,
	A_reg_int   = 4,
	A_reg_long  = 8,
	A_reg_mem   = A_reg_long +1,
	A_mem_reg   = A_reg_long +2,
	/* - */
	A_stack,
	A_reg_reg,
	A_reg,
	A_mem,
};


enum Flags {
	F_z  = 001,
	F_nz = 002,
	F_lt = 004,
	F_gt = 010,
	F_eq = 020,
	F_ne = 040,
};




union {
	int r[32];
	struct {
		int r0;
		int public[25];
		int private[2];
		int prev;
		union {
			int flags;
			struct {
				int f_z  : 1;
				int f_nz : 1;
				int f_lt : 1;
				int f_gt : 1;
				int f_eq : 1;
				int f_ne : 1;
			};
		};
	};
} reg = {0};

struct {
	unsigned int ind;
	int arr[0x100];
} stack = { 0, {0}, };

int *memory;
char *s = "";




char *
jump_to(int i)
{
	return s + i;
}


int
stack_pop() {
	if (stack.ind) return (stack.arr[--stack.ind]);
	else return 0;
}


void
stack_push(int a) {
	if (stack.ind >= 0x100) exit(1);
	stack.arr[++stack.ind] = a;
}




int *
parse_arg(char **str, int single)
{
	char c;
	int *p;
	size_t a;
	unsigned char *s = *str;
	switch (c = *s) {
	case A_reg_char:
	case A_reg_short:
	case A_reg_int:
	case A_reg_long:
		p = reg.r + *(++s);
		reg.r0 = (signed char) *(++s);
		while (--c)  reg.r0 = (reg.r0 << 8) + *(++s);
		goto end;
	case A_reg_mem:
	case A_mem_reg: /* 8b, 1b */
		for (c = 8, a = 0; c--;)  a = (a << 8) + *(++s);
		if (*(s -8) == A_reg_mem) {
			reg.r0 = *((int *) a);
			p = &(reg.r[*(++s)]);
		} else {
			p = (int *) a;
			reg.r0 = reg.r[*(++s)];
		};
		goto end;
	case A_reg_reg:
		p = reg.r + *(++s);
		reg.r0 = reg.r[*(++s)];
		goto end;
	case A_stack:
		reg.r0 = 0;
		p = stack.arr + *(++s);
		goto end;
	case A_reg:
		p = reg.r + *(++s);
		reg.r0 = 0;
		goto end;
	case A_mem:
		reg.r0 = 0;
		return &reg.r0;
	};
end:
	*str = s + 1;
	return p;
}


int
parse(char *str)
{
	char c;
	int a, *p;
	while (*str) {
		if ((c = *str++) > IL__args) p = parse_arg(&str, 0);
		switch (c) {
		case I_null: return 0; /* does not reach */
		case I_nop: break;
		case I_if:
			if ((*str & reg.flags) == *str) {
				str++;
				break;
			} else if (*str < IL__args) {
				str++;
				parse_arg(&str, 0);
			} else {
				switch (*str++) {
				case I_null: break;
				case I_loop: parse_arg(&str, 0); /* FALLTHROW */
				case I_call:
				case I_jump: parse_arg(&str, 1); break;
				};
			};
			break;
		case I_loop:
			p = parse_arg(&str, 0);
			if (*p -= reg.r0) {
		case I_call: /* TODO: push on stack */
		case I_jump:	str = jump_to(*parse_arg(&str, 1));
			} else {
				parse_arg(&str, 1);
			};
			break;
		case I_cmp:
			reg.flags = 0;
			reg.flags |= *p ? F_nz : F_z;
			reg.flags |= (*p == reg.r0)? F_eq : F_ne;
			reg.flags |= (*p < reg.r0)? F_lt : 0;
			reg.flags |= (*p > reg.r0)? F_gt : 0;
			break;
		case I_move: *p  = reg.r0;  break;
		case I_add:  *p += reg.r0;  break;
		case I_sub:  *p -= reg.r0;  break;
		case I_lsh:  *p<<= reg.r0;  break;
		case I_rsh:  *p>>= reg.r0;  break;
		case I_and:  *p &= reg.r0;  break;
		case I_or:   *p |= reg.r0;  break;
		case I_xor:  *p ^= reg.r0;  break;
		case I_mul:  *p *= reg.r0;  break;
		case I_div: if (!reg.r0) exit(8);  *p /= reg.r0;  break;
		case I_mod: if (!reg.r0) exit(8);  *p %= reg.r0;  break;
		case I_not:  *p = !*p;  break;
		};
	};
	return 0;
}




int
main()
{
	char str[] = {
		I_move, A_reg_char, 2, 0,
		I_move, A_reg_char, 3, 12,
		I_move, A_reg_char, 1, 2,
		I_add,  A_reg_char, 2, 5,
		I_loop, A_reg_char, 1, 1, A_reg, 3,
		I_move, A_reg_reg,  1, 2,
		I_null,
	};
	s = str;
	parse(str);
	return reg.r[1];
}
