g++ -Wall -O2 -c -o src/llist.o src/llist.cpp
g++ -Wall -O2 -c -o src/misc.o src/misc.cpp
g++ -Wall -O2 -c -o src/proto.o src/proto.cpp
g++ -Wall -O2 -c -o src/hsen.o src/hsen.cpp
g++ -Wall -O2 -static-libgcc -static-libstdc++ -o hsen src/llist.o src/misc.o src/proto.o src/hsen.o -lwpcap -lws2_32
