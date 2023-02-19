build: so_stdio.dll

so_stdio.dll: file_struct.obj read_write.obj buffering.obj other_func.obj process_run.obj cursor_position.obj utils.obj util/so_stdio.h util/utils.h
	link /nologo /dll /out:so_stdio.dll file_struct.obj read_write.obj buffering.obj other_func.obj process_run.obj cursor_position.obj utils.obj

file_struct.obj: file_struct.c file_struct.h
	cl /MD /c file_struct.c

read_write.obj: read_write.c read_write.h
	cl /MD /c read_write.c

buffering.obj: buffering.c buffering.h
	cl /MD /c buffering.c

other_func.obj: other_func.c other_func.h
	cl /MD /c other_func.c

process_run.obj: process_run.c process_run.h
	cl /MD /c process_run.c

cursor_position.obj: cursor_position.c cursor_position.h
	cl /MD /c cursor_position.c

utils.obj: util/utils.c util/utils.h
	cl /MD /c util/utils.c

clean:
	del -f main *.obj *.dll *.exp *.lib
