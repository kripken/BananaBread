
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

  // based on https://developer.mozilla.org/en-US/docs/Web/WebGL/Animating_textures_in_WebGL
  var video = document.createElement('video');
  video.addEventListener("canplaythrough", function() {
    video.play();
    var glid = 0;
    setInterval(function() {
      if (!glid) glid = Module._getglid();
      if (!glid) return;
      var gl = Module.ctx;
      gl.bindTexture(gl.TEXTURE_2D, GL.textures[glid]);
      gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, video);
    }, 1000/10);
  }, true);
  video.preload = 'auto';
  video.loop = true;
  video.src = 'Firefox.ogv';
  video.crossOrigin = 'anonymous';
  document.body.appendChild(video);
};

Module.loadDefaultMap = function() {
  if (Module.benchmark) {
    var bots = [];
    for (var i = 0; i < 30; i++) {
      bots.push('addbot ' + (i+50));
    }
    BananaBread.execute('showfps 0 ; sleep 10 [ effic colos ; ' + bots.join(' ; ') + ' ]');
  } else {
    BananaBread.execute('sleep 10 [ effic colos ; sleep 20000 [ addbot 50 ] ]');
  }
};

