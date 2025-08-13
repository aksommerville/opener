let originalText; // string
let palette; // Uint32Array(16), xbgr
let mapw,maph; // int
let map_data; // Uint8Array(mapw*maph)
let imageData; // 4x4, staging area for a single tile
let paintbrush = 0; // Cell value, ie (colorindex<<5)|tileid.
let painting = false;
let lastx=0, lasty=0;

function isident(ch) {
  if ((ch >= 0x30) && (ch <= 0x39)) return true;
  if ((ch >= 0x41) && (ch <= 0x5a)) return true;
  if ((ch >= 0x61) && (ch <= 0x7a)) return true;
  if (ch === 0x5f) return true;
  return false;
}

class Tokenizer {
  constructor(src, includeSpace) {
    this.src = src;
    this.srcp = 0;
    this.includeSpace = includeSpace;
  }
  
  next() {
    for (;;) {
      if (this.srcp >= this.src.length) return "";
      const ch = this.src.charCodeAt(this.srcp);
    
      // Skip whitespace.
      if (ch <= 0x20) {
        let endp = this.srcp + 1;
        while ((endp < this.src.length) && (this.src.charCodeAt(endp) <= 0x20)) endp++;
        if (this.includeSpace) {
          const token = this.src.substring(this.srcp, endp);
          this.srcp = endp;
          return token;
        }
        this.srcp = endp;
        continue;
      }
    
      // Slash begins and ends a block comment. No line comments or division, or slashes inside strings or comments.
      if (ch === 0x2f) {
        let endp = this.src.indexOf("/", this.srcp + 1);
        if (endp < 0) throw new Error("Unclosed block comment or illegal use of slash.");
        endp++;
        if (this.includeSpace) {
          const token = this.src.substring(this.srcp, endp);
          this.srcp = endp;
          return token;
        }
        this.srcp = endp;
        continue;
      }
      
      // Letters, digits, and underscores form an identifier or literal. No floats.
      if (isident(ch)) {
        let endp = this.srcp + 1;
        while ((endp < this.src.length) && isident(this.src.charCodeAt(endp))) endp++;
        const token = this.src.substring(this.srcp, endp);
        this.srcp = endp;
        return token;
      }
      
      // Anything else is one byte of punctuation. No strings.
      const token = this.src[this.srcp];
      this.srcp++
      return token;
    }
  }
  
  lineno() {
    let ln = 1;
    for (let i=this.srcp; i-->0; ) {
      if (this.src.charCodeAt(i) === 0x0a) ln++;
    }
    return ln;
  }
}

// Call after the opening "{" has been consumed. We will consume the closing "}" and not the semicolon after.
function readPalette(tokenizer) {
  palette = new Uint32Array(16);
  let token, dstc=0;
  while (token = tokenizer.next()) {
    if (token === ",") continue;
    if (token === "}") break;
    if (isNaN(palette[dstc++] = +token)) throw new Error(`Expected integer, found ${JSON.stringify(token)}`);
  }
}

// Call after the opening "{" has been consumed. We will consume the closing "}" and not the semicolon after.
function readMapData(tokenizer) {
  if (!mapw || !maph) throw new Error(`"mapw" and "maph" required before "map_data"`);
  map_data = new Uint8Array(mapw * maph);
  let token, dstc=0;
  while (token = tokenizer.next()) {
    if (token === ",") continue;
    if (token === "}") break;
    if (isNaN(map_data[dstc++] = +token)) throw new Error(`Expected integer, found ${JSON.stringify(token)}`);
  }
}

function parseModel(src) {
  const tokenizer = new Tokenizer(src, false);
  mapw = maph = 0;
  map_data = palette = null;
  let token;
  try {
    while (token = tokenizer.next()) {
      if (token !== "const") throw new Error(`Top level statement must begin 'const' (found ${JSON.stringify(token)})`);
      token = tokenizer.next();
      if (token === "unsigned") token = tokenizer.next();
      switch (token) {
        case "int": case "char": case "short": break;
        default: throw new Error(`Expected 'int', 'char', or 'short', found ${JSON.stringify(token)}`);
      }
      const k = tokenizer.next();
      let expectArray = false;
      switch (k) {
        case "palette": case "map_data": expectArray = true; break;
        case "mapw": case "maph": break;
        default: throw new Error(`Unexpected member ${JSON.stringify(k)}`);
      }
      if (expectArray) {
        token = tokenizer.next();
        if (token !== "[") throw new Error(`Expected "[" before ${JSON.stringify(token)}`);
        token = tokenizer.next();
        if (isident(token.charCodeAt(0))) token = tokenizer.next(); // We don't care about the declared length if there is one.
        if (token !== "]") throw new Error(`Expected "]" before ${JSON.stringify(token)}`);
        token = tokenizer.next();
        // All members must have an initializer.
        if (token !== "=") throw new Error(`Expected "=" before ${JSON.stringify(token)}`);
        token = tokenizer.next();
        if (token !== "{") throw new Error(`Expected "}" before ${JSON.stringify(token)}`);
        switch (k) {
          case "palette": readPalette(tokenizer); break;
          case "map_data": readMapData(tokenizer); break;
          default: throw new Error(`oops, k=${JSON.stringify(k)}`);
        }
      } else {
        token = tokenizer.next();
        // All members must have an initializer.
        if (token !== "=") throw new Error(`Expected "=" before ${JSON.stringify(token)}`);
        token = tokenizer.next();
        switch (k) {
          case "mapw": mapw = +token; break;
          case "maph": maph = +token; break;
          default: throw new Error(`d'oh, k=${JSON.stringify(k)}`);
        }
      }
      token = tokenizer.next();
      if (token !== ";") throw new Error(`Expected ";" before ${JSON.stringify(token)}`);
    }
  } catch (e) {
    e.message = tokenizer.lineno() + ": " + e.message;
    throw e;
  }
  if (!mapw || !maph || !palette || !map_data) throw new Error(`Missing required member`);
}

function onOpen(event) {
  if (event.target.files.length !== 1) throw new Error(`Expected exactly one file.`);
  const file = event.target.files[0];
  file.text().then(src => {
    originalText = src;
    parseModel(src);
    console.log(`Acquired file`, { originalText, palette, mapw, maph, map_data });
    render();
    renderPalette();
  });
}

function writePalette() {
  let dst = "const unsigned int palette[16]={\n";
  for (let p=0, i=8; i-->0; ) {
    dst += `0x${palette[p++].toString(16).padStart(8,'0')},`;
    dst += `0x${palette[p++].toString(16).padStart(8,'0')},\n`;
  }
  dst += "\n};\n";
  return dst;
}

function writeMapw() {
  return `const unsigned int mapw=${mapw};\n`;
}

function writeMaph() {
  return `const unsigned int maph=${maph};\n`;
}

function writeMapData() {
  let dst = "const unsigned char map_data[]={\n";
  let linep = dst.length;
  for (let i=0; i<map_data.length; i++) {
    dst += map_data[i] + ",";
    if (dst.length - linep >= 100) {
      dst += "\n";
      linep = dst.length;
    }
  }
  dst += "\n};\n";
  return dst;
}

function onSave() {
  if (!map_data) return;
  // Preserve top-level comments and order from the original text.
  // Rewrite each declaration.
  let dst = "";
  let wrotePalette=false, wroteMapw=false, wroteMaph=false, wroteMapData=false;
  const tokenizer = new Tokenizer(originalText, true);
  let token;
  while (token = tokenizer.next()) {
    
    // Whitespace or comment, emit it verbatim.
    // But collapse consecutive LFs; I suspect we're going to create them by accident.
    const ch = token.charCodeAt(0);
    if ((ch <= 0x20) || (ch === 0x2f)) {
      dst += token.replace(/\n\n+/g, "\n");
      continue;
    }
    
    // It's a declaration. Validate, yoink the key, and skip thru the semicolon.
    if (token !== "const") throw new Error(`Expected "const", found ${JSON.stringify(token)}`);
    tokenizer.includeSpace = false;
    token = tokenizer.next();
    if (token === "unsigned") token = tokenizer.next();
    switch (token) {
      case "int": case "char": case "short": break;
      default: throw new Error(`Expected "int", "char", or "short", found ${JSON.stringify(token)}`);
    }
    const k = tokenizer.next();
    for (;;) {
      token = tokenizer.next();
      if (!token) throw new Error(`Unclosed declaration of ${JSON.stringify(k)}`);
      if (token === ";") break;
    }
    tokenizer.includeSpace = true;
    
    // Rewrite the entire declaration.
    switch (k) {
      case "palette": wrotePalette = true; dst += writePalette(); break;
      case "mapw": wroteMapw = true; dst += writeMapw(); break;
      case "maph": wroteMaph = true; dst += writeMaph(); break;
      case "map_data": wroteMapData = true; dst += writeMapData(); break;
      default: throw new Error(`Unexpected member ${JSON.stringify(k)}`);
    }
  }
  // Write any declarations we didn't get.
  if (!wrotePalette) dst += writePalette();
  if (!wroteMapw) dst += writeMapw();
  if (!wroteMaph) dst += writeMaph();
  if (!wroteMapData) dst += writeMapData();
  // "Save" it by forcing a download.
  const blob = new Blob([dst], { type: "application/octet-stream" });
  const url = URL.createObjectURL(blob);
  window.open(url, "_blank");
  URL.revokeObjectURL(url);
}

function render(canvas, mx, my, mw, mh) {
  if (!map_data) return;
  if (!canvas) canvas = document.getElementById("visual");
  if (!mw || !mh) {
    mx = 0;
    my = 0;
    mw = mapw;
    mh = maph;
  }
  const bounds = canvas.getBoundingClientRect();
  const ctx = canvas.getContext("2d");
  
  /* The map is almost certainly smaller than the canvas.
   * Let's assume it won't grow large enough to need scrolling.
   * In order to keep the blitting simple, we actually make the canvas's framebuffer the map's natural size.
   * So we can't do anything high-res on top of the map image.
   */
  const tilesize = 4;
  const wmin = tilesize * mapw;
  const hmin = tilesize * maph;
  if ((canvas.width !== wmin) || (canvas.height !== hmin)) {
    canvas.width = wmin;
    canvas.height = hmin;
  }
  
  for (let y=my*tilesize, row=my; row<my+mh; row++, y+=tilesize) {
    for (let x=mx*tilesize, col=mx; col<mx+mw; col++, x+=tilesize) {
      prepareTile(map_data[row * mapw + col]);
      ctx.putImageData(imageData, x, y);
    }
  }
}

function prepareTile(src) {
  if (!imageData) imageData = new ImageData(4, 4);
  const ix = src >> 5;
  const bgcolor = palette[ix * 2] || 0xff000000; // xbgr
  const fgcolor = palette[ix * 2 + 1] || 0xffffffff; // ''
  const tileid = src & 0x1f;
  const srcx = (tileid & 7) * 4;
  const srcy = (tileid >> 3) * 4;
  for (let suby=0, dstp=0; suby<4; suby++) {
    for (let subx=0; subx<4; subx++) {
      const srcp = (srcy + suby) * 32 + (srcx + subx);
      const xbgr = GRAPHICS[srcp] ? fgcolor : bgcolor;
      imageData.data[dstp++] = (xbgr & 0xff);
      imageData.data[dstp++] = (xbgr >> 8) & 0xff;
      imageData.data[dstp++] = (xbgr >> 16) & 0xff;
      imageData.data[dstp++] = 0xff;
    }
  }
}

function renderPalette() {
  if (!palette) return;
  const canvas = document.getElementById("palette");
  const ctx = canvas.getContext("2d");
  prepareTile(paintbrush);
  ctx.putImageData(imageData, 0, 0);
}

function onResize(events) {
  const event = events[events.length - 1];
  const canvas = event.target;
  render(canvas);
}

/* Bafflingly, Canvas API does not give us a way to project to and from screen coordinates.
 * We're using mismatched sizes and object-fit:contain, so it really is a question.
 * But we can fake it.
 */
function mapCoordsForEvent(event) {
  if (!mapw || !maph) return [-1, -1];
  const bounds = event.target.getBoundingClientRect();
  const canvasAspect = bounds.width / bounds.height;
  const mapAspect = mapw / maph;
  let mx, my, mw, mh; // Map's projected bounds in the canvas.
  if (mapAspect > canvasAspect) { // Letterbox.
    mx = 0;
    mw = bounds.width;
    mh = bounds.width / mapAspect;
    my = (bounds.height >> 1) - (mh >> 1);
  } else { // Pillarbox.
    my = 0;
    mh = bounds.height;
    mw = bounds.height * mapAspect;
    mx = (bounds.width >> 1) - (mw >> 1);
  }
  const x = Math.floor(((event.clientX - bounds.x - mx) * mapw) / mw);
  const y = Math.floor(((event.clientY - bounds.y - my) * maph) / mh);
  return [x, y];
}

function onCanvasDown(event) {
  event.preventDefault();
  const [x, y] = mapCoordsForEvent(event);
  if ((x < 0) || (y < 0) || (x >= mapw) || (y >= maph)) return;
  if (event.ctrlKey) {
    paintbrush = map_data[y * mapw + x];
    renderPalette();
  } else {
    map_data[y * mapw + x] = paintbrush;
    painting = true;
  }
  lastx = x;
  lasty = y;
  render();
}

function onCanvasUp(event) {
  painting = false;
}

function onCanvasMove(event) {
  const [x, y] = mapCoordsForEvent(event);
  if ((x < 0) || (y < 0) || (x >= mapw) || (y >= maph)) {
    document.querySelector(".tattle").innerText = "";
  } else {
    document.querySelector(".tattle").innerText = `${x},${y}`;
  }
  if (painting && ((x !== lastx) || (y !== lasty))) {
    map_data[y * mapw + x] = paintbrush;
    lastx = x;
    lasty = y;
    render(null, x, y, 1, 1);
  }
}

function onCanvasContextMenu(event) {
  event.preventDefault();
}

function cssColorFromXbgr(xbgr) {
  const r = xbgr & 0xff;
  const g = (xbgr >> 8) & 0xff;
  const b = (xbgr >> 16) & 0xff;
  return `rgb(${r},${g},${b})`;
}

// (colorid) in 0..7
function onEditColor(colorid, cb) {
  const dialog = document.createElement("DIALOG");
  dialog.classList.add("color");
  
  const xbgrFromInputs = (pfx) => {
    const red = +dialog.querySelector(`input[name='${pfx}red']`).value;
    const green = +dialog.querySelector(`input[name='${pfx}green']`).value;
    const blue = +dialog.querySelector(`input[name='${pfx}blue']`).value;
    return 0xff000000 | (blue << 16) | (green << 8) | red;
  };
  
  const drawPreviews = () => {
    bgpreview.style.backgroundColor = cssColorFromXbgr(xbgrFromInputs("bg"));
    fgpreview.style.backgroundColor = cssColorFromXbgr(xbgrFromInputs("fg"));
  };
  
  const addSlider = (parent, name, ix, shift) => {
    const slider = document.createElement("INPUT");
    parent.appendChild(slider);
    slider.type = "range";
    slider.name = name;
    slider.min = 0;
    slider.max = 255;
    slider.value = (palette[ix] >> shift) & 0xff;
    slider.addEventListener("input", drawPreviews);
  };
  
  const bg = document.createElement("DIV");
  dialog.appendChild(bg);
  const bgpreview = document.createElement("DIV");
  bg.appendChild(bgpreview);
  bgpreview.classList.add("preview");
  addSlider(bg, "bgred", colorid*2, 0);
  addSlider(bg, "bggreen", colorid*2, 8);
  addSlider(bg, "bgblue", colorid*2, 16);
  
  const fg = document.createElement("DIV");
  dialog.appendChild(fg);
  const fgpreview = document.createElement("DIV");
  fg.appendChild(fgpreview);
  fgpreview.classList.add("preview");
  addSlider(fg, "fgred", colorid*2+1, 0);
  addSlider(fg, "fggreen", colorid*2+1, 8);
  addSlider(fg, "fgblue", colorid*2+1, 16);
  
  const submit = document.createElement("INPUT");
  fg.appendChild(submit);
  submit.type = "button";
  submit.value = "OK";
  submit.addEventListener("click", () => {
    palette[colorid * 2] = xbgrFromInputs("bg");
    palette[colorid * 2 + 1] = xbgrFromInputs("fg");
    dialog.remove();
    renderPalette();
    cb?.();
  });
  
  drawPreviews();
  document.body.appendChild(dialog);
  dialog.showModal();
}

function onPaletteClick(event) {
  let color = paintbrush >> 5;
  let tileid = paintbrush & 0x1f;
  const dialog = document.createElement("DIALOG");
  dialog.classList.add("palette");
  
  const contexts = [];
  const redrawTiles = () => {
    for (let etileid=0; etileid<32; etileid++) {
      const ctx = contexts[etileid];
      if (!ctx) continue;
      prepareTile((color << 5) | etileid);
      ctx.putImageData(imageData, 0, 0);
    }
  };
  
  const left = document.createElement("DIV");
  dialog.appendChild(left);
  for (let i=0; i<8; i++) {
    const colorid = i;
    const row = document.createElement("DIV");
    left.appendChild(row);
    const input = document.createElement("INPUT");
    row.appendChild(input);
    input.type = "radio";
    input.id = `color-radio-${i}`;
    input.name = "color";
    input.value = i;
    input.checked = (i === color);
    input.addEventListener("change", e => {
      if (input.checked) color = colorid;
      redrawTiles();
    });
    const label = document.createElement("LABEL");
    row.appendChild(label);
    label.setAttribute("for", `color-radio-${i}`);
    label.innerText = `Color ${i}`;
    const bgtattle = document.createElement("DIV");
    row.appendChild(bgtattle);
    bgtattle.classList.add("colortattle");
    bgtattle.style.backgroundColor = cssColorFromXbgr(palette[i * 2]);
    bgtattle.addEventListener("click", () => onEditColor(colorid, () => {
      bgtattle.style.backgroundColor = cssColorFromXbgr(palette[i * 2]);
      redrawTiles();
    }));
    const fgtattle = document.createElement("DIV");
    row.appendChild(fgtattle);
    fgtattle.classList.add("colortattle");
    fgtattle.style.backgroundColor = cssColorFromXbgr(palette[i * 2 + 1]);
    fgtattle.addEventListener("click", () => onEditColor(colorid, () => {
      fgtattle.style.backgroundColor = cssColorFromXbgr(palette[i * 2 + 1]);
      redrawTiles();
    }));
  }
  const submit = document.createElement("INPUT");
  left.appendChild(submit);
  submit.type = "button";
  submit.value = "OK";
  submit.addEventListener("click", event => {
    paintbrush = (color << 5) | tileid;
    dialog.remove();
    renderPalette();
  });
  
  const right = document.createElement("DIV");
  dialog.appendChild(right);
  for (let row=0, etileid=0; row<4; row++) {
    const uirow = document.createElement("DIV");
    right.appendChild(uirow);
    uirow.classList.add("row");
    for (let col=0; col<8; col++, etileid++) {
      const eetileid = etileid;
      const canvas = document.createElement("CANVAS");
      uirow.appendChild(canvas);
      canvas.classList.add("paletteTile");
      canvas.width = 4;
      canvas.height = 4;
      canvas.setAttribute("data-tileid", etileid);
      if (etileid === tileid) {
        canvas.style.borderColor = "#0ff";
      }
      canvas.addEventListener("click", e => {
        for (const other of dialog.querySelectorAll("canvas")) other.style.borderColor = "#fff";
        canvas.style.borderColor = "#0ff";
        tileid = eetileid;
      });
      const ctx = canvas.getContext("2d");
      contexts.push(ctx);
    }
  }
  
  redrawTiles();
  document.body.appendChild(dialog);
  dialog.showModal();
}

addEventListener("load", () => {
  
  const toolbar = document.createElement("DIV");
  toolbar.classList.add("toolbar");
  document.body.appendChild(toolbar);
  const openButton = document.createElement("INPUT");
  openButton.type = "file";
  openButton.addEventListener("change", onOpen);
  toolbar.appendChild(openButton);
  const saveButton = document.createElement("INPUT");
  saveButton.type = "button";
  saveButton.value = "Save";
  saveButton.addEventListener("click", onSave);
  toolbar.appendChild(saveButton);
  const tattle = document.createElement("DIV");
  tattle.classList.add("tattle");
  toolbar.appendChild(tattle);
  const paletteCanvas = document.createElement("CANVAS");
  paletteCanvas.id = "palette";
  paletteCanvas.width = 4;
  paletteCanvas.height = 4;
  paletteCanvas.style.width = "16px";
  paletteCanvas.style.height = "16px";
  paletteCanvas.style.border = "1px solid #fff";
  paletteCanvas.style.imageRendering = "pixelated";
  paletteCanvas.addEventListener("click", onPaletteClick);
  toolbar.appendChild(paletteCanvas);
  renderPalette();
  
  const canvas = document.createElement("CANVAS");
  canvas.id = "visual";
  canvas.addEventListener("pointerdown", onCanvasDown);
  canvas.addEventListener("pointerup", onCanvasUp);
  canvas.addEventListener("pointermove", onCanvasMove);
  canvas.addEventListener("contextmenu", onCanvasContextMenu);
  document.body.appendChild(canvas);
  
  const resizeObserver = new ResizeObserver(onResize);
  resizeObserver.observe(canvas);
});

/* Using file protocol, we are not allowed to read the imageData of a canvas once our graphics were drawn there,
 * due to cross-origin policy. The fuck is that.
 * But whatever, the source graphics are trivial. When we change the original, get a dump of it, one word per pixel.
 * $ hexdump -e '32/1 "%u," "\n"' src/editor/graphics.y8 | sed 's/255/1/g'
 */
const GRAPHICS = [
0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,
0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,1,0,1,0,1,0,1,1,0,0,0,0,0,0,1,
0,0,0,0,1,0,0,1,1,1,1,1,1,0,0,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,0,1,
0,0,0,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,0,1,0,1,1,0,1,1,1,1,0,1,
1,1,1,1,1,0,1,0,1,0,1,0,1,1,0,1,1,0,1,0,0,1,1,0,1,0,0,0,0,0,0,1,
1,0,0,1,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,
1,0,0,1,1,0,1,0,1,0,1,0,1,1,0,1,1,0,1,0,1,0,1,0,0,0,0,1,1,0,0,0,
1,1,1,1,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,1,0,0,0,
0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,
1,0,0,1,1,1,0,0,0,0,0,0,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,1,1,1,1,1,1,1,
1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
];
