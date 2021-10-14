#include "client.h"
<<<<<<< Updated upstream
=======
#include <android/log.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <chrono>
#include <thread>
#include <functional>
#include <sys/time.h>
#define SEC_PER_DAY   86400
#define SEC_PER_HOUR  3600
#define SEC_PER_MIN   60

string get_tcp_info(int sock) {
    int len;
    //int sock;
    struct tcp_info info;

    //sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    len = sizeof(info);
    getsockopt(sock, IPPROTO_TCP, TCP_INFO, &info, (socklen_t*)&len);
    __android_log_print(ANDROID_LOG_INFO, "CWND", "CWND: %u, RTT: %f, Inflight: %u, lost: %u, retx: %u", info.tcpi_snd_cwnd*info.tcpi_snd_mss, float(info.tcpi_rtt)/1000000, info.tcpi_unacked*info.tcpi_snd_mss,
            info.tcpi_state, info.tcpi_total_retrans);
    struct timeval tv;
    gettimeofday(&tv, NULL);

    char char_arr[100];
    sprintf(char_arr, "%f\t%u\t%f\t%u\t%u\t%u\t%u", (tv.tv_sec)%1000 + (tv.tv_usec)/1000000.0, info.tcpi_snd_cwnd*info.tcpi_snd_mss, float(info.tcpi_rtt)/1000000, info.tcpi_unacked*info.tcpi_snd_mss, info.tcpi_total_retrans, info.tcpi_lost, info.tcpi_ca_state);
    string str(char_arr);
    return str;
}
>>>>>>> Stashed changes


int sclient::connectToServer(string hostname, int port) {
  int sockets = 0;
  __android_log_print(ANDROID_LOG_INFO, "connectToServer","Init!");
  struct sockaddr_in addr;
  if ((sockets = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    return -1;
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  if (inet_pton(AF_INET, hostname.c_str(), &addr.sin_addr) <= 0) {
    return -1;
  }

  if (connect(sockets, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    return -1;
  }
  return sockets;
}

void sclient::sendData(int sock, string data) {
<<<<<<< Updated upstream
    send(sock, data.c_str(), sizeof(data), 0);
}

=======
    //send(sock, data.c_str(), sizeof(data), 0);
    write(sock, data.c_str(), sizeof(data));
    __android_log_print(ANDROID_LOG_INFO, "sendData","send data!");
    //get_tcp_info(sock);
}

string sclient::createTcpInfo(int sock, string filePath){
    std::ofstream outfile;
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
// Form the seconds of the day
    long hms = tv.tv_sec % SEC_PER_DAY;
    hms += tz.tz_dsttime * SEC_PER_HOUR;
    hms -= tz.tz_minuteswest * SEC_PER_MIN;
// mod `hms` to insure in positive range of [0...SEC_PER_DAY)
    hms = (hms + SEC_PER_DAY) % SEC_PER_DAY;
    int hour = hms / SEC_PER_HOUR;
    int min = (hms % SEC_PER_HOUR) / SEC_PER_MIN;
    int sec = (hms % SEC_PER_HOUR) % SEC_PER_MIN;

    string now = std::to_string((tv.tv_sec)%1000);
    string file_name =  filePath + "/tcp_info_" + to_string(hour) +":"+ to_string(min) +":"+ to_string(sec) + ".txt" ;
    outfile.open(file_name, std::ofstream::trunc);
    //__android_log_print(ANDROID_LOG_INFO, "tcp_info", "writeTcpInfo %u", (tv.tv_sec));
    outfile<<"Time\tCWND\tRTT\tInflight\tretx\tlost\tstate"<<std::endl;
    outfile.close();
    //timer_start(writeTcpInfo,  10, sock, filePath);

    return file_name;
}

void sclient::writeTcpInfo(int sock, string filePath) {
    std::ofstream outfile;
    outfile.open(filePath, std::ofstream::app);
    outfile<<get_tcp_info(sock)<<std::endl;
    outfile.close();

    //__android_log_print(ANDROID_LOG_INFO, "tcp_info", "writeTcpInfo %s", (filePath+"/tcp_info.txt").c_str());
}

void sclient::closeTcpInfo(std::ofstream& file) {
    __android_log_print(ANDROID_LOG_INFO, "tcp_info", "closeTcpInfo");
    file.close();
}


void sclient::sendObject(int sock, string data, string filePath, string externalPath){
    string file_name = createTcpInfo(sock, externalPath);
    bool _execute = true;
    /*
    std::thread tcp_info_thread([&]()
                                {
                                    int k = 0;
                                    while (_execute)
                                    {
                                        k += 1;
                                        __android_log_print(ANDROID_LOG_INFO, "tcp_info", "loop: %u", k);
                                        __android_log_print(ANDROID_LOG_INFO, "tcp_info", "timer run");
                                        writeTcpInfo(sock, file_name);
                                         __android_log_print(ANDROID_LOG_INFO, "tcp_info", "timer run");
                                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                                    }
                                });
    tcp_info_thread.detach();
    */
    std::thread socket_thread([=]()
                              {
                                  FILE * file;
                                  char ch [100];
                                  strcpy(ch, filePath.c_str());
                                  file = fopen(ch,"r");
                                  char send_buffer[1024];
                                  while (!feof(file)) {
                                      fread(send_buffer, 1, sizeof(send_buffer), file);
                                      write(sock, send_buffer, sizeof(send_buffer));
                                      __android_log_print(ANDROID_LOG_INFO, "sendObject", "Transmit object", feof(file));
                                  }
                              });
    socket_thread.join();

    if (!socket_thread.joinable())
    {
        string done = "done";
        write(sock, done.c_str(), sizeof(done));
        __android_log_print(ANDROID_LOG_INFO, "sendObject", "Transmission completed");
        _execute = false;
    }
}


>>>>>>> Stashed changes
string sclient::recvData(int sock) {
    char buffer[1024] = {0};
    int valread = read(sock, buffer, 1024);
    if (valread != -1)
        return buffer;
    else return "Error";
}

void sclient::disconnectFromServer(int sock) {
    close(sock);
}
