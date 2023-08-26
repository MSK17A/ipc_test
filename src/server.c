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

int handle_connection(int client_socket);

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

  // set IO file setting
  struct timeval timeOut;
  fd_set FDs, FDs_copy;

  FD_ZERO(&FDs);
  FD_SET(server_socket, &FDs);
  int fd_max = server_socket;
  int fdNum;

  while (1) {
    FDs_copy = FDs;
    timeOut.tv_sec = 5;
    timeOut.tv_usec = 0;

    fdNum = select(fd_max + 1, &FDs_copy, 0, 0, &timeOut);
    if (fdNum == -1) {
      perror("Select");
      break;
    } else if (fdNum == 0) {
      continue;
    } else {
      for (int i = 0; i < fd_max + 1; i++) {
        if (FD_ISSET(i, &FDs_copy)) {
          if (i == server_socket) {
            // new connection
            int clen = sizeof(client_addr);
            client_socket =
                accept(server_socket, (struct sockaddr *)&client_addr, &clen);
            FD_SET(client_socket, &FDs);
            if (client_socket == -1) {
              perror("Accept");
              continue;
            } else if (fd_max < client_socket) {
              fd_max = client_socket;
            } else {
              // code
            }
            fprintf(stdout, "\nClient id: %d is connected!", client_socket);
          } else {
            if (handle_connection(i) == -1) {
              // Remove the file desciptor from the array of FDs!
              FD_CLR(i, &FDs);
              close(i);
              fprintf(stdout, "\nClient id: %d disconnected!", client_socket);
            }
          }
        }
      }
    }
  }
  close(server_socket);
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