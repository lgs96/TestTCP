# TestTCP
## TCP measurement android application and server
- The app sends dummy data to the server and collects TCPInfo over time.

## Run server (In Test-server folder)
- python3 tcp_server.py <interface> <port> ex) python3 tcp_server.py enp4s0 8888
- Logging folders
  - Test-server/downlink_data folder
    - Downlink object latency
  - Test-server/uplink_data folder
    - Uplink object latency
  - Test-server/result
    - Cwnd, rtt and summary of object tansmissions

## Run client (Android project)
- Build android project and install on android device
- Android app setting
  - Ip addr
  - Port numbre
  - Object size
  - Object period
  - Number of objects to send
  - Save pcap file name (in server)
  - Server interface
- Start sending for downlink transmission
- Start receiving for uplink transmission
- Disconnect to stop transmission

![image](https://github.com/lgs96/TestTCP/assets/33349919/c5e0c55a-a9e6-401f-83be-3f7c9d4fd1ed)
