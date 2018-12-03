all:
	gcc main.c engine.c input.c reader.c playback.c -o swearbox9001 -lao -lpthread -lsndfile -lSDL2

clean:
	rm ./swearbox9001
