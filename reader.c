#include "reader.h"
#include "playback.h"  // for testing only (queue_playback)

#include <sndfile.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>


char *const SOUND_FILE_SUBDIRECTORY = ".swearbox9001/sounds";
char* const SOUND_FILENAME_STUBS[] = {"test_f"};

static int load_sample(char* directory, char* filename_stub, struct Sample* sample);

// TODO: a free samples function that gets called on program exit

void load_samples(void) {
    struct Sample* sample = calloc(1, sizeof (struct Sample));

    char* home_dir = getenv("HOME");
    char full_sound_files_path[strlen(home_dir) + strlen(SOUND_FILE_SUBDIRECTORY) + 2];  // + '/' + '\0'
    sprintf(full_sound_files_path, "%s/%s", getenv("HOME"), SOUND_FILE_SUBDIRECTORY);
    load_sample(full_sound_files_path, SOUND_FILENAME_STUBS[0], sample);
    queue_playback(sample);
}

static int load_sample(char* const directory, char* const filename_stub, struct Sample* sample) {
    short* temp_sample_data;
    char full_path[strlen(directory) + 1 + strlen(filename_stub) + 7];  // + '/' ... + "_x.wav" including '\0'
    sprintf(full_path, "%s/%s_a.wav", directory, filename_stub);

    SF_INFO sf_info;
    sf_info.format = 0;  // TODO: remove this if we're certain struct gets zeroed out on creation
    SNDFILE* sound_file = sf_open(full_path, SFM_READ, &sf_info);
    if (sound_file == NULL) {
        fprintf(stderr, "Could not open sound file %s, error was %s", full_path, sf_strerror(NULL));
        return 1;
    }

    if (!(sf_info.format & SF_FORMAT_WAV)) {
        fprintf(stderr, "Sound file not in WAV format: %s\n", full_path);
        return 2;
    }

    int subformat = sf_info.format & ~SF_FORMAT_WAV;
    if (subformat != SF_FORMAT_PCM_16) {
        fprintf(stderr, "Sound file not 16 bit PCM format: %s\n", full_path);
        return 3;
    }

    if (sf_info.channels != 1) {
        fprintf(stderr, "Sound file did not have exactly 1 channel: %s\n", full_path);
        return 4;
    }

    size_t num_temp_data_bytes = (size_t)sf_info.frames * sizeof (short);
    temp_sample_data = malloc(num_temp_data_bytes);
    if (temp_sample_data == NULL) {
        fprintf(stderr, "Could not allocated temp_sample_data.\n");
        return 5;
    }

    sf_count_t frames_read = sf_readf_short(sound_file, temp_sample_data, sf_info.frames);
    sample->num_bytes = frames_read * 2;
    sample->sample_data = calloc((size_t)(sf_info.frames * 2l), sizeof (char));
    if (sample->sample_data == NULL) {
        fprintf(stderr, "Could not allocate sample_data.\n");
        return 6;
    }

    for (uint32_t i = 0; i < sf_info.frames; i++) {
        // copy the bytes into two adjacent sample_data indexes in big endian order
        sample->sample_data[2 * i] = (temp_sample_data[i] >> 8) & 0xff;
        sample->sample_data[2 * i + 1] = temp_sample_data[i] & 0xff;
    }

    if (sf_close(sound_file)) {
        fprintf(stderr, "Could not close sound file %s, exiting.\n", full_path);
        exit(EXIT_FAILURE);
    }

    return 0;
}
