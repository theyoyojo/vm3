#include "tm.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

DefineStruct(Tp);

Tp * TpCtr(size_t n, ...) {
	va_list list;
	size_t i;
	UL * data = NULL ;
	Tp * new;
	CtrBoiler(Tp);

	va_start(list, n);
	if (n == 0) {
		new->n = va_arg(list, size_t);
		data = va_arg(list, UL *);
	} else {
		new->n = n;
	}

	new->t = (UL * )malloc(sizeof(UL) * new->n);
	if (!new->t) {
		free(new);
		return NULL;
	}

	if (n == 0) {
		memcpy(new->t, data, new->n);
	} else {
		for (i = 0; i < n; ++i) {
			new->t[i] = va_arg(list, UL);
		}
	}

	new->h = new->t;
	new->p = 0;

	va_end(list);
	new->_d = TpDtr;

	return new;
}

void TpDtr(Tp ** old) {
	free((*old)->t);
	DefDtr(Tp)(old);
}


static void Tp_left(Tp * t) {
	if (t->p > 0) {
		--t->h;
		--t->p;
	}
}

static void Tp_right(Tp * t) {
	if (t->p >= t->n - 1) {
		t->t = (UL *)realloc(t->t, sizeof(UL) * (++t->n));
		if (!t->t) {
			// panic
			printf("panic: the tape is gone!\n");
			exit(1);
		}
		t->t[t->n - 1] = 0UL;
		// set head to tape start plus current integral position
		t->h = t->t + t->p;
	}
	++t->h;
	++t->p;
}

DefineStruct(Tm);

// args: tape, dfa, rejection bitmask
Tm * TmCtr(size_t n, ...) {
	va_list list;
	Tm * new;
	CtrBoiler(Tm);

	va_start(list, n);

	new->t = va_arg(list, Tp *);
	new->d = va_arg(list, Df *);
	new->r = va_arg(list, UL);

	va_end(list);

	new->_d = TmDtr;
	new->d->s->tmdat = 1;
	new->v = 0;
	return new;
}

void TmDtr(Tm ** old) {
	Rm((*old)->d);
	Rm((*old)->t);
	DefDtr(Tm)(old);
}

char * Tm_str(Tm * t) {
	static char buf[_1K];
	UL cnt = 0, i;
	int firstchar;
	
	cnt += sprintf(buf, "Turing machine \"%s\"\n", t->d->n);
	cnt += sprintf(buf + cnt, "Machine state: %s\n", Tmstate_str(t->s));

	cnt += sprintf(buf + cnt, "Tape: ");
	for (i = 0; i < t->t->n; ++i) {
		cnt += sprintf(buf + cnt, "[%03lu]%s", t->t->t[i], i == t->t->n - 1 ? "\n" : " ");
	}
	cnt += sprintf(buf + cnt, "      ");
	for (i = 0; i < t->t->p; ++i) {
		cnt += sprintf(buf + cnt, "      ");
	}
	cnt += sprintf(buf + cnt," ^^^\n");
	cnt += sprintf(buf + cnt, "Tm.%s", Sm_str(t->d->s));
	cnt += sprintf(buf + cnt, "accepts: {");
	firstchar = 1;
	for (i = 1; i < sizeof(UL) * 8; ++i) {
		/* printf("t->d->a=0x%lx, i=%lu, (1<<i)=0x%lx, (cond)=0x%032lx\n", */
		/* 		t->d->a, i, (1UL << i), (t->d->a & (1UL << i))); */
		if (t->d->a & (1UL << i)) {
			cnt += sprintf(buf + cnt, "%s%lu", firstchar ? "" : ", ", i);
			firstchar = 0;
		}
	}
	cnt += sprintf(buf + cnt, "}\n");
	cnt += sprintf(buf + cnt, "rejects: {");
	firstchar = 1;
	for (i = 1; i < sizeof(UL) * 8; ++i) {
		if (t->r & (1UL << i)) {
			cnt += sprintf(buf + cnt, "%s%lu", firstchar ? "" : ", ", i);
			firstchar = 0;
		}
	}
	cnt += sprintf(buf + cnt, "}\n");

	return buf;
}

static int Tm_accepts(Tm * tm) {
	return (tm->d->a & (1UL << tm->d->s->p));
}

static int Tm_rejects(Tm * tm) {
	return (tm->r & (1UL << tm->d->s->p));
}

#define PANICNUMBER (1UL << 8)

static const char * Tmstate_strtab[TMSTATECOUNT] = {
	[HALT_START] 	= "HALT START",
	[HALT_ACCEPT] 	= "HALT ACCEPT",
	[HALT_REJECT] 	= "HALT REJECT",
	[HALT_ERROR] 	= "HALT ERROR",
	[RUNNING]	= "RUNNING",
};

const char * Tmstate_str(Tmstate st) {
	return Tmstate_strtab[st];
}

// check if (Tm.State ∈  Tm.Accept U Tm.reject)
Tmstate Tm_updatestate(Tm * t) {
	if (Tm_accepts(t)) {
		t->s = HALT_ACCEPT;
	} else if (Tm_rejects(t)) {
		t->s = HALT_REJECT;
	} else {
		t->s = RUNNING;
	}
	return t->s;
}

// 0 accept
int Tm_run(Tm * tm) {
	// acceptance and rejection are disjoint
	if (tm->d->a & tm->r) {
		printf("error: non-disjoint acceptance and rejection states\n");
		return -1;
	}

	// must have some tape!
	if (!(tm->t->n > 0)) {
		printf("error: no tape!\n");
		return -1;
	}

	printf("RUN Turing Machine %s\n", Tm_name(tm));

	if (tm->v) {
		printf("%s\n", Tm_str(tm));
	}

	size_t tpos = 0, i = 0;
	Tmdat code;

	while (Tm_updatestate(tm) == RUNNING && i++ < PANICNUMBER) {
		code = (Tmdat)Sm_do(tm->d->s, *tm->t->h);
		printf("(%lu) LOAD Tape[%lu] = %s\n", i, tpos, Tmdat_str(code));

		// Writing the blank character implements no-op (0UL)
		if (code.dat != 0) {
			*tm->t->h = code.dat;
		}

		switch (code.op) {
		case L:
			Tp_left(tm->t);
			break;
		case R:
			Tp_right(tm->t);
		case X:
		default:
			break;
		}

		Tm_updatestate(tm);

		if (tm->v) {
			printf("%s\n", Tm_str(tm));
		}

	}

	if (i >= PANICNUMBER) {
		tm->s = HALT_ERROR;
	}

	printf("%s %s\n", Tmstate_str(tm->s), tm->d->n);

	return (Tm_accepts(tm) ? 0 : ( Tm_rejects(tm) ? 1 : -1 ));
}
