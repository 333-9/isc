/*
 * date:19.12. 2019
 * hash table used in the compiler
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>


struct table_item {
	void *next;
	void *p;
	char *str;
};


struct table {
	struct table_item *t[0x100];
};


int
get_hash(const char *str)
{
	int h = 17;
	int a = 0;
	do {
		h += (*str * 651269) ^ ++a;
	} while (*(++str));
	return (a * 101) * h;
}


struct table *
table_init()
{
	return (calloc(0x100, sizeof(void *)));
}


int
table_add(struct table *t, const char *str, void *ptr)
{
	unsigned char i;
	struct table_item *p;
	struct table_item *itm;
	i = get_hash(str);
	if((itm = malloc(sizeof(struct table_item))) == NULL) return 1;
	itm->next = NULL;
	itm->p = ptr;
	if ((itm->str = strdup(str)) == NULL) return 1;
	if (p = t->t[i]) {
		while (p->next) p = p->next;
		p->next = itm;
	} else  {
		t->t[i] = itm;
	};
	return 0;
}


void *
table_get(struct table *t, const char *str)
{
	struct table_item *p;
	p = t->t[(unsigned char) get_hash(str)];
	if (p == NULL) return NULL;
	for (;;) {
		if (strcmp(p->str, str) == 0) return p->p;
		if ((p = p->next) == NULL) return NULL;
	};
}


int
main()
{
	void *p;
	p = table_init();
	table_add(p, "hello", (void *) 5);
	table_add(p, "world", (void *) 4);
	table_add(p, "help",  (void *) 2);
	printf("%p\n", table_get(p, "hello"));
	return 0;
}
