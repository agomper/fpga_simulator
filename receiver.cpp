#include "receiver.h"

Receiver::Receiver() {

}

void Receiver::receiver_socket_test()
{
    unsigned char buf[2048];
    socklen_t addrlen = sizeof(isAddress);
    int recvlen;            /* # bytes received */

    /* now loop, receiving data and printing what we received */
    while(1) {
        printf("waiting on port %d\n", portNumber);
        recvlen = recvfrom(socketfd, buf, 2048,
                           0, (struct sockaddr *)&isAddress, &addrlen);
        printf("received %d bytes\n", recvlen);
        if (recvlen > 0) {
            buf[recvlen] = 0;
            printf("received message: \"%s\"\n", buf);
        }
    }
    /* never exits */
}

void Receiver::finish()
{
    cout<<"Cerrando y liberando. \n";
    close(socketfd);
    jack_ringbuffer_free(ringBuffer);
    jack_client_close(clientfd);
    close(comPipe[0]);
    close(comPipe[1]);
    free(jackBuffer);
}



