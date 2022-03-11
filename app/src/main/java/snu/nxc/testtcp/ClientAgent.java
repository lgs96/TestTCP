package snu.nxc.testtcp;

import android.content.Context;
import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;


public class ClientAgent {
    
    private int sock;
    private static boolean ongoing = true;
    
    static {
	System.loadLibrary("native-lib");
    }

    public interface ClientAgentListener {
		public void onTestingEnded();
    }

    private native int init(String hostname, int port, String savename, String interfaceName);
    private native void recordTcpInfo(int sock, String externalPath);
    private native void sendData(int sock, String data, boolean fin);
    private native void sendObject (int sock, String filePath);
    private native String recvData(int sock);
    private native void disconnect(int sock, String savename);

    // Uplink object transmission
    public void startSending(final String hostname,
			     final int port,
			     final int size,
			     final int interval,
			     final int object_to_tx,
			     final String savename,
			     final String interfaceName,
			     final ClientAgentListener listener, final Context context) {
		ongoing = true;

		File externalDir = context.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS);
		final String externalPath = externalDir.getAbsolutePath();

		byte [] object = new byte[1024*size];
		File file = new File(context.getFilesDir(), "temp.txt");
		final String filePath = file.getAbsolutePath();
		try {
			//if (!file.exists()) {
			Log.i("startSending", "File does not exist!");
			file.createNewFile();
			FileOutputStream fos = new FileOutputStream(file);
			fos.write(object);
			fos.close();
			//}
		} catch (Exception e) {
			Log.e("startSending", e.getMessage());
		}
		Log.i("startSending", "File ready!");

		final long currentTime = System.currentTimeMillis();
		Log.i("startHandshake", "Handshake init" + currentTime);
		sock = init(hostname, port, savename, interfaceName);
		if (sock==-1) {
			Log.i("Handshake failed", "socket is -1");
			return;
		}
		long handshakeFin = System.currentTimeMillis();
		Log.i("startSending", "Handshake time: "+ (handshakeFin - currentTime));

		Thread t = new Thread(new Runnable() {
			@Override
			public void run() {
				int objectNum = object_to_tx;
				//recordTcpInfo(sock, externalPath);
				String uplink_msg = "u_start_"+savename+"_"+currentTime+"_"+size+"_"+interval+"_"+object_to_tx;
				sendData(sock, uplink_msg, false);
				Log.i("startSending",uplink_msg);
				for(int i = 0 ; i <objectNum; i++) {
					sendObject(sock, filePath);
					try
					{
						Thread.sleep(interval);
						Log.i("startSending", "Sleep and repeat"+ i);
					} catch (InterruptedException e)
					{
						e.printStackTrace();
					}
					if (!ongoing)
						break;
				}
				Log.i("startSending","Finished!" + (System.currentTimeMillis() - currentTime));
				sendData(sock, "u_done", true);
				disconnect(sock, savename);
				listener.onTestingEnded();
			}
	    });
	t.start();
    }

    // Downlink object transmission
    public void startReceiving(final String hostname,
			       final int port,
				   final int size,
				   final int interval,
				   final int object_to_rx,
			       final String savename,
			       final String interfaceName,
			       final ClientAgentListener listener) {
	ongoing = true;
	Thread t = new Thread(new Runnable() {
		@Override
		public void run() {
			double [] tx_time = new double[object_to_rx];
			double [] init_time = new double[object_to_rx];
		    sock = init(hostname, port, savename, interfaceName);
		    String downlink_msg = "d_start_" +savename+"_"+size+"_"+interval+"_"+object_to_rx;
			Log.i("startReceiving","Start!" + downlink_msg);
		    sendData(sock, downlink_msg, false);
		    for (int i = 0; i < object_to_rx; i++) {
				Log.i("startReceiving","Start2!");
				String tx_init = recvData(sock);
				String[] result = tx_init.split("_");
				tx_time[i] = Double.parseDouble(result[0]);
				init_time[i] = Double.parseDouble(result[1]);
				if (!ongoing)
					break;
			}
		    double first_tx_time = 0;
		    double mean_tx_time = 0;
		    double std_tx_time = 0;

			double first_init_time = 0;
			double mean_init_time = 0;
			double std_init_time = 0;

			double mean_throughput = 0;
			double std_throughput = 0;

			double valid_num = 0;

		    first_tx_time = tx_time[0];
			first_init_time = init_time[0];

			Log.i("startReceiving","tx time/init time for object " + 0 + " is "+ tx_time[0]+"ms, "+init_time[0]+"ms");

			for (int i = 1; i < object_to_rx; i++){
		    	if(tx_time[i] !=0){
		    		valid_num += 1;
					mean_tx_time += tx_time[i];
					std_tx_time += Math.pow(tx_time[i],2);
					mean_init_time += init_time[i];
					std_init_time += Math.pow(init_time[i],2);
					mean_throughput += (size*8)/tx_time[i];
					std_throughput += Math.pow((size*8)/tx_time[i],2);

					Log.i("startReceiving","tx time/init time for object " + i + " is "+ tx_time[i]+"ms, "+init_time[i]+"ms");
				}
			}
			mean_tx_time /= (double)valid_num;
			std_tx_time /= (double)valid_num;
			std_tx_time -= Math.pow(mean_tx_time,2);
			std_tx_time = Math.sqrt(std_tx_time);

			mean_init_time /= (double)valid_num;
			std_init_time /= (double)valid_num;
			std_init_time -= Math.pow(mean_init_time,2);
			std_init_time = Math.sqrt(std_init_time);

			mean_throughput /= (double)valid_num;
			std_throughput /= (double)valid_num;
			std_throughput -= Math.pow(mean_throughput, 2);
			std_throughput = Math.sqrt(std_throughput);

			double throughput = (size*8/(mean_tx_time));

			Log.i("startReceiving","First, Mean, Std of tx time:  " + first_tx_time + "ms "+mean_tx_time+"ms "+ std_tx_time + "ms");
			Log.i("startReceiving","First, Mean, Std of init time:  " + first_init_time + "ms "+mean_init_time+"ms "+ std_init_time + "ms");
			Log.i("startReceiving","mean, std throughput: " + mean_throughput+"Mbps "+std_throughput+"Mbps");
			Log.i("startReceiving","Finished");

			try
			{
				Thread.sleep(500);
				Log.i("startReceiving", "Sleep and send msg");
			} catch (InterruptedException e)
			{
				e.printStackTrace();
			}

			String downlink_finish_msg = "d_fin_"+first_tx_time+"_"+mean_tx_time+"_"+std_tx_time+"_"+first_init_time+"_"+mean_init_time+"_"+std_init_time + "_"+Math.round(mean_throughput*100)/100.0 + "_"+Math.round(std_throughput*100)/100.0;
			sendData(sock, downlink_finish_msg, true);

		    listener.onTestingEnded();		    
		    disconnect(sock, savename);
		}
	    });
	t.start();
    }

    public void disconnect() {
	ongoing = false;
    }
}
