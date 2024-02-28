#include "util.h"

#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <utility>

using namespace std;

// See util.h.
void Util::err(const char *msg) {
  cerr << msg << endl;
  exit(1);
}

// See util.h.
tuple<int, string> Util::openTcpGeneric() {
  struct sockaddr_in tcpServ;
  int rPortNo = 1000;
  int cliTransSockFD = socket(AF_INET, SOCK_STREAM, 0);
  if (cliTransSockFD < 0) {
    Util::err("Error opening tcp socket for transaction.");
  }

  // Keep retrying until free port found.
  while (rPortNo < MAX_PORT_NO) {
    tcpServ.sin_family = AF_INET;
    tcpServ.sin_port = htons(rPortNo);
    tcpServ.sin_addr.s_addr = INADDR_ANY;

    if (bind(cliTransSockFD, (struct sockaddr *)&tcpServ, sizeof(tcpServ)) <
        0) {
      rPortNo++;
      continue;
    }
    break;
  }
  string rPortStr = to_string(rPortNo);

  return {cliTransSockFD, rPortStr};
}

// See util.h.
tuple<int> Util::openTcpSpecific(int port, char *src, size_t size) {
  struct sockaddr_in tcpServ;
  int sockFd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockFd < 0) {
    Util::err("Error opening TCP socket for transmission.");
  }

  tcpServ.sin_family = AF_INET;
  tcpServ.sin_port = htons(port);
  bcopy(src, (char *)&tcpServ.sin_addr.s_addr, size);
  if (connect(sockFd, (struct sockaddr *)&tcpServ, sizeof(tcpServ)) < 0) {
    Util::err("Error connecting server to client for TCP transmission.");
  }

  return {sockFd};
}

// See util.h.
tuple<int, string, struct sockaddr_in> Util::openUdp(struct hostent *server,
                                                     string nPortStr) {
  int sockFd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockFd < 0) {
    Util::err("Error opening udp socket for negotiation.");
  }

  if (server) {
    struct sockaddr_in udpServ;
    udpServ.sin_family = AF_INET;
    udpServ.sin_port = htons(stoi(nPortStr));
    bcopy((char *)server->h_addr_list[0], (char *)&udpServ.sin_addr.s_addr,
          server->h_length);
    return {sockFd, nPortStr, udpServ};
  }

  // Keep retrying until free port found.
  int portNo = 1000;
  struct sockaddr_in udpServ;
  while (portNo < MAX_PORT_NO) {
    udpServ.sin_family = AF_INET;
    udpServ.sin_addr.s_addr = INADDR_ANY;
    udpServ.sin_port = htons(portNo);

    if (bind(sockFd, (struct sockaddr *)&udpServ, sizeof(udpServ)) < 0) {
      portNo++;
      continue;
    }
    break;
  }

  string portStr = to_string(portNo);
  return {sockFd, portStr, udpServ};
}

// See util.h.
void Util::readData(string dest, int sockFd) {
  ofstream received_file(dest, ios::trunc);
  char buffer[1024];
  int bytes_received;

  // Keep reading until EOF.
  while ((bytes_received = read(sockFd, buffer, sizeof(buffer))) > 0) {
    received_file.write(buffer, bytes_received);
  }
  received_file.close();
}

// See util.h.
void Util::sendData(string dest, int sockFd) {
  ifstream file_to_send(dest, std::ios::binary);
  char buffer[1024];
  int bytes_read;

  // Keep receiving until EOF.
  while ((bytes_read = file_to_send.readsome(buffer, sizeof(buffer))) > 0) {
    cout << "Sending " << bytes_read << " bytes of data..." << endl;
    if (write(sockFd, buffer, bytes_read) < 0) {
      Util::err("Error transmitting file to client.");
    }
  }

  file_to_send.close();
}

// See util.h.
unordered_set<string> Util::findPresentFiles(string directoryPath) {
  unordered_set<string> filenames;

  try {
    for (const auto &entry : filesystem::directory_iterator(directoryPath)) {
      if (entry.is_regular_file()) {
        filenames.insert(entry.path().filename().string());
      }
    }
  } catch (const filesystem::filesystem_error &ex) {
    cerr << "Error accessing directory: " << ex.what() << endl;
    exit(1);
  }

  return filenames;
}
