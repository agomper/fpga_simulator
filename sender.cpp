#include "sender.h"


Sender::Sender() {
    strcpy(hostname, "10.0.0.2");
    soundFileName = "RF64File";
}

void Sender::sender_socket_test() {
    char my_message[30] = "This is a test message.";
    /* send a message to the server */
    if (sendto(socketfd, my_message, strlen(my_message),
               0, (struct sockaddr *)&isAddress, sizeof(isAddress)) < 0) {
        perror("Sender socket test failed. \n");
        exit(1);
    }
}

void Sender::finish()
{
    close(socketfd);
    jack_ringbuffer_free(ringBuffer);
    jack_client_close(clientfd);
    close(comPipe[0]);
    close(comPipe[1]);
    free(jackBuffer);
}

