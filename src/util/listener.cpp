//
// Created by rjcunningham on 9/7/17.
//

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "listener.hpp"

/*
 * Requires:
 *   "port" should be a valid port number on the machine in use.
 *
 * Effects:
 *   Opens a socket to listen on "port". Returns the file descriptor of the
 *   socket or -1 on error.
 */
int
open_listen(int port)
{
    int listenfd;
    struct sockaddr_in serveraddr = {};

    /* Set "listenfd" to a newly created stream socket */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("TCP Socket Creation Error");
        return (-1);
    }

    /*
    * Set the IP address of serveraddr to be the special ANY IP address
    * and set port to be the input port.  Be careful to ensure that the
    * IP address and port are in network byte order.
    */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons((uint16_t) port);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Use bind to set the address of "listenfd" to be serveraddr */
    if (bind(listenfd, (struct sockaddr *) &serveraddr,
             sizeof(struct sockaddr)) == -1) {
        perror("TCP Socket Bind Error");
        return (-1);
    }

    /* Use listen to make the socket ready to accept connection requests */
    if (listen(listenfd, LISTEN_MAX) == -1) {
        perror("TCP Socket Listen Error");
        return (-1);
    }

    #ifdef DEBUG_LISTENER
        fprintf(stderr, "Opened listening fd on %d\n", listenfd);
    #endif /*DEBUG_LISTENER*/
    return listenfd;
}

int wait_for_connection(int port, sockaddr *sa) {
    socklen_t clientlen = sizeof(struct sockaddr_in);
    int listenfd, connfd;

    /* Create a TCP port, and check that it is valid. */
    listenfd = open_listen(port);
    if (listenfd < 0) {
        // TODO have a procedure (on a new thread) to create a new port and listen again
        fprintf(stderr, "Failed to open listener on %d\n", port);
        exit(1);
    }

    /*
     * Accept a new connection and return the file descriptor of
     * the client.
     */
    if((connfd = accept(listenfd, sa, &clientlen)) == -1) {
        perror("TCP Client Accept Error");
        return (-1);
    };

    #ifdef DEBUG_LISTENER
        fprintf(stderr, "Received request on connfd on %d\n", connfd);
    #endif /*DEBUG_LISTENER*/

    // Close this since we only care about what the client sends to us.
    close(listenfd);
    return connfd;
}

int
create_send_fd(int port, sockaddr_in *sa) {
    int udp_server_fd;

    // Create a new UDP socket
    if((udp_server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("UDP Socket Creation Error");
        exit(-1);
    }

    // These settings must match those on the mission control end
    sa->sin_family = AF_INET;
    sa->sin_port = htons((uint16_t) port);
    sa->sin_addr.s_addr = htonl(INADDR_ANY);
    bzero(sa->sin_zero, 8);

    #ifdef DEBUG_LISTENER
        fprintf(stderr, "Sending on fd %d\n", udp_server_fd);
    #endif /*DEBUG_LISTENER*/

    return udp_server_fd;
}
