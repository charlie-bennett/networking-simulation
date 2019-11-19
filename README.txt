struct sockaddr getPort;
socklen_t len = sizeof(getPort);
getsockname(sockfd, &getPort, &len);
And you can get your port from

(struct sockaddr_in *)&getPort)->sin_port
