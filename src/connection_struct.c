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

void set_timeout(Connection *connection, int timeOut_sec) {
  /* Set timeout for waiting for change in the select function */
  connection->timeOut.tv_sec = timeOut_sec;
  connection->timeOut.tv_usec = 0;
}

void check_for_fd_updates(Connection *connection) {
  /* Make a copy of the file descriptors (because select function is
   * desctructive) */
  connection->FDs_copy = connection->FDs;
  /* Select will search for any change in file descriptors, it will detect
   * incoming connections */
  connection->fd_Num = select(connection->fd_max + 1, &connection->FDs_copy, 0,
                              0, &connection->timeOut);
}

int search_for_new_connections_and_handle(Connection *connection,
                                          int (*ptr)(int)) {
  set_timeout(connection, 5);

  check_for_fd_updates(connection);
  if (connection->fd_Num == -1) {
    /* error occured */
    perror("Select");
    return -1;
  } else if (connection->fd_Num == 0) {
    /* No change, skip below code and continue */
    return 0;
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
          if ((*ptr)(i) == -1) {
            // Remove the file desciptor from the array of FDs!
            FD_CLR(i, &connection->FDs);
            close(i);
            printf("\nClient id: %d disconnected!\n", i);
          }
        }
      }
    }
  }
  return EXIT_SUCCESS;
}