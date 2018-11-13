all:
	gcc main.c playback.c reader.c -o swearbox9001 -lao -lpthread -lsndfile

clean:
	rm ./swearbox9001
