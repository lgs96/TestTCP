#include "client.h"
#include <android/log.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <chrono>
#include <thread>
#include <functional>
#include <sys/time.h>

char * get_tcp_info(int sock) {
    int len;
    //int sock;
    struct tcp_info info;

    //sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    len = sizeof(info);
    getsockopt(sock, IPPROTO_TCP, TCP_INFO, &info, (socklen_t*)&len);
    __android_log_print(ANDROID_LOG_INFO, "CWND", "CWND: %u, RTT: %f, Inflight: %u, lost: %u, retx: %u", info.tcpi_snd_cwnd*info.tcpi_snd_mss, float(info.tcpi_rtt)/1000000, info.tcpi_unacked*info.tcpi_snd_mss,
            info.tcpi_state, info.tcpi_total_retrans);
    char str[100];
    sprintf(str, "%u\t%f\t%u\t%u\t%u", info.tcpi_snd_cwnd*info.tcpi_snd_mss, float(info.tcpi_rtt)/1000000, info.tcpi_unacked*info.tcpi_snd_mss, info.tcpi_total_retrans, info.tcpi_reordering);
    return str;
}


int sclient::connectToServer(string hostname, int port) {
  int sockets = 0;
  
  struct sockaddr_in addr;
  if ((sockets = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
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
    //send(sock, data.c_str(), sizeof(data), 0);
    write(sock, data.c_str(), sizeof(data));

    //get_tcp_info(sock);
}

void sclient::createTcpInfo(int sock, string filePath){
    std::ofstream outfile;
    outfile.open(filePath+"/tcp_info.txt",std::ofstream::app);
    __android_log_print(ANDROID_LOG_INFO, "tcp_info", "writeTcpInfo");
    outfile<<"CWND\tRTT\tInflight\tretx\tlost\treordering"<<std::endl;
    outfile.close();
    //timer_start(writeTcpInfo,  10, sock, filePath);
}

void sclient::writeTcpInfo(int sock, string filePath) {
    std::ofstream outfile;
    outfile.open(filePath+"/tcp_info.txt", ios::app);
    outfile<<get_tcp_info(sock)<<std::endl;
    outfile.close();
    __android_log_print(ANDROID_LOG_INFO, "tcp_info", "writeTcpInfo %s", (filePath+"/tcp_info.txt").c_str());
}

void sclient::closeTcpInfo(std::ofstream& file) {
    __android_log_print(ANDROID_LOG_INFO, "tcp_info", "closeTcpInfo");
    file.close();
}

void sclient::timer_start(std::function<void(int, string)> func, unsigned int interval, int sock, string filePath)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    __android_log_print(ANDROID_LOG_INFO, "tcp_info", "timer starts: %d", (double)(tv.tv_sec) + (double)(tv.tv_usec/ 1000000.0));
    std::thread([=]()
    {
        int k = 0;
        while (true)
        {
            k += 1;
            __android_log_print(ANDROID_LOG_INFO, "tcp_info", "loop: %u", k);
            //__android_log_print(ANDROID_LOG_INFO, "tcp_info", "timer run");
            func(sock, filePath);
           // __android_log_print(ANDROID_LOG_INFO, "tcp_info", "timer run");
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
    }).detach();
    __android_log_print(ANDROID_LOG_INFO, "tcp_info", "loop fin");
}

void sclient::sendObject(int sock, string data, string filePath, string externalPath){
    createTcpInfo(sock, externalPath);
    bool _execute = true;
    std::thread tcp_info_thread([&]()
    {
        int k = 0;
        while (_execute)
        {
            k += 1;
            __android_log_print(ANDROID_LOG_INFO, "tcp_info", "loop: %u", k);
            //__android_log_print(ANDROID_LOG_INFO, "tcp_info", "timer run");
            writeTcpInfo(sock, externalPath);
            // __android_log_print(ANDROID_LOG_INFO, "tcp_info", "timer run");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    tcp_info_thread.detach();

    std::thread socket_thread([=]()
    {
        FILE * file;
        char ch [100];
        strcpy(ch, filePath.c_str());
        file = fopen(ch,"r");
        char send_buffer[8192];
        while (!feof(file)) {
            __android_log_print(ANDROID_LOG_INFO, "sendObject", "Transmit object", feof(file));
            fread(send_buffer, 1, sizeof(send_buffer), file);
            write(sock, send_buffer, sizeof(send_buffer));
        }
    });
    socket_thread.join();
    string done = "done";
    write(sock, done.c_str(), sizeof(done));

    if (!socket_thread.joinable())
    {
        __android_log_print(ANDROID_LOG_INFO, "sendObject", "Transmission completed");
        _execute = false;
    }
}


string sclient::recvData(int sock) {
    char buffer[1024] = {0};
    int valread = read(sock, buffer, 1024);

    //__android_log_print(ANDROID_LOG_INFO, "ACK", "ACK rx");

    if (valread != -1)
        return buffer;
    else return "Error";
}

void sclient::disconnectFromServer(int sock) {
    close(sock);
}