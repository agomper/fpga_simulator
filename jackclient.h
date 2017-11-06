#ifndef JACKCLIENT_H
#define JACKCLIENT_H
//Jack
#include <jack/jack.h>
#include <jack/ringbuffer.h>
//Comunes
#include <iostream>
#include <stdio.h> //For fprintf
#include <string.h>
#include <unistd.h> /* POSIX (Pipe) */
#include <pthread.h>
//Otras
#include "soundfile.h"
using namespace std;

class JackClient {
protected:
    char *clientName;
    jack_client_t *clientfd;
    jack_options_t options;
    jack_status_t status;
    double sampleRate;
    int jackBufferSize;             //Internal buffer size in frames.
    float *jackBuffer;              //Internal buffer of 32 bit floats.
    int channels;            //Num channels
    jack_ringbuffer_t *ringBuffer;  //Pointer to a jack ring buffer
    int comPipe[2];          //Interprocess communication pipe. Thread related.
    jack_port_t *jackPort[2];       //Jack ports = Channels
public:
    JackClient();
    jack_client_t *open_jack_client(char *name);
    jack_client_t *getClientfd() const;
    void jack_client_activate(jack_client_t *client);
    void jack_port_make_standard(int mode);
    void setClientfd(jack_client_t *value);
    void jack_ringbuffer_read_exactly(int nBytesToRead);
    void jack_ringbuffer_read_exactly(char *buffer, int nBytesToRead);
    void jack_ringbuffer_write_exactly(int nBytesToWrite);
    void jack_ringbuffer_write_exactly(const char *buffer, int nBytesToWrite);
    int getJackBufferSize() const;
    int getChannels() const;
    jack_ringbuffer_t *getRingBuffer() const;
    jack_port_t *getJackPort(int index);
    int getComPipe(int index);
    float *getJackBuffer() const;
    int jack_ringbuffer_wait_for_read(int payload, int pipeFd);
};

#endif // JACKCLIENT_H
