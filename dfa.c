#include "tm.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

DefineStruct(Df);

// state machine, name, acceptmask
Df * DfCtr(size_t n, ...) {
	va_list list;
	Df * new;

	if (n != 3) {
		printf("%s: bad argc\n", __func__);
		return NULL;
	}
	CtrBoiler(Df);

	va_start(list, n);
	new->s = va_arg(list, Sm *);
	new->n = va_arg(list, const char *);
	new->a = va_arg(list, UL);
	va_end(list);

	new->_d = DfDtr;

	return new;
}

void DfDtr(Df ** old) {
	Rm((*old)->s);
	DefDtr(Df)(old);
}

// realistically when will I need more than 64-1 states? I can always change this later
UL Mkmask(size_t n, ...) {
	va_list list;
	size_t i;
	UL res = 0, tmp;

	va_start(list, n);
	for (i = 0; i < n; i++) {
		if ((tmp = va_arg(list, UL)) > sizeof(UL) * 8) {
			return -1;
		}
		res |= (1UL << tmp);
	}
	va_end(list);

	return res;
}

// 0 accept
int Df_run(Df * d, UL * s, size_t n) {
	size_t i;
	int res;
	UL p, _;

	if (n < 1) {
		// if empty input, accept if start is an accept state
		return !(d->a & (1UL << d->s->s));
	}

	printf("RUN DFA %s\n", d->n);
	d->s->p = d->s->s;
	for (i = 0; i < n; ++i) {
		p = d->s->p;
		_ = Sm_do(d->s, s[i]);
		printf("delta(%lu, %lu) = (%lu, %lu)\n", p, s[i],d->s->p, _);
	}

	// check for accept state
	res = !(d->a & (1UL << d->s->p));
	printf("%s\n", !res ? "ACCEPT" : "REJECT");
	return res;
}
