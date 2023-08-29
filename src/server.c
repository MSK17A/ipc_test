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
  Connection *connection = malloc(sizeof(Connection));

  create_socket(connection);
  configure_unix_server_address(connection);
  binding_socket(connection);
  start_listening(connection);
  set_file_IO_settings(connection);

  while (1) {
    /* Make a copy of the file descriptors (because select function is
     * desctructive) */
    connection->FDs_copy = connection->FDs;
    /* Set timeout for waiting for change in the select function */
    connection->timeOut.tv_sec = 5;
    connection->timeOut.tv_usec = 0;

    connection->fd_Num = select(connection->fd_max + 1, &connection->FDs_copy,
                                0, 0, &connection->timeOut);
    /* Select will search for any change in file descriptors, it will detect
     * incoming connections */
    if (connection->fd_Num == -1) {
      /* error occured */
      perror("Select");
      break;
    } else if (connection->fd_Num == 0) {
      /* No change, skip below code and continue */
      continue;
    } else {
      /* if FD num is not 0 then check all file descriptors for ISSET */
      for (int i = 0; i < connection->fd_max + 1; i++) {
        if (FD_ISSET(i, &connection->FDs_copy)) {
          if (i == connection->server_socket) {
            // new connection when the file descriptor is the sane as
            // server_socket fd
            uint32_t clen = sizeof(connection->client_addr);
            int client_socket =
                accept(connection->server_socket,
                       (struct sockaddr *)&connection->client_addr, &clen);
            /* Set the new client socket */
            FD_SET(client_socket, &connection->FDs);
            if (client_socket == -1) {
              perror("Accept");
              continue;
            } else if (connection->fd_max < client_socket) {
              /* update the maximum number of the file descriptors to account
               * the new client connections */
              connection->fd_max = client_socket;
            } else {
              // code
            }
            printf("Client id: %d is connected!\n", client_socket);
          } else {
            /* this is a client asking, handle the connection of this socket */
            if (handle_connection(i) == -1) {
              // Remove the file desciptor from the array of FDs!
              FD_CLR(i, &connection->FDs);
              close(i);
              printf("\nClient id: %d disconnected!\n", i);
            }
          }
        }
      }
    }
  }
  close(connection->server_socket);
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