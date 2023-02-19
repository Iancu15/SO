build: libso_stdio.so

libso_stdio.so: file_struct.o read_write.o buffering.o other_func.o process_run.o cursor_position.o utils.o util/so_stdio.h util/utils.h
	gcc -shared file_struct.o read_write.o buffering.o other_func.o process_run.o cursor_position.o utils.o -o libso_stdio.so

file_struct.o: file_struct.c file_struct.h
	gcc -fPIC -c file_struct.c

read_write.o: read_write.c read_write.h
	gcc -fPIC -c read_write.c

buffering.o: buffering.c buffering.h
	gcc -fPIC -c buffering.c

other_func.o: other_func.c other_func.h
	gcc -fPIC -c other_func.c

process_run.o: process_run.c process_run.h
	gcc -fPIC -c process_run.c

cursor_position.o: cursor_position.c cursor_position.h
	gcc -fPIC -c cursor_position.c

utils.o: util/utils.c util/utils.h
	gcc -fPIC -c util/utils.c

clean:
	rm -f main *.o *.so
