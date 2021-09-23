#include "tm.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

// arbitary number of edges (consoooms the edge)
Ekvl * EkvlCtr(UL v, size_t n, ...) {
	Ekvl * new;
	Ed * e;
	va_list list;
	size_t i;

	size_t vl = v + 1;
	new = (Ekvl *)malloc(sizeof(Ekvl) * vl * vl);
	if (!new) {
		return NULL;
	}
	for (i = 0; i < vl * vl; ++i) {
		new[i] = NULL;
	}
	va_start(list, n);
	for (i = 1; i <= n; ++i) {
		e = va_arg(list, Ed *);
		Ekvl_append(new + e->from * vl + e->key, (struct ekv){ .v = e->to, .d = e->data});
		Rm(e);
	}
	va_end(list);

	return new;
}

void EkvlDtr(Ekvl ** old, size_t len) {
	size_t i;

	Ekvl * ekvl = * old;
	Ekvl head, tmp;

	for (i = 0; i < len; ++i) {
		if (ekvl[i]) for (head = ekvl[i]; head;) {
			tmp = head->next;
			free(head);
			head = tmp;
		}
	}

	free(*old);
	*old = NULL;
}

int Ekvl_append(Ekvl * l, struct ekv d) {
	Ekvl *p, new = (Ekvl)malloc(sizeof(struct ekvl));
	memset(new, 0, sizeof(struct ekvl));
	if (!new) {
		return -1;
	}
	
	p = l;
	while (*p) {
		p = &(*p)->next;
	}
	new->data = d;
	*p = new;
	return 0;
}

char * Ekvl_str(Ekvl ekvl) {
	static char buf[_1K];
	Ekvl p;
	size_t cnt = 0;

	cnt += sprintf(buf, "{");
	p = ekvl;
	while (p) {
		cnt += sprintf(buf + cnt, "(%lu, %lu)%s", p->data.v, p->data.d,
				p->next ? ", " : "");
		p = p->next;
	}
	cnt += sprintf(buf + cnt, "}");

	return buf;
}

void PtDtr(Pt * old) {
	size_t i;
	if (old && *old) {
		for (i = 0; i < (*old)->kidcount; ++i) {
			PtDtr(&(*old)->kids[i]);
		}
		free((*old)->kids);
		(*old)->kids = NULL;
	}
	free(*old);
	old = NULL;
}

char * Pt_str(Pt t) {
	static char buf[_1K];
	static struct {
		Pt t;
		size_t i;
	} stack[_1K];
	size_t i, cnt = 0;
	int sp = 0;

	stack[sp].t = t;
	stack[sp].i = 0;

	while (sp >= 0) if (stack[sp].t->kidcount == 0) {
		// leaf
		for (i = 0; (int)i < sp; ++i) {
			cnt += sprintf(buf + cnt, "    ");
		}
		cnt += sprintf(buf + cnt, "(%02lu, %02lu) (leaf)\n",
				stack[sp].t->c, stack[sp].t->d);
		--sp;
	} else if (stack[sp].i < stack[sp].t->kidcount) {
		// unprocessed kids
		// push next kid onto stack to process its kids
		stack[sp + 1].t = stack[sp].t->kids[stack[sp].i++];
		stack[++sp].i = 0;
	} else {
		// all kids are processed, print and pop of stack
		for (i = 0; (int)i < sp; ++i) {
			cnt += sprintf(buf + cnt, "    ");
		}
		cnt += sprintf(buf + cnt, "(%02lu, %02lu)\n",
				stack[sp].t->c, stack[sp].t->d);
		--sp;
	}

	return buf;
}


DefineStruct(Nf);

// args: dfa, altenative tranitions list made from dfa (ekvl)
Nf * NfCtr(size_t n, ...) {
	size_t i, j;
	Nf * new;
	va_list list;
	if (n != 2) {
		printf("%s: bad argc\n", __func__);
		return NULL;
	}
	CtrBoiler(Nf);

	va_start(list, n);
	new->d = va_arg(list, Df *);
	new->o = va_arg(list, Ekvl *);
	va_end(list);

	memset(new->altv, 0, 2 * sizeof(struct altv) * ALTVLEN);

	size_t vl = new->d->s->g->v + 1;
	struct ekv * e;
	for (i = 1; i < vl; ++i) for (j = 1; j < vl; ++j) {
		e = EMAP(new->d->s, i, j);
		Ekvl_append(new->o + i * vl + j,
				(struct ekv){ .v = e->v, .d = e->d });
	}
	new->d->s->alt = new->o;

	new->_d = NfDtr;

	return new;
}

void NfDtr(Nf ** old) {
	if ((*old)->t) {
		PtDtr(&(*old)->t);
	}
	EkvlDtr(&(*old)->o, ((*old)->d->s->g->v + 1) * ((*old)->d->s->g->v + 1));
	Rm((*old)->d);
	DefDtr(Nf)(old);
}


// 0 accept, TODO make this not gigantic
int Nf_run(Nf * nf, UL *s, size_t n) {
	if (n < 1) {
		// if empty input, accept if start is an accept state
		return !(nf->d->a & (1UL << nf->d->s->s));
	}

	int res, xXx = 0;
	size_t i, j;
	Ekvl possible, trans;
	struct altv * new;
	Pt curr, root = (Pt)malloc(sizeof(struct pt));

	if (!root) {
		return 0;
	}
	root->c = s[0];
	root->d = 0;
	root->parent = NULL;
	root->kidcap = KIDCAP_DEFAULT;
	root->kids = (Pt *)malloc(sizeof(Pt) * root->kidcap);
	if (!root->kids) {
		return -1;
	}
	root->kidcount = 0;

	/* curr = root; */
	memset(nf->altv, 0, 2 * ALTVLEN * sizeof(struct altv));
	nf->altv[xXx][0].c = nf->d->s->s;
	nf->altv[xXx][0].t = &root;
	nf->altc[xXx] = 1;
	nf->altc[xXx ^ 1] = 0;
	
	printf("RUN NFA %s\n", nf->d->n);

	size_t vl = nf->d->s->g->v + 1;
	// loop through string
	for (i = 0; i < n; ++i) {
		// loop through alternate states/tree branches
		for (j = 0; j < nf->altc[xXx]; ++j) {
			// get current tree to append to
			curr = *nf->altv[xXx][j].t;
			// loop through possible transitions
			possible = nf->o[nf->altv[xXx][j].c * vl + curr->c];
			for (trans = possible; trans; trans = trans->next) {
				// write next state and tree node to here
				new = &nf->altv[xXx ^ 1][nf->altc[xXx ^ 1]];
				// one of the possible next vertices
				new->c = trans->data.v;

				// renovate house for birth of new kids
				if (curr->kidcap == curr->kidcount) {
					curr->kidcap *= 2;
					curr->kids = (Pt *)realloc(curr->kids, curr->kidcap);
					if (!curr->kids) {
						return -1;
					}
				}


				// add a new child to the current tree and reference it in the new altv
				if (!(curr->kids[curr->kidcount] =
							(Pt)malloc(sizeof(struct pt)))) {
					// panic!
					return -1;
				}
				new->t = &curr->kids[curr->kidcount++];
				(*new->t)->parent = curr;
				(*new->t)->kidcap = KIDCAP_DEFAULT;
				(*new->t)->kids = (Pt *)malloc(sizeof(Pt)
						* (*new->t)->kidcap);
				(*new->t)->kidcount = 0;
				(*new->t)->c = trans->data.v;
				(*new->t)->d = trans->data.d;

				printf("delta(%lu, %lu) includes (%lu, %lu)\n",
						nf->altv[xXx][j].c, curr->c,
						(*new->t)->c, (*new->t)->d);


				++nf->altc[xXx ^ 1];
			}
		}
		// reset current alt{v,c}
		memset(nf->altv + xXx, 0, ALTVLEN * sizeof(struct altv));
		nf->altc[xXx] = 0;
		xXx ^= 1; //switch alt{v,c} to the other one
	}

	res = 1;
	for (i = 0; i < nf->altc[xXx]; ++i) {
		if (nf->d->a & (1UL << nf->altv[xXx][i].c)) {
			res = 0;
		}
	}

	nf->t = root;

	printf("%s\n", !res ? "ACCEPT" : "REJECT");
	return res;
}
