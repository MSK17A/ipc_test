#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

typedef struct {
  int server_socket;
  struct sockaddr_un server_addr;
  struct sockaddr_un client_addr;
  // set IO file setting
  struct timeval timeOut;
  fd_set FDs, FDs_copy;
  int fd_max, fd_Num;

} Connection;

void create_socket(Connection *connection) {
  // Socket creation
  connection->server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (connection->server_socket == -1) {
    perror("Socket");
    exit(EXIT_FAILURE);
  }
}
void configure_unix_server_address(Connection *connection) {
  // Configure server address
  connection->server_addr.sun_family = AF_UNIX;
  strcpy(connection->server_addr.sun_path, "unix_socket");
}

void binding_socket(Connection *connection) {
  // Binding the socket
  if (bind(connection->server_socket,
           (struct sockaddr *)&(connection->server_addr),
           sizeof(connection->server_addr)) == -1) {
    perror("Bind");
    exit(EXIT_FAILURE);
  }
}
void start_listening(Connection *connection) {
  // Start listening
  listen(connection->server_socket, 5);
  puts("Server listening!!!");
}

void set_file_IO_settings(Connection *connection) {
  FD_ZERO(&connection->FDs);
  FD_SET(connection->server_socket, &connection->FDs);
  connection->fd_max = connection->server_socket;
}