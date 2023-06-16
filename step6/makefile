FLAG = -std=c++11 -pthread

all:
	g++ -c $(FLAG) *.cpp
	g++ $(FLAG) -o server server.o TCP.o command_function.o threeway_handshake.o  Header.o
	g++ $(FLAG) -o client client.o TCP.o command_function.o threeway_handshake.o  Header.o

server:	server.o TCP.o command_function.o threeway_handshake.o command_function.o Header.o
	g++ $(FLAG) -o server server.o TCP.o command_function.o threeway_handshake.o  Header.o

client:	client.o TCP.o command_function.o threeway_handshake.o command_function.o Header.o
	g++ $(FLAG) -o client client.o TCP.o command_function.o threeway_handshake.o  Header.o

server.o:	server.cpp TCP.o Header.o command_function.o threeway_handshake.o
	g++ $(FLAG) -c server.cpp

client.o:	client.cpp TCP.o Header.o command_function.o threeway_handshake.o
	g++ $(FLAG) -c client.cpp

TCP.o:	TCP.cpp TCP.h Header.o command_function.o threeway_handshake.o
	g++ $(FLAG) -c TCP.cpp

command_function.o:	command_function.cpp command_function.h Header.o threeway_handshake.o TCP.o
	g++ $(FLAG) -c command_function.cpp

threeway_handshake.o:	threeway_handshake.cpp threeway_handshake.h Header.o TCP.o command_function.o
	g++ $(FLAG) -c threeway_handshake.cpp

Header.o:	Header.cpp Header.h TCP.o command_function.o threeway_handshake.o
	g++ $(FLAG) -c Header.cpp

dep:
	g++ -MM *.cpp > dep

clean:
	rm server client *.o
