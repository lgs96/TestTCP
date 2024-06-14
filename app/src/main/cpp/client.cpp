#include "client.h"
#include <android/log.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <chrono>
#include <thread>
#include <functional>
#include <sys/time.h>
#include <iostream>
#include <cstring>

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


int sclient::connectToServer(string hostname, int port) {
  int sockets = 0;
  __android_log_print(ANDROID_LOG_INFO, "connectToServer","Init!");
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
  __android_log_print(ANDROID_LOG_INFO, "connectToServer","Init fin!");
  return sockets;
}

void sclient::sendData(int sock, string data, bool fin) {
    //send(sock, data.c_str(), sizeof(data), 0);
    write(sock, data.c_str(), data.size());
    __android_log_print(ANDROID_LOG_INFO, "sendData","send data: %s %d", data.c_str(), data.size());
    if (fin) {
        _execute = false;
        __android_log_print(ANDROID_LOG_INFO, "sendData","finished!");
    }
    else{
        _execute = true;
        __android_log_print(ANDROID_LOG_INFO, "sendData","execute: %u",_execute);
    }
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


void sclient::recordTcpInfo(int sock, string externalPath){
    string file_name = createTcpInfo(sock, externalPath);
    _execute = true;
    __android_log_print(ANDROID_LOG_INFO, "tcp_info", "execute1: %u", _execute);
    std::thread tcp_info_thread([&]()
                                {
                                    int k = 0;
                                    while (_execute)
                                    {
                                        k += 1;
                                        writeTcpInfo(sock, file_name);
                                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                                    }
                                    __android_log_print(ANDROID_LOG_INFO, "tcp_info", "finished %u", _execute);
                                });
    tcp_info_thread.detach();
}


void sclient::sendObject(int sock, string filePath){

    // Allocate copy buffer
    FILE * file;
    char ch [100];
    strcpy(ch, filePath.c_str());
    file = fopen(ch,"r");
    char send_buffer[1024];

    struct timeval start;
    gettimeofday(&start, NULL);
    time_t current_milli = start.tv_sec*1000 + start.tv_usec/1000;

    // Notify the first byte of the object
    string start_data = "o_first"+to_string(current_milli);

    __android_log_print(ANDROID_LOG_INFO, "sendObject", "start %s", start_data.c_str());
    write(sock, start_data.c_str(), start_data.size());

    // Transmit file data
    while (!feof(file)) {
      fread(send_buffer, 1, sizeof(send_buffer), file);
      write(sock, send_buffer, sizeof(send_buffer));
      __android_log_print(ANDROID_LOG_INFO, "sendObject", "Transmit object", feof(file));
    }

    // Notify the last byte of the object
    string last_data = "o_last";
    __android_log_print(ANDROID_LOG_INFO, "sendObject", "last %s", last_data.c_str());
    write(sock, last_data.c_str(), last_data.size());

    // Wait for receive upload completion message
    char buffer[4096];
    char buffer2[4096] = "o_recv";

    while(true){
        __android_log_print(ANDROID_LOG_INFO, "sendObject", "umm..what?");
        if(read(sock, buffer, 4096)!=-1) {
            //__android_log_print(ANDROID_LOG_INFO, "sendObject", "upload complete %s %s %s, %d", buffer, buffer2, strstr(buffer,"o_recv"), strcmp(buffer,buffer2));
            //Xiaomi
            //if (strcmp(buffer,buffer2)==0){
            if (strstr(buffer, "o_recv")){
                __android_log_print(ANDROID_LOG_INFO, "sendObject", "upload complete %s", buffer);
                break;
            }
        }
    }


}


string sclient::recvData(int sock) {

    int buffer_size = 4096;
    std::string my_buffer(buffer_size, 0);

    int recv_size = 0;
    int read_size = 0;
    int start_msg_idx = 0;
    int end_msg_idx = 0;
    string msg;
    string time_msg;
    double start_time;
    time_t first_byte_time;

    while(true) {
        read_size = read(sock, &my_buffer[0], buffer_size);
        start_msg_idx = my_buffer.find("o_first");
        end_msg_idx = my_buffer.find("l");

        if(read_size>0) {
            recv_size += read_size;

            // Notified start of object tx
            if (start_msg_idx!=string::npos) {
                msg = my_buffer.substr(start_msg_idx);
                time_msg = msg.substr(8);
                start_time = stod(time_msg)*1e3;

                struct timeval start;
                gettimeofday(&start, NULL);
                first_byte_time = start.tv_sec*1000 + start.tv_usec/1000;

                __android_log_print(ANDROID_LOG_INFO, "recvData", "start msg %u %f", start_msg_idx, start_time);
            }

            // Notified end of object tx
            if (end_msg_idx!=string::npos) {
                msg = my_buffer.substr(end_msg_idx);

                struct timeval start;
                gettimeofday(&start, NULL);
                time_t current_milli = start.tv_sec*1000 + start.tv_usec/1000;

                int download_completion_time = (int)(current_milli - (long)first_byte_time);

                __android_log_print(ANDROID_LOG_INFO, "recvData", "end msg idx %u %s %ld", end_msg_idx, msg.c_str(), current_milli);
                __android_log_print(ANDROID_LOG_INFO, "recvData", "object tx time: %u", download_completion_time);

                string notify_recv = "o_recv";

                __android_log_print(ANDROID_LOG_INFO, "recvData", "recv msg: %s", notify_recv.c_str());
                write(sock, notify_recv.c_str(), sizeof(notify_recv));

                string return_string = to_string(download_completion_time) +"_"+to_string(download_completion_time - (current_milli - first_byte_time));

                return return_string;
            }
        }
        else if(read_size == 0){
            __android_log_print(ANDROID_LOG_INFO, "recvData", "disconnected, but socket is still open (break)");
            break;
        }
    }
    //__android_log_print(ANDROID_LOG_INFO, "ACK", "ACK rx");

    /*
    if (valread != -1)
        return buffer;
    else return "Error";
     */
}

void sclient::disconnectFromServer(int sock) {
    __android_log_print(ANDROID_LOG_INFO, "Diconnect from server","done");
    close(sock);
}
