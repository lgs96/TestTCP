package snu.nxc.testudp;

import android.util.Log;

<<<<<<< Updated upstream:app/src/main/java/snu/nxc/testtcp/ClientAgent.java
=======
import java.io.File;
import java.io.FileOutputStream;


>>>>>>> Stashed changes:app/src/main/java/snu/nxc/testudp/ClientAgent.java
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
    private native String recvData(int sock);
    private native void disconnect(int sock, String savename);
    
    // public void initConnection(String hostname, int port) {
    // 	sock = init(hostname, port);
    // 	// sendData(sock, "Hello World");
    // 	// disconnect(sock);
    // }

    public void startSending(final String hostname,
			     final int port,
			     final int size,
			     final String savename,
			     final String interfaceName,
			     final ClientAgentListener listener) {
	ongoing = true;
	Thread t = new Thread(new Runnable() {
		@Override
		public void run() {
<<<<<<< Updated upstream:app/src/main/java/snu/nxc/testtcp/ClientAgent.java
		    sock = init(hostname, port, savename, interfaceName);
		    sendData(sock, "u" + savename);
		    long stime = System.currentTimeMillis();
		    byte[] bytes = new byte[5000];
		    while (ongoing) {
		    	String sdata = new String(bytes);
		    	sendData(sock, sdata);
			if (System.currentTimeMillis() - stime > interval * 1000) {
			    ongoing = false;
			}
		    }
		    listener.onTestingEnded();
		    disconnect(sock);
=======
			File externalDir = context.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS);
			String externalPath = externalDir.getAbsolutePath();

			byte [] object = new byte[1024*size];
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
			Log.i("startSending", "socket ready!");
		    sock = init(hostname, port, savename, interfaceName);

			sendData(sock, "u" + savename);
		    for(int i = 0; i < 1; i ++) {
				long currentTime = System.currentTimeMillis();
				sendData(sock, "up" +currentTime);
				sendObject(sock, "up" + savename, filePath, externalPath);
				try
				{
					Thread.sleep(50);
					//Log.i("startSending", "Sleep and repeat"+ i);
					//sendData(sock, "up" + savename);
				} catch (InterruptedException e)
				{
					e.printStackTrace();
				}
			}
			disconnect(sock, savename);
			listener.onTestingEnded();
>>>>>>> Stashed changes:app/src/main/java/snu/nxc/testudp/ClientAgent.java
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
		    disconnect(sock, savename);
		}
	    });
	t.start();
    }

    public void disconnect() {
	ongoing = false;
    }
}
