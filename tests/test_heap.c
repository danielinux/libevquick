#include <stdio.h>
typedef struct {
	int somedata;
	unsigned long long order;
	int someotherdata;
} xxx;

#include "heap.h"

DECLARE_HEAP(xxx,order);

int main(void) {
	heap_xxx *h;
	xxx a = {30,4ULL,50}; 
	xxx b = {45,11ULL,51}; 
	xxx c = {25,2ULL,52}; 
	xxx d = {50,25ULL,53}; 
	xxx e = {50,14ULL,53}; 
	xxx f = {50,32ULL,53}; 
	xxx g = {50,33ULL,53}; 
	xxx i = {50,1ULL,53}; 
	h = heap_init();
	heap_insert(h, &a);
	heap_insert(h, &b);
	heap_insert(h, &c);
	heap_insert(h, &d);
	heap_insert(h, &e);
	heap_insert(h, &f);
	heap_insert(h, &g);
	heap_insert(h, &i);

	printf("First is %llu\n", heap_first(h)->order);
	// reuse a
	while(heap_peek(h, &a) == 0) {
		printf("Peek %llu\n", a.order);
		a.order = a.order+1000;
		heap_insert(h, &a);
	}
}
