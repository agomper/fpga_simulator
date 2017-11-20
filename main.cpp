#include <unistd.h> /* POSIX */
#include <pthread.h>

#include "sender.h"

Sender senderObj;
int overflow_counter = 0; //Test

void jack_client_shutdown(void *arg) {
  printf("Jack shutdown \n");
}

void jack_client_error_handler(const char *desc) {
  printf("Jack error: %s\n", desc);
}


//*************************** JACK CLIENTS (LOCAL) ***************************/

/************************************************************ JACK CLIENT SEND*/
// Write data from the JACK input ports to the ring buffer.
//jack_nframes_t n = Frames/Period.
int jack_callback_sender (jack_nframes_t nframes, void *arg){
    int i, j;                         //Iteradores auxiliares
    float *in[senderObj.getChannels()];

    float fileBuffer[nframes*senderObj.getChannels()];
    senderObj.getSndfd().read(fileBuffer, nframes*senderObj.getChannels());

    //Los punteros in apuntan a lo mismo que los Jack port.
    //Se hace un for para igual cada i a un channel.
    for(i = 0; i < senderObj.getChannels(); i++) {
        in[i] = (float *) jack_port_get_buffer(senderObj.getJackPort(i), nframes);
    }
    //El buffer del paquete d se llena con la informacion de los Jack ports.
    for(i = 0; i < nframes; i++) {  //Siendo n = 1024 (Frames/Period)
        for(j = 0; j < senderObj.getChannels(); j++) {
            //El j_buffer guarda los datos de cada frame para cada uno
            //de los canales. in[Canal][Frame]
            senderObj.getJackBuffer()[(i*senderObj.getChannels())+j]
                    = (float) in[j][i]; //JACK PORTS
                      //= fileBuffer[(i*senderObj.getChannels())+j]; //FILE
        }
    }

    //Comprueba si hay espacio en buffer.
    int bytes_available = (int) jack_ringbuffer_write_space(senderObj.getRingBuffer());
    int bytes_to_write = nframes * sizeof(float) * senderObj.getChannels();
    if(bytes_to_write > bytes_available) {
        printf ("jack-udp send: buffer overflow error (UDP thread late)\n");
        overflow_counter++;
        cout<<"Overflow Counter: "<<overflow_counter<<endl;
    } else {
        senderObj.jack_ringbuffer_write_exactly(bytes_to_write);
    }

    char b = 1;
    if(write(senderObj.getComPipe(1), &b, 1)== -1) {
        printf ("jack-udp send: error writing communication pipe.\n");
        exit(1);
    }
    return 0;
}



/************************ THREADS (NETWORK) ******************************/

//SENDER
// Read data from ring buffer and write to udp port. Packets are
// always sent with a full payload.
void *sender_thread(void *arg) {
    Sender *sender = (Sender *) arg;
    networkPacket p;                           //Network package
    p.index = 0;                          //Inicializa el indice a 0
    uint32_t localIndex = 0;

    while(1) {
        sender->jack_ringbuffer_wait_for_read(sender->getPayloadBytes(),
                                              sender->getComPipe(0));

        localIndex++;
        p.index = localIndex;
        if (p.index % 1000 == 1)
            printf("Indice nuevo paquete: %d \n", p.index);
        p.channels = sender->getChannels();
        p.frames = sender->getPayloadSamples() / sender->getChannels();

        sender->jack_ringbuffer_read_exactly((char *)&(p.data),
                                             sender->getPayloadBytes());
        sender->packet_sendto(&p);
    }
    return NULL;
}



/*********************************** MAIN *************************************/
int main () {
    int modeSelected;
    int intAux;
    pthread_t netcomThread;         //Thread to manage UDP communication

    cout<<"FPGA Simulator: 1. Start - 2. Exit \n";
    cout<<"Mode: ";
    cin>>modeSelected;

    if (modeSelected==1) { //Sender
        cout<<"Sender mode selected. \n";
        senderObj.create_socket_connection();
        senderObj.init_isAddress(1);
        //senderObj.sender_socket_test();

        senderObj.open_file();

        senderObj.open_jack_client("sender_client");
        //Sensibilidad de los mensajes de error a mostrar de Jack. Minimo.
        jack_set_error_function(jack_client_error_handler);
        //Register a function (and argument) to be called if and when the
        //JACK server shuts down the client thread
        jack_on_shutdown(senderObj.getClientfd(),
                         jack_client_shutdown, 0);
        //Tell the Jack server to call @a process_callback whenever there is
        //work be done, passing @a arg as the second argument.
        jack_set_process_callback(senderObj.getClientfd(), jack_callback_sender, NULL);
        senderObj.jack_port_make_standard(1);
        senderObj.jack_client_activate(senderObj.getClientfd());

        cout<<"Creating sender thread. \n";
        intAux = pthread_create(&netcomThread,NULL,
                       sender_thread, &senderObj);
        if (intAux != 0)
            cout<<"Sender thread error. \n";

        //The pthread_join() function suspends execution of
        //the calling thread until the target thread terminates
        pthread_join(netcomThread, NULL);

        senderObj.finish();
        pthread_exit(NULL);
    }

    return 0;
}
