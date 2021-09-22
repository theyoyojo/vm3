#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef unsigned long UL;

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

// Vertex
Struct(Vr,
	UL id;
);

Vr * VrCtr(size_t n, ...) {
	Vr * new;
	va_list list;

	// nothing but ID
	if (n != 1) return NULL;

	CtrBoiler(Vr);

	va_start(list, n);
	new->id = va_arg(list, UL);
	va_end(list);
	
	return new;
}
#define MkVr VrCtr

// Graph
Struct(Gr,
	VrList * v;
	EdList * e;
	UL vl;
	Vr ** vm;
);

void GrDtr(Gr ** old) {
	RmList(Ed)(&(*old)->e);
	RmList(Vr)(&(*old)->v);
	free((*old)->vm); (*old)->vm = NULL;
	DefDtr(Gr)(old);
}

Gr * GrCtr(size_t n, ...) {
	Gr * new;
	va_list list;

	CtrBoiler(Gr);

	va_start(list, n);
	new->v = va_arg(list, VrList *);
	new->e = va_arg(list, EdList *);
	va_end(list);

	Vr * v;
	new->vl = 0;
	for_each(Vr, v, new->v) {
		if (v->id > new->vl) {
			new->vl = v->id;
		}
	}


	new->vm = (Vr **)malloc(sizeof(Vr *) * new->vl + 1);
	if (!new->vm) {
		free(new); return NULL;
	}
	for_each(Vr, v, new->v) {
		printf("id: %lu\n", v->id);
		new->vm[v->id] = v;
	}

	new->_d = GrDtr;

	return new;
}
#define MkGr GrCtr

Vr * Gr_v(Gr * g, UL d) {
	if (d <= 0 || d > g->vl) {
		return NULL;
	}
	return g->vm[d];
}



// Tape
Struct(Tp,
	UL * t;
	size_t n;
);

// State machine
Struct(Sm,
	Gr * g;
	UL l;
	Vr * s;
	Vr * c;

	struct ekv {
		Vr * n;
		UL d;
	} *m;

);

void SmDtr(Sm ** old) {
	Rm((*old)->g);
	free((*old)->m);
	DefDtr(Sm)(old);
}

// graph, state alphabet max, start state
Sm * SmCtr(size_t n, ...) {
	Sm * new;
	va_list list;

	if (n != 3) {
		printf("%s: bad argc\n", __func__);
		return NULL;
	}

	CtrBoiler(Sm);

	va_start(list, n);

	new->g = va_arg(list, Gr *);
	new->l = va_arg(list, UL);
	UL sid = va_arg(list, UL);
	new->s = Gr_v(new->g, sid);

	va_end(list);

	new->c = new->s;

	// l by l edgemap (from l vertices we have l edges)
	new->m = (struct ekv *)malloc(sizeof(struct ekv) * new->l * new->l);
	if (!new->m) {
		free(new); new = NULL;
	}

	memset(new->m, 0, sizeof(struct ekv) * new->l * new->l);

	Ed * e;
	for_each(Ed, e, new->g->e) {
		*(new->m + new->c->id * new->l + e->key) = (struct ekv){
			.n = Gr_v(new->g, e->to), .d = e->data};
	}


	new->_d = SmDtr;

	return new;
}
#define  MkSm SmCtr

UL Sm_do(Sm * s, UL d) {
	// must be positive int not greater than limit
	if (d <= 0 || d > s->l) {
		return 0;
	}

	struct ekv * next;

	next = s->m + s->c->id * s->l * sizeof(struct ekv) + d * sizeof(struct ekv);

	s->c = next->n;
	return next->d;
}

// Turing machine
Struct(Tm,
	Tp t;
	Sm s;
);


int main(void) {
	printf("hello world\n");

	Tm * t = MkTm(NULL);
	Tm * q = Cp(t);

	Ed * e3 = MkEd(4,3,3,3,3);
	Rm(e3);

	EdList * el = MkList(Ed)(2, MkEd(4,1,2,3,4), MkEd(4,5,6,7,8));
			
	Ed * ea;
	for_each(Ed, ea, el) {
		printf("e: from %lu on %lu goto %lu do %lu\n", ea->from, ea->key, ea->to, ea->data);
	}

	VrList * vl = MkList(Vr)(2, MkVr(1,5), MkVr(1,6));

	Gr * g = MkGr(2, vl, el);

	Sm * s = MkSm(3, g, 7, 1);

	/* Rm(g); */

	Rm(s);

	/* RmList(Ed)(&el); */

	Rm(t);
	Rm(q);


	return 0;
}
