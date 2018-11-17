#include "reader.h"

#include <sndfile.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>


char *const SOUND_FILE_SUBDIRECTORY = ".swearbox9001/sounds";

static int load_sample(char* path, struct Sample* sample);
static int load_asr_samples(char* directory, char* filename_stub, struct ASRSamples* samples);

int load_sample_bank(struct SampleBank* bank) {
    char* home_dir = getenv("HOME");
    char sound_files_directory[strlen(home_dir) + strlen("/") + strlen(SOUND_FILE_SUBDIRECTORY) + 1];
    sprintf(sound_files_directory, "%s/%s", home_dir, SOUND_FILE_SUBDIRECTORY);

    int return_error = 0;
    for (uint16_t i = 0; i < BANK_SIZE && bank->filename_stubs[i] != NULL; i++) {
        bank->array[i] = calloc(1, sizeof (struct ASRSamples));
        return_error = return_error != 0 ? return_error : load_asr_samples(sound_files_directory, bank->filename_stubs[i], bank->array[i]);
    }

    return return_error;
}

static int load_asr_samples(char* const directory, char* const filename_stub, struct ASRSamples* samples) {
    samples->attack = malloc(sizeof (struct Sample));
    samples->sustain = malloc(sizeof (struct Sample));
    samples->release = malloc(sizeof (struct Sample));
    if (samples->attack == NULL || samples->sustain == NULL || samples-> release == NULL) {
        fprintf(stderr, "Could not allocate memory for struct Sample.\n");
        return 1;
    }

    char path[strlen(directory) + strlen("/") + strlen(filename_stub) + strlen("_x.wav") + 1];
    int return_error = 0;

    sprintf(path, "%s/%s_a.wav", directory, filename_stub);
    if (load_sample(path, samples->attack) > 0) {
        fprintf(stderr, "Error loading sample %s.\n", path);
        return_error = 1;
    }

    sprintf(path, "%s/%s_s.wav", directory, filename_stub);
    if (load_sample(path, samples->sustain) > 0) {
        fprintf(stderr, "Error loading sample %s.\n", path);
        return_error = 2;
    }

    sprintf(path, "%s/%s_r.wav", directory, filename_stub);
    if (load_sample(path, samples->release) > 0) {
        fprintf(stderr, "Error loading sample %s.\n", path);
        return_error = 3;
    }

    return return_error;
}

static int load_sample(char* path, struct Sample* sample) {
    // Looks long but the bulk of it is error handling
    SF_INFO sf_info;
    sf_info.format = 0;
    SNDFILE* sound_file = sf_open(path, SFM_READ, &sf_info);
    if (sound_file == NULL) {
        fprintf(stderr, "Could not open sound file %s, error was %s", path, sf_strerror(NULL));
        return 1;
    }

    if (!(sf_info.format & SF_FORMAT_WAV)) {
        fprintf(stderr, "Sound file not in WAV format: %s\n", path);
        return 2;
    }

    if (!(sf_info.format & SF_FORMAT_PCM_16)) {
        fprintf(stderr, "Sound file not 16 bit PCM format: %s\n", path);
        return 3;
    }

    if (sf_info.channels != 1) {
        fprintf(stderr, "Sound file did not have exactly one channel: %s\n", path);
        return 4;
    }

    // libsndfile has no way to read data into a char array, so we read it into shorts then
    // manually copy to chars
    size_t num_temp_data_bytes = (size_t)sf_info.frames * sizeof (short);
    short* temp_sample_data = malloc(num_temp_data_bytes);
    if (temp_sample_data == NULL) {
        fprintf(stderr, "Could not allocate temp_sample_data.\n");
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
        fprintf(stderr, "Could not close sound file %s, exiting.\n", path);
        exit(EXIT_FAILURE);
    }

    return 0;
}
