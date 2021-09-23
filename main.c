#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "tm.h"

int main(void) {

	/* DFA/NFA demo */

	UL vmax = 2;
	EdList * el = MkList(Ed)(2, MkEd(4,1,2,1,1), MkEd(4,2,1,1,1));
	Gr * g = MkGr(2, vmax, el);
	Sm * s = MkSm(2, g, 1);
	Df * d = MkDf(3, s, "test", Df_mkmask(1, 2));
	Ekvl * ekvl = MkEkvl(d->s->g->v, 1, MkEd(4, 1,1,1,0));
	UL input[] = {1, 1};
	Df_run(d, input, sizeof(input)/sizeof(*input));

	Nf * n = MkNf(2, d, ekvl);
	Nf_run(n, input, 2);
	printf("pt:\n%s", Pt_str(n->t));
	Rm(n);

	/* Turing Machine demo */

	struct tmgen tg = (struct tmgen) {
		.tape = MkTp(3,1,1,1),
		.edges = MkList(Ed)(2,
			MkEd(4,1,2,1,MkTmdat(3, R)),
			MkEd(4,1,2,1,MkTmdat(3, R))),
		.vertices = 2,
		.start = 1,
		.acceptmask = Df_mkmask(0),
		.rejectmask = Df_mkmask(1,2),
		.name = "TM test2",
	};

	Tm * tm = Tm_gen(&tg);
				

	printf("%s\n", Tm_str(tm));

	Tm_run(tm);

	Rm(tm);

	return 0;
}
