Nume: Iancu Alexandru-Gabriel
Grupă: 333CB

# Tema 1

Organizare
-

***Structura temei***

* Am implementat un hashmap pentru a retine maparile define-urilor.
* Am implementat un vector cu dimensiune variabila pentru a retine directoarele specificate cu -I
la argumente.
* In utils.c am implementat niste functii ajutatoare de lucru cu string-uri.
* Implementarea propriu-zisa a temei se afla in tema1.c si a fost modularizata folosind functii.
Procesarile se fac top-down pana se ajunge la cea mai mica unitate independenta de a fi procesata.
Astfel cea mai de sus procesare e cea a unui fisier (process_infile) din care se ajunge in
process_code care se imparte in procesari mai specifice: process_define, process_undef,
process_ifdef etc. O astfel de organizare mi-a permis reutilizarea functiilor mai generale precum
process_code sau process_else(care e folosit si la if-uri si la ifdef-uri). In prelucrarea
directivelor conditionale(if, else, ifndef) si a directivei include se apeleaza process_code pentru
procesarea codului aferent directivei.

***Utilitatea temei***

Tema a fost utila in descoperirea mai amanuntita a functionarii unui preprocesor si intelegerea
directivelor de preprocesare ale unui fisier. Aceasta tema te antreneaza in rezolvarea problemelor
de prelucrare si procesare de siruri de caractere, tipuri de probleme destul de des intalnite.
Mi-a oferit o introducere in utilizarea descriptilor de fisiere, fiind nevoit spre exemplu sa
folosesc fprintf pentru a afisa la stdout in loc de printf pentru a nu repeta cod.

***Eficienta temei***

Implementarea hashmap-ului este destul de eficienta datorita:
* hash code-ului care e calculat folosind Horner's rule si ofera o distributie decenta a intrarilor
* rehash-uirea atunci cand load factor-ul depaseste 1 (capacitatea e mai mica decat dimensiunea)

Implementarea preprocesorului conceptual vorbind este buna pentru ca parcurge fiecare linie o
singura data pentru citire si maxim o data pentru prelucrare. Totusi o problema ar fi faptul ca
aloc si eliberez memorie pentru siruri de caractere foarte des, o problema necesara datorita
faptului ca am fost fortat sa lucrez in C. Insa puteam sa rezolv mai elegant concatenarea unui
caracter la fisier in locul utilizarii str_cat care are o complexitate de O(n), unde n este lungimea
noului sir rezultat concatenarii, cand putea fi O(1) (analiza structurala) daca foloseam un vector
de caractere care isi dubla capacitatea de fiecare data cand se umplea. Totusi sunt multumit de
cum am abordat inlocuirea define-urilor in sirurile de caractere (replace_define_str) pentru ca
am adaugat caracter cu caracter la un vector alocat static in loc sa folosesc str_cat.

Implementare
-

Intregul enunt al temei a fost implementat.

***Dificultati intampinate***

Partea cea mai complicata in tema dupa mine a fost gasirea unui mod cat mai eficient sa inlocuiesc
define-urile in sirul de caractere. Am avut mai multe metode in minte:
* folosirea strstr pentru cautarea subsirului, insa problema era ca practic trebuia sa testez
fiecare intrare din hashmap pentru fiecare sir; practic parcurgeam hashmap-ul in loc sa folosesc
get, in acest caz puteam sa am la fel de bine un vector
* folosirea strtok pentru tokenizare nu mergea pentru ca pierdeam delimitatorii pe parcurs, partea
buna era ca asa puteam folosi get pe token
* am decis in final sa tokenizez eu de mana parcurgand caracter cu caracter, astfel putand sa
aleg sa pastrez delimitatorii

Cum se compilează și cum se rulează?
-
* Linux - make pentru build si make clean pentru clean
* Windows - nmake pentru build si nmake clean pentru clean
* Se ruleaza executabilul so-cpp pe Linux si so-cpp.exe pe Windows
* argumentele sunt primite din linia de comanda conform enuntului

Bibliografie
-

* hash function - https://cseweb.ucsd.edu/~kube/cls/100/Lectures/lec16/lec16-14.html
* makefile - https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-01
