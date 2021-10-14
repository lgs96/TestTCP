#include <string>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;

class sclient {
public:
  int connectToServer(string hostname, int port);
  void sendData(int sock, string data);
<<<<<<< Updated upstream
=======
  string createTcpInfo(int sock, string filePath);
  static void writeTcpInfo(int sock, string filePath);
  void closeTcpInfo(ofstream& file);
  void sendObject(int sock, string data, string filePath, string externalPath);
>>>>>>> Stashed changes
  string recvData(int sock);
  void disconnectFromServer(int sock);
};
