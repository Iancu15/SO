build: tema1.obj hashmap.obj vector.obj utils.obj
	cl /MD /Wall /o so-cpp.exe tema1.obj hashmap.obj vector.obj utils.obj

tema1.obj: tema1.c hashmap.h vector.h tema1.h utils.h
	cl /MD /c tema1.c

hashmap.obj: hashmap.c hashmap.h utils.h
	cl /MD /c hashmap.c

vector.obj: vector.c vector.h utils.h
	cl /MD /c vector.c

utils.obj: utils.c utils.h
	cl /MD /c utils.c

.PHONY : clean
clean:
	del -f so-cpp.exe
	del -f *.obj
