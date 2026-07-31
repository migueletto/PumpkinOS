// retrieve the context to draw on the HTML canvas
const canvas = document.getElementById("canvas");
const ctx = canvas.getContext("2d");

// create an ImageData to hold the image
const imageData = ctx.createImageData(canvas.width, canvas.height);
const data = imageData.data;

for (let i = 0; i < data.length; i += 4) {
  data[i    ] = 255; // Red channel
  data[i + 1] = 0;   // Green channel
  data[i + 2] = 0;   // Blue channel
  data[i + 3] = 255; // Alpha channel (Fully opaque)
}

// draw the image on the canvas
ctx.putImageData(imageData, 0, 0);

async function receiveBinaryData(url) {
  const ws = new WebSocket(url);
  ws.binaryType = 'arraybuffer'; 

  // Wait for the connection to open
  await new Promise((resolve, reject) => {
    ws.onopen = resolve;
    ws.onerror = reject;
  });

  console.log("WebSocket connected. Waiting for binary data...");

  // Infinite loop handling the data stream
  while (ws.readyState === WebSocket.OPEN) {
    const data = await new Promise((resolve) => {
      ws.onmessage = (event) => {
        if (event.data instanceof ArrayBuffer) {
          resolve(event.data);
        }
      };
      ws.onclose = () => resolve(null); 
    });

    if (data === null) break; 

    // Process your binary data here
    const view = new DataView(data);
    console.log("Received bytes:", view.byteLength);
  }

  console.log("WebSocket connection closed.");
}

async function startApp() {
  const serverUrl = "ws://127.0.0.1:65432";

  try {
    // Execution pauses here while the socket connects and streams data
    await receiveBinaryData(serverUrl); 
    console.log("Stream finished cleanly.");
  } catch (error) {
    console.error("Failed to connect or stream data:", error);
  }
}

startApp();
