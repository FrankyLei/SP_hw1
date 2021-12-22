all: host player 

host: host.c
	gcc host.c -o host

player: player.c
	gcc player.c -o player

clear: 
	rm -rf host 	
	rm -rf ./fifo_*.tmp
	rm -rf player 

