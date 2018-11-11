all:
	gcc main.c playback.c -o swearbox9001 -lm -lao -lpthread

clean:
	rm ./swearbox9001
