#include "jackclient.h"

jack_client_t *JackClient::getClientfd() const
{
    return clientfd;
}

void JackClient::setClientfd(jack_client_t *value)
{
    clientfd = value;
}

int JackClient::getJackBufferSize() const
{
    return jackBufferSize;
}

int JackClient::getChannels() const
{
    return channels;
}

jack_ringbuffer_t *JackClient::getRingBuffer() const
{
    return ringBuffer;
}

jack_port_t *JackClient::getJackPort(int index)
{
    return jackPort[index];
}

int JackClient::getComPipe(int index){
    return comPipe[index];
}

float *JackClient::getJackBuffer() const
{
    return jackBuffer;
}


JackClient::JackClient() {
    options = JackNoStartServer; //No arranca JACK si no lo estaba.
    //JackNullOption; //Ninguna condicion a Jack Server, solo arranca.
    channels = 32;
    //Total = Num frames * Num channels * 32 bits
    jackBufferSize = (4096*channels) * sizeof(float);
    //xmalloc(). The motto is succeed or die. If it fails to allocate memory,
    //it will terminate your program and print an error message
    jackBuffer = (float *) malloc(jackBufferSize);
    //cout<<"JackRF64 buffer created with a size of: "<<jackBufferSize<<" bytes. \n";
    //Allocates a ringbuffer data structure of a specified size.
    ringBuffer = jack_ringbuffer_create(jackBufferSize);
    //The pipe is then used for communication either between the parent or child
    //processes, or between two sibling processes.
    int err = pipe(comPipe);
    if(err) {
      perror("pipe() failed");
      exit(1);
    }
    clientfd = NULL;
}


jack_client_t *JackClient::open_jack_client(char *name) {
    this->clientName = name;

    /* open a client connection to the JACK server */
    clientfd = jack_client_open (clientName, options, &status);

    if (clientfd == NULL){
        //If connection failed, say why
        printf ("open_jack_client() failed, status = 0x%2.0x\n", status);
        if (status & JackServerFailed) {
            printf ("Unable to connect to JACK server.\n");
        }
        exit (1);
    }

    /* if connection was successful, check if the name we proposed is not in use */
    if (status & JackNameNotUnique){
        clientName = jack_get_client_name(clientfd);
        printf ("Warning: other agent with our name is "
                "running, `%s' has been assigned to us.\n", clientName);
    }

    /* display the current sample rate. */
    sampleRate = (double)jack_get_sample_rate(clientfd);
    printf ("Engine sample rate: %0.0f\n", sampleRate);

    return clientfd;
}


void JackClient::jack_client_activate(jack_client_t *client)
{
  int err = jack_activate(client);
  if(err) {
    printf("jack_activate() failed\n");
    exit(1);
  }
}


void JackClient::jack_ringbuffer_read_exactly(int nBytesToRead)
{
  int BytesRead = jack_ringbuffer_read(ringBuffer,
                                       (char *) jackBuffer, nBytesToRead);
  if(BytesRead != nBytesToRead) {
    cout<<"Error reading RingBuffer. Required = "<<nBytesToRead<<
          " Done= "<<BytesRead<<" \n.";
    exit(1);
  }
}

void JackClient::jack_ringbuffer_read_exactly(char *buffer, int nBytesToRead)
{
  int BytesRead = jack_ringbuffer_read(ringBuffer, buffer, nBytesToRead);
  if(BytesRead != nBytesToRead) {
    cout<<"Error reading RingBuffer. Required = "<<nBytesToRead<<
          " Done= "<<BytesRead<<" \n.";
    exit(1);
  }
}

void JackClient::jack_ringbuffer_write_exactly(int nBytesToWrite)
{
  int BytesWrote = jack_ringbuffer_write(ringBuffer,
                                         (char *) jackBuffer, nBytesToWrite);
  if(BytesWrote != nBytesToWrite) {
      cout<<"Error writing RingBuffer. Required = "<<nBytesToWrite<<
            " Done= "<<BytesWrote<<" \n.";
    exit(1);
  }
}

void JackClient::jack_ringbuffer_write_exactly(const char *buffer, int nBytesToWrite)
{
  int BytesWrote = jack_ringbuffer_write(ringBuffer, buffer, nBytesToWrite);
  if(BytesWrote != nBytesToWrite) {
      cout<<"Error writing RingBuffer. Required = "<<nBytesToWrite<<
            " Done= "<<BytesWrote<<" \n.";
    exit(1);
  }
}


void JackClient::jack_port_make_standard(int mode)
{
  int i;
  int direction;
  for(i = 0; i < channels; i++) {
    char name[64];
    if (mode == 1) { //Sender - Writable client
        direction = JackPortIsInput;
        snprintf(name, 64, "in_%d", i + 1);
    }
    else { //Receiver - Readable client
        direction = JackPortIsOutput;
        snprintf(name, 64, "out_%d", i + 1);
    }

    jackPort[i] = jack_port_register(clientfd, name, JACK_DEFAULT_AUDIO_TYPE, direction, 0);
    if(jackPort[i] == NULL) {
      printf("jack_port_register() failed\n");
      exit(1);
    }
  }
}


int JackClient::jack_ringbuffer_wait_for_read(int payload, int pipeFd)
{
    int spaceRB = (int) jack_ringbuffer_read_space(ringBuffer);
    while(spaceRB < payload) {
        char b;
        if(read(pipeFd, &b, 1)== -1) {
            printf("%s: error reading communication pipe\n", __func__);
            exit(1);
        }
        spaceRB = (int) jack_ringbuffer_read_space(ringBuffer);
    }
    return spaceRB;
}

