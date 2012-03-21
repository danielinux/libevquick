#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "libevquick.h"
#include <errno.h>
void enabler(void *arg);

void recurrent(void *arg)
{
	printf("Recurrent timer elapsed!\n");
}

void disabler(void *arg)
{
	evquick_timer *en, *rec = (evquick_timer *) arg;
	printf("Disabler timer elapsed!\n");
	evquick_deltimer(rec);
	en = evquick_addtimer(1000, 0, enabler, NULL);
}

void enabler(void *arg)
{
	evquick_timer *rec = evquick_addtimer(100, EVQUICK_EV_RETRIGGER, recurrent, NULL);
	evquick_timer *dis = evquick_addtimer(1000, 0, disabler, rec);
	printf("Enabler timer elapsed!\n");
}


struct reader_obj {
	evquick_event *ev;
	int counter;
};

void reader_enable(void *arg);
void reader(int fd, short rev, void *arg)
{
	char in;
	int r = read(fd, &in, 1);
	struct reader_obj *e_stdin = (struct reader_obj *)arg;
	if (r <= 0)
		fprintf(stderr, "Read error in reader: %s \n", strerror(errno));
	else
		printf("Total char received so far: %d\n", ++(e_stdin->counter));
	if (e_stdin->counter > 10) {
		printf("####### STDIN suspended for 2 seconds #######\n");
		evquick_delevent(e_stdin->ev);
		printf("Event deleted.\n");
		evquick_addtimer(2000, 0, reader_enable, arg);
		printf("Timer added.\n");

	}
}

void reader_enable(void *arg)
{
	struct reader_obj *e_stdin = (struct reader_obj *)arg;
	char buf[1024];
	printf("Reader re-enabled, counter reset.\n");
	e_stdin->counter = 0;
	e_stdin->ev = evquick_addevent(STDIN_FILENO, EVQUICK_EV_READ, reader, NULL, arg);
}


int main(void)
{
	evquick_timer *t_disabler;
	evquick_timer *t_recurrent;
	struct reader_obj e_stdin;

	if(evquick_init() < 0)
		exit(2);

	t_recurrent = evquick_addtimer(100, EVQUICK_EV_RETRIGGER, recurrent, NULL);
	t_disabler = evquick_addtimer(1000, 0, disabler, t_recurrent);
	e_stdin.counter = 0;
	e_stdin.ev = evquick_addevent(STDIN_FILENO, EVQUICK_EV_READ, reader, NULL, &e_stdin);

	evquick_loop();
	exit(0);

}
