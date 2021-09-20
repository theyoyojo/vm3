#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef unsigned long UL;

#define ASSERT(__x) assert(__x)

#define Ctr(T,__c) T * (*__c)(void*)
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
		printf("data: %p\n", (*next)->data);\
		next = &(*next)->next;	\
	}				\
	va_end(list);			\
	return head;			\
}					\
void DtrList##T(T##List ** old)	{	\
	ASSERT(old);			\
	T##List *tmp, *head = *old;	\
	while (head) {			\
		tmp=head->next;		\
		Rm(head->data);		\
		free(head);		\
		head=tmp;		\
	}				\
	*old = NULL;			\
}

#define for_each(T, __it, list) 	\
	__it = list ? list->data : NULL;\
	for (T##List * tmp=list;	\
	__it; tmp=tmp->next,		\
	__it=tmp?tmp->data:NULL)



#define MkList(T) CtrList##T
#define RmList(T) DtrList##T

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
T* DefCtr(T)(void*_) {			\
	T * new;(void)_;		\
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
	if (!(new = DefCtr(T)(arg))) {  \
		return NULL;		\
	}				\


// Edge
Struct(Ed,
	UL from;
	UL to;
	UL key;
	UL data;
);


Ed * EdCtr(void * arg) {
	Ed * new;
	CtrBoiler(Ed);
	
	UL * data = (UL *)arg;

	new->from 	= data[0];
	new->to 	= data[1];
	new->key 	= data[2];
	new->data 	= data[3];

	return new;
}
#define MkEd EdCtr

struct EdPreInit {
	union {
		UL data[6];
		struct {
			UL head[2];
			Ed e;
		};
	};
};

// Vertex
Struct(Vr,
	UL id;
);

Vr * VrCtr(void * arg) {
	Vr * new;
	CtrBoiler(Vr);

	new->id = (UL)arg;
	
	return new;
}
#define MkVr VrCtr

// Graph
Struct(Gr,
	Vr * v;
	Ed * e;
);

Gr * GrCtr(void * arg) {
	Gr * new;
	CtrBoiler(Gr);

	new->v = (Vr *)arg;
	new->e = (Ed *)(arg + sizeof(Vr*));

	return new;
}

#define MKGr GrCtr


// Tape
Struct(Tp,
	int tmp ;
);

// State machine
Struct(Sm,
	Gr g;
	Vr * s;
);

// Turing machine
Struct(Tm,
	Tp t;
	Sm s;
);

int main(void) {
	printf("hello world\n");

	Tm * t = MkTm(NULL);
	Tm * q = Cp(t);

	UL data[2][4] = {
		{1,2,3,4},
		{5,6,7,8},};

	/* Ed2 */

	/* Ed * ea = MkArr(Ed)(2, MkEd(data[0]), MkEd(data[1])); */

	EdList * el = MkList(Ed)(2, MkEd(data[0]), MkEd(data[1]));
			
	/* VrArr va = CtrArrVr(2); */

	/* Ed * e = MkEd((void*)edge); */

	Ed * ea;
	for_each(Ed, ea, el) {
		printf("e: from %lu on %lu goto %lu do %lu\n", ea->from, ea->key, ea->to, ea->data);
	}

	/* Rm(e); */

	RmList(Ed)(&el);

	Rm(t);
	Rm(q);


	return 0;
}
