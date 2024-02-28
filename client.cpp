#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <unordered_set>

#include "util.h"

using namespace std;

int main(int argc, char *argv[]) {
  // --------------------------------------------------------------------------
  // 0- Setup.
  // --------------------------------------------------------------------------
  if (argc != 4 && argc != 5 && argc != 6) {
    cerr << "usage: " << argv[0]
         << " <server_address> <n_port> <command> <filename>\nOR...\nusage: "
         << argv[0]
         << " <server_address> <n_port> <command> <filename> "
            "<receive_directory>\nOR...\nusage: "
         << argv[0] << " <server_address> <n_port> KILL " << endl;
    exit(1);
  }

  string serverAddrStr(argv[1]);
  string negoPortStr(argv[2]);
  string command(argv[3]);
  string fileName;
  if (argc == 5) {
    fileName = argv[4];
  }
  // Default directory to receive files is ./clientfiles.
  string receiveDirectory = "./clientfiles";
  if (argc == 6) {
    receiveDirectory = string(argv[5]);
  }

  // --------------------------------------------------------------------------
  // 1- Create a socket for the negotiation stage (client side) using UDP.
  // --------------------------------------------------------------------------
  struct hostent *server = gethostbyname(serverAddrStr.c_str());
  if (server == nullptr) {
    Util::err("Error connecting to host to start negotiation.");
  }
  auto [negoSockFd, _, udpServ] = Util::openUdp(server, negoPortStr);

  // --------------------------------------------------------------------------
  // 2- Construct a terminate request to gracefully terminate the server.
  // --------------------------------------------------------------------------
  if (command == "KILL") {
    if (sendto(negoSockFd, command.c_str(), command.size(), 0,
               (struct sockaddr *)&udpServ, sizeof(udpServ)) < 0) {
      Util::err("Error sending GET request (negotiation).");
    }
    close(negoSockFd);
    return 0;
  }

  // --------------------------------------------------------------------------
  // 3- Construct a GET request.
  // --------------------------------------------------------------------------
  if (command == "GET") {
    auto [sockFd, portStr] = Util::openTcpGeneric();

    // Start listening for transactions from the server.
    listen(sockFd, 5);

    string getmessage = "GET " + fileName + " " + portStr;

    // Send the GET request to the server using UDP (negotiation stage).
    if (sendto(negoSockFd, getmessage.c_str(), getmessage.size(), 0,
               (struct sockaddr *)&udpServ, sizeof(udpServ)) < 0) {
      Util::err("Error sending GET request (negotiation).");
    }

    // Receive the UDP response from the server.
    char buffer[256];
    bzero(buffer, 256);
    if (recvfrom(negoSockFd, buffer, sizeof(buffer), 0, NULL, NULL) < 0) {
      Util::err(
          "Error receiving response from server after GET request "
          "(negotiation).");
    }
    cout << "Server responded: " << buffer << endl;

    if (strcmp("200 OK", buffer) == 0) {
      // Accept the TCP connection from the server if we have an OK status.
      int newCliTransSockFD =
          accept(sockFd, (struct sockaddr *)nullptr, nullptr);
      if (newCliTransSockFD < 0) {
        Util::err("Error on accepting server through TCP.");
      }

      // Read the data from the TCP connection.
      string receivedFileLocation = receiveDirectory + "/" + fileName;

      Util::readData(receivedFileLocation, newCliTransSockFD);

      close(newCliTransSockFD);
    }
  }

  // --------------------------------------------------------------------------
  // 4- Construct a PUT request.
  // --------------------------------------------------------------------------
  if (command == "PUT") {
    string getmessage = "PUT " + fileName;

    // Send the PUT request to the server using UDP (negotiation stage).
    if (sendto(negoSockFd, getmessage.c_str(), getmessage.size(), 0,
               (struct sockaddr *)&udpServ, sizeof(udpServ)) < 0) {
      Util::err("Error sending PUT request (negotiation).");
    }

    // Receive the UDP response from the server.
    char buffer[256];
    bzero(buffer, 256);
    if (recvfrom(negoSockFd, buffer, sizeof(buffer), 0, NULL, NULL) < 0) {
      Util::err(
          "Error receiving response from server after PUT request "
          "(negotiation).");
    }
    cout << "Negotiation message from server: " << buffer << endl;

    // Create the client's TCP socket.
    string rPortStr(buffer);
    auto [transSockFd] = Util::openTcpSpecific(
        stoi(rPortStr), server->h_addr_list[0], server->h_length);

    // Send file over TCP, 1024 bytes at a time.
    string fileNameWithDirectory = receiveDirectory + "/" + fileName;
    Util::sendData(fileNameWithDirectory, transSockFd);
    close(transSockFd);
  }

  close(negoSockFd);
  return 0;
}
