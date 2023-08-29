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

typedef struct {
  int server_socket;
  struct sockaddr_un server_addr;
  struct sockaddr_un *client_addr;
  // set IO file setting
  struct timeval timeOut;
  fd_set *FDs, FDs_copy;
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
  FD_ZERO(connection->FDs);
  FD_SET(connection->server_socket, connection->FDs);
  connection->fd_max = connection->server_socket;
}


int handle_connection(int client_socket);

int main() {
  int server_socket;
  int client_socket;
  struct sockaddr_un server_addr;
  struct sockaddr_un client_addr;

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
  puts("Server listening!!!");
  // set IO file setting
  struct timeval timeOut;
  fd_set FDs, FDs_copy;

  FD_ZERO(&FDs);
  FD_SET(server_socket, &FDs);
  int fd_max = server_socket;
  int fdNum;

  while (1) {
    /* Make a copy of the file descriptors (because select function is
     * desctructive) */
    FDs_copy = FDs;
    /* Set timeout for waiting for change in the select function */
    timeOut.tv_sec = 5;
    timeOut.tv_usec = 0;

    fdNum = select(fd_max + 1, &FDs_copy, 0, 0, &timeOut);
    /* Select will search for any change in file descriptors, it will detect
     * incoming connections */
    if (fdNum == -1) {
      /* error occured */
      perror("Select");
      break;
    } else if (fdNum == 0) {
      /* No change, skip below code and continue */
      continue;
    } else {
      /* if FD num is not 0 then check all file descriptors for ISSET */
      for (int i = 0; i < fd_max + 1; i++) {
        if (FD_ISSET(i, &FDs_copy)) {
          if (i == server_socket) {
            // new connection when the file descriptor is the sane as
            // server_socket fd
            uint32_t clen = sizeof(client_addr);
            client_socket =
                accept(server_socket, (struct sockaddr *)&client_addr, &clen);
            /* Set the new client socket */
            FD_SET(client_socket, &FDs);
            if (client_socket == -1) {
              perror("Accept");
              continue;
            } else if (fd_max < client_socket) {
              /* update the maximum number of the file descriptors to account
               * the new client connections */
              fd_max = client_socket;
            } else {
              // code
            }
            printf("Client id: %d is connected!\n", client_socket);
          } else {
            /* this is a client asking, handle the connection of this socket */
            if (handle_connection(i) == -1) {
              // Remove the file desciptor from the array of FDs!
              FD_CLR(i, &FDs);
              close(i);
              printf("\nClient id: %d disconnected!\n", client_socket);
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