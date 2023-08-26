/* This jenni will take the SQL commands from clients and execute them to the
 * database, this was made to ensure only single connection to the database. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
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

  int clen = sizeof(client_addr);
  client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &clen);

  while (1) {
    // read(client_socket, read_buffer, sizeof(read_buffer));
    // Recieve from client, if not recieved then close the socket.
    if (recv(client_socket, read_buffer, sizeof(read_buffer), 0) == 0) {
      close(client_socket);
      break;
    };
    printf("\nServer: I recieved %s from client: %d!\n", read_buffer,
           client_socket);
    // write(client_socket, read_buffer, 1);
    // Send to the client, if error occured then close the socket.
    if (send(client_socket, read_buffer, sizeof(read_buffer) - 1, 0) == -1) {
      close(client_socket);
      break;
    };
    if (strcmp(read_buffer, "q") == 0) {
      close(client_socket);
      break;
    }
    memset(read_buffer, 0, 100);
  }
  close(server_socket);
  exit(0);
  return 0;
}