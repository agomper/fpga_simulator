#include <unistd.h> /* POSIX */
#include <pthread.h>

#include "receiver.h"
#include "sender.h"

Sender senderObj;
Receiver recvObj;



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
            //        = (float) in[j][i]; //JACK PORTS
                      = fileBuffer[(i*senderObj.getChannels())+j];
        }
    }

    //Comprueba si hay espacio en buffer.
    int bytes_available = (int) jack_ringbuffer_write_space(senderObj.getRingBuffer());
    int bytes_to_write = nframes * sizeof(float) * senderObj.getChannels();
    if(bytes_to_write > bytes_available) {
        printf ("jack-udp send: buffer overflow error (UDP thread late)\n");
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


/************************************************************ JACK CLIENT RECV*/
// Write data from ring buffer to JACK output ports.
int jack_callback_receiver (jack_nframes_t nframes, void *arg){
    //Nframes = Frames/Period = = Buffer de JACK = 1024
    if(nframes >= recvObj.getJackBufferSize()) {
        cout<< "Fatal error. Not enough space!. "
               "JackRF64 buffer: "<<recvObj.getJackBufferSize()<<
               " and Frames: "<<nframes<<"\n";
        return -1;
    }

    //Conversion del RingBuffer a Jack Ports
    //Le dice los punteros out que apunten al mismo sitio que los Jack ports.
    int i, j;
    float *out[recvObj.getChannels()];
    float localFrame [recvObj.getChannels()*nframes];

    for(i = 0; i < recvObj.getChannels(); i++) {
        out[i] = (float *) jack_port_get_buffer(recvObj.getJackPort(i), nframes);
    }

    //Comprueba si tiene informacion para el Jack
    int nsamples = nframes * recvObj.getChannels();
    int nbytes = nsamples * sizeof(float);
    int bytes_available = (int) jack_ringbuffer_read_space(recvObj.getRingBuffer());
    //Si no tiene suficiente informacion, reseta los punteros out.
    if(nbytes > bytes_available) {
        printf("jack-udp recv: buffer underflow (%d > %d)\n",
                nbytes, bytes_available);
        for(i = 0; i < nframes; i++) {
            for(j = 0; j < recvObj.getChannels(); j++) {
                out[j][i] = (float) 0.0;
            }
        }

    }
    //Pero si tiene suficiente informacion se la da a Jack ports, mediante
    //los punteros out.
    else {
        recvObj.jack_ringbuffer_read_exactly(nbytes);
        for(i = 0; i < nframes; i++) {
            for(j = 0; j < recvObj.getChannels(); j++) {
                out[j][i] = (float)
                        recvObj.getJackBuffer()[(i * recvObj.getChannels()) + j];
                localFrame[(i * recvObj.getChannels()) + j] = out [j][i];
            }

        }
        recvObj.getSndfd().write(localFrame, recvObj.getChannels()*nframes);
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
    int localIndex = 0;

    while(1) {
        sender->jack_ringbuffer_wait_for_read(sender->getPayloadBytes(),
                                              sender->getComPipe(0));

        localIndex++;
        p.index = localIndex;
        printf("Indice nuevo paquete. (%d) %d\n", p.index, localIndex);
        p.channels = sender->getChannels();
        p.frames = sender->getPayloadSamples() / sender->getChannels();

        sender->jack_ringbuffer_read_exactly((char *)&(p.data),
                                             sender->getPayloadBytes());
        sender->packet_sendto(&p);
    }
    return NULL;
}


//RECEIVER
// Read data from UDP port and write to ring buffer.
void *receiver_thread(void *arg) {
    Receiver *receiver = (Receiver *) arg;
    networkPacket p;                       //Paquete P = Network
    int next_packet = -1;

    //Fichero
    FILE *filed;
    if ((filed = fopen ("Test", "w")) == NULL) {
        printf("Error opening the file. \n");
        exit(1);
    }
    else {
        printf("File created. \n");
    }

    while(1) {
        //Llama al metodo para recibir 1 paquete de P.
        receiver->packet_recv(&p);

        //Comprobaciones del indice y numero de canales
        if(p.index != next_packet/* || next_packet != -1*/) {
            printf("jack-udp recv: out of order packet "
                    "arrival (Esperado, Recibido) (%d, %d)\n",
                    next_packet, p.index);
            //exit(1);
        }
        if(p.channels != receiver->getChannels()) {
            printf("jack-udp recv: channel mismatch packet "
                    "arrival (Esperado, Recibido) (%d != %d)\n",
                    p.channels, receiver->getChannels());
            exit(1);
        }

        //Comprueba el espacio que tiene para escribir en el RingBuffer
        int bytes_available = (int) jack_ringbuffer_write_space(receiver->getRingBuffer());
        //Si no hay espacio, avisa.
        if(receiver->getPayloadBytes() > bytes_available) {
            printf("jack-udp recv: buffer overflow (%d > %d)\n",
                    (int) receiver->getPayloadBytes(), bytes_available);
        } else {
            receiver->jack_ringbuffer_write_exactly((char *) p.data,
                                          (size_t) receiver->getPayloadBytes());
            //Fichero
            fprintf(filed, "%d \n", p.index);
        }

        //Actualiza el indice del paquete que debe llegar.
        next_packet = p.index + 1;
    }
    return NULL;
}



/*********************************** MAIN *************************************/
int main () {
    int modeSelected;
    int intAux;
    pthread_t netcomThread;         //Thread to manage UDP communication

    cout<<"Please select mode: 1. Sender - 2. Receiver \n";
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
    }
    else { //Receiver
        cout<<"Receiver mode selected. \n";
        recvObj.create_socket_connection();
        recvObj.init_isAddress(2);
        recvObj.bind_isAddress();
        //recvObj.receiver_socket_test();

        recvObj.create_file("WAVFile", 2, 44100, SF_FORMAT_WAV | SF_FORMAT_PCM_16);

        recvObj.open_jack_client("receiver_client");
        //Sensibilidad de los mensajes de error a mostrar de Jack. Minimo.
        jack_set_error_function(jack_client_error_handler);
        //Register a function (and argument) to be called if and when the
        //JACK server shuts down the client thread
        jack_on_shutdown(recvObj.getClientfd(),
                         jack_client_shutdown, 0);
        //Tell the Jack server to call @a process_callback whenever there is
        //work be done, passing @a arg as the second argument.
        jack_set_process_callback(recvObj.getClientfd(), jack_callback_receiver, NULL);
        recvObj.jack_port_make_standard(2);
        recvObj.jack_client_activate(recvObj.getClientfd());

        cout<<"Creating receiver thread. \n";
        intAux = pthread_create(&netcomThread,NULL,
                       receiver_thread, &recvObj);
        if (intAux != 0)
            cout<<"Receiver thread error. \n";
    }

    //The pthread_join() function suspends execution of
    //the calling thread until the target thread terminates
    pthread_join(netcomThread, NULL);

    if (modeSelected==1)
        senderObj.finish();
    else
        recvObj.finish();

    pthread_exit(NULL);
    return 0;
}
