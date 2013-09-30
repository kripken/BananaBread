(function() {

  // connect to canvas
  var Boon = {
    preRun: [],
    postRun: [],
    print: Module.print,
    printErr: Module.printErr,
    setStatus: function(text) {},
    monitorRunDependencies: function(left) {}
  };




  // proxy to/from worker

  var worker = new Worker('boon.js');

  worker.onmessage = function(event) {
    var data = event.data;
    switch (data.target) {
      case 'stdout': {
        Boon.print(data.content);
        break;
      }
      case 'stderr': {
        Boon.printErr(data.content);
        break;
      }
      case 'window': {
        window[data.method]();
        break;
      }
      case 'canvas': {
        switch (data.op) {
          case 'resize': {
            break;
          }
          case 'render': {
            render(data.image.data);
            break;
          }
          default: throw 'eh?';
        }
        break;
      }
      default: throw 'what?';
    }
  };

  function cloneEvent(event) {
    var ret = {};
    for (var x in event) {
      if (x == x.toUpperCase()) continue;
      var prop = event[x];
      if (typeof prop === 'number' || typeof prop === 'string') ret[x] = prop;
    }
    return ret;
  };

  ['keydown', 'keyup', 'keypress'].forEach(function(event) {
    document.addEventListener(event, function(event) {
      event.preventDefault();
      var event = cloneEvent(event);
      //console.log(event.keyCode);
      switch (event.keyCode) {
        case 220: event.keyCode = 27; break; // map | to escape
      }
      worker.postMessage({ target: 'document', event: event });
    });
  });


  // render
  var glid = 0;
  function render(data) {
    var first = false;
    if (!glid) {
      glid = Module._getglid(2);
      if (glid) first = true;
    }
    if (!glid) return;
    var gl = Module.ctx;
    gl.bindTexture(gl.TEXTURE_2D, GL.textures[glid]);
    if (first) {
      // disable mipmap for this texture
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
    }
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 256, 256, 0, gl.RGBA, gl.UNSIGNED_BYTE, data);
    gl.bindTexture(gl.TEXTURE_2D, null);
  }

})();

