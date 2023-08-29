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

} IOsocket;

void create_socket(IOsocket *iosocket) {
  // Socket creation
  iosocket->server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (iosocket->server_socket == -1) {
    perror("Socket");
    exit(EXIT_FAILURE);
  }
}

void configure_unix_server_address(IOsocket *iosocket) {
  // Configure server address
  iosocket->server_addr.sun_family = AF_UNIX;
  strcpy(iosocket->server_addr.sun_path, "unix_socket");
}

void binding_socket(IOsocket *iosocket) {
  // Binding the socket
  if (bind(iosocket->server_socket, (struct sockaddr *)&(iosocket->server_addr),
           sizeof(iosocket->server_addr)) == -1) {
    perror("Bind");
    exit(EXIT_FAILURE);
  }
}
void start_listening(IOsocket *iosocket) {
  // Start listening
  listen(iosocket->server_socket, 5);
  puts("Server listening!!!");
}

void set_file_IO_settings(IOsocket *iosocket) {
  FD_ZERO(&iosocket->FDs);
  FD_SET(iosocket->server_socket, &iosocket->FDs);
  iosocket->fd_max = iosocket->server_socket;
}

void set_timeout(IOsocket *iosocket, int timeOut_sec) {
  /* Set timeout for waiting for change in the select function */
  iosocket->timeOut.tv_sec = timeOut_sec;
  iosocket->timeOut.tv_usec = 0;
}

void check_for_fd_updates(IOsocket *iosocket) {
  /* Make a copy of the file descriptors (because select function is
   * desctructive) */
  iosocket->FDs_copy = iosocket->FDs;
  /* Select will search for any change in file descriptors, it will detect
   * incoming IOsockets */
  iosocket->fd_Num = select(iosocket->fd_max + 1, &iosocket->FDs_copy, 0, 0,
                            &iosocket->timeOut);
}

int search_for_new_connections_and_handle(IOsocket *iosocket, int (*ptr)(int)) {
  set_timeout(iosocket, 5);

  check_for_fd_updates(iosocket);
  if (iosocket->fd_Num == -1) {
    /* error occured */
    perror("Select");
    return -1;
  } else if (iosocket->fd_Num == 0) {
    /* No change, skip below code and continue */
    return 0;
  } else {
    /* if FD num is not 0 then check all file descriptors for ISSET */
    for (int i = 0; i < iosocket->fd_max + 1; i++) {
      if (FD_ISSET(i, &iosocket->FDs_copy)) {
        if (i == iosocket->server_socket) {
          // new IOsocket when the file descriptor is the sane as
          // server_socket fd
          uint32_t clen = sizeof(iosocket->client_addr);
          int client_socket =
              accept(iosocket->server_socket,
                     (struct sockaddr *)&iosocket->client_addr, &clen);
          /* Set the new client socket */
          FD_SET(client_socket, &iosocket->FDs);
          if (client_socket == -1) {
            perror("Accept");
            continue;
          } else if (iosocket->fd_max < client_socket) {
            /* update the maximum number of the file descriptors to account
             * the new client IOsockets */
            iosocket->fd_max = client_socket;
          } else {
            // code
          }
          printf("Client id: %d is connected!\n", client_socket);
        } else {
          /* this is a client asking, handle the IOsocket of this socket */
          if ((*ptr)(i) == -1) {
            // Remove the file desciptor from the array of FDs!
            FD_CLR(i, &iosocket->FDs);
            close(i);
            printf("\nClient id: %d disconnected!\n", i);
          }
        }
      }
    }
  }
  return EXIT_SUCCESS;
}