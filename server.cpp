#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <unordered_set>
#include <vector>

#include "util.h"

using namespace std;

int main(int argc, char *argv[]) {
  // --------------------------------------------------------------------------
  // 0- Setup.
  // --------------------------------------------------------------------------
  if (argc != 2) {
    cerr << "usage: " << argv[0] << " <storage_directory>" << endl;
    exit(1);
  }
  string directoryPath(argv[1]);

  // --------------------------------------------------------------------------
  // 1- Create a socket for the negotiation stage (client side) using UDP.
  // --------------------------------------------------------------------------
  auto [negoSockFd, negoPortStr, _] = Util::openUdp();
  cout << "SERVER_PORT=" << negoPortStr << endl;

  // --------------------------------------------------------------------------
  // 2- Handle all incoming requests.
  // --------------------------------------------------------------------------
  while (true) {
    // Receive data from a client through UDP, and save its info.
    struct sockaddr_in cliAddr;
    socklen_t cliLen = sizeof(cliAddr);

    char negoBuffer[1024];
    memset(negoBuffer, 0, sizeof(negoBuffer));

    if (recvfrom(negoSockFd, negoBuffer, sizeof(negoBuffer), 0,
                 (struct sockaddr *)&cliAddr, &cliLen) < 0) {
      Util::err("Error receiving from socket through udp during negotiation.");
    }
    string receivedStr(negoBuffer);
    cout << "Received message: " << receivedStr << endl;

    // See if the server has received a kill signal.
    if (receivedStr == "KILL") {
      close(negoSockFd);
      exit(0);
    }

    // Deconstruct the message.
    vector<string> tokens;
    istringstream iss(receivedStr);
    string token;
    while (iss >> token) {
      tokens.push_back(token);
    }

    string reqCommand = tokens[0];
    string reqFileName = tokens[1];

    // ------------------------------------------------------------------------
    // 3- Handle GET requests.
    // ------------------------------------------------------------------------
    if (reqCommand == "GET") {
      string reqRPort = tokens[2];

      // Find all files present in the server directory.
      unordered_set<string> presentFiles =
          Util::findPresentFiles(directoryPath);

      // Found the file in the directory.
      if (presentFiles.count(reqFileName)) {
        // Open the server's TCP socket and connect to the client.
        auto [transSockFd] = Util::openTcpSpecific(
            stoi(reqRPort), (char *)&cliAddr.sin_addr.s_addr,
            sizeof(cliAddr.sin_addr.s_addr));

        // Respond to the client that contacted the server through UDP.
        if (sendto(negoSockFd, "200 OK", 20, 0, (struct sockaddr *)&cliAddr,
                   cliLen) < 0) {
          Util::err("Error responding back to client during negotiation.");
        }

        // Send file over TCP, 1024 bytes at a time.
        string fileNameWithDirectory = directoryPath + "/" + reqFileName;
        Util::sendData(fileNameWithDirectory, transSockFd);
        close(transSockFd);
      }

      // File not found.
      else {
        if (sendto(negoSockFd, "404 NOT FOUND", 20, 0,
                   (struct sockaddr *)&cliAddr, cliLen) < 0) {
          Util::err("Error responding back to client during negotiation.");
        }
      }

      cout << "Done processing request." << endl;
    }

    // ------------------------------------------------------------------------
    // 4- Handle PUT requests.
    // ------------------------------------------------------------------------
    if (reqCommand == "PUT") {
      // Open a generic TCP socket.
      auto [servTransSockFd, portStr] = Util::openTcpGeneric();

      // Start listening for transactions from the client.
      listen(servTransSockFd, 5);

      // Respond to the client that contacted the server through UDP.
      if (sendto(negoSockFd, portStr.c_str(), 20, 0,
                 (struct sockaddr *)&cliAddr, cliLen) < 0) {
        Util::err("Error responding back to client during negotiation.");
      }

      // Accept the client through TCP.
      int cliTransSockFd =
          accept(servTransSockFd, (struct sockaddr *)nullptr, nullptr);
      if (cliTransSockFd < 0) {
        Util::err("Error on accepting client through TCP.");
      }

      // Read the data from the TCP connection.
      string receivedFileLocation = directoryPath + "/" + reqFileName;
      Util::readData(receivedFileLocation, cliTransSockFd);
      close(cliTransSockFd);
    }
  }
}
