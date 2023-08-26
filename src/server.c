/* This jenni will take the SQL commands from clients and execute them to the
 * database, this was made to ensure only single connection to the database. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

int main() {
  int server_socket;
  int client_socket;
  struct sockaddr_un server_addr;
  struct sockaddr_un client_addr;
  char read_buffer[100];

  // Socket creation
  server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_socket == -1) {
    perror("Socket");
    exit(EXIT_FAILURE);
  }

  // Configure server address
  server_addr.sun_family = AF_UNIX;
  strcpy(server_addr.sun_path, "unix_socket");

  // Binding the socket
  if (bind(server_socket, (struct sockaddr *)&server_addr,
           sizeof(server_addr)) == -1) {
    perror("Bind");
    exit(EXIT_FAILURE);
  }

  // Start listening
  listen(server_socket, 5);
  printf("Server listening!!!");

  fd_set current_sockets, ready_sockets;

  FD_ZERO(&current_sockets);
  FD_SET(server_socket, &current_sockets);

  while (1) {
    // Because select is destructive
    ready_sockets = current_sockets;
    if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0) {
      perror("Select");
      exit(EXIT_FAILURE);
    }

    // Loop through all ready sockets
    for (int i = 0; FD_SETSIZE; i++) {
      // if file descriptor is set means i is the file ready ot be read.
      if (FD_ISSET(i, &ready_sockets)) {
        if (i == server_socket) {
          // new connection
          client_socket =
              accept(i, (struct sockaddr *)&client_addr, sizeof(client_addr));

          // add this socket to the current sockets
          FD_SET(client_socket, &current_sockets);
        } else {
          // do whatever with the connection
          handle_connection(i);
        }
      }
    }
  }
  exit(0);
  return 0;
}