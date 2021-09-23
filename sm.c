#include "tm.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

// Edge
DefineStruct(Ed);

Ed * EdCtr(size_t n, ...) {
	Ed * new;
	//
	// data can default to 0 but everything else must be provided
	if (n != 3 && n != 4) {
		printf("%s: bad argc\n", __func__);
		return NULL;
	}
	va_list list;
	CtrBoiler(Ed);


	va_start(list, n);
	
	new->from 	= va_arg(list, UL);
	new->to 	= va_arg(list, UL);
	new->key 	= va_arg(list, UL);
	new->data 	= n != 4
			? 0
			: va_arg(list, UL);

	va_end(list);

	return new;
}

DefineStruct(Gr);

Gr * GrCtr(size_t n, ...) {
	Gr * new;
	va_list list;

	CtrBoiler(Gr);

	va_start(list, n);
	new->v = va_arg(list, UL);
	new->e = va_arg(list, EdList *);
	va_end(list);

	new->_d = GrDtr;

	return new;
}

void GrDtr(Gr ** old) {
	RmList(Ed)(&(*old)->e);
	DefDtr(Gr)(old);
}

DefineStruct(Sm);

void SmDtr(Sm ** old) {
	Rm((*old)->g);
	free((*old)->m);
	DefDtr(Sm)(old);
}

// graph, start state
Sm * SmCtr(size_t n, ...) {
	Sm * new;
	va_list list;
	size_t i, j;

	if (n != 2) {
		printf("%s: bad argc\n", __func__);
		return NULL;
	}

	CtrBoiler(Sm);

	va_start(list, n);
	new->g = va_arg(list, Gr *);
	new->s = new->p = va_arg(list, UL);
	va_end(list);

	// allow for transition to zero on error/bottom/whatever
	UL vl = new->g->v + 1;

	// l by l edgemap (from l vertices we have l edges)
	new->m = (struct ekv *)malloc(sizeof(struct ekv) * (vl * vl));
	if (!new->m) {
		free(new); new = NULL;
	}

	memset(new->m, 0, sizeof(struct ekv) * vl * vl);
	for (i = 1; i <= new->g->v; ++i) for (j = 0; j <= new->g->v; ++j) {
		// default transition is self loop
		EMAP(new, i, j)->v = i;
	}

	Ed * e;
	for_each(Ed, e, new->g->e) {
		*EMAP(new, e->from, e->key) = (struct ekv){.v = e->to, .d = e->data};
	}


	new->_d = SmDtr;
	new->tmdat = 0;

	return new;
}

UL Sm_do(Sm * s, UL d) {
	// must be positive int not greater than limit
	if (d <= 0 || d > s->g->v) {
		return 0;
	}

	struct ekv * next;

	/* next = s->m + s->p * s->l * sizeof(struct ekv) + d * sizeof(struct ekv); */

	next = EMAP(s, s->p, d);

	s->p = next->v;
	return next->d;
}

// key is a Tmdat.op
static char * Tmop_strtab[] = {
	[X] = "No-op",
	[L] = "Left",
	[R] = "Right",
};

char * Tmdat_str(Tmdat d) {
	static char buf[_1K];
	sprintf(buf, "{%d, %s}", d.dat, Tmop_strtab[d.op]);
	return buf;
}

char * Sm_str(Sm * s) {
	static char buf[_1K];
	size_t i, j, cnt = 0;
	struct ekv * tmp;
	cnt += sprintf(buf, "Sm.p=%lu\n", s->p);

	for (i = 1; i <= s->g->v; ++i) {
		cnt += sprintf(buf + cnt, "[%lu] ", i);
		for (j = 1; j <= s->g->v; ++j) {
			if (s->tmdat) {
				tmp = EMAP(s, i, j);
				cnt += sprintf(buf + cnt, "%lu->(%lu, %s) ",
						j, tmp->v, Tmdat_str((Tmdat)tmp->d));
			} else if (!s->alt) {
				tmp = EMAP(s, i, j);
				cnt += sprintf(buf + cnt, "%lu->(%lu, %lu) ", j, tmp->v, tmp->d);
			} else {
				cnt += sprintf(buf + cnt, "%lu->%s ", j, Ekvl_str(
					*(s->alt + i * (s->g->v + 1) + j)));
			}
		}
		cnt += sprintf(buf + cnt, "\n");
	}

	return buf;
}

