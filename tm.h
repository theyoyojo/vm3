#ifndef TM_H
#define TM_H

#include <stdio.h>
#include <stdint.h>

typedef uint64_t 	UL;
typedef uint32_t 	UI;

#define _1K 		(1UL << 10)

#define ASSERT(__x) 	assert(__x)

#define Ctr(T,__c) 	T * (*__c)(size_t n, ...)
#define Cpy(T,__p) 	T * (*__p)(T*)
#define Dtr(T,__d) 	void (*__d)(T**)

#define Cp(__x) 	__x->_p(__x)
#define Rm(__x) 	__x->_d(&__x)

#define DeclList(T)			\
struct _##T##List;			\
typedef struct _##T##List {		\
	T * data;			\
	struct _##T##List * next;	\
} T##List;				\
T##List * CtrList##T(size_t n, ...);	\
void DtrList##T(T##List ** old);	\
T * GetList##T(T##List * l, size_t n);	\

#define for_each(T, __it, list) 	\
	__it = list ? list->data : NULL;\
	for (T##List * tmp=list;	\
	__it; tmp=tmp->next,		\
	__it=tmp?tmp->data:NULL)


#define DefineList(T)			\
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
T * GetList##T(T##List * l, size_t n) { \
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



#define MkList(T) 	CtrList##T
#define RmList(T) 	DtrList##T
#define GL(T) 		GetList##T

#define _st(T) 		struct _##T

#define DefCtr(T) 	_Ctr##T
#define DefCpy(T) 	_Cpy##T
#define DefDtr(T) 	_Dtr##T

#define DeclStruct(T,__the__goods__) 	\
	_st(T);				\
	_st(T) { 			\
		Cpy(_st(T), _p);	\
		Dtr(_st(T), _d); 	\
		__the__goods__ 		\
	};				\
	typedef _st(T) T;		\
T * DefCpy(T)(T * old);		\
void DefDtr(T)(T ** old);	\
T* DefCtr(T)(size_t n, ...);	\
DeclList(T);

#define DefineStruct(T)			\
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
DefineList(T);

#define CtrBoiler(T)			\
	if (!(new = DefCtr(T)(n))) {  	\
		return NULL;		\
	}				\

// Edge
DeclStruct(Ed,
	UL from;
	UL to;
	UL key;
	UL data;
);
Ed * 		EdCtr		(size_t n, ...);
#define MkEd 	EdCtr
void EdDtr			(Ed ** old);

// Graph
DeclStruct(Gr,
	UL v;
	EdList * e;
);
Gr * 		GrCtr		(size_t n, ...);
#define MkGr 	GrCtr
void 		GrDtr		(Gr ** old);
#define EMAP(_s, _v, _k) 	(((_s)->m) + ((_s)->g->v + 1) * (_v) + (_k))

// State machine
DeclStruct(Sm,
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
Sm * 		SmCtr		(size_t n, ...);
#define MkSm 	SmCtr
void 		SmDtr		(Sm ** old);
char * 		Sm_str		(Sm * s);
UL 		Sm_do		(Sm * s, UL d);

enum Tmop { X=0, L=1, R=2, TMOPCOUNT };

typedef union Tmdat {
	struct {
		enum Tmop op;
		UI dat;
	};
	UL data;
} Tmdat;

char * 		Tmdat_str	(Tmdat d);

static inline Tmdat MkTmdat(UI dat, enum Tmop op) {
	return (Tmdat){ .op = op, .dat = dat };
}

// Deterministic Finite Automaton
DeclStruct(Df,
	Sm * s;
	const char * n;
	UL a;
);
Df * 		DfCtr		(size_t n, ...);
#define MkDf 	DfCtr
void 		DfDtr		(Df ** old);
UL 		Df_mkmask	(size_t n, ...);
int 		Df_run		(Df * d, UL * s, size_t n);

struct ekvl;
// list of edge key-value pairs to use as alternative transitions
typedef struct ekvl {
	struct ekv data;
	struct ekvl * next;
} * Ekvl;
Ekvl * 		EkvlCtr		(UL v, size_t n, ...);
#define MkEkvl	EkvlCtr
void 		EkvlDtr		(Ekvl ** old, size_t len /* TODO */);
int 		Ekvl_append	(Ekvl * l, struct ekv d);
char * 		Ekvl_str	(Ekvl ekvl);


// NFA parse tree
struct pt;
typedef struct pt {
	UL c;
	UL d;
	struct pt ** kids;
	struct pt * parent;
	size_t kidcount;
	size_t kidcap;
} * Pt;
char * 		Pt_str		(Pt t);

#define KIDCAP_DEFAULT 8

struct altv {
	UL c;
	struct pt ** t;
};

#define ALTVLEN 64
// Nondeterministic Finite Automaton
DeclStruct(Nf,
	Df * d;
	struct ekvl ** o;
	struct pt * t;
	struct altv altv[2][ALTVLEN]; // unlikely/impossible to be in more than 64 states at once
	size_t altc[2];
);
Nf * 		NfCtr		(size_t n, ...);
#define MkNf 	NfCtr
void 		NfDtr		(Nf ** old);
int 		Nf_run		(Nf * nf, UL *s, size_t n);

// Tape
DeclStruct(Tp,
	UL * t;
	size_t n;
	UL * h;
	size_t p;
);

Tp * 		TpCtr		(size_t n, ...);
#define MkTp 	TpCtr
void 		TpDtr		(Tp ** old);

// Turing machine
DeclStruct(Tm,
	Df * d;
	Tp * t;
	UL r;
);
Tm * 		TmCtr		(size_t n, ...);
#define MkTm 	TmCtr
void 		TmDtr		(Tm ** old);
char * 		Tm_str		(Tm * t);
int 		Tm_run		(Tm * tm);

struct tmgen {
	Tp * tape;
	EdList * edges;
	UL vertices,
	   start,
	   acceptmask,
	   rejectmask;
	char * name;
};

static inline Tm * Tm_gen(struct tmgen * g) {
	return MkTm(2, g->tape, MkDf(3, MkSm(2, MkGr(2, g->vertices, g->edges),
		    g->start), g->name, g->acceptmask), g->rejectmask);
}


#endif /* TM_H */
