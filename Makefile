all:
	gcc main.c input.c playback.c reader.c -o swearbox9001 -lao -lpthread -lsndfile -lSDL2

clean:
	rm ./swearbox9001
