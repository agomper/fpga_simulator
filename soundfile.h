#ifndef SOUNDFILE_H
#define SOUNDFILE_H

#include <sndfile.hh>
#include <cstdio>
#include <cstring>
#include <iostream>
using namespace std;

class SoundFile {
protected:
    const char *soundFileName;
    int nChannels;
    int sampleRate;
    int format;
    SndfileHandle sndfd;
public:
    SoundFile();
    void create_file(const char *fname, int channels, int srate, int fileFormat);
    void open_file();
    void write_file();
    void read_file();
    SndfileHandle getSndfd() const;
};

#endif // SOUNDFILE_H

