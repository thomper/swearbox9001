all:
	gcc main.c playback.c -o swearbox9001 -lm -lao

clean:
	rm ./swearbox9001
