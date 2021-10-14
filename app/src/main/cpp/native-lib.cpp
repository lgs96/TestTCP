#include <jni.h>
#include <string>
#include <iostream>
#include <android/log.h>
#include <stdlib.h>
#include <stdio.h>
#include "client.h"
<<<<<<< Updated upstream

=======
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <unistd.h>
#include <thread>
>>>>>>> Stashed changes
/**
 *
 * @param env - reference to the JNI environment
 * @param jStr - incoming jstring
 * @return - outputs a proper C++ string
 */
std::string jstring2string(JNIEnv *env, jstring jStr) {
  if (!jStr)
    return "";

  const jclass stringClass = env->GetObjectClass(jStr);
  const jmethodID getBytes = env->GetMethodID(stringClass, "getBytes", "(Ljava/lang/String;)[B");
  const jbyteArray stringJbytes = (jbyteArray) env->CallObjectMethod(jStr, getBytes,
								     env->NewStringUTF("UTF-8"));

  size_t length = (size_t) env->GetArrayLength(stringJbytes);
  jbyte *pBytes = env->GetByteArrayElements(stringJbytes, NULL);

  std::string ret = std::string((char *) pBytes, length);
  env->ReleaseByteArrayElements(stringJbytes, pBytes, JNI_ABORT);

  env->DeleteLocalRef(stringJbytes);
  env->DeleteLocalRef(stringClass);
  return ret;
}

extern "C" {

  /**
   *
   * @param env - reference to the JNI environment
   * @param instance - reference to *this* Java object
   * @param hostname - the IP address of the remote server
   * @param port  - the port to connect to on the remote server
   * @return  - returns a socket to be stored and used or -1 for error
   */
  JNIEXPORT jint JNICALL
  Java_snu_nxc_testudp_ClientAgent_init(JNIEnv *env,
						  jobject instance,
						  jstring hostname,
						  jint port,
						  jstring savename,
						  jstring interfacename) {
    __android_log_print(ANDROID_LOG_INFO, "InitCpp","Init!");
    sclient k;
    string hname = jstring2string(env, hostname);
    string sname = jstring2string(env, savename);
    string iname = jstring2string(env, interfacename);
    char cmd0[600];
    sprintf(cmd0, "su -c rm -rf /sdcard/%s.pcap", sname.c_str());
    system(cmd0);
    __android_log_print(ANDROID_LOG_INFO, "InitCpp","Init2!");
    system("su -c killall tcpdump");
    char cmd[600];
    int done = sprintf(cmd, "su -c tcpdump -i any -v -U -w /sdcard/%s.pcap &", sname.c_str());
    __android_log_print(ANDROID_LOG_INFO, "InitCpp","Init3! %s %u", cmd, done);
    system(cmd);
    usleep(1000*100);
    __android_log_print(ANDROID_LOG_INFO, "InitCpp","Init4!");
    return k.connectToServer(hname, (int) port);
  }

  /**
   *
   * @param env - reference to the JNI environment
   * @param instance - reference to *this* Java object
   * @param sock - active socket to send data through
   * @param data - data to be sent to the remote server
   */
  JNIEXPORT void JNICALL
  Java_snu_nxc_testudp_ClientAgent_sendData(
						       JNIEnv *env,
						       jobject instance,
						       jint sock,
						       jstring data) {
    sclient k;
    string sData = jstring2string(env, data);
    k.sendData((int) sock, sData);
  }

<<<<<<< Updated upstream
=======
    JNIEXPORT void JNICALL
    Java_snu_nxc_testudp_ClientAgent_sendObject(
            JNIEnv *env,
            jobject instance,
            jint sock,
            jstring data,
            jstring filePath,
            jstring externalPath) {
        sclient k;
        string sData = jstring2string(env, data);
        string sPath = jstring2string(env, filePath);
        string ePath = jstring2string(env, externalPath);
        k.sendObject((int) sock, sData, sPath, ePath);
    }

>>>>>>> Stashed changes
  /**
   *
   * @param env - reference to the JNI environment
   * @param instance - reference to *this* Java object
   * @param sock - active socket to disconnect
   */
  JNIEXPORT void JNICALL
  Java_snu_nxc_testudp_ClientAgent_disconnect(JNIEnv *env,
							 jobject instance,
							 jint sock,
							 jstring savename) {
    sclient k;
    string sname = jstring2string(env, savename);
    std::thread t1([&]()
    {
        __android_log_print(ANDROID_LOG_INFO, "Disconnect", "transimt msg");
        k.disconnectFromServer((int) sock);
        sleep(3);
    });
    t1.join();
    if (!t1.joinable()) {
        int done = system("su -c killall tcpdump");
        __android_log_print(ANDROID_LOG_INFO, "Disconnect", "Kill %u!", done);
    }
  }

  /**
   *
   * @param env - reference to the JNI environment
   * @param instance - reference to *this* Java object
   * @param sock - active socket to listen for incoming data from
   * @return - returns incoming data from remote server
   */
  JNIEXPORT jstring JNICALL
  Java_snu_nxc_testudp_ClientAgent_recvData(JNIEnv *env,
						       jobject instance,
						       jint sock) {
    sclient k;
    string data = k.recvData((int) sock);
    return env->NewStringUTF(data.c_str());
  }
}
