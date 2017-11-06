#include "soundfile.h"

SndfileHandle SoundFile::getSndfd() const
{
    return sndfd;
}

SoundFile::SoundFile()
{

}

//Last argument: Format | Subtype
//rf64File.create_file("Testfile", 2, 44100, SF_FORMAT_RF64 | SF_FORMAT_FLOAT);
void SoundFile::create_file(const char *fname, int channels, int srate, int fileFormat){
    soundFileName = fname;
    nChannels = channels;
    sampleRate = srate;
    format = fileFormat;

    cout<<"Creating sound file with the name: "<< fname <<"\n";

    sndfd = SndfileHandle (soundFileName, SFM_WRITE, format, nChannels, sampleRate) ;
    if (sndfd != NULL) {
        cout<<"File created. \n ";
    }
    else {
        cout<<"File creation error. \n";
        printf("%s\n",sf_strerror(NULL));
        exit(1);
    }
}

void SoundFile::open_file() {
    sndfd = SndfileHandle(soundFileName);

    if (sndfd != NULL) {
        cout<<"File opened \n";
        cout<<"Name: "<< soundFileName  <<endl;
        cout<<"Sample rate: "<< sndfd.samplerate() <<endl;
        //cout<<"Format: "<< sndfd.format() <<endl;
        cout<<"Channels: "<< sndfd.channels() <<endl;
    }
    else {
        cout<<"File creation error. \n";
        printf("%s\n",sf_strerror(NULL));
        exit(1);
    }
}
