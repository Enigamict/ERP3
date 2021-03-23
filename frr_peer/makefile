all:
	sudo gcc -o frrpeer frrpeer.c -ljansson
	gcc -o libconfig libconfig.c -ljansson
run:
	sudo ./r1_route test.json
