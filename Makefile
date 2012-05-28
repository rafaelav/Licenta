build:
	g++ proxy.cpp -o proxy
init: 
	sudo iptables -A PREROUTING -t nat -s 192.168.1.6 -d 192.168.1.2 -p udp --dport 1234 -j REDIRECT --to-ports 30002
	sudo iptables -t nat -A OUTPUT --dst 127.0.0.1 -p udp --dport 1234 -j ACCEPT