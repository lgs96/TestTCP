#include <string>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>

using namespace std;

class sclient {
public:
  int connectToServer(string hostname, int port);
  void sendData(int sock, string data, bool fin);
  string createTcpInfo(int sock, string filePath);
  static void writeTcpInfo(int sock, string filePath);
  void recordTcpInfo(int sock, string externalPath);
  void sendObject(int sock,  string filePath);
  string recvData(int sock);
  void disconnectFromServer(int sock);

  bool _execute = 0;
};
