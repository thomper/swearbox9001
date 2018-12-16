import os


PICO2WAVE_BINARY = 'pico2wave'

UNCUT_DIR = os.path.join('..', 'assets', 'uncut_raw')
LANGUAGES = ('en-GB', 'en-US')
WORDLIST_FULL_PATH = os.path.join(UNCUT_DIR, 'wordlist.txt')


def main():
    with open(WORDLIST_FULL_PATH) as f:
        for word in f:
            word = word.strip()
            if len(word) < 1:
                continue

            for language in LANGUAGES:
                out_full_path = os.path.join(UNCUT_DIR, 'pico_{}_{}.wav'.format(language, word))
                command_line = '{} -l={} -w={} "{}"'.format(PICO2WAVE_BINARY, language, out_full_path, word)
                os.popen(command_line)


if __name__ == '__main__':
    main()
