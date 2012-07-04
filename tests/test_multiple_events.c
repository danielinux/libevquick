#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "libevquick.h"
#include <errno.h>

#define N_EVENTS 10

evquick_event *ev_array[N_EVENTS];
void error(int fd, short rev, void *arg)
{
  printf("Hello from error\n");
}

void reader(int fd, short rev, void *arg)
{
  char c;
  int pos = *(int*)arg;
  read(fd, &c, 1);
  printf("Hello from reader %d\n", pos);
  evquick_delevent(ev_array[pos]);
	ev_array[pos] = evquick_addevent(fd, EVQUICK_EV_READ, reader, error, arg);
}


void recurrent(void *arg)
{
  char content = 'c';
  write((*(int*)arg), &content, 1);
}

int main(void)
{

  int i = 0;
  int pipefd[2];


	if(evquick_init() < 0)
		exit(2);

  for (i = 0; i < N_EVENTS; i++)
  {
    int *arg = malloc(sizeof(int));
    int *argfd = malloc(sizeof(int));
    if(pipe(pipefd) != 0) {
      perror("pipe");
      exit(1);
    }
    *arg = i;
    *argfd = pipefd[1];
	  ev_array[i] = evquick_addevent(pipefd[0], EVQUICK_EV_READ, reader, error, arg);
    evquick_addtimer(1000, EVQUICK_EV_RETRIGGER, recurrent, argfd);
  }
  evquick_loop();
  return 0;
}
