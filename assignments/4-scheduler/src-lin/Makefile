CC = gcc
CFLAGS = -fPIC -Wall
LDFLAGS = -shared

build: libscheduler.so

libscheduler.so: scheduler.o priority_queue.o linked_list.o
	$(CC) $(LDFLAGS) -o $@ $^

scheduler.o: scheduler.c util/so_scheduler.h
	$(CC) $(CFLAGS) -o $@ -c $<

linked_list.o: linked_list.c linked_list.h
	$(CC) $(CFLAGS) -o $@ -c $<

priority_queue.o: priority_queue.c priority_queue.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.o libscheduler.so