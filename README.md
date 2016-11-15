## libevquick
Event wrapper library designed for performance

I find both libevent and libev a bit overkill for a task as simple as providing a quick, non-invasive event wrapper.

For this reason I created my own implementation, based on POSIX syscalls.

The timer interface, in particular, is studied to use the smallest amount of system calls. 
This approach significantly reduces the number of context switches required during timers inactivity.

### Usage
Blocking point in your event-based application are managed internally by the event wrapper loop.

- To initialize the library, call `evquick_init()`. 
- Set up your initial events and/or timers using `evquick_addevent()` and/or `evquick_addtimer()`
- Enter the main loop via `evquick_loop()`.

The main loop never returns, and will yield the control to your callbacks upon the requested event or
timer expiration.

For a more detailed explaination of the API, see `libevquick.h`

To link with your application, simply add `libevquick.c` to your project and put `libevquick.h` 
in your include path.




