Nume: Iancu Alexandru-Gabriel
Grupă: 333CB

# Tema 3

Organizare
-
1. Structura

Initializez in so_init_loader rutina pentru interceptarea de SIGSEGV.
Scriu o functie handler (loader_handler) care se va ocupa de
maparea paginilor la interceptarea de SIGSEGV. Aceasta va mapa
pagina in care se afla adresa la care a dat SIGSEGV. Pentru a stii daca 
pagina respectiva a fost mapata folosesc campul data al so_seg_t.
In acesta voi tine un vector pentru care la index-ul i va fi stocat 0
daca pagina i nu a fost mapata si 1 in caz contrar. Pentru maparea paginii
folosesc mmap cu flagul MAP_FIXED pentru a mapa fix la adresa
paginii si MAP_ANONYMOUS pentru a initializa pagina cu 0. Pot folosi
MAP_ANONYMOUS pentru ca nu dau handler-ul fisierului executabil
catre mmap ci ma ocup eu personal de citire cu read() ca mai apoi
sa dau memcpy() pe memoria unde e mapata pagina. Pe Windows folosesc ReadFile()
pentru citirea din fisier si VirtualAlloc() pentru maparea paginii unde flag-ul
MEM_COMMIT se va asigura ca memoria imi va fi initializata cu 0 cand o accesez
eu sau programul.

Pentru a stii ca adresa se afla intr-un segment trec prin toate segmentele
si verific intai ca adresa nu se afla inaintea adresei segmentului. Apoi
ma asigur ca offset-ul adresei fata de adresa segmentului este mai mic decat
dimensiunea in memorie a segmentului. Daca aceste 2 conditii nu sunt respectate
de niciun segment, atunci se va rula handler-ul default pentru ca adresa este
una neaccesibila.

2. Utilitatea temei

Consider ca tema a fost utila pentru a te obisnui cu rutinele si interceptarea de
semnale. De asemenea, am observat mai bine diferentele dintre segmente si pagini
fiind ambele folosite in acest loader.

3. Eficienta temei

Folosesc eficient memoria pentru ca folosesc un vector simplu pentru campul data
din so_seg_t care contine int-uri si nu structuri. Pentru a fi si mai eficient as
fi putut sa stochez in char-uri in loc de int-uri. In rest nu sunt prea multe lucruri
in care se poate pune problema eficientiei. Segmentele trebuiau parcurse printr-un for,
asta nu putea fi mai eficient. Totusi sunt sanse ca mmap sa fi facut scrierea in pagina
mai eficient decat mine daca i-as fi dat handler-ul fisierului executabil.

Implementare
-

Intregul enunt a fost realizat, problema e ca testul 9 imi pica pe linux din cauza unui
corner case pe care nu-l tratez.

1. Dificultati intampinate

Dificultatea mea a fost sa-mi dau seama efectiv ce ar trebui sa fac. Citisem de vreo 10
ori enuntul temei pana sa imi dau seama cum ar trebui sa scriu handler-ul pentru ca desi ofera
informatiile teoretice, legat de implementarea propriu-zisa nu ofera prea multe detalii.
Mai ales faptul ca loader-ul foloseste si segmentare si paginari, lucru care complica situatia.
Scrierea codului pentru Windows nu a fost cu mult mai complicata pentru ca partea grea
a fost sa imi dau seama de flow-ul programului din perspectiva implementarii, cum aveam implementarea
de la linux nu a fost greu sa scriu echivalentul in Windows.

Am pierdut cateva ore pana sa imi dau seama ca checker-ul trebuia compilat, ar fi fost o idee sa
fi fost mentionat asta in enunt. Si faptul ca n-am folosit o rutina cu siginfo_t info la laborator
a fost o problema pana sa citesc mai in detaliu manualul pentru sigaction.

Faptul ca n-aveam cum sa printez si sa vad pe checker, greoi debug-ul la tema. A trebuit sa
fac un fel de logger pentru a printa cand rula run_tests.sh. In rest am avut probleme legate
de mici greseli de neatentie la formulele pe care le foloseam pentru variabilele folosite in
comparatii.

Cum se compilează și cum se rulează?
-
* Linux - make sau make build pentru build si make clean pentru clean,
pentru a folosi loader-ul se adauga -lso_loader la gcc-ul
in care se compileaza propriul program, apoi se poate rula programul
* Windows - nmake sau nmake build pentru build si nmake clean pentru clean,
pentru a folosi loader-ul se adauga so_loader.lib la cl-ul
in care se compileaza propriul program, apoi se poate rula programul

