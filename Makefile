all: init res2h

init:
	git submodule update --init --recursive

res2h: res2h.c
	cc -o res2h res2h.c
