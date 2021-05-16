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
  void sendData(int sock, string data);
  void createTcpInfo(int sock, string filePath);
  static void writeTcpInfo(int sock, string filePath);
  void closeTcpInfo(ofstream& file);
  void timer_start(std::function<void(int, string)> func, unsigned int interval, int sock, string filePath);
  void sendObject(int sock, string data, string filePath, string externalPath);
  string recvData(int sock);
  void disconnectFromServer(int sock);
};
