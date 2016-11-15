# libevquick
Event wrapper library designed for performance

I find both libevent and libev a bit overkill for a task as simple as providing a quick, non-invasive event wrapper.

For this reason I created my own implementation, based on POSIX syscalls.

The timer interface, in particular, is studied to use the smallest amount of system calls. 
This approach significantly reduces the number of context switches required during timers inactivity.

