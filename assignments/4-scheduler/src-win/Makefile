CFLAGS = /nologo /W3 /EHsc /Wall
CC = cl
LINK = link

build: libscheduler.dll

libscheduler.dll: scheduler.obj priority_queue.obj linked_list.obj
	$(LINK) /nologo /dll /out:$@ /implib:libscheduler.lib $**

scheduler.obj: scheduler.c util/so_scheduler.h
	$(CC) $(CFLAGS) /Fo$@ /c $**

linked_list.obj: linked_list.c linked_list.h
	$(CC) $(CFLAGS) /Fo$@ /c $**

priority_queue.obj: priority_queue.c priority_queue.h
	$(CC) $(CFLAGS) /Fo$@ /c $**

clean:
	del /Q scheduler.obj linked_list.obj priority_queue.obj 2>NUL
	del /Q libscheduler.dll libscheduler.lib libscheduler.exp 2>NUL