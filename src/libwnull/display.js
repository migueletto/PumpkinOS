const CMD_WINDOW  = 1;
const CMD_FINISH  = 2;
const CMD_MOTION  = 3;
const CMD_BUTTON  = 4;
const CMD_KEYDOWN = 5;
const CMD_KEYUP   = 6;
const CMD_DRAW    = 7;

const canvas = document.getElementById("canvas");
const context = canvas.getContext("2d");
const imageData = context.createImageData(canvas.width, canvas.height);
const data = imageData.data;

function getMousePos(canvas, e) {
  const rect = canvas.getBoundingClientRect();
  return {
    x: e.clientX - rect.left,
    y: e.clientY - rect.top
  };
}

function getMouseButton(e, down) {
  switch (e.button) {
    case 0: return down ? 0x8001 : 1;
    case 2: return down ? 0x8002 : 2;
  }
}

function getKey(e) {
  let key = 0;

  if (e.key == 'ArrowUp') {
    key = 0x81;
  } else if (e.key == 'ArrowDown') {
    key = 0x82;
  } else if (e.key == 'ArrowLeft') {
    key = 0x83;
  } else if (e.key == 'ArrowRight') {
    key = 0x84;
  } else if (e.key == 'PageUp') {
    key = 0x85;
  } else if (e.key == 'PageDown') {
    key = 0x86;
  } else if (e.key == 'Home') {
    key = 0x87;
  } else if (e.key == 'F5') {
    key = 0x95;
  } else {
    let code = e.key.charCodeAt(0);
    if (code < 32 || code > 127) key = 0;
  }

  return key;
}

function r565(rgb) {
  let red = ((rgb & 0xF800) >> 8);
  if (red > 0x0F) red |= 0x07;
  return red;
}

function g565(rgb) {
  let green = ((rgb & 0x07E0) >> 3);
  if (green > 0x1F) green |= 0x03;
  return green;
}

function b565(rgb) {
  let blue = ((rgb & 0x001F) << 3);
  if (blue > 0x0F) blue |= 0x07;
  return blue;
}

var iscmd = true;
var screen_width = 0;
var screen_height = 0;
var x = 0;
var y = 0;
var width = 0;
var height = 0;
var row = 0;

function handleCommand(dataView) {
  const cmd = dataView.getUint16(0, true);

  if (iscmd) {
    switch (cmd) {
      case CMD_WINDOW:
        const encoding = dataView.getUint16(2, true);
        screen_width = dataView.getUint16(4, true);
        screen_height = dataView.getUint16(6, true);
        //console.log(`CMD_WINDOW encoding ${encoding} width ${screen_width} height ${screen_height}`);
        break;
      case CMD_FINISH:
        //console.log("CMD_FINISH");
        break;
      case CMD_DRAW:
        x = dataView.getUint16(2, true);
        y = dataView.getUint16(4, true);
        width = dataView.getUint16(6, true);
        height = dataView.getUint16(8, true);
        row = 0;
        iscmd = false;
        break;
    }
  } else {
    let pos = ((y + row) * screen_width + x) * 4;
    for (let i = 0; i < width; i++) {
      const rgb = dataView.getUint16(i*2, true);
      data[pos++] = r565(rgb);
      data[pos++] = g565(rgb);
      data[pos++] = b565(rgb);
      data[pos++] = 255;
    }
    row++;
    if (row == height) {
      context.putImageData(imageData, 0, 0, x, y, width, height);
      iscmd = true;
    }
  }
}

async function eventLoop(url) {
  const ws = new WebSocket(url);
  ws.binaryType = 'arraybuffer'; 

  await new Promise((resolve, reject) => {
    ws.onopen = resolve;
    ws.onerror = reject;
  });
  //console.log("WebSocket connected");

  canvas.addEventListener('contextmenu', (e) => {
    e.preventDefault(); 
  });

  canvas.addEventListener('mousemove', (e) => {
    const pos = getMousePos(canvas, e);
    const buf = new ArrayBuffer(6);
    const view = new DataView(buf);
    view.setUint16(0, CMD_MOTION, true);
    view.setUint16(2, pos.x, true);
    view.setUint16(4, pos.y, true);
    ws.send(buf);
  });

  canvas.addEventListener('mousedown', (e) => {
    const button = getMouseButton(e, true);
    if (button > 0) {
      const pos = getMousePos(canvas, e);
      const buf = new ArrayBuffer(4);
      const view = new DataView(buf);
      view.setUint16(0, CMD_BUTTON, true);
      view.setUint16(2, button, true);
      ws.send(buf);
    }
  });

  canvas.addEventListener('mouseup', (e) => {
    const button = getMouseButton(e, false);
    if (button > 0) {
      const pos = getMousePos(canvas, e);
      const buf = new ArrayBuffer(4);
      const view = new DataView(buf);
      view.setUint16(0, CMD_BUTTON, true);
      view.setUint16(2, button, true);
      ws.send(buf);
    }
  });

  window.addEventListener('keydown', (e) => {
    if (e.repeat) return;
    const key = getKey(e);
    if (key > 0) {
      const buf = new ArrayBuffer(4);
      const view = new DataView(buf);
      view.setUint16(0, CMD_KEYDOWN, true);
      view.setUint16(2, key, true);
      ws.send(buf);
      e.preventDefault(); 
    }
  });

  window.addEventListener('keyup', (e) => {
    const key = getKey(e);
    if (key > 0) {
      const buf = new ArrayBuffer(4);
      const view = new DataView(buf);
      view.setUint16(0, CMD_KEYUP, true);
      view.setUint16(2, key, true);
      ws.send(buf);
      e.preventDefault(); 
    }
  });

  while (ws.readyState === WebSocket.OPEN) {
    const data = await new Promise((resolve) => {
      ws.onmessage = (e) => {
        if (e.data instanceof ArrayBuffer) {
          resolve(e.data);
        }
      };
      ws.onclose = () => resolve(null); 
    });

    if (data === null) break; 

    const dataView = new DataView(data);
    handleCommand(dataView);
  }

  //console.log("WebSocket closed");
}

async function main() {
  let currentPort = window.location.port;
  if (!currentPort) currentPort = '80';
  let nextPort = Number(currentPort) + 1;
  const serverUrl = "ws://" + window.location.hostname + ":" + nextPort;

  try {
    await eventLoop(serverUrl); 
  } catch (ex) {
    console.error("Failed to connect: ", ex);
  }
}

main();
