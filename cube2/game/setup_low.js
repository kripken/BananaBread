
Module.setPlayerModels = function() {
  //BananaBread.setPlayerModelInfo("frankie", "frankie", "frankie", 0, "nada", 0, 0, 0, 0, "frankie", "frankie", "frankie", false);
  BananaBread.setPlayerModelInfo("snoutx10k", "snoutx10k", "snoutx10k", "snoutx10k/hudguns", 0, 0, 0, 0, 0, "snoutx10k", "snoutx10k", "snoutx10k", true);
};

Module.tweakDetail = function() {
  BananaBread.execute('fog 10000'); // disable fog
  BananaBread.execute('maxdebris 10');
  if (Module.benchmark) {
    BananaBread.execute('shaderdetail 1');
    BananaBread.execute('maxdynlights 0');
  }

  if (1) {
    var boon = document.createElement('script');
    boon.src = 'miniBoon.js';
    document.body.appendChild(boon);
  } else {
    // based on https://developer.mozilla.org/en-US/docs/Web/WebGL/Animating_textures_in_WebGL
    var video = document.createElement('video');
    video.addEventListener("canplaythrough", function() {
      video.play();
      var glid = 0;
      setInterval(function() {
        var first = false;
        if (!glid) {
          glid = Module._getglid();
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
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, video);
      }, 1000/30);
    }, true);
    video.preload = 'auto';
    video.loop = true;
    video.src = 'Firefox.ogv';
    video.crossOrigin = 'anonymous';
    video.style = 'display: none';
    document.body.appendChild(video);
  }
};

Module.loadDefaultMap = function() {
  if (Module.benchmark) {
    var bots = [];
    for (var i = 0; i < 30; i++) {
      bots.push('addbot ' + (i+50));
    }
    BananaBread.execute('showfps 0 ; sleep 10 [ effic colos ; ' + bots.join(' ; ') + ' ]');
  } else {
    BananaBread.execute('sleep 10 [ effic colos ]');
    //BananaBread.execute('sleep 10 [ effic colos ; sleep 20000 [ addbot 50 ] ]');
  }
};

