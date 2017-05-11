#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "libevquick.h"
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

pthread_t T0, T1;

void enabler0(void *arg);
void disabler0(void *arg);

void enabler1(void *arg);
void disabler1(void *arg);

void recurrent0(void *arg)
{
    assert(pthread_self() == T0);
	printf("Recurrent timer 0 elapsed!\n");
}

void recurrent1(void *arg)
{
    assert(pthread_self() == T1);
	printf("Recurrent timer 1 elapsed!\n");
}

void disabler0(void *arg)
{
	evquick_timer *en, *rec = (evquick_timer *) arg;
    assert(pthread_self() == T0);
	printf("Disabler 0 timer elapsed!\n");
	evquick_deltimer(rec);
	en = evquick_addtimer(1000, 0, enabler0, NULL);
}

void enabler0(void *arg)
{
    assert(pthread_self() == T0);
	evquick_timer *rec = evquick_addtimer(100, EVQUICK_EV_RETRIGGER, recurrent0, NULL);
	evquick_timer *dis = evquick_addtimer(1000, 0, disabler1, rec);
	printf("Enabler timer elapsed!\n");
}

void disabler1(void *arg)
{
	evquick_timer *en, *rec = (evquick_timer *) arg;
    assert(pthread_self() == T1);
	printf("Disabler 1 timer elapsed!\n");
	evquick_deltimer(rec);
	en = evquick_addtimer(1000, 0, enabler1, NULL);
}

void enabler1(void *arg)
{
    assert(pthread_self() == T1);
	evquick_timer *rec = evquick_addtimer(100, EVQUICK_EV_RETRIGGER, recurrent1, NULL);
	evquick_timer *dis = evquick_addtimer(1000, 0, disabler1, rec);
	printf("Enabler timer elapsed!\n");
}



struct reader_obj {
	evquick_event *ev;
	int counter;
};

void reader0_enable(void *arg);
void reader1_enable(void *arg);
void reader0(int fd, short rev, void *arg)
{
	char in;
	int r = read(fd, &in, 1);
	struct reader_obj *e_stdin = (struct reader_obj *)arg;

    assert(pthread_self() == T0);
	if (r <= 0)
		fprintf(stderr, "Read error in reader: %s \n", strerror(errno));
	else
		printf("Total char received so far: %d\n", ++(e_stdin->counter));
	if (e_stdin->counter > 10) {
		printf("####### thread 0: STDIN suspended for 2 seconds #######\n");
		evquick_delevent(e_stdin->ev);
		printf("Event deleted.\n");
		evquick_addtimer(3000, 0, reader0_enable, arg);
		printf("Timer added.\n");

	}
}

void reader1(int fd, short rev, void *arg)
{
	char in;
	int r = read(fd, &in, 1);
	struct reader_obj *e_stdin = (struct reader_obj *)arg;

    assert(pthread_self() == T1);
	if (r <= 0)
		fprintf(stderr, "Read error in reader: %s \n", strerror(errno));
	else
		printf("Total char received so far: %d\n", ++(e_stdin->counter));
	if (e_stdin->counter > 10) {
		printf("####### thread 1: STDIN suspended for 2 seconds #######\n");
		evquick_delevent(e_stdin->ev);
		printf("Event deleted.\n");
		evquick_addtimer(5000, 0, reader1_enable, arg);
		printf("Timer added.\n");

	}
}

void reader0_enable(void *arg)
{
	struct reader_obj *e_stdin = (struct reader_obj *)arg;
	char buf[1024];
    assert(pthread_self() == T0);
	printf("Reader re-enabled, counter reset.\n");
	e_stdin->counter = 0;
	e_stdin->ev = evquick_addevent(STDIN_FILENO, EVQUICK_EV_READ, reader0, NULL, arg);
}

void reader1_enable(void *arg)
{
	struct reader_obj *e_stdin = (struct reader_obj *)arg;
	char buf[1024];
    assert(pthread_self() == T1);
	printf("Reader re-enabled, counter reset.\n");
	e_stdin->counter = 0;
	e_stdin->ev = evquick_addevent(STDIN_FILENO, EVQUICK_EV_READ, reader1, NULL, arg);
}


void t0_init(void*arg)
{
	evquick_timer *t_disabler;
	evquick_timer *t_recurrent;
	struct reader_obj e_stdin;

	if(evquick_init() < 0)
		exit(2);

	t_recurrent = evquick_addtimer(100, EVQUICK_EV_RETRIGGER, recurrent0, NULL);
	t_disabler = evquick_addtimer(1000, 0, disabler0, t_recurrent);
	e_stdin.counter = 0;
	e_stdin.ev = evquick_addevent(STDIN_FILENO, EVQUICK_EV_READ, reader0, NULL, &e_stdin);
	evquick_loop();
}

void t1_init(void*arg)
{
	evquick_timer *t_disabler;
	evquick_timer *t_recurrent;
	struct reader_obj e_stdin;

	if(evquick_init() < 0)
		exit(2);

	t_recurrent = evquick_addtimer(100, EVQUICK_EV_RETRIGGER, recurrent1, NULL);
	t_disabler = evquick_addtimer(1000, 0, disabler1, t_recurrent);
	e_stdin.counter = 0;
	//e_stdin.ev = evquick_addevent(STDIN_FILENO, EVQUICK_EV_READ, reader1, NULL, &e_stdin);
	evquick_loop();
}


int main(void) 
{
    if (pthread_create(&T0, NULL, t0_init, NULL) != 0) {
        perror("pthread_create");
        exit(1);
    }
    if (pthread_create(&T1, NULL, t1_init, NULL) != 0) {
        perror("pthread_create");
        exit(1);
    }

    pthread_join(T0, NULL);
    pthread_join(T1, NULL);
    exit(0);

}
