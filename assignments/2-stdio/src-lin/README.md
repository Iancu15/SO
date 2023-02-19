Nume: Iancu Alexandru-Gabriel
Grupă: 333CB

# Tema 2

Organizare
-
1. Structura

SO_FILE contine memoria asociata unui fisier deschis sau proces creat, asa ca acesta contine file descriptor-ul prin care
se stabileste comunicarea. Am considerat necesar ca fiecare astfel de structura sa aiba buffer-ul sau asociat
ca sa nu se intercaleze cu comunicarea realizata intre program si alte fisiere sau procese. Pentru a te plimba prin
buffer exista campul curr_buffer_index care e incrementat la fiecare octet scris sau citit alaturi de dimensiunea
curenta a buffer-ului(nu capacitatea) si anume cati octeti ce urmeaza a fi cititi si scrisi se afla in buffer la
momentul curent. In rest am campuri specifice pentru a rezolva probleme specifice:
* eof este un flag ce tine cont daca am ajuns la eof sau nu, acesta poate sa fie NO_EOF si anume n-am ajuns la EOF,
EOF_IN_BUFFER si anume dupa golirea buffer-ului daca nu a avut loc vreo modificare a cursor-ului se va ajunge la EOF
sau EOF_SET ce semnifica ca s-a ajuns la EOF
* error care este flag-ul de eroare si acesta va fi setat pe stream cand se intalnesti o eroare inainte de a intoarce rezultatul
de eroare functiei
* last_op ce specifica care a fost ultima operatie realizata: de read, write sau altceva relevant(fseek). Este folositor in
fseek pentru a stii daca trebuie sa fac ceva la buffer si ce trebuie sa-i fac mai exact(invalidare sau fflush)
* offset-ul ce retine pozitia cursorului in fisier si este rezultatul la ftell, il actualizez la fiecare fseek si il incrementez
la fiecare scriere de caracter sau citire de caracter pentru ca o astfel de operatie imi muta cursorul cu o pozitie
* pid-ul in linux ce contine fd-ul procesului copil creat cu popen si pi-ul (PROCESS_INFORMATION) pe windows ce contine
fd-ul procesului in campul hProcess, il folosesc pentru a da wait pe proces in pclose
* write_mode exclusiv pe windows pentru a diferentia intre modul normal de scriere si modul append pentru care trebuie
sa mut cursor-ul la finalul fisierului inainte de fiecare scriere(n-am gasit optiune de append pe windows)

2. Utilitatea temei

Consider ca tema a fost utila in a ne familiariza cu modul in care se folosesc bufferi pentru comunicare, lucru care este
folosit pentru orice fel de comunicare fie ca e vorba de comunicare intre procese, fisier <-> proces sau host cu alt host
prin socketi. Am descoperit cum este realizata in spate o parte din biblioteca stdio si rolul structurii FILE in cadrul
functiilor din aceasta. M-am familiarizat cu functiile de sistem din Win32 si mi-am format o imagine initiala legata de
diferentele intre functiile de sistem linux si Win32.

3. Eficienta temei

Tema este cat se poate de eficienta:
* folosesc un singur buffer in loc de 2 per structura lucru care imi salveaza memorie
* pe baza buffer-ului se fac doar minimul de apeluri de sistem pentru umplerea buffer-ului ca
mai apoi apelurile urmatoare sa se foloseasca de datele stocate in buffer
* folosesc campul offset pentru a retine pozitia cursor-ului in fisier in loc sa ma folosesc
de apeluri de sistem in ftell pentru a muta cursor-ul la final si a stii astfel offset-ul
* desi folosesc fputc si fgetc in fwrite, respectiv fread, este o metoda la fel de eficienta ca
memcpy pentru ca copiaza acelasi numar de caractere 1 singura data, pentru ca si memcpy are o
complexitate proportionala liniar cu numarul de caractere copiate

Implementare
-

Intregul enunt al temei a fost implementat.

1. Dificultati intampinate

Au fost 2 dificultati principale intampinate:
* realizarea modului de read/write care a fost complicat pana sa-mi dau seama cum ar trebui sa ma folosesc de buffer pentru
scriere si citire.
* realizarea modului de creare de procese in care a durat ceva pana sa imi dau seama cum functioneaza pipe-ul si anume faptul
ca nu eu alegeam in implementare care capat sa fie de read si de write, ci unul e doar de read si celalalt doar de write; de
asemenea a durat ceva sa imi dau seama ca imi astepta la infinit la wait pentru ca dadeam close in parinte la capatul
de pipe dupa wait in loc de inainte, astfel procesul copil nu stia ca procesul parinte nu mai are nimic de comunicat si nu se mai termina
si asa si parintele nu ajungea sa inchida capatul de pipe ca astepta ca procesul copil sa se termine; pe Win32 a fost mai complicata
crearea de procese pentru ca se foloseau de multe structuri date ca parametru si abordarea era diferita pentru ca CreateProcess
se ocupa si de fork si de exec, eu fiind obisnuit cu abordarea de pe linux.

Cum se compilează și cum se rulează?
-
* Linux - make sau make build pentru build si make clean pentru clean, pentru a folosi functiile din biblioteca se adauga -lso_stdio la gcc-ul
in care se compileaza propriul program, apoi se poate rula programul
* Windows - nmake sau nmake build pentru build si nmake clean pentru clean, pentr a folosi functiile din biblioteca se adauga so_stdio.lib la cl-ul
in care se compileaza propriul program, apoi se poate rula programul

