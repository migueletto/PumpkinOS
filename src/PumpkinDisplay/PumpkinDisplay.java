import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferUShort;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.SwingUtilities;

public class PumpkinDisplay extends JComponent {
	private static final long serialVersionUID = 1L;

	private static final int PORT = 65432;

	private static final int CMD_WINDOW  = 1;
	private static final int CMD_FINISH  = 2;
	private static final int CMD_MOTION  = 3;
	private static final int CMD_BUTTON  = 4;
	private static final int CMD_KEYDOWN = 5;
	private static final int CMD_KEYUP   = 6;
	private static final int CMD_DRAW    = 7;

	private static final int WINDOW_KEY_UP     = 0x81;
	private static final int WINDOW_KEY_DOWN   = 0x82;
	private static final int WINDOW_KEY_LEFT   = 0x83;
	private static final int WINDOW_KEY_RIGHT  = 0x84;
	private static final int WINDOW_KEY_PGUP   = 0x85;
	private static final int WINDOW_KEY_PGDOWN = 0x86;
	private static final int WINDOW_KEY_HOME   = 0x87;
	private static final int WINDOW_KEY_F1     = 0x91;
	private static final int WINDOW_KEY_F2     = 0x92;
	private static final int WINDOW_KEY_F3     = 0x93;
	private static final int WINDOW_KEY_F4     = 0x94;
	private static final int WINDOW_KEY_F5     = 0x95;

	private static final int ENC_RGB565 = 6;

	private static final int WINDOW_WIDTH  = 1024;
	private static final int WINDOW_HEIGHT =  768;

	private BufferedImage image;
	private Socket clientSocket;

	public PumpkinDisplay() {
		image = new BufferedImage(WINDOW_WIDTH, WINDOW_HEIGHT, BufferedImage.TYPE_USHORT_565_RGB);

		setLayout(new BorderLayout());
		setPreferredSize(new Dimension(WINDOW_WIDTH, WINDOW_HEIGHT));
		setFocusable(true);

		addMouseMotionListener(new MouseAdapter() {
			@Override
			public void mouseMoved(MouseEvent e) {
				mouseMotion(e, clientSocket);
			}

			@Override
			public void mouseDragged(MouseEvent e) {
				mouseMotion(e, clientSocket);
			}
		});

		addMouseListener(new MouseAdapter() {
			@Override
			public void mousePressed(MouseEvent e) {
				mouseClick(e, true, clientSocket);
			}

			@Override
			public void mouseReleased(MouseEvent e) {
				mouseClick(e, false, clientSocket);
			}
		});

		addKeyListener(new KeyAdapter() {
			@Override
			public void keyPressed(KeyEvent e) {
				keyboardClick(e, true, clientSocket);
			}

			@Override
			public void keyReleased(KeyEvent e) {
				keyboardClick(e, false, clientSocket);
			}
		});

		Thread thread = new Thread(new Runnable() {
			@Override
			public void run() {
				server();
			}
		});
		thread.start();
	}

	private void mouseMotion(MouseEvent e, Socket socket) {
		if (socket != null && !socket.isClosed()) {
			short[] cmd = new short[3];
			cmd[0] = CMD_MOTION;
			cmd[1] = (short)e.getX();
			cmd[2] = (short)e.getY();
			sendCmdArgs(socket, cmd);
		}
	}

	private void mouseClick(MouseEvent e, boolean down, Socket socket) {
		int button = e.getButton();

		if (clientSocket != null && (button == MouseEvent.BUTTON1 || button == MouseEvent.BUTTON3)) {
			short[] cmd = new short[2];
			cmd[0] = CMD_BUTTON;
			cmd[1] = (short)(button == MouseEvent.BUTTON1 ? 1 : 2);
			if (down) cmd[1] |= 0x8000;
			sendCmdArgs(clientSocket, cmd);
		}
	}

	private void keyboardClick(KeyEvent e, boolean down, Socket socket) {
		if (clientSocket != null) {
			short[] cmd = new short[2];
			cmd[0] = (short)(down ? CMD_KEYDOWN : CMD_KEYUP);
			cmd[1] = (short)(mapKey(e.getKeyCode(), e.getKeyChar()));
			if (cmd[1] > 0) {
				sendCmdArgs(clientSocket, cmd);
			}
		}
	}

	private void sendCmdArgs(Socket socket, short[] cmd) {
		ByteBuffer byteBuffer = ByteBuffer.allocate(cmd.length * 2);
		byteBuffer.order(ByteOrder.LITTLE_ENDIAN); 
		byteBuffer.asShortBuffer().put(cmd);

		try {
			OutputStream os = socket.getOutputStream();
			os.write(byteBuffer.array());
		} catch (Exception ex) {
			System.out.println(ex);
			ex.printStackTrace();
		}
	}

	private int mapKey(int code, int ch) {
		//System.out.println("map code " + code + " ch " + ch);
		if (ch == 65535) {
			switch (code) {
			case 33:  ch = WINDOW_KEY_PGUP; break;
			case 34:  ch = WINDOW_KEY_PGDOWN; break;
			case 36:  ch = WINDOW_KEY_HOME; break;
			case 37:  ch = WINDOW_KEY_LEFT; break;
			case 38:  ch = WINDOW_KEY_UP; break;
			case 39:  ch = WINDOW_KEY_RIGHT; break;
			case 40:  ch = WINDOW_KEY_DOWN; break;
			case 112: ch = WINDOW_KEY_F1; break;
			case 113: ch = WINDOW_KEY_F2; break;
			case 114: ch = WINDOW_KEY_F3; break;
			case 115: ch = WINDOW_KEY_F4; break;
			case 116: ch = WINDOW_KEY_F5; break;
			default:  ch = 0;
			}
		}

		return ch;
	}

	public void paintComponent(Graphics g) {
		Graphics2D g2d = (Graphics2D)g;
		synchronized (image) {
			g2d.drawImage(image, 0, 0, null);
		}
	}

	private static void createFrame() throws ClassNotFoundException {
		JFrame frame = new JFrame("PumpkinOS");
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frame.setResizable(false);
		frame.getContentPane().add(new PumpkinDisplay());
		frame.pack();
		frame.setVisible(true);
	}

	private void cmdDraw(int x, int y, int width, int height, int screenWidth, InputStream is) throws Exception {
		int rowlen = width * 2;
		byte[] row = new byte[rowlen];
		ByteBuffer byteBuffer = ByteBuffer.wrap(row);
		byteBuffer.order(ByteOrder.LITTLE_ENDIAN);
		short[] shortArray = new short[rowlen / 2];

		synchronized (image) {
			short[] displayArray = ((DataBufferUShort)image.getRaster().getDataBuffer()).getData();
			for (int i = 0; i < height; i++) {
				int n = is.readNBytes(row, 0, rowlen);
				if (n == rowlen) {
					byteBuffer.asShortBuffer().get(shortArray);
					System.arraycopy(shortArray, 0, displayArray, (y + i) * screenWidth + x, width);
				}
			}
		}

		repaint(x, y, width, height);
	}

	private int u16(InputStream is) throws Exception {
		int u = 0;

		byte[] buffer = new byte[2];
		int n = is.readNBytes(buffer, 0, 2);
		if (n == 2) {
			int b1 = buffer[1];
			if (b1 < 0) b1 += 256;
			int b0 = buffer[0];
			if (b0 < 0) b0 += 256;
			u = b1 * 256 + b0;
		}

		return u;
	}

	private void handle(Socket socket) throws Exception {
		InputStream is = socket.getInputStream();
		boolean finish = false;

		while (!finish) {
			int cmd = u16(is);
			switch (cmd) {
			case CMD_WINDOW:
				int encoding = u16(is);
				int width = u16(is);
				int height = u16(is);
				if (encoding != ENC_RGB565) {
					System.out.println("CMD_WINDOW: invalid encoding " + encoding);
				}
				if (width != WINDOW_WIDTH || height != WINDOW_HEIGHT) {
					System.out.println("CMD_WINDOW: invalid dimensions " + width + "x" + height);
				}
				break;
			case CMD_DRAW:
				int x = u16(is);
				int y = u16(is);
				int w = u16(is);
				int h = u16(is);
				//System.out.println("CMD_DRAW: " + x + " " + y + " " + w + " " + h);
				cmdDraw(x, y, w, h, WINDOW_WIDTH, is);
				break;
			case CMD_FINISH:
				//System.out.println("CMD_FINISH");
				finish = true;
				break;
			default:
				System.out.println("Unknown cmd " + cmd);
				finish = true;
				break;
			}
		}

		//System.out.println("Client finished");
	}

	private void server() {
		ServerSocket serverSocket = null;

		try {
			serverSocket = new ServerSocket(PORT);
			//System.out.println("Socket created");

			while (true) {
				clientSocket = serverSocket.accept();
				//System.out.println("Client accepted");
				handle(clientSocket);
				clientSocket.close();

				synchronized (image) {
					Graphics2D g2d = image.createGraphics();
					g2d.setColor(Color.BLACK);
					g2d.fillRect(0, 0, image.getWidth(), image.getHeight());
					g2d.dispose();
				}
				repaint();
			}
			//serverSocket.close();
			
		} catch (Exception ex) {
			System.out.println(ex);
			ex.printStackTrace();
		}
	}

	public static void main(String[] args) {
		SwingUtilities.invokeLater(new Runnable() {
			public void run() {
				try {
					createFrame();
				} catch (Exception ex) {
					ex.printStackTrace();
				}
			}
		});
	}
}
