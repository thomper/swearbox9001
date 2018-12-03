import os
import sys


SOUNDSTRETCH_BINARY = os.getenv('SOUNDSTRETCH_BINARY')
if SOUNDSTRETCH_BINARY is None:
    print('$SOUNDSTRETCH_BINARY not set, exiting.')
    sys.exit(1)

RAW_DIR = os.path.join('..', 'assets', 'raw')
DESTINATION_DIR = os.path.join(os.getenv('HOME'), '.swearbox9001', 'sounds')
SEMITONES_DOWN = SEMITONES_UP = 12


def out_filename_from_in_filename(in_filename, semitones_shift):
    assert in_filename.endswith('.wav') and len(in_filename) > 4, 'Bad file in raw directory'
    if semitones_shift == 0:
        return in_filename

    if semitones_shift < 0:
        shift_text = '_down{:02d}'.format(abs(semitones_shift))
    else:
        shift_text = '_up{:02d}'.format(semitones_shift)
    return '{}{}.wav'.format(in_filename[:-4], shift_text)


def main():
    for _, _, filenames in os.walk(RAW_DIR):
        for in_filename in filenames:
            for pitch in range(-SEMITONES_DOWN, SEMITONES_UP + 1):
                in_full_path = os.path.join(RAW_DIR, in_filename)
                out_filename = out_filename_from_in_filename(in_filename, pitch)
                out_full_path = os.path.join(DESTINATION_DIR, out_filename)
                if pitch == 0:
                    command_line = 'cp {} {}'.format(in_full_path, out_full_path)
                else:
                    command_line = '{} {} {} -pitch={}'.format(SOUNDSTRETCH_BINARY,
                                                               in_full_path,
                                                               out_full_path,
                                                               pitch)
                os.popen(command_line)


if __name__ == '__main__':
    main()
