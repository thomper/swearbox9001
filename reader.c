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

    int last_error = 0;
    for (uint16_t i = 0; i < BANK_SIZE; i++) {
        bank->array[i] = NULL;
    }
    for (uint16_t i = 0; i < BANK_SIZE && bank->filename_stubs[i] != NULL; i++) {
        bank->array[i] = calloc(1, sizeof (struct ASRSamples));
        int temp_error = load_asr_samples(sound_files_directory, bank->filename_stubs[i], bank->array[i]);
        last_error = temp_error != 0 ? temp_error : last_error;
    }

    return last_error;
}

static void free_sample(struct Sample* sample) {
    if (sample != NULL) {
        if (sample->sample_data != NULL) {
            free(sample->sample_data);
            sample->sample_data = NULL;
        }
        free(sample);
    }
}

static void free_asrsamples(struct ASRSamples* samples) {
    if (samples != NULL) {
        for (uint i = 0; i < PITCHES_AVAILABLE; i++) {
            free_sample(samples->attack[i]);
            samples->attack[i] = NULL;

            free_sample(samples->sustain[i]);
            samples->sustain[i] = NULL;

            free_sample(samples->release[i]);
            samples->release[i] = NULL;
        }

        free(samples);
    }
}

void free_sample_bank_array(struct SampleBank* bank) {
    for (uint16_t i = 0; i < BANK_SIZE; i++) {
        free_asrsamples(bank->array[i]);
        bank->array[i] = NULL;
    }
}

static int load_asr_samples(char* const directory, char* const filename_stub, struct ASRSamples* samples) {
    for (uint i = 0; i < PITCHES_AVAILABLE; i++) {
        samples->attack[i] = malloc(sizeof(struct Sample));
        samples->sustain[i] = malloc(sizeof(struct Sample));
        samples->release[i] = malloc(sizeof(struct Sample));
        if (samples->attack[i] == NULL || samples->sustain[i] == NULL || samples->release[i] == NULL) {
            fprintf(stderr, "Could not allocate memory for struct Sample.\n");
            return 1;
        }
    }

    char path[strlen(directory) + strlen("/") + strlen(filename_stub) + strlen("_x_downxx.wav") + 1];
    int return_error = 0;

    for (uint i = 0; i < PITCHES_AVAILABLE; i++) {
        char pitch_suffix[strlen("_downxx") + 1];
        if (i < SEMITONES_RANGE) {
            sprintf(pitch_suffix, "_down%02d", SEMITONES_RANGE - i);
        } else if (i == SEMITONES_RANGE) {
            pitch_suffix[0] = '\0';
        } else {
            sprintf(pitch_suffix, "_up%02d", i - SEMITONES_RANGE);
        }

        sprintf(path, "%s/%s_a%s.wav", directory, filename_stub, pitch_suffix);
        if (load_sample(path, samples->attack[i]) > 0) {
            fprintf(stderr, "Error loading sample %s.\n", path);
            return_error = 1;
        }

        sprintf(path, "%s/%s_s%s.wav", directory, filename_stub, pitch_suffix);
        if (load_sample(path, samples->sustain[i]) > 0) {
            fprintf(stderr, "Error loading sample %s.\n", path);
            return_error = 2;
        }

        sprintf(path, "%s/%s_r%s.wav", directory, filename_stub, pitch_suffix);
        if (load_sample(path, samples->release[i]) > 0) {
            fprintf(stderr, "Error loading sample %s.\n", path);
            return_error = 3;
        }
    }

    if (!asr_samples_lengths_consistent(samples)) {
        fprintf(stderr, "Error loading sample with filename stub %s.\n", filename_stub);
        return_error = 4;
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
    free(temp_sample_data);

    if (sf_close(sound_file)) {
        fprintf(stderr, "Could not close sound file %s, exiting.\n", path);
        exit(EXIT_FAILURE);
    }

    return 0;
}
