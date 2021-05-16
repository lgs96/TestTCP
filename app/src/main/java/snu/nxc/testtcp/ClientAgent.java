package snu.nxc.testtcp;

import android.content.Context;
import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;


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
    private native void sendData(int sock, String data);
    private native void sendObject (int sock, String data, String filePath, String externalPath);
    private native String recvData(int sock);
    private native void disconnect(int sock);
    
    // public void initConnection(String hostname, int port) {
    // 	sock = init(hostname, port);
    // 	// sendData(sock, "Hello World");
    // 	// disconnect(sock);
    // }

    public void startSending(final String hostname,
			     final int port,
			     final int interval,
			     final String savename,
			     final String interfaceName,
			     final ClientAgentListener listener, final Context context) {
	ongoing = true;
	Thread t = new Thread(new Runnable() {
		@Override
		public void run() {
			File externalDir = context.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS);
			String externalPath = externalDir.getAbsolutePath();

			byte [] object = new byte[1024*1024*3];
			File file = new File(context.getFilesDir(), "temp.txt");
			String filePath = file.getAbsolutePath();
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

		    sock = init(hostname, port, savename, interfaceName);

			sendData(sock, "u" + savename);
		    for(int i = 0; i < 1; i ++) {
				sendObject(sock, "u" + savename, filePath, externalPath);
				try
				{
					Thread.sleep(50);
					Log.i("startSending", "Sleep and repeat"+ i);
					sendData(sock, "u" + savename);
				} catch (InterruptedException e)
				{
					e.printStackTrace();
				}
			}
		    listener.onTestingEnded();
		    disconnect(sock);
		}
	    });
	t.start();
    }

    public void startReceiving(final String hostname,
			       final int port,
			       final int interval,
			       final String savename,
			       final String interfaceName,
			       final ClientAgentListener listener) {
	ongoing = true;
	Thread t = new Thread(new Runnable() {
		@Override
		public void run() {
		    sock = init(hostname, port, savename, interfaceName);
		    sendData(sock, "d" + savename);
		    long stime = System.currentTimeMillis();		    
		    while (ongoing) {
		    	recvData(sock);
			if (System.currentTimeMillis() - stime > interval * 1000) {
			    ongoing = false;
			}			
		    }
		    listener.onTestingEnded();		    
		    disconnect(sock);
		}
	    });
	t.start();
    }

    public void disconnect() {
	ongoing = false;
    }
}