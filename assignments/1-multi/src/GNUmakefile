build: tema1.o hashmap.o vector.o utils.o
	gcc -g -Wall tema1.o hashmap.o vector.o utils.o -o so-cpp

tema1.o: tema1.c hashmap.h vector.h tema1.h utils.h
	gcc -c tema1.c

hashmap.o: hashmap.c hashmap.h utils.h
	gcc -c hashmap.c

vector.o: vector.c vector.h utils.h
	gcc -c vector.c

utils.o: utils.c utils.h
	gcc -c utils.c

clean:
	rm -f so-cpp
	rm -f *.o
