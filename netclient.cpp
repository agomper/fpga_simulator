#include "netclient.h"


int NetClient::getPayloadBytes() const
{
    return payloadBytes;
}

int NetClient::getPayloadSamples() const
{
    return payloadSamples;
}

NetClient::NetClient() {
    portNumber = 57160;
}

void NetClient::create_socket_connection() {
    //int socket(int domain, int type, int protocol)
    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketfd < 0) {
        perror("Socket creation failed \n");
        exit(1);
    }
}

void NetClient::init_isAddress(int mode) {
  isAddress.sin_family = AF_INET;
  isAddress.sin_port = htons(portNumber);

  if(mode == 1) { //Sender
      struct hostent *hostinfo = gethostbyname(hostname);
      if (hostinfo == NULL) {
          fprintf(stderr, "init_sockaddress: unknown host: %s.\n", hostname);
          exit(1);
      }
      else {
          isAddress.sin_addr = *(struct in_addr *) hostinfo->h_addr_list[0];
      }
  }
  else if (mode == 2) { //Receiver
      isAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  }
  else {
      cout<<"Error! init_sockaddress() Mode no valid. \n";
      exit (1);
  }
}

void NetClient::bind_isAddress() {
    //int bind(int fd, struct sockaddr *local_addr, socklen_t addr_length)
    int err = bind(socketfd, (struct sockaddr *)&isAddress, sizeof(isAddress));
    if(err < 0) {
        perror("bind_inet: bind() failed");
        exit(1);
    }
}

void NetClient::packet_sendto(networkPacket *p) {
  packet_hton(p); //Network byte order
  sendto_exactly(socketfd, (unsigned char *)p, sizeof(networkPacket), isAddress);
}

void NetClient::packet_recv(networkPacket *p) {
  recv_exactly(socketfd, (char *)p, sizeof(networkPacket), 0); //Network
  packet_ntoh(p); //Network byte order
}


//Network byte order
void NetClient::packet_hton(networkPacket *p)
{
  u_int8_t *d = p->data;
  int i = p->channels * p->frames;
  while(i--){
    hton32_buf(d, d);
    d += 4;
  }
  p->index = hton_i32(p->index);
  p->channels = hton_i16(p->channels);
  p->frames = hton_i16(p->frames);
}

//Network byte order
void NetClient::packet_ntoh(networkPacket *p)
{
  p->index = ntoh_i32(p->index);
  p->channels = ntoh_i16(p->channels);
  p->frames = ntoh_i16(p->frames);
  u8 *d = p->data;
  u32 i = p->channels * p->frames;
  while(i--) {
    ntoh32_buf(d, d);
    d += 4;
  }
}


void NetClient::sendto_exactly(int fd, const u8 *data, int n, struct sockaddr_in address)
{
  int err = xsendto(fd, data, n, 0, (struct sockaddr *)&address, sizeof(address));
  if(err != n) {
    fprintf(stderr, "sendto_exactly: partial write\n");
    exit(1);
  }
}

void NetClient::recv_exactly(int fd, void *buf, size_t n, int flags)
{
  int err = (int) xrecv(fd, buf, n, flags);
  if(err != n) {
    fprintf(stderr, "recv_exactly: partial recv (%d != %ul)\n", err, (int) n);
    exit(1);
  }
}

int NetClient::xsendto(int fd, const void *data, size_t n, int flags, struct sockaddr *addr, socklen_t length)
{
  int err = sendto(fd, data, n, flags, addr, length);
  if(err < 0) {
    perror("sendto() failed");
    exit(1);
  }
  return err;
}

int NetClient::xrecv(int fd, void *buf, size_t n, int flags)
{
  int err = recv(fd, buf, n, flags);
  if(err < 0) {
    perror("recv() failed");
    exit(1);
  }
  return err;
}
