all: test_heap test_evquick test_timers
clean:
	rm -f test_heap test_evquick test_timers tags core
test_heap:
	gcc -o test_heap test_heap.c -I..
test_evquick:
	gcc -o test_evquick test_evquick.c ../libevquick.c  -I.. -g -ggdb -lrt
test_timers:
	gcc -o test_timers test_timers.c ../libevquick.c  -I.. -g -ggdb -lrt
