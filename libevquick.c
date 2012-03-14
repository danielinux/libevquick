/*************************************************************
 *          Libevquick - event wrapper library
 *   (c) 2012 Daniele Lacamera <root@danielinux.net>
 *              Released under the terms of
 *            GNU Lesser Public License (LGPL)
 *              Version 2.1, February 1999
 *
 *             see COPYING for more details
 */
#include <sys/poll.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include "heap.h"
#include "libevquick.h"


struct evquick_timer_instance
{
	unsigned long long expire;
	struct evquick_timer *ev_timer;
};
typedef struct evquick_timer_instance evquick_timer_instance;

DECLARE_HEAP(evquick_timer_instance, expire);

struct evquick_ctx
{
	int interrupt;
	int changed;
	int n_events;
	int last_served;
	struct pollfd *pfd;
	struct evquick_event *events;
	struct evquick_event *_array;
	heap_evquick_timer_instance *timers;
};

static struct evquick_ctx *ctx;

evquick_event *evquick_addevent(int fd, short events,
	void (*callback)(int fd, short revents, void *arg),
	void (*err_callback)(int fd, short revents, void *arg),
	void *arg)
{
	evquick_event *e = malloc(sizeof(evquick_event));
	if (!e)
		return e;
	e->fd = fd;
	e->events = events;
	e->callback = callback;
	e->err_callback = err_callback;
	e->arg = arg;

	ctx->changed = 1;

	e->next = ctx->events;
	ctx->events = e;
	ctx->n_events++;
	return e;
}

void evquick_delevent(evquick_event *e)
{
	evquick_event *cur, *prev;
	cur = ctx->events;
	prev = NULL;
	while(cur) {
		if (cur == e) {
			if (!prev)
				ctx->events = e->next;
			 else
				prev->next = e->next;
			free(e);
		}
		prev = cur;
		cur = cur->next;
	}
	ctx->n_events--;
	ctx->changed = 1;
}

static void timer_trigger(evquick_timer *t, unsigned long long now,
	unsigned long long expire)
{
	evquick_timer_instance tev, *first;
	tev.ev_timer = t;
	tev.expire = expire;
	heap_insert(ctx->timers, &tev);
	first = heap_first(ctx->timers);
	if (first) {
		unsigned long long interval = (first->expire - now);
		if (interval >= 1000)
			alarm(interval / 1000);
		else
			ualarm((useconds_t)(1000 * (first->expire - now)), 0);
	}
}

static unsigned long long gettimeofdayms(void)
{
	struct timeval tv;
	unsigned long long ret;
	gettimeofday(&tv, NULL);
	ret = (unsigned long long)tv.tv_sec * 1000ULL;
	ret += (unsigned long long)tv.tv_usec / 1000ULL;
	return ret;
}


evquick_timer *evquick_addtimer(
	unsigned long long interval, short flags,
	void (*callback)(void *arg),
	void *arg)
{
	unsigned long long now = gettimeofdayms();

	evquick_timer *t = malloc(sizeof(evquick_timer));
	if (!t)
		return t;
	t->interval = interval;
	t->flags = flags;
	t->callback = callback;
	t->arg = arg;
	timer_trigger(t, now, now + t->interval);
	return t;
}


void evquick_deltimer(evquick_timer *t)
{
	t->flags |= EVQUICK_EV_DISABLED;
}




void give_me_a_break(int signo)
{
	if (signo == SIGALRM)
		ctx->interrupt = 1;
}

int evquick_init(void)
{
	ctx = calloc(1, sizeof(struct evquick_ctx));
	if (!ctx)
		return -1;
	ctx->timers = heap_init();
	if (!ctx->timers)
		return -1;
	signal(SIGALRM, give_me_a_break);
	return 0;
}

static void rebuild_poll(void)
{
	int i = 0;
	evquick_event *e = ctx->events;
	if (ctx->n_events == 0) {
		ctx->pfd = NULL;
		return;
	}
	ctx->pfd = realloc(ctx->pfd, sizeof(struct pollfd) * ctx->n_events);
	ctx->_array = realloc(ctx->_array, sizeof(evquick_event) * ctx->n_events);
	if ((!ctx->pfd) || (!ctx->_array)) {
		/* TODO: notify error, events are disabled.
		 * perhaps provide a context-wide callback for errors.
		 */
		ctx->n_events = 0;
		ctx->changed = 0;
		return;
	}

	while(e) {
		memcpy(ctx->_array + i, e, sizeof(evquick_event));
		ctx->pfd[i].fd = e->fd;
		ctx->pfd[i++].events = (e->events & (POLLIN | POLLOUT)) | (POLLHUP | POLLERR);
		e = e->next;
	}
	ctx->last_served = 0;
	ctx->changed = 0;
}

static void serve_event(int n)
{
	evquick_event *e = ctx->_array + n;
	if (e) {
		ctx->last_served = n;
		if ((ctx->pfd[n].revents & (POLLHUP | POLLERR)) && e->err_callback)
			e->err_callback(e->fd, ctx->pfd[n].revents, e->arg);
		else {
			e->callback(e->fd, ctx->pfd[n].revents, e->arg);
		}
	}
}


void evquick_loop(void)
{
	int pollret, i; 
	evquick_timer_instance t, *first;
	while(1) {
		if (ctx->changed) {
			rebuild_poll();
		}
		if (ctx->n_events == 0) {
			sleep(3600);
			goto timer_check;
		}
		pollret = poll(ctx->pfd, ctx->n_events, 3600 * 1000);
		if (pollret <= 0)
			goto timer_check;



		for (i = ctx->last_served +1; i < ctx->n_events; i++) {
			if (ctx->pfd[i].revents != 0) {
				serve_event(i);
				goto timer_check;
			}
		}
		for (i = 0; i <= ctx->last_served; i++) {
			if (ctx->pfd[i].revents != 0) {
				serve_event(i);
				goto timer_check;
			}
		}

	timer_check:
		if (ctx->interrupt) {
			unsigned long long now = gettimeofdayms();
			ctx->interrupt = 0;
			first = heap_first(ctx->timers);
			while(first && (first->expire <= now)) {
				heap_peek(ctx->timers, &t);
				if (t.ev_timer->flags & EVQUICK_EV_DISABLED) {
					/* Timer was disabled in the meanwhile.
					 * Take no action, and destroy it.
					 */
					free(t.ev_timer);
				} else if (t.ev_timer->flags & EVQUICK_EV_RETRIGGER) {
					timer_trigger(t.ev_timer, now, now + t.ev_timer->interval);
					t.ev_timer->callback(t.ev_timer->arg);
					/* Don't free the timer, reuse for next instance
					 * that has just been scheduled.
					 */
				} else {
					/* One shot, invoke callback,
					 * then destroy the timer. */
					t.ev_timer->callback(t.ev_timer->arg);
					free(t.ev_timer);
				}
				first = heap_first(ctx->timers);
			}
			if(first) {
				unsigned long long interval = first->expire - now;
				if (interval >= 1000)
					alarm(interval / 1000);
				else
					ualarm((useconds_t) (1000 * (first->expire - now)), 0);
			}
		}
	} /* while loop */
}
