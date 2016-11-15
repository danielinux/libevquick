/*************************************************************
 *          Libevquick - event wrapper library
 *   (c) 2012 Daniele Lacamera <root@danielinux.net>
 *              Released under the terms of
 *            GNU Lesser Public License (LGPL)
 *              Version 2.1, February 1999
 *
 *             see COPYING for more details
 */
#ifndef __LIBEVQUICK
#define __LIBEVQUICK
#include <sys/poll.h>
#define EVQUICK_EV_READ POLLIN
#define EVQUICK_EV_WRITE POLLOUT
#define EVQUICK_EV_RETRIGGER 0x4000

struct evquick_event;
struct evquick_timer;
typedef struct evquick_event evquick_event;
typedef struct evquick_timer evquick_timer;

/* Initialize evquick loop
 * =========
 * To be called before any other function.
 *
 * Returns: 0 upon success, -1 otherwise.
 *          'errno' is set accordingly
 */
int evquick_init(void);

/* Main loop
 * =========
 *
 * This is your application main loop and
 * should never return.
 *
 */
void evquick_loop(void);

/* Deallocate event loop
 * ========
 *
 * To be called to clean up the resources used by your
 * event loop.
 *
 */
void evquick_fini(void);

/* Event wrapper for file fd.
 * ==========
 * Arguments:
 * fd: file descriptor to watch for events
 * events: type of event to monitor.
 *         Can be EVQUICK_EV_READ, EVQUICK_EV_WRITE or both, using "|"
 * callback: function called by the loop upon events
 * err_callback: function called by the loop upon errors
 * arg: extra argument passed to callbacks
 *
 * Returns:
 * A pointer to the event object created, or NULL if an error occurs,
 * and errno is set accordingly.
 *
 */
evquick_event *evquick_addevent(int fd, short events,
                                void (*callback)
                                    (int fd, short revents, void *arg),
                                void (*err_callback)
                                    (int fd, short revents, void *arg),
                                void *arg);

/* Delete event
 * ==========
 *
 * Delete a previously created event.
 */
void evquick_delevent(evquick_event *e);


/* Timer
 * ==========
 * Arguments:
 * interval: number of milliseconds until the timer expiration
 * flags: 0, or EVQUICK_EV_RETRIGGER to set automatic retriggering with the
 *        same interval
 * callback: function called upon expiration
 * arg: extra argument passed to the callback
 */
evquick_timer *evquick_addtimer(unsigned long long interval, short flags,
                                void (*callback)(void *arg),
                                void *arg);


/* Delete timer
 * ==========
 *
 * Delete a previously created timer.
 */
void evquick_deltimer(evquick_timer *t);


#endif



