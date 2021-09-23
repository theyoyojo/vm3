#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

typedef uint64_t UL;
typedef uint32_t UI;

#define _1K (1UL << 10)

#define ASSERT(__x) assert(__x)

#define Ctr(T,__c) T * (*__c)(size_t n, ...)
#define Cpy(T,__p) T * (*__p)(T*)
#define Dtr(T,__d) void (*__d)(T**)

#define Cp(__x) __x->_p(__x)
#define Rm(__x) __x->_d(&__x)

#define DefineArr(T)			\
typedef struct {			\
	size_t size;			\
	size_t cap;			\
	T * arr;			\
} T##Arr;				\
T* CtrArr##T(size_t n, ...) {		\
	size_t i;			\
	va_list list;			\
	va_start(list, n);		\
	T##Arr * new; 			\
	new=(T##Arr*)			\
		malloc(sizeof(T##Arr));	\
	size_t len = n * sizeof(T);	\
	if (new) {			\
		new->arr = malloc(len);	\
	}				\
	if (new && new->arr) {		\
		memset(new->arr,0,len); \
		for (i=0;i<n;++i) {	\
			memcpy(		\
			&new->arr[i],	\
			va_arg(list,T*),\
			sizeof(T));	\
			printf("%lu\n", i);\
		}			\
		new->cap=new->size=n;	\
	} else if (new){		\
		free(new); new=NULL;	\
	}				\
	va_end(list);			\
	if (new && new->arr) {		\
	return (T*)(new +		\
		2 * sizeof(size_t));	\
	} else return NULL;		\
}					\
void DtrArr##T(T ** old) {		\
	ASSERT(old);			\
	T##Arr * _old;			\
	_old = (T##Arr*)		\
		(*old-2*sizeof(size_t));\
	ASSERT(_old);			\
	free(_old->arr);		\
	free(_old);			\
	*old = NULL;			\
}

#define MkArr(T) CtrArr##T
#define RmArr(T) DtrArr##T
#define SizeArr(__a) ((size_t)(__a-2*sizeof(size_t)))
#define CapArr(__a) ((size_t)(__a-sizeof(size_t)))

#define DefineList(T)			\
struct _##T##List;			\
typedef struct _##T##List {		\
	T * data;			\
	struct _##T##List * next;	\
} T##List;				\
T##List * CtrList##T(size_t n, ...) {	\
	size_t i;			\
	T##List *head=NULL,		\
		**next=NULL;		\
	next=&head;			\
	va_list list;			\
	va_start(list, n);		\
	for (i=0;i<n;++i) {		\
		*next = (T##List*)	\
		malloc(sizeof(T##List));\
		if (!*next) {		\
			return NULL;	\
		}			\
		(*next)->data=		\
			va_arg(list,T*);\
		(*next)->next = NULL;	\
		next = &(*next)->next;	\
	}				\
	va_end(list);			\
	return head;			\
}					\
void DtrList##T(T##List ** old)	{	\
	ASSERT(old);			\
	T##List *tmp, *head = *old;	\
	while (head && head->data) {	\
		Rm(head->data);		\
		tmp=head->next;		\
		free(head);		\
		head=tmp;		\
	}				\
	*old = NULL;			\
}					\
T * GetList##T(T##List * l, size_t n) {  \
	size_t i = 0;			\
	T##List * p = l;		\
	while (p && i<= n) {		\
		if (i == n) {		\
			return p->data; \
		}			\
		p = p->next;		\
		++i;			\
	}				\
	return NULL;			\
}

#define for_each(T, __it, list) 	\
	__it = list ? list->data : NULL;\
	for (T##List * tmp=list;	\
	__it; tmp=tmp->next,		\
	__it=tmp?tmp->data:NULL)



#define MkList(T) CtrList##T
#define RmList(T) DtrList##T
#define GL(T) GetList##T

#define _st(T) struct _##T

#define DefCtr(T) _Ctr##T
#define DefCpy(T) _Cpy##T
#define DefDtr(T) _Dtr##T

#define Struct(T,__the__goods__) 	\
	_st(T);				\
	_st(T) { 			\
		Cpy(_st(T), _p);	\
		Dtr(_st(T), _d); 	\
		__the__goods__ 		\
	};				\
	typedef _st(T) T;		\
T * DefCpy(T)(T * old) {		\
	T * new;			\
	if (!(new=malloc(sizeof(T)))) { \
		return NULL;		\
	}				\
	memcpy(new, old, sizeof(T));	\
	return new;			\
}					\
void DefDtr(T)(T ** old) {		\
	ASSERT(old);			\
	ASSERT(*old);			\
	free(*old);			\
	old = NULL;			\
}					\
T* DefCtr(T)(size_t n, ...) {		\
	T * new;(void)n;		\
	if (!(new=malloc(sizeof(T)))) { \
		return NULL;		\
	}				\
	memset(new,0,sizeof(T));	\
	new->_p = DefCpy(T);		\
	new->_d = DefDtr(T);		\
	return new;			\
}					\
static void * _Mk##T = (void*)DefCtr(T);\
T* Mk##T(void *arg) {			\
	return				\
	((T*(*)(void*))_Mk##T)(arg); 	\
}					\
DefineList(T)
/* DefineArr(T); */

#define DefArrSize 32

#define CtrBoiler(T)			\
	if (!(new = DefCtr(T)(n))) {  	\
		return NULL;		\
	}				\


// Edge
Struct(Ed,
	UL from;
	UL to;
	UL key;
	UL data;
);


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
#define MkEd EdCtr

// Graph
Struct(Gr,
	UL v;
	EdList * e;
);

void GrDtr(Gr ** old) {
	RmList(Ed)(&(*old)->e);
	DefDtr(Gr)(old);
}

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
#define MkGr GrCtr



// State machine
Struct(Sm,
	Gr * g;
	UL s;
	UL p;

	struct ekv {
		UL v;
		UL d;
	} *m;

	struct ekvl ** alt;
	int tmdat;
);

#define EMAP(_s, _v, _k) (((_s)->m) + ((_s)->g->v + 1) * (_v) + (_k))

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
		/* *(new->m + sizeof(struct ekv) * new->p * new->l + sizeof(struct ekv) * e->key) = (struct ekv){ */
		*EMAP(new, e->from, e->key) = (struct ekv){.v = e->to, .d = e->data};
	}


	new->_d = SmDtr;
	new->tmdat = 0;

	return new;
}
#define  MkSm SmCtr

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

struct ekvl;
// list of edge key-value pairs to use as alternative transitions
struct ekvl {
	struct ekv data;
	struct ekvl * next;
};
char * ekvl_str(struct ekvl * ekvl);

enum Tmop { X=0, L=1, R=2 };

char * Tmop_strtab[] = {
	[X] = "No-op",
	[L] = "Left",
	[R] = "Right",
};

typedef union Tmdat {
	struct {
		enum Tmop op;
		UI dat;
	};
	UL data;
} Tmdat;

static inline Tmdat MkTmdat(enum Tmop op, UI dat) {
	return (Tmdat){ .op = op, .dat = dat };
}

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
				cnt += sprintf(buf + cnt, "%lu->%s ", j, ekvl_str(
					*(s->alt + i * (s->g->v + 1) + j)));
			}
		}
		cnt += sprintf(buf + cnt, "\n");
	}

	return buf;
}

// Deterministic Finite Automaton
Struct (Df,
	Sm * s;
	const char * n;
	UL a;
);
// realistically when will I need more than 64-1 states? I can always change this later

UL Df_mkmask(size_t n, ...) {
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

void DfDtr(Df ** old) {
	Rm((*old)->s);
	DefDtr(Df)(old);
}

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

#define MkDf DfCtr

// NFA parse tree
struct pt;
struct pt {
	UL c;
	UL d;
	struct pt ** kids;
	struct pt * parent;
	size_t kidcount;
	size_t kidcap;
};

#define KIDCAP_DEFAULT 8

struct altv {
	UL c;
	struct pt ** t;
};

#define ALTVLEN 64
// Nondeterministic Finite Automaton
Struct(Nf,
	Df * d;
	struct ekvl ** o;
	struct pt * t;
	struct altv altv[2][ALTVLEN]; // unlikely/impossible to be in more than 64 states at once
	size_t altc[2];
);

// 0 accept
int Nf_run(Nf * nf, UL *s, size_t n) {
	if (n < 1) {
		// if empty input, accept if start is an accept state
		return !(nf->d->a & (1UL << nf->d->s->s));
	}

	int res, xXx = 0;
	size_t i, j;
	struct ekvl * possible, *trans;
	struct altv * new;
	struct pt * curr, *root = (struct pt *)malloc(sizeof(struct pt));

	if (!root) {
		return 0;
	}
	root->c = s[0];
	root->d = 0;
	root->parent = NULL;
	root->kidcap = KIDCAP_DEFAULT;
	root->kids = (struct pt **)malloc(sizeof(struct pt *) * root->kidcap);
		/* static int bar= 0; */
		/* printf("bar: %d\n", bar++); */
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
					curr->kids = (struct pt **)realloc(curr->kids, curr->kidcap);
					if (!curr->kids) {
						return -1;
					}
				}


				// add a new child to the current tree and reference it in the new altv
				if (!(curr->kids[curr->kidcount] =
							(struct pt *)malloc(sizeof(struct pt)))) {
					// panic!
					return -1;
				}
				new->t = &curr->kids[curr->kidcount++];
				(*new->t)->parent = curr;
				(*new->t)->kidcap = KIDCAP_DEFAULT;
				(*new->t)->kids = (struct pt **)malloc(sizeof(struct pt *)
						* (*new->t)->kidcap);
				/* static int foo = 0; */
				/* printf("foo: %d\n", foo++); */
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


int ekvl_append(struct ekvl ** l, struct ekv d) {
	struct ekvl **p, *new = (struct ekvl *)malloc(sizeof(struct ekvl));
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

// arbitary number of edges (consoooms the edge)
struct ekvl ** Df_mkekvl(Df * d, size_t n, ...) {
	struct ekvl ** new;
	Ed * e;
	va_list list;
	size_t i;

	size_t vl = d->s->g->v + 1;
	new = (struct ekvl **)malloc(sizeof(struct ekvl *) * vl * vl);
	if (!new) {
		return NULL;
	}
	/* memset(new, 0UL, sizeof(struct ekvl *) * vl * vl); */
	for (i = 0; i < vl * vl; ++i) {
		new[i] = NULL;
	}
	/* printf("!!!!!%p\n", new[8]); */
	/* exit(1); */
	va_start(list, n);
	for (i = 1; i <= n; ++i) {
		e = va_arg(list, Ed *);
		ekvl_append(new + e->from * vl + e->key, (struct ekv){ .v = e->to, .d = e->data});
		Rm(e);
	}
	va_end(list);

	return new;
}

void ekvl_free(struct ekvl *** old, size_t len) {
	size_t i;

	struct ekvl ** ekvl = * old;
	struct ekvl * head, *tmp;

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

char * ekvl_str(struct ekvl * ekvl) {
	static char buf[_1K];
	struct ekvl * p;
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

void pt_free(struct pt ** old) {
	size_t i;
	if (old && *old) {
		for (i = 0; i < (*old)->kidcount; ++i) {
			pt_free(&(*old)->kids[i]);
		}
		free((*old)->kids);
		(*old)->kids = NULL;
	}
	free(*old);
	old = NULL;
}

char * pt_str(struct pt * t) {
	static char buf[_1K];
	static struct {
		struct pt * t;
		size_t i;
	} stack[_1K];
	size_t i, cnt = 0;
	int sp = 0;

	stack[sp].t = t;
	stack[sp].i = 0;

	while (sp >= 0) {
		// leaf
		if (stack[sp].t->kidcount == 0) {
			for (i = 0; (int)i < sp; ++i) {
				cnt += sprintf(buf + cnt, "    ");
			}
			cnt += sprintf(buf + cnt, "(%02lu, %02lu) (leaf)\n",
					stack[sp].t->c, stack[sp].t->d);
			/* for (i = 0; i < stack[sp].t->kidcount; ++i) { */
			/* 	cnt += sprintf(buf + cnt, "(%02lu, %02lu)%s", */
			/* 			stack[sp].t->c, stack[sp].t->d, */
			/* 			i == stack[sp].t->kidcount - 1 ? " (leaf)\n" : ", "); */
			/* } */
			// leaf was printed
			--sp;
		} else {
			// unprocessed kids
			if (stack[sp].i < stack[sp].t->kidcount) {
				// push next kid into stack to process its kids
				stack[sp + 1].t = stack[sp].t->kids[stack[sp].i++];
				stack[++sp].i = 0;
			// all kids are processed
			} else {
				for (i = 0; (int)i < sp; ++i) {
					cnt += sprintf(buf + cnt, "    ");
				}
				cnt += sprintf(buf + cnt, "(%02lu, %02lu)\n",
						stack[sp].t->c, stack[sp].t->d);
				--sp;
			}
		}

	}
	

	return buf;
}



void NfDtr(Nf ** old) {
	if ((*old)->t) {
		pt_free(&(*old)->t);
	}
	ekvl_free(&(*old)->o, ((*old)->d->s->g->v + 1) * ((*old)->d->s->g->v + 1));
	Rm((*old)->d);
	DefDtr(Nf)(old);
}

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
	new->o = va_arg(list, struct ekvl **);
	va_end(list);

	memset(new->altv, 0, 2 * sizeof(struct altv) * ALTVLEN);

	size_t vl = new->d->s->g->v + 1;
	struct ekv * e;
	for (i = 1; i < vl; ++i) for (j = 1; j < vl; ++j) {
	/* for_each(Ed, e, new->d->s->g->e) { */
		/* printf("process edge: (%lu,%lu)->(%lu,%lu)\n", e->from, e->key, e->to, e->data); */
		e = EMAP(new->d->s, i, j);
		/* printf("(%lu,%lu) e-v:%lu\n", i, j, e->v); */
		ekvl_append(new->o + i * vl + j,
				(struct ekv){ .v = e->v, .d = e->d });
	}
	new->d->s->alt = new->o;

	new->_d = NfDtr;

	return new;
}

#define MkNf NfCtr

// Tape
Struct(Tp,
	UL * t;
	size_t n;
	UL * h;
	size_t p;
);

void TpDtr(Tp ** old) {
	free((*old)->t);
	DefDtr(Tp)(old);
}

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

#define MkTp TpCtr

void Tp_left(Tp * t) {
	if (t->p > 0) {
		--t->h;
		--t->p;
	}
}

void Tp_right(Tp * t) {
	if (t->p >= t->n - 1) {
		t->t = (UL *)realloc(t->t, sizeof(UL) * (++t->n));
		if (!t->t) {
			// panic
			printf("panic: the tape is gone!\n");
			exit(1);
		}
		t->t[t->n - 1] = 0UL;
	}
	++t->h;
	++t->p;
}

// Turing machine
Struct(Tm,
	Df * d;
	Tp * t;
	UL r;
);

void TmDtr(Tm ** old) {
	Rm((*old)->d);
	Rm((*old)->t);
	DefDtr(Tm)(old);
}

// args: dfa, tape, rejection bitmask
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
	return new;
}

#define MkTm TmCtr

char * Tm_str(Tm * t) {
	static char buf[_1K];
	UL cnt = 0, i;
	int firstchar;
	
	cnt += sprintf(buf, "Turing machine \"%s\"\nTape: ", t->d->n);
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

int Tm_accepts(Tm * tm) {
	return (tm->d->s->p & tm->d->a);
}

int Tm_rejects(Tm * tm) {
	return (tm->d->s->p & tm->r);
}

#define PANICNUMBER (1 << 8)

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

	int accepts, rejects;
	size_t tpos = 0, i = 0;
	Tmdat code;

	while (!(accepts = Tm_accepts(tm)) && !(rejects = Tm_rejects(tm)) && i++ < PANICNUMBER) {
		code = (Tmdat)Sm_do(tm->d->s, *tm->t->h);
		printf("(%lu) LOAD Tape[%lu] = %s", i, tpos, Tmdat_str(code));

		// Writing the blank character implements no-op (0UL)
		if (code.dat != 0) {
			*tm->t->h = code.dat;
		}

		switch (code.op) {
		case L:
			printf(" (LEFT)\n");
			Tp_left(tm->t);
			break;
		case R:
			printf(" (RIGHT)\n");
			Tp_right(tm->t);
		case X:
		default:
			printf("\n");
			break;
		}

		printf("%s", Tm_str(tm));
	}

	return 0;
}

int main(void) {
	/* Tm * t = MkTm(NULL); */
	/* Tm * q = Cp(t); */

	/* Ed * e3 = MkEd(4,3,3,3,3); */
	/* Rm(e3); */

	EdList * el = MkList(Ed)(2,
			MkEd(4,1,2,1,1),
			MkEd(4,2,1,1,1)
	);
			
	/* Ed * ea; */
	/* for_each(Ed, ea, el) { */
	/* 	printf("e: from %lu on %lu goto %lu do %lu\n", ea->from, ea->key, ea->to, ea->data); */
	/* } */


	UL vmax = 2;
	Gr * g = MkGr(2, vmax, el);

	Sm * s = MkSm(2, g, 1);

	printf("%s\n", Sm_str(s));

	/* UL accept = 2; */
	Df * d = MkDf(3, s, "test", Df_mkmask(1, 2));

	struct ekvl ** ekvl = Df_mkekvl(d, 1, MkEd(4, 1,1,1,0));

	UL input[] = {1, 1};
	/* UL input[] = {1, 1, 2}; */
	Df_run(d, input, 2);

	Nf * n = MkNf(2, d, ekvl);

	printf("%s\n", Sm_str(n->d->s));

	Nf_run(n, input, 2);

	/* Rm(g); */

	/* Rm(d); */

	printf("pt:\n%s", pt_str(n->t));

	Rm(n);

	/* RmList(Ed)(&el); */

	/* Rm(t); */
	/* Rm(q); */

	Tm * tm = MkTm(2, MkTp(3,1,1,1),
		MkDf(3, MkSm(2,
			MkGr(2, 2,
				MkList(Ed)(1, MkEd(4,1,1,1,MkTmdat(2, L))
		    )), 1), "TM test", Df_mkmask(1,2)), Df_mkmask(0,0));

	printf("%s\n", Tm_str(tm));

	Tm_run(tm);

	Rm(tm);

	/* Tp * tp = MkTp(3,1,2,1); */
	/* printf("tp: %p\n", tp); */
	/* Rm(tp); */


	return 0;
}
