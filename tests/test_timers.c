#include "libevquick.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NUMTIMERS 80

void timer_cb(void *arg)
{
	int *n = (int *)arg;
	printf("Timer %d elapsed!\n", *n);
	free(arg);
}

void register_timer1(evquick_timer *t, int time, int e)
{

	int *perm = malloc(sizeof(int));
	memcpy(perm, &e, sizeof(int));
	printf("%d (in %d ms)\n", *perm, time);
	t = evquick_addtimer(time, 0, timer_cb, perm);
}

void register_timer2(int time, int e, evquick_timer *t)
{
	register_timer1(t, time, e);
}

int main(void)
{
	int i;
	evquick_timer *timers[NUMTIMERS];
	evquick_timer *retriggered[NUMTIMERS];
	if(evquick_init() < 0)
		exit(2);

	for (i = 0; i < NUMTIMERS; i++) {
		if (i % 2)
			register_timer1(timers[i], i*100, i);
		else
			register_timer2(i*200, i, timers[i]);
	}
	evquick_loop();
	exit(0);
}
