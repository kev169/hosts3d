g++ -Wall -O2 -c -o src/llist.o src/llist.cpp
g++ -Wall -O2 -c -o src/misc.o src/misc.cpp
g++ -Wall -O2 -c -o src/glwin.o src/glwin.cpp
g++ -Wall -O2 -c -o src/objects.o src/objects.cpp
g++ -Wall -O2 -c -o src/hosts3d.o src/hosts3d.cpp
g++ -Wall -O2 -static-libgcc -static-libstdc++ -o Hosts3D src/llist.o src/misc.o src/glwin.o src/objects.o src/hosts3d.o -lglfw -lopengl32 -lglu32 -lws2_32
