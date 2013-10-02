
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

  BananaBread.execute('thirdperson 1');

  var glids = {};

  function uploadTexture(id, element, mip) {
    var first = false;
    if (!glids[id]) {
      glids[id] = Module._getglid(id);
      if (glids[id]) first = true;
    }
    var glid = glids[id];
    if (!glid) return false;
    assert(GL.textures[glid]);
    var gl = Module.ctx;
    gl.bindTexture(gl.TEXTURE_2D, GL.textures[glid]);
    if (first && !mip) {
      // disable mipmap for this texture
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
    } else if (mip) {
      gl.generateMipmap(gl.TEXTURE_2D);
    }
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, element);
    gl.bindTexture(gl.TEXTURE_2D, null);
    return true;
  }

  if (1) {
    var updateTweets = null;

    var tweety = document.createElement('script');

    tweety.onload = function() {
      var canvas = document.createElement('canvas');
      canvas.width = 512;
      canvas.height = 512;
      var ctx = canvas.getContext("2d");

      function render(text, callback) {
        canvas.width = canvas.width; // clear it
        var data = "<svg xmlns='http://www.w3.org/2000/svg' width='512' height='512'>" +
                     "<foreignObject width='100%' height='100%'>" +
                       "<div xmlns='http://www.w3.org/1999/xhtml' style='font-size:18px; border: 10px solid white; background-color: #200c05'>" +
                         "<span style='margin: 6px; color: #ff9955; text-shadow:0 0 5px red;'>" + text + "</span>" + //  style=''
                       "</div>" +
                     "</foreignObject>" +
                   "</svg>";
        var DOMURL = self.URL || self.webkitURL || self;
        var img = new Image();
        var svg = new Blob([data], {type: "image/svg+xml;charset=utf-8"});
        var url = DOMURL.createObjectURL(svg);
        img.onload = function() {
          ctx.drawImage(img, 0, 0);
          DOMURL.revokeObjectURL(url);
          callback(canvas);
        };
        img.src = url;
      }

      function getTweets(callback) {
        function handleTweets(tweets){
          var x = tweets.length;
          var n = 0;
          var html = '';
          while(n < x) {
            var tweet = tweets[n];
            tweet = tweet.replace('class="user"', 'style="font-weight: bold"');
            tweet = tweet.replace('class="timePosted"', 'style="font-style: italic"');
            console.log(JSON.stringify(tweet, null, '  '));
            html += '<p>' + tweet + '</p><hr style="color: black"></hr>';
            n++;
          }
          html += '';
          callback(html);
        }
        twitterFetcher.fetch('384367035376873472', 'example1', 5, false, true, true, '', false, handleTweets, false);
      }

      updateTweets = function() {
        console.log('updating tweets');

        ctx.fillStyle = '#200c05';
        ctx.fillRect(0, 0, 512, 512);
        uploadTexture(0, canvas, true);

        setTimeout(function() {
          getTweets(function(html) {
            html = html.replace(/[\n'&]/g, '');
            for (var i = 0; i < 20; i++) html = html.replace(/  /g, ' ')
            render(html, function doRender(canvas) {
              if (!uploadTexture(0, canvas, true)) {
                setTimeout(function() {
                  doRender(canvas);
                }, 500);
              }
            });
          });
        }, 100); // slight lag, to show an update is happening
      };

      updateTweets();
    };

    tweety.src = 'tweety.js';
    document.body.appendChild(tweety);

    ['keyup'].forEach(function(event) {
      document.addEventListener(event, function(event) {
        if (event.keyCode === 85) { // 'u'
          updateTweets();
        }
      });
    });
  }

  if (1) {
    // boon
    var boon = document.createElement('script');
    boon.src = 'miniBoon.js';
    document.body.appendChild(boon);
  }

  if (1) {
    // video
    // based on https://developer.mozilla.org/en-US/docs/Web/WebGL/Animating_textures_in_WebGL
    var video = document.createElement('video');
    video.addEventListener("canplaythrough", function() {
      video.play();
      setInterval(function() {
        uploadTexture(1, video);
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

