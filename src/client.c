#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

int main() {
  int server_socket;
  struct sockaddr_un server_addr;
  int connection_result;

  char *ch = "Hello World!!!";
  char read_buffer[100];

  server_socket = socket(AF_UNIX, SOCK_STREAM, 0);

  server_addr.sun_family = AF_UNIX;
  strcpy(server_addr.sun_path, "../unix_socket");

  connection_result = connect(server_socket, (struct sockaddr *)&server_addr,
                              sizeof(server_addr));

  if (connection_result == -1) {
    perror("Error:");
    exit(1);
  }

  // write(server_socket, ch, sizeof(&ch));
  send(server_socket, ch, strlen(ch) + 1, 0);
  // read(server_socket, read_buffer, 100);
  recv(server_socket, read_buffer, sizeof(read_buffer) - 1, 0);
  printf("Client: I recieved %s from server!\n", read_buffer);

  close(server_socket);
  exit(0);
}