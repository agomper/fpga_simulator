#ifndef RECEIVER_H
#define RECEIVER_H
#include "jackclient.h"
#include "netclient.h"
#include "soundfile.h"

class Receiver : public JackClient, public NetClient, public SoundFile{

public:
    Receiver();
    void receiver_socket_test();
    void finish();
};

#endif // RECEIVER_H

