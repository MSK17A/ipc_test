/* This jenni will take the SQL commands from clients and execute them to the
 * database, this was made to ensure only single connection to the database. */

#include "connection_struct.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

int handle_connection(int client_socket);

int main() {
  IOsocket *io_socket = malloc(sizeof(IOsocket));

  create_socket(io_socket);
  configure_unix_server_address(io_socket);
  binding_socket(io_socket);
  start_listening(io_socket);
  set_file_IO_settings(io_socket);

  while (1) {
    if (search_for_new_connections_and_handle(io_socket, handle_connection) ==
        -1) {
      exit(EXIT_FAILURE);
    }
  }

  close(io_socket->server_socket);
  free(io_socket);

  return EXIT_SUCCESS;
}

int handle_connection(int client_socket) {
  char read_buffer[100];
  // read(client_socket, read_buffer, sizeof(read_buffer));
  // Recieve from client, if not recieved then close the socket.
  if (recv(client_socket, read_buffer, sizeof(read_buffer), 0) == 0) {
    return -1;
  };
  printf("\nServer: I recieved %s from client: %d!\n", read_buffer,
         client_socket);
  // write(client_socket, read_buffer, 1);
  // Send to the client, if error occured then close the socket.
  if (send(client_socket, read_buffer, sizeof(read_buffer) - 1, 0) == -1) {
    return -1;
  };
  if (strcmp(read_buffer, "q") == 0) {
    return -1;
  }
  return 1;
  memset(read_buffer, 0, 100);
}