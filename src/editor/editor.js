let graphics; // Image
let originalText; // string
let palette; // Uint32Array(16), xbgr
let mapw,maph; // int
let map_data; // Uint8Array(mapw*maph)

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
  });
}

function writePalette() {
  let dst = "const unsigned int palette[16]={\n";
  for (let i=0; i<16; i++) dst += palette[i] + ",";
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

function render(canvas) {
  if (!canvas) canvas = document.getElementById("visual");
  const ctx = canvas.getContext("2d");
  ctx.fillStyle = "#333";
  ctx.fillRect(0, 0, canvas.width, canvas.height);
  if (!map_data) return;
}

function onResize(events) {
  const event = events[events.length - 1];
  const canvas = event.target;
  canvas.width = event.contentRect.width;
  canvas.height = event.contentRect.height;
  render(canvas);
}

addEventListener("load", () => {
  graphics = document.getElementById("graphics");
  
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
  
  const canvas = document.createElement("CANVAS");
  canvas.id = "visual";
  document.body.appendChild(canvas);
  
  const resizeObserver = new ResizeObserver(onResize);
  resizeObserver.observe(canvas);
});
