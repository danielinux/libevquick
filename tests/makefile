all: test_heap test_evquick
clean:
	rm -f test_heap test_evquick tags core
test_heap:
	gcc -o test_heap test_heap.c -I..
test_evquick:
	gcc -o test_evquick test_evquick.c ../libevquick.c  -I.. -g -ggdb
