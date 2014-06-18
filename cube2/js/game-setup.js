// Setup compiled code parameters and interaction with the web page

var shell = typeof window == 'undefined';

if (shell) {
  load('game/headless.js');
  load('game/headlessCanvas.js');
}

if (typeof pageParams === 'undefined') {
  var pageParams = window.location.search || '';
}
if (pageParams[0] == '?') pageParams = pageParams.substr(1);

function checkPageParam(param) {
  return pageParams.split(',').indexOf(param) >= 0
}

Date.realNow = Date.now;
if (checkPageParam('deterministic')) {
  (function() {
    var MAGIC = 0;
    Math.random = function() {
      MAGIC = Math.pow(MAGIC + 1.8912, 3) % 1;
      return MAGIC;
    };
    var TIME = 10000;
    Date.now = function() {
      TIME += 17;
      return TIME;
    };
    performance.now = Date.now;
  })();
}

var Module = {
  // If the url has 'serve' in it, run a listen server and let others connect to us
  arguments: checkPageParam('serve') ? ['-d1', '-j28780'] : [],
  benchmark: checkPageParam('benchmark') ? { totalIters: 2000, iter: 0 } : null,
  failed: false,
  preRun: [],
  postRun: [],
  preloadPlugins: [],
  print: function(text) {
    console.log('[STDOUT] ' + text);
    //if (!Module.stdoutElement) Module.stdoutElement = document.getElementById('stdout');
    //Module.stdoutElement.value += text + '\n';
  },
  printErr: function(text) {
    console.log(text);
  },
  canvas: checkPageParam('headlessCanvas') ? headlessCanvas() : document.getElementById('canvas'),
  statusMessage: 'Starting...',
  progressElement: document.getElementById('progress'),
  setStatus: function(text) {
console.log(text);
    if (Module.setStatus.interval) clearInterval(Module.setStatus.interval);
    var statusElement = document.getElementById('status-text');
    if (Module.finishedDataFileDownloads >= 1 && Module.finishedDataFileDownloads < Module.expectedDataFileDownloads) {
      // If we are in the middle of multiple datafile downloads, do not show preloading progress - show only download progress
      var m2 = text.match(/([^ ]+) .*/);
      if (m2) {
        if (m2[1] == 'Preparing...') return;
      }
    }
    var m = text.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/);
    if (m) {
      text = m[1];
      Module.progressElement.value = parseInt(m[2])*100;
      Module.progressElement.max = parseInt(m[4])*100;
      Module.progressElement.hidden = false;
    } else {
      Module.progressElement.value = null;
      Module.progressElement.max = null;
      Module.progressElement.hidden = true;
    }
    statusElement.innerHTML = text;
  },
  totalDependencies: 0,
  monitorRunDependencies: function(left) {
    this.totalDependencies = Math.max(this.totalDependencies, left);
    Module.setStatus(left ? 'Preparing... (' + (this.totalDependencies-left) + '/' + this.totalDependencies + ')' : 'All downloads complete.');
  },
  onFullScreen: function(isFullScreen) {
    Module.isFullScreen = isFullScreen;
    if (isFullScreen) {
      Module.resumeMainLoop();
      Module.setOpacity(1);
      Module.setStatus('');
      document.querySelector('.status .ingame').classList.add( 'hide' );
      Module.canvas.classList.remove( 'paused' );
      Module.canvas.classList.remove( 'hide' );
      //BananaBread.execute('musicvol $oldmusicvol'); // XXX TODO: need to restart the music by name here
    } else {
      Module.pauseMainLoop();
      Module.setOpacity(0.333);
      Module.setStatus('<b>paused (enter fullscreen to resume)</b>');
      Module.canvas.classList.add( 'paused' );
      document.querySelector('.status .ingame').classList.remove( 'hide' );
      Module.canvas.classList.add( 'hide' );
      //BananaBread.execute('oldmusicvol = $musicvol ; musicvol 0');
    }
  }
};

if (Module.benchmark) {
  Module.print('<< start preload >>');
  preloadStartTime = Date.realNow();

  Module.preRun.push(function() {
    var main = Module._main;
    Module._main = function() {
      Module.print('<< start startup >>');
      Module.startupStartTime = Date.realNow();
      return main.apply(null, arguments);
    };
  });

  Module.benchmark.progressTick = Math.floor(Module.benchmark.totalIters / 100);

  Module.preMainLoop = function() {
    if (Module.gameStartTime) Module.frameStartTime = Date.realNow();
  };

  Module.postMainLoop = function() {
    if (Module.gameStartTime) Module.gameTotalTime += Date.realNow() - Module.frameStartTime;

    var iter = Module.benchmark.iter++;
    if (iter == 1) {
      Module.progressElement.hidden = false;
      Module.progressElement.max = Module.benchmark.totalIters;
      BananaBread.execute('spectator 1 ; nextfollow'); // do not get shot at by bots
    } else if (iter % Module.benchmark.progressTick == 1) {
      Module.progressElement.value = iter; // TODO: check if this affects performance
    } else if (iter >= Module.benchmark.totalIters) {
      window.stopped = true;
      Module.pauseMainLoop();

      // show results
      Module.canvas.classList.add('hide');
      var results = '';
      var end = Date.realNow();
      results += 'finished, times:\n';
      results += '  preload : ' + (Module.startupStartTime - preloadStartTime)/1000 + ' seconds\n';
      results += '  startup : ' + (Module.gameStartTime - Module.startupStartTime)/1000 + ' seconds\n';
      results += '  gameplay: ' + (end - Module.gameStartTime)/1000 + ' total seconds\n';
      results += '  gameplay: ' + Module.gameTotalTime/1000 + ' JS seconds\n';
      if (window.headless) {
        Module.print(results);
      } else {
        document.getElementById('main_text').classList.remove('hide');
        document.getElementById('main_text').innerHTML = results.replace(/\n/g, '<br>');
      }
    } else if (iter % 333 == 5) {
      BananaBread.execute('nextfollow');
    }
  };
}

// Checks for features we cannot run without
// Note: Modify this for your needs. If your level does not use
//       texture compression, remove the check for it here.

(function() {
  function fail(text) {
    Module._main = null;
    document.querySelector('.status-content.error .details').innerHTML = text + ' is missing.';
    document.querySelector('.status-content.loading .progress-container').classList.add('hide');
    document.querySelector('.status-content.error').classList.remove('hide');
    Module.failed = true;
  }
  if (checkPageParam('headlessCanvas')) return;
  try {
    var canvas = document.createElement('canvas');
  } catch(e){}
  if (!canvas) fail('canvas element');
  try {
    var context = canvas.getContext('experimental-webgl');
  } catch(e){}
  if (!context) fail('WebGL');
  var s3tc = context.getExtension('WEBGL_compressed_texture_s3tc') ||
             context.getExtension('MOZ_WEBGL_compressed_texture_s3tc') ||
             context.getExtension('WEBKIT_WEBGL_compressed_texture_s3tc');
  if (!s3tc) fail('texture compression');
  var pointerLock = canvas['requestPointerLock'] ||
                    canvas['mozRequestPointerLock'] ||
                    canvas['webkitRequestPointerLock'];
  if (!pointerLock) fail('pointer lock/mouse lock');
})();

// Loading music. Will be stopped once the first frame of the game runs

Module.loadingMusic = new Audio();
if (!Module.benchmark) Module.loadingMusic.src = 'assets/OutThere_0.ogg';
Module.loadingMusic.play();

Module.readySound = new Audio();
Module.readySound.src = 'assets/alarmcreatemiltaryfoot_1.ogg';

// Pre-unzip ogz files, we can do this in parallel in a worker during preload

(function() {
  // TODO: use zee.js
  var zeeWorker = new Worker('game/zee-worker.js');

  var zeeCallbacks = [];

  zeeWorker.onmessage = function(msg) {
    zeeCallbacks[msg.data.callbackID](msg.data.data);
    console.log("zee'd " + msg.data.filename + ' in ' + msg.data.time + ' ms, ' + msg.data.data.length + ' bytes');
    zeeCallbacks[msg.data.callbackID] = null;
  };

  function requestZee(filename, data, callback) {
    zeeWorker.postMessage({
      filename: filename,
      data: new Uint8Array(data), // do not send over the underlying ArrayBuffer
      callbackID: zeeCallbacks.length
    });
    zeeCallbacks.push(callback);
  }

  Module.postRun.push(function() {
    zeeWorker.terminate();
  });

  Module.preloadPlugins.push({
    canHandle: function(name) {
      return name.substr(-4) == '.ogz';
    },
    handle: function(byteArray, name, onload, onerror) {
      requestZee(name, byteArray, function(byteArray) {
        onload(byteArray);
      });
    }
  });
})();

// Hooks

Module.setOpacity = function(opacity) {
  var rule = 'canvas.emscripten';
  var more = 'border: 1px solid black';
  var styleSheet = document.styleSheets[0];
  var rules = styleSheet.cssRules;
  if (rules) {
    for (var i = 0; i < rules.length; i++) {
      if (rules[i].cssText.substr(0, rule.length) == rule) {
        styleSheet.deleteRule(i);
        i--;
      }
    }
  }
  styleSheet.insertRule(rule + ' { opacity: ' + opacity + '; ' + (more || '') + ' }', 0);
}

Module.setOpacity(0.1);

Module.fullscreenCallbacks = [];

Module.postLoadWorld = function() {
  document.title = 'BananaBread';

  if (Module.loadingMusic) {
    Module.loadingMusic.pause();
    Module.loadingMusic = null;
  }
  Module.tweakDetail();

  BananaBread.execute('sensitivity 10');
  BananaBread.execute('clearconsole');

  setTimeout(function() {
    BananaBread.execute('oldmusicvol = $musicvol ; musicvol 0');
  }, 1); // Do after startup finishes so music will be prepared up

  if (checkPageParam('windowed')) {
    Module.canvas.classList.remove('hide');
    Module.isFullScreen = 1;
    Module.requestFullScreen = function() {
      setTimeout(function() {
        Module.onFullScreen(1);
        Module.canvas.classList.remove('hide');
      }, 0);
    }
  }

  if (!Module.isFullScreen) {
    // Pause and fade out until the user presses fullscreen

    Module.pauseMainLoop();
    setTimeout(function() {
      document.querySelector('.status-content.loading').classList.add('hide');
      document.querySelector('.status-content.fullscreen-buttons').classList.remove('hide');
    }, 0);

    Module.resume = function() {
      Module.requestFullScreen();
      Module.setOpacity(1);
      Module.setStatus('');
      Module.resumeMainLoop();
   };

    Module.fullscreenLow = function() {
      document.querySelector('.status-content.fullscreen-buttons').classList.add('hide');
      Module.canvas.classList.remove('hide');
      Module.requestFullScreen(true);
      Module.setOpacity(1);
      Module.setStatus('');
      Module.resumeMainLoop();
      Module.fullscreenCallbacks.forEach(function(callback) { callback() });
    };

    Module.fullscreenHigh = function() {
      document.querySelector('.status-content.fullscreen-buttons').classList.add('hide');
      Module.canvas.classList.remove('hide');
      Module.requestFullScreen(true);
      Module.setOpacity(1);
      Module.setStatus('');
      BananaBread.execute('screenres ' + screen.width + ' ' + screen.height);
      Module.resumeMainLoop();
      Module.fullscreenCallbacks.forEach(function(callback) { callback() });
    };

    // All set!
    if (Module.readySound) {
      Module.readySound.play();
      Module.readySound = null;
    }
  }

  if (Module.benchmark) {
    Module.print('<< start game >>');
    Module.gameStartTime = Date.realNow();
    Module.gameTotalTime = 0;
    if (!window.headless) document.getElementById('main_text').classList.add('hide');
    Module.canvas.classList.remove('hide');
  }
};

Module.autoexec = function(){}; // called during autoexec on load, so useful to tweak settings that require gl restart
Module.tweakDetail = function(){}; // called from postLoadWorld, so useful to make changes after the map has been loaded

(function() {
  var fraction = 0.65;
  var desired = Math.min(fraction*screen.availWidth, fraction*screen.availHeight, 600);
  var w, h;
  if (screen.width >= screen.height) {
    h = desired;
    w = Math.floor(desired * screen.width / screen.height);
  } else {
    w = desired;
    h = Math.floor(desired * screen.height / screen.width);
  }
  Module.desiredWidth = w;
  Module.desiredHeight = h;
})();

// Public API

var BananaBread = {
  // TODO: use api.js
  init: function() {
    BananaBread.setPlayerModelInfo = Module.cwrap('_ZN4game18setplayermodelinfoEPKcS1_S1_S1_S1_S1_S1_S1_S1_S1_S1_S1_b', null,
      ['string', 'string', 'string', 'string', 'string', 'string', 'string', 'string', 'string', 'string', 'string', 'string', 'number']);
    BananaBread.execute = Module.cwrap('_Z7executePKc', 'number', ['string']);
    BananaBread.executeString = Module.cwrap('_Z10executestrPKc', 'string', ['string']);

    var forceCamera = Module.cwrap('setforcecamera', null, ['number', 'number', 'number', 'number', 'number', 'number']);
    BananaBread.forceCamera = function(position, orientation) {
      forceCamera(position[0], position[1], position[2], orientation[0], orientation[1], orientation[2]);
    };

    BananaBread.PARTICLE = {};
    var i = 0;
    BananaBread.PARTICLE.BLOOD = (i++);
    BananaBread.PARTICLE.WATER = (i++);
    BananaBread.PARTICLE.SMOKE = (i++);
    BananaBread.PARTICLE.STEAM = (i++);
    BananaBread.PARTICLE.FLAME = (i++);
    BananaBread.PARTICLE.FIREBALL1 = (i++);
    BananaBread.PARTICLE.FIREBALL2 = (i++);
    BananaBread.PARTICLE.FIREBALL3 = (i++);
    BananaBread.PARTICLE.STREAK = (i++);
    BananaBread.PARTICLE.LIGHTNING = (i++);
    BananaBread.PARTICLE.EXPLOSION = (i++);
    BananaBread.PARTICLE.EXPLOSION_BLUE = (i++);
    BananaBread.PARTICLE.SPARK = (i++);
    BananaBread.PARTICLE.EDIT = (i++);
    BananaBread.PARTICLE.SNOW = (i++);
    BananaBread.PARTICLE.MUZZLE_FLASH1 = (i++);
    BananaBread.PARTICLE.MUZZLE_FLASH2 = (i++);
    BananaBread.PARTICLE.MUZZLE_FLASH3 = (i++);
    BananaBread.PARTICLE.HUD_ICON = (i++);
    BananaBread.PARTICLE.HUD_ICON_GREY = (i++);
    BananaBread.PARTICLE.TEXT = (i++);
    BananaBread.PARTICLE.METER = (i++);
    BananaBread.PARTICLE.METER_VS = (i++);
    BananaBread.PARTICLE.LENS_FLARE = (i++);
    var splash = Module.cwrap('bb_splash', null, ['number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number']);
    BananaBread.splash = function(type, color, radius, num, fade, p, size, gravity) {
      splash(type, color, radius, num, fade, p[0], p[1], p[2], size, gravity);
    };

    var playSoundName = Module.cwrap('bb_playsoundname', null, ['string', 'number', 'number', 'number']);
    BananaBread.playSound = function(name, position) {
      playSoundName(name, position[0], position[1], position[2]);
    };
  },
};

Module.postRun.push(BananaBread.init);
Module.postRun.push(function() {
  var n = 0;
  for (var x in Module.preloadedAudios) n++;
  console.log('successfully preloaded audios: ' + n);
  if (n == 0) alert('Your browser could not load the audio files. Running will continue without sound effects.');
});

// Additional APIs

BananaBread.Utils = {
  randomPick: function(items) {
    return items[Math.floor(Math.random()*items.length)];
  },
};

BananaBread.Event = function(data) {
  this.run = function() {
    var start = Date.now();
    var last = start;
    function iteration() {
      var now = Date.now();
      var ms = now - last;
      last = now;
      if (ms > data.totalMs) return;
      data.onFrame(ms);
      Module.requestAnimationFrame(iteration);
    }
    iteration();
  };
  if (data.onInit) data.onInit(data);
}

BananaBread.Effects = {
  Fireworks: function(shots) {
    var event = new BananaBread.Event({
      totalMs: Infinity,

      onFrame: function(ms) {

        var secs = ms/1000;
        var newShots = [];

        shots = shots.filter(function(shot) {
          LinearMath.vec3.add(shot.position, LinearMath.vec3.scale(LinearMath.vec3.create(shot.velocity), secs));
          shot.velocity[2] -= secs * 200; // gravity
          shot.msLeft -= ms;
          if (shot.msLeft > 0) {
            BananaBread.splash(BananaBread.PARTICLE.SPARK, 0xffffff, 1, 20, Math.max(50, ms*2), shot.position, 1, 1);
            return true;
          } else {
            var size = Math.ceil(Math.random()*3); // 1, 2 or 3
            var color;
            for (var i = 0; i < 2; i++) {
              color = Math.floor(Math.random()*255) + (Math.floor(Math.random()*255) << 8) + (Math.floor(Math.random()*255) << 16);
              BananaBread.splash(BananaBread.PARTICLE.SPARK, color, 100+25*size, 7+3*size, Math.max(300, ms*7), shot.position, 1+size, 1);
            }
            if (size > 1) {
              BananaBread.splash(BananaBread.PARTICLE.EXPLOSION, color, 0, 1, Math.max(175, ms*3), shot.position, 5*size, 0);
            }
            BananaBread.playSound(size == 3 ? 'q009/explosion.ogg' : 'olpc/MichaelBierylo/sfx_DoorSlam.wav', shot.position);
            return false;
          }
        });
      
        shots.push.apply(shots, newShots);

        if (shots.length == 0) this.totalMs = 0;
      },
    });

    event.run();
  }
};

function CameraPath(data) { // TODO: namespace this
  var steps = data.steps;
  var n = data.steps.length;
  var timeScale = data.timeScale;
  var position = LinearMath.vec3.create();
  var temp = LinearMath.vec3.create();
  var orientation = LinearMath.vec3.create();
  var cancelled = false;
  var sigma = data.sigma || 0.75;
  var lasti = -1;
  var debug = data.debug;
  var loop = data.loop;

  if (!data.uncancellable) addEventListener('keydown', function() { cancelled = true });

  this.execute = function() {
    var startTime = null;
    function moveCamera() {
      if (!startTime) startTime = Date.now();
      if (cancelled) return;
      var now = Date.now();
      var t = (Date.now() - startTime)/(timeScale*1000);
      if (t > n-1 && !loop) return;
      var i = Math.round(t);
      if (debug && i != lasti) {
        lasti = i;
        alert('now on ' + i);
        startTime += (Date.now() - now); // ignore alert wait time
      }
      var factors = 0;
      position[0] = position[1] = position[2] = orientation[0] = orientation[1] = orientation[2] = 0;
      for (var j = i-2; j <= i+2; j++) {
        var jj = j;
        if (loop && jj >= n) jj = jj % n;
        var curr = steps[jj];
        if (!curr) continue;
        var factor = Math.exp(-Math.pow((j-t)/sigma, 2));
        LinearMath.vec3.scale(curr.position, factor, temp);
        LinearMath.vec3.add(position, temp);
        LinearMath.vec3.scale(curr.orientation, factor, temp);
        LinearMath.vec3.add(orientation, temp);
        factors += factor;
      }
      LinearMath.vec3.scale(position, 1/factors);
      LinearMath.vec3.scale(orientation, 1/factors);
      BananaBread.forceCamera(position, orientation);
      Module.requestAnimationFrame(moveCamera);
    }
    Module.fullscreenCallbacks.push(moveCamera);
  }
}

// Load scripts

(function() {
  function loadChildScript(name, then) {
    var js = document.createElement('script');
    if (then) js.onload = then;
    js.src = name;
    document.body.appendChild(js);
  }

  var urlParts = pageParams.split(',');
  var setup = urlParts[0], preload = urlParts[1];

  var levelTitleContainer = document.querySelector('.level-title span');
  if (levelTitleContainer) {
    var levelTitle;
    switch(setup) {
      case 'low':    levelTitle = 'Arena';        break;
      case 'medium': levelTitle = 'Two Towers';   break;
      case 'high':   levelTitle = 'Lava Chamber'; break;
      case 'four':   levelTitle = 'Future';       break;
      case 'five':   levelTitle = 'Lava Rooms';   break;
      default: throw('unknown setup: ' + setup);
    };
    levelTitleContainer.innerHTML = levelTitle;
  }

  var previewContainer = document.querySelector('.preview-content.' + setup );
  if (previewContainer) previewContainer.classList.add('show');

  if(!Module.failed){
    loadChildScript('game/gl-matrix.js', function() {
      loadChildScript('game/setup_' + setup + '.js', function() {
        loadChildScript('game/preload_base.js', function() {
          loadChildScript('game/preload_character.js', function() {
            loadChildScript('game/preload_' + preload + '.js', function() {
              var scriptParts = ['bb'];
              if (checkPageParam('debug')) scriptParts.push('debug');
              loadChildScript(scriptParts.join('.') + '.js');
            });
          });
        });
      });
    });
  }
})();

(function(){
  var lowResButton = document.querySelector('.fullscreen-button.low-res');
  if (lowResButton) lowResButton.addEventListener('click', function(e){
    Module.fullscreenLow();
  }, false);
  var highResButton = document.querySelector('.fullscreen-button.high-res');
  if (highResButton) highResButton.addEventListener('click', function(e){
    Module.fullscreenHigh();
  }, false);
  var resumeButton = document.querySelector('.fullscreen-button.resume');
  if (resumeButton) resumeButton.addEventListener('click', function(e){
    Module.resume();
  }, false);
  var quitButton = document.querySelector('.fullscreen-button.quit');
  if (quitButton) quitButton.addEventListener('click', function(e){
    window.location = 'index.html';
  }, false);
})();

//

if (shell) window.runEventLoop();

