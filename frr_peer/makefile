all:
	sudo gcc -o frrpeer frrpeer.c
run:
	nohup sudo ./frrpeer 2>&1 | tee -a /tmp/frrpeer.log &
kill:
	pgrep -f frrpeer |sudo xargs kill -9
