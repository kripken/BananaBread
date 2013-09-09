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
            Boon.drawBuffer = data.image.data;
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
      worker.postMessage({ target: 'document', event: cloneEvent(event) });
      event.preventDefault();
    });
  });


  // render
  var glid = 0;
  setInterval(function() {
    if (!Boon.drawBuffer) return;
    if (!glid) glid = Module._getglid();
    if (!glid) return;
    var gl = Module.ctx;
    gl.bindTexture(gl.TEXTURE_2D, GL.textures[glid]);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 256, 256, 0, gl.RGBA, gl.UNSIGNED_BYTE, Boon.drawBuffer);
  }, 1000/10);

})();

