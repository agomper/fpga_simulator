#ifndef NETCLIENT_H
#define NETCLIENT_H
//Network
#include <netdb.h>
#include <sys/socket.h>
extern "C" {
#include "c-common/byte-order.h"
}
//Comunes
#include "jackclient.h"
#include <stdint.h>

typedef struct {
  int index;                //Identificador del paquete
  uint16_t channels;             //Num channels
  uint16_t frames;               //Num frames
  uint8_t data[1024];            //Payload
} networkPacket;

class NetClient {
protected:
    sockaddr_in isAddress;            //Internet socket address
    int socketfd;                     //Socket File descriptor
    int portNumber;                   //Puerto comunicacion.
    char hostname[20];
    int payloadSamples = 256;
    int payloadBytes =  payloadSamples*sizeof(float);
public:
    NetClient();
    void create_socket_connection();
    void init_isAddress(int mode);
    void bind_isAddress();
    networkPacket getP() const;
    void setP(const networkPacket &value);
    int getPayloadBytes() const;
    int getPayloadSamples() const;
    void packet_sendto(networkPacket *p);
    void packet_recv(networkPacket *p);
    void packet_hton(networkPacket *p);
    void packet_ntoh(networkPacket *p);
    void sendto_exactly(int fd, const u8 *data, int n, struct sockaddr_in address);
    void recv_exactly(int fd, void *buf, size_t n, int flags);
    int xsendto(int fd, const void *data, size_t n, int flags, struct sockaddr *addr, socklen_t length);
    int xrecv(int fd, void *buf, size_t n, int flags);

};

#endif // NETCLIENT_H
