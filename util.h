#ifndef __UTIL_H__
#define __UTIL_H__

#define MAX_PORT_NO 99999

#include <string>
#include <tuple>
#include <unordered_set>
using namespace std;

struct Util {
  // A simple function to err and return 1.
  static void err(const char* msg);

  // Open a TCP socket, being able to accept any connection.
  static tuple<int, string> openTcpGeneric();

  // Open a TCP socket and connect to a specific other host.
  static tuple<int> openTcpSpecific(int port, char* src, size_t size);

  // Open a UDP connection.
  static tuple<int, string, struct sockaddr_in> openUdp(
      struct hostent* server = nullptr, string nPortStr = "");

  // Read data from the socket into a file dest.
  static void readData(string dest, int sockFd);

  // Send data from a file dest through the socket.
  static void sendData(string dest, int sockFd);

  // Generate a hashset of all files present in the given directory.
  static unordered_set<string> findPresentFiles(string directoryPath);
};

#endif  // __UTIL_H__
