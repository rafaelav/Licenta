build: attacker defender

attacker: proxy.o attacker.o
	g++ proxy.o attacker.o -o attacker

defender: proxy.o defender.o
	g++ proxy.o defender.o -o defender

proxy.o: proxy.cpp proxy.h
	g++ -c proxy.cpp -o proxy.o

attacker.o: attacker.cpp attacker.h
	g++ -c attacker.cpp -o attacker.o	

defender.o: defender.cpp defender.h
	g++ -c defender.cpp -o defender.o	

init: 
	sudo iptables -A PREROUTING -t nat -p udp --dport 1234 -j REDIRECT --to-ports 30002
	sudo iptables -A PREROUTING -t nat -p udp --dport 30003 -j REDIRECT --to-ports 1234
