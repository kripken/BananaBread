
var Module;

if (typeof Module === 'undefined') Module = eval('(function() { try { return Module || {} } catch(e) { return {} } })()');

if (!Module.expectedDataFileDownloads) {
  Module.expectedDataFileDownloads = 0;
  Module.finishedDataFileDownloads = 0;
}
Module.expectedDataFileDownloads++;
(function() {
 var loadPackage = function(metadata) {

    var PACKAGE_PATH;
    if (typeof window === 'object') {
      PACKAGE_PATH = window['encodeURIComponent'](window.location.pathname.toString().substring(0, window.location.pathname.toString().lastIndexOf('/')) + '/');
    } else if (typeof location !== 'undefined') {
      // worker
      PACKAGE_PATH = encodeURIComponent(location.pathname.toString().substring(0, location.pathname.toString().lastIndexOf('/')) + '/');
    } else {
      throw 'using preloaded data can only be done on a web page or in a web worker';
    }
    var PACKAGE_NAME = 'base.data';
    var REMOTE_PACKAGE_BASE = 'base.data';
    if (typeof Module['locateFilePackage'] === 'function' && !Module['locateFile']) {
      Module['locateFile'] = Module['locateFilePackage'];
      Module.printErr('warning: you defined Module.locateFilePackage, that has been renamed to Module.locateFile (using your locateFilePackage for now)');
    }
    var REMOTE_PACKAGE_NAME = typeof Module['locateFile'] === 'function' ?
                              Module['locateFile'](REMOTE_PACKAGE_BASE) :
                              ((Module['filePackagePrefixURL'] || '') + REMOTE_PACKAGE_BASE);
  
    var REMOTE_PACKAGE_SIZE = metadata.remote_package_size;
    var PACKAGE_UUID = metadata.package_uuid;
  
    function fetchRemotePackage(packageName, packageSize, callback, errback) {
      var xhr = new XMLHttpRequest();
      xhr.open('GET', packageName, true);
      xhr.responseType = 'arraybuffer';
      xhr.onprogress = function(event) {
        var url = packageName;
        var size = packageSize;
        if (event.total) size = event.total;
        if (event.loaded) {
          if (!xhr.addedTotal) {
            xhr.addedTotal = true;
            if (!Module.dataFileDownloads) Module.dataFileDownloads = {};
            Module.dataFileDownloads[url] = {
              loaded: event.loaded,
              total: size
            };
          } else {
            Module.dataFileDownloads[url].loaded = event.loaded;
          }
          var total = 0;
          var loaded = 0;
          var num = 0;
          for (var download in Module.dataFileDownloads) {
          var data = Module.dataFileDownloads[download];
            total += data.total;
            loaded += data.loaded;
            num++;
          }
          total = Math.ceil(total * Module.expectedDataFileDownloads/num);
          if (Module['setStatus']) Module['setStatus']('Downloading data... (' + loaded + '/' + total + ')');
        } else if (!Module.dataFileDownloads) {
          if (Module['setStatus']) Module['setStatus']('Downloading data...');
        }
      };
      xhr.onerror = function(event) {
        throw new Error("NetworkError for: " + packageName);
      }
      xhr.onload = function(event) {
        if (xhr.status == 200 || xhr.status == 304 || xhr.status == 206 || (xhr.status == 0 && xhr.response)) { // file URLs can return 0
          var packageData = xhr.response;
          callback(packageData);
        } else {
          throw new Error(xhr.statusText + " : " + xhr.responseURL);
        }
      };
      xhr.send(null);
    };

    function handleError(error) {
      console.error('package error:', error);
    };
  
      var fetched = null, fetchedCallback = null;
      fetchRemotePackage(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE, function(data) {
        if (fetchedCallback) {
          fetchedCallback(data);
          fetchedCallback = null;
        } else {
          fetched = data;
        }
      }, handleError);
    
  function runWithFS() {

    function assert(check, msg) {
      if (!check) throw msg + new Error().stack;
    }
Module['FS_createPath']('/', 'data', true, true);
Module['FS_createPath']('/', 'packages', true, true);
Module['FS_createPath']('/packages', 'textures', true, true);
Module['FS_createPath']('/packages', 'fonts', true, true);
Module['FS_createPath']('/packages', 'icons', true, true);
Module['FS_createPath']('/packages', 'particles', true, true);
Module['FS_createPath']('/packages', 'sounds', true, true);
Module['FS_createPath']('/packages/sounds', 'aard', true, true);
Module['FS_createPath']('/packages/sounds', 'q009', true, true);
Module['FS_createPath']('/packages/sounds', 'yo_frankie', true, true);
Module['FS_createPath']('/packages', 'gk', true, true);
Module['FS_createPath']('/packages/gk', 'lava', true, true);
Module['FS_createPath']('/packages', 'caustics', true, true);
Module['FS_createPath']('/packages', 'models', true, true);
Module['FS_createPath']('/packages/models', 'debris', true, true);
Module['FS_createPath']('/packages/models', 'projectiles', true, true);
Module['FS_createPath']('/packages/models/projectiles', 'grenade', true, true);
Module['FS_createPath']('/packages/models/projectiles', 'rocket', true, true);
Module['FS_createPath']('/packages', 'brushes', true, true);
Module['FS_createPath']('/packages', 'hud', true, true);

    function DataRequest(start, end, crunched, audio) {
      this.start = start;
      this.end = end;
      this.crunched = crunched;
      this.audio = audio;
    }
    DataRequest.prototype = {
      requests: {},
      open: function(mode, name) {
        this.name = name;
        this.requests[name] = this;
        Module['addRunDependency']('fp ' + this.name);
      },
      send: function() {},
      onload: function() {
        var byteArray = this.byteArray.subarray(this.start, this.end);

          this.finish(byteArray);

      },
      finish: function(byteArray) {
        var that = this;

        Module['FS_createPreloadedFile'](this.name, null, byteArray, true, true, function() {
          Module['removeRunDependency']('fp ' + that.name);
        }, function() {
          if (that.audio) {
            Module['removeRunDependency']('fp ' + that.name); // workaround for chromium bug 124926 (still no audio with this, but at least we don't hang)
          } else {
            Module.printErr('Preloading file ' + that.name + ' failed');
          }
        }, false, true); // canOwn this data in the filesystem, it is a slide into the heap that will never change

        this.requests[this.name] = null;
      }
    };

        var files = metadata.files;
        for (i = 0; i < files.length; ++i) {
          new DataRequest(files[i].start, files[i].end, files[i].crunched, files[i].audio).open('GET', files[i].filename);
        }

  
    function processPackageData(arrayBuffer) {
      Module.finishedDataFileDownloads++;
      assert(arrayBuffer, 'Loading data file failed.');
      assert(arrayBuffer instanceof ArrayBuffer, 'bad input to processPackageData');
      var byteArray = new Uint8Array(arrayBuffer);
      var curr;
      
        // copy the entire loaded file into a spot in the heap. Files will refer to slices in that. They cannot be freed though
        // (we may be allocating before malloc is ready, during startup).
        if (Module['SPLIT_MEMORY']) Module.printErr('warning: you should run the file packager with --no-heap-copy when SPLIT_MEMORY is used, otherwise copying into the heap may fail due to the splitting');
        var ptr = Module['getMemory'](byteArray.length);
        Module['HEAPU8'].set(byteArray, ptr);
        DataRequest.prototype.byteArray = Module['HEAPU8'].subarray(ptr, ptr+byteArray.length);
  
          var files = metadata.files;
          for (i = 0; i < files.length; ++i) {
            DataRequest.prototype.requests[files[i].filename].onload();
          }
              Module['removeRunDependency']('datafile_base.data');

    };
    Module['addRunDependency']('datafile_base.data');
  
    if (!Module.preloadResults) Module.preloadResults = {};
  
      Module.preloadResults[PACKAGE_NAME] = {fromCache: false};
      if (fetched) {
        processPackageData(fetched);
        fetched = null;
      } else {
        fetchedCallback = processPackageData;
      }
    
  }
  if (Module['calledRun']) {
    runWithFS();
  } else {
    if (!Module['preRun']) Module['preRun'] = [];
    Module["preRun"].push(runWithFS); // FS is not initialized yet, wait for it
  }

 }
 loadPackage({"files": [{"audio": 0, "start": 0, "crunched": 0, "end": 84487, "filename": "/data/glsl.cfg"}, {"audio": 0, "start": 84487, "crunched": 0, "end": 84623, "filename": "/data/default_map_models.cfg"}, {"audio": 0, "start": 84623, "crunched": 0, "end": 89507, "filename": "/data/guioverlay.png"}, {"audio": 0, "start": 89507, "crunched": 0, "end": 107069, "filename": "/data/background.png"}, {"audio": 0, "start": 107069, "crunched": 0, "end": 154919, "filename": "/data/menus.cfg"}, {"audio": 0, "start": 154919, "crunched": 0, "end": 244549, "filename": "/data/stdshader.cfg"}, {"audio": 0, "start": 244549, "crunched": 0, "end": 247832, "filename": "/data/crosshair.png"}, {"audio": 0, "start": 247832, "crunched": 0, "end": 252716, "filename": "/data/mapshot_frame.png"}, {"audio": 0, "start": 252716, "crunched": 0, "end": 266268, "filename": "/data/background_decal.png"}, {"audio": 0, "start": 266268, "crunched": 0, "end": 269949, "filename": "/data/loading_frame.png"}, {"audio": 0, "start": 269949, "crunched": 0, "end": 275466, "filename": "/data/brush.cfg"}, {"audio": 0, "start": 275466, "crunched": 0, "end": 275625, "filename": "/data/background_detail.png"}, {"audio": 0, "start": 275625, "crunched": 0, "end": 278536, "filename": "/data/sounds.cfg"}, {"audio": 0, "start": 278536, "crunched": 0, "end": 279549, "filename": "/data/stdlib.cfg"}, {"audio": 0, "start": 279549, "crunched": 0, "end": 282861, "filename": "/data/teammate.png"}, {"audio": 0, "start": 282861, "crunched": 0, "end": 287107, "filename": "/data/guiskin.png"}, {"audio": 0, "start": 287107, "crunched": 0, "end": 290873, "filename": "/data/game_fps.cfg"}, {"audio": 0, "start": 290873, "crunched": 0, "end": 421079, "filename": "/data/logo.png"}, {"audio": 0, "start": 421079, "crunched": 0, "end": 429583, "filename": "/data/stdedit.cfg"}, {"audio": 0, "start": 429583, "crunched": 0, "end": 437748, "filename": "/data/game_rpg.cfg"}, {"audio": 0, "start": 437748, "crunched": 0, "end": 440155, "filename": "/data/keymap.cfg"}, {"audio": 0, "start": 440155, "crunched": 0, "end": 444086, "filename": "/data/guicursor.png"}, {"audio": 0, "start": 444086, "crunched": 0, "end": 447369, "filename": "/data/hit.png"}, {"audio": 0, "start": 447369, "crunched": 0, "end": 450352, "filename": "/data/loading_bar.png"}, {"audio": 0, "start": 450352, "crunched": 0, "end": 457569, "filename": "/data/defaults.cfg"}, {"audio": 0, "start": 457569, "crunched": 0, "end": 457641, "filename": "/data/font.cfg"}, {"audio": 0, "start": 457641, "crunched": 0, "end": 460473, "filename": "/data/guislider.png"}, {"audio": 0, "start": 460473, "crunched": 0, "end": 461695, "filename": "/data/default_map_settings.cfg"}, {"audio": 0, "start": 461695, "crunched": 0, "end": 811518, "filename": "/packages/textures/watern.jpg"}, {"audio": 0, "start": 811518, "crunched": 0, "end": 967521, "filename": "/packages/textures/water.jpg"}, {"audio": 0, "start": 967521, "crunched": 0, "end": 1209691, "filename": "/packages/textures/waterfalldudv.jpg"}, {"audio": 0, "start": 1209691, "crunched": 0, "end": 1212727, "filename": "/packages/textures/notexture.png"}, {"audio": 0, "start": 1212727, "crunched": 0, "end": 1467900, "filename": "/packages/textures/waterdudv.jpg"}, {"audio": 0, "start": 1467900, "crunched": 0, "end": 1505094, "filename": "/packages/textures/waterfall.jpg"}, {"audio": 0, "start": 1505094, "crunched": 0, "end": 1682656, "filename": "/packages/textures/waterfalln.jpg"}, {"audio": 0, "start": 1682656, "crunched": 0, "end": 1683327, "filename": "/packages/textures/readme.txt"}, {"audio": 0, "start": 1683327, "crunched": 0, "end": 1769451, "filename": "/packages/fonts/font.png"}, {"audio": 0, "start": 1769451, "crunched": 0, "end": 1774176, "filename": "/packages/fonts/font_readme.txt"}, {"audio": 0, "start": 1774176, "crunched": 0, "end": 1776418, "filename": "/packages/fonts/default.cfg"}, {"audio": 0, "start": 1776418, "crunched": 0, "end": 1791552, "filename": "/packages/icons/frankie.jpg"}, {"audio": 0, "start": 1791552, "crunched": 0, "end": 1804447, "filename": "/packages/icons/cube.jpg"}, {"audio": 0, "start": 1804447, "crunched": 0, "end": 1822439, "filename": "/packages/icons/menu.jpg"}, {"audio": 0, "start": 1822439, "crunched": 0, "end": 1835935, "filename": "/packages/icons/snoutx10k.jpg"}, {"audio": 0, "start": 1835935, "crunched": 0, "end": 1854611, "filename": "/packages/icons/server.jpg"}, {"audio": 0, "start": 1854611, "crunched": 0, "end": 1867891, "filename": "/packages/icons/radio_on.jpg"}, {"audio": 0, "start": 1867891, "crunched": 0, "end": 1880948, "filename": "/packages/icons/exit.jpg"}, {"audio": 0, "start": 1880948, "crunched": 0, "end": 1899676, "filename": "/packages/icons/radio_off.jpg"}, {"audio": 0, "start": 1899676, "crunched": 0, "end": 1903765, "filename": "/packages/icons/menu.png"}, {"audio": 0, "start": 1903765, "crunched": 0, "end": 1917143, "filename": "/packages/icons/info.jpg"}, {"audio": 0, "start": 1917143, "crunched": 0, "end": 1935450, "filename": "/packages/icons/action.jpg"}, {"audio": 0, "start": 1935450, "crunched": 0, "end": 1948954, "filename": "/packages/icons/hand.jpg"}, {"audio": 0, "start": 1948954, "crunched": 0, "end": 1949051, "filename": "/packages/icons/readme.txt"}, {"audio": 0, "start": 1949051, "crunched": 0, "end": 1962119, "filename": "/packages/icons/chat.jpg"}, {"audio": 0, "start": 1962119, "crunched": 0, "end": 1973781, "filename": "/packages/icons/arrow_bw.jpg"}, {"audio": 0, "start": 1973781, "crunched": 0, "end": 1991984, "filename": "/packages/icons/checkbox_on.jpg"}, {"audio": 0, "start": 1991984, "crunched": 0, "end": 2004094, "filename": "/packages/icons/arrow_fw.jpg"}, {"audio": 0, "start": 2004094, "crunched": 0, "end": 2020534, "filename": "/packages/icons/checkbox_off.jpg"}, {"audio": 0, "start": 2020534, "crunched": 0, "end": 2023432, "filename": "/packages/particles/base.png"}, {"audio": 0, "start": 2023432, "crunched": 0, "end": 2349332, "filename": "/packages/particles/lensflares.png"}, {"audio": 0, "start": 2349332, "crunched": 0, "end": 2407194, "filename": "/packages/particles/lightning.jpg"}, {"audio": 0, "start": 2407194, "crunched": 0, "end": 2426216, "filename": "/packages/particles/muzzleflash2.jpg"}, {"audio": 0, "start": 2426216, "crunched": 0, "end": 2483380, "filename": "/packages/particles/bullet.png"}, {"audio": 0, "start": 2483380, "crunched": 0, "end": 2553572, "filename": "/packages/particles/flames.png"}, {"audio": 0, "start": 2553572, "crunched": 0, "end": 2573473, "filename": "/packages/particles/muzzleflash1.jpg"}, {"audio": 0, "start": 2573473, "crunched": 0, "end": 2573718, "filename": "/packages/particles/readme.txt~"}, {"audio": 0, "start": 2573718, "crunched": 0, "end": 2589344, "filename": "/packages/particles/blood.png"}, {"audio": 0, "start": 2589344, "crunched": 0, "end": 2608849, "filename": "/packages/particles/circle.png"}, {"audio": 0, "start": 2608849, "crunched": 0, "end": 2648685, "filename": "/packages/particles/scorch.png"}, {"audio": 0, "start": 2648685, "crunched": 0, "end": 2710837, "filename": "/packages/particles/ball2.png"}, {"audio": 0, "start": 2710837, "crunched": 0, "end": 2713104, "filename": "/packages/particles/blob.png"}, {"audio": 0, "start": 2713104, "crunched": 0, "end": 2767036, "filename": "/packages/particles/ball1.png"}, {"audio": 0, "start": 2767036, "crunched": 0, "end": 2787174, "filename": "/packages/particles/muzzleflash3.jpg"}, {"audio": 0, "start": 2787174, "crunched": 0, "end": 2791686, "filename": "/packages/particles/smoke.png"}, {"audio": 0, "start": 2791686, "crunched": 0, "end": 2793491, "filename": "/packages/particles/spark.png"}, {"audio": 0, "start": 2793491, "crunched": 0, "end": 2793735, "filename": "/packages/particles/readme.txt"}, {"audio": 0, "start": 2793735, "crunched": 0, "end": 3527214, "filename": "/packages/particles/explosion.png"}, {"audio": 0, "start": 3527214, "crunched": 0, "end": 3534629, "filename": "/packages/particles/steam.png"}, {"audio": 0, "start": 3534629, "crunched": 0, "end": 3535490, "filename": "/packages/particles/flare.jpg"}, {"audio": 1, "start": 3535490, "crunched": 0, "end": 3543450, "filename": "/packages/sounds/aard/pain5.wav"}, {"audio": 1, "start": 3543450, "crunched": 0, "end": 3554102, "filename": "/packages/sounds/aard/die2.wav"}, {"audio": 1, "start": 3554102, "crunched": 0, "end": 3563452, "filename": "/packages/sounds/aard/pain3.wav"}, {"audio": 1, "start": 3563452, "crunched": 0, "end": 3574814, "filename": "/packages/sounds/aard/land.wav"}, {"audio": 1, "start": 3574814, "crunched": 0, "end": 3584224, "filename": "/packages/sounds/aard/pain2.wav"}, {"audio": 1, "start": 3584224, "crunched": 0, "end": 3587918, "filename": "/packages/sounds/aard/grunt2.wav"}, {"audio": 1, "start": 3587918, "crunched": 0, "end": 3591976, "filename": "/packages/sounds/aard/outofammo.wav"}, {"audio": 1, "start": 3591976, "crunched": 0, "end": 3599956, "filename": "/packages/sounds/aard/pain4.wav"}, {"audio": 1, "start": 3599956, "crunched": 0, "end": 3606602, "filename": "/packages/sounds/aard/weapload.wav"}, {"audio": 1, "start": 3606602, "crunched": 0, "end": 3614268, "filename": "/packages/sounds/aard/pain6.wav"}, {"audio": 1, "start": 3614268, "crunched": 0, "end": 3618400, "filename": "/packages/sounds/aard/jump.wav"}, {"audio": 1, "start": 3618400, "crunched": 0, "end": 3628114, "filename": "/packages/sounds/aard/die1.wav"}, {"audio": 1, "start": 3628114, "crunched": 0, "end": 3640428, "filename": "/packages/sounds/aard/itempick.wav"}, {"audio": 1, "start": 3640428, "crunched": 0, "end": 3665898, "filename": "/packages/sounds/aard/pain1.wav"}, {"audio": 1, "start": 3665898, "crunched": 0, "end": 3677760, "filename": "/packages/sounds/aard/bang.wav"}, {"audio": 1, "start": 3677760, "crunched": 0, "end": 3689166, "filename": "/packages/sounds/aard/grunt1.wav"}, {"audio": 1, "start": 3689166, "crunched": 0, "end": 3690870, "filename": "/packages/sounds/aard/tak.wav"}, {"audio": 1, "start": 3690870, "crunched": 0, "end": 3819907, "filename": "/packages/sounds/q009/rifle.ogg"}, {"audio": 1, "start": 3819907, "crunched": 0, "end": 3922973, "filename": "/packages/sounds/q009/ren2.ogg"}, {"audio": 1, "start": 3922973, "crunched": 0, "end": 3951367, "filename": "/packages/sounds/q009/pistol.ogg"}, {"audio": 1, "start": 3951367, "crunched": 0, "end": 4010066, "filename": "/packages/sounds/q009/rlauncher2.ogg"}, {"audio": 1, "start": 4010066, "crunched": 0, "end": 4038448, "filename": "/packages/sounds/q009/pistol2.ogg"}, {"audio": 1, "start": 4038448, "crunched": 0, "end": 4154887, "filename": "/packages/sounds/q009/ren3.ogg"}, {"audio": 1, "start": 4154887, "crunched": 0, "end": 4277570, "filename": "/packages/sounds/q009/rifle3.ogg"}, {"audio": 1, "start": 4277570, "crunched": 0, "end": 4303743, "filename": "/packages/sounds/q009/teleport.ogg"}, {"audio": 0, "start": 4303743, "crunched": 0, "end": 4323183, "filename": "/packages/sounds/q009/license.txt"}, {"audio": 1, "start": 4323183, "crunched": 0, "end": 4350891, "filename": "/packages/sounds/q009/quaddamage_shoot.ogg"}, {"audio": 1, "start": 4350891, "crunched": 0, "end": 4383513, "filename": "/packages/sounds/q009/quaddamage_out.ogg"}, {"audio": 1, "start": 4383513, "crunched": 0, "end": 4441158, "filename": "/packages/sounds/q009/rlauncher3.ogg"}, {"audio": 1, "start": 4441158, "crunched": 0, "end": 4469045, "filename": "/packages/sounds/q009/minigun.ogg"}, {"audio": 1, "start": 4469045, "crunched": 0, "end": 4593443, "filename": "/packages/sounds/q009/shotgun3.ogg"}, {"audio": 1, "start": 4593443, "crunched": 0, "end": 4628885, "filename": "/packages/sounds/q009/glauncher2.ogg"}, {"audio": 1, "start": 4628885, "crunched": 0, "end": 4686822, "filename": "/packages/sounds/q009/rlauncher.ogg"}, {"audio": 1, "start": 4686822, "crunched": 0, "end": 4704697, "filename": "/packages/sounds/q009/outofammo.ogg"}, {"audio": 1, "start": 4704697, "crunched": 0, "end": 4730953, "filename": "/packages/sounds/q009/minigun3.ogg"}, {"audio": 1, "start": 4730953, "crunched": 0, "end": 4764181, "filename": "/packages/sounds/q009/glauncher3.ogg"}, {"audio": 1, "start": 4764181, "crunched": 0, "end": 4889261, "filename": "/packages/sounds/q009/shotgun.ogg"}, {"audio": 1, "start": 4889261, "crunched": 0, "end": 4912589, "filename": "/packages/sounds/q009/minigun2.ogg"}, {"audio": 1, "start": 4912589, "crunched": 0, "end": 5038691, "filename": "/packages/sounds/q009/shotgun2.ogg"}, {"audio": 1, "start": 5038691, "crunched": 0, "end": 5072378, "filename": "/packages/sounds/q009/glauncher.ogg"}, {"audio": 1, "start": 5072378, "crunched": 0, "end": 5092821, "filename": "/packages/sounds/q009/weapswitch.ogg"}, {"audio": 1, "start": 5092821, "crunched": 0, "end": 5111712, "filename": "/packages/sounds/q009/jumppad.ogg"}, {"audio": 1, "start": 5111712, "crunched": 0, "end": 5245498, "filename": "/packages/sounds/q009/ren.ogg"}, {"audio": 1, "start": 5245498, "crunched": 0, "end": 5275480, "filename": "/packages/sounds/q009/explosion.ogg"}, {"audio": 1, "start": 5275480, "crunched": 0, "end": 5399700, "filename": "/packages/sounds/q009/rifle2.ogg"}, {"audio": 0, "start": 5399700, "crunched": 0, "end": 5401016, "filename": "/packages/sounds/q009/readme.txt"}, {"audio": 1, "start": 5401016, "crunched": 0, "end": 5427916, "filename": "/packages/sounds/q009/pistol3.ogg"}, {"audio": 1, "start": 5427916, "crunched": 0, "end": 5447525, "filename": "/packages/sounds/yo_frankie/amb_waterdrip_2.ogg"}, {"audio": 1, "start": 5447525, "crunched": 0, "end": 5454938, "filename": "/packages/sounds/yo_frankie/sfx_interact.ogg"}, {"audio": 1, "start": 5454938, "crunched": 0, "end": 5478843, "filename": "/packages/sounds/yo_frankie/watersplash2.ogg"}, {"audio": 0, "start": 5478843, "crunched": 0, "end": 5479473, "filename": "/packages/sounds/yo_frankie/readme.txt"}, {"audio": 0, "start": 5479473, "crunched": 0, "end": 5654377, "filename": "/packages/gk/lava/lava_cc.dds"}, {"audio": 0, "start": 5654377, "crunched": 0, "end": 6004057, "filename": "/packages/gk/lava/lava_nm.dds"}, {"audio": 0, "start": 6004057, "crunched": 0, "end": 6028536, "filename": "/packages/caustics/caust17.png"}, {"audio": 0, "start": 6028536, "crunched": 0, "end": 6053055, "filename": "/packages/caustics/caust00.png"}, {"audio": 0, "start": 6053055, "crunched": 0, "end": 6077504, "filename": "/packages/caustics/caust15.png"}, {"audio": 0, "start": 6077504, "crunched": 0, "end": 6101079, "filename": "/packages/caustics/caust03.png"}, {"audio": 0, "start": 6101079, "crunched": 0, "end": 6125333, "filename": "/packages/caustics/caust30.png"}, {"audio": 0, "start": 6125333, "crunched": 0, "end": 6149449, "filename": "/packages/caustics/caust02.png"}, {"audio": 0, "start": 6149449, "crunched": 0, "end": 6173806, "filename": "/packages/caustics/caust16.png"}, {"audio": 0, "start": 6173806, "crunched": 0, "end": 6198298, "filename": "/packages/caustics/caust01.png"}, {"audio": 0, "start": 6198298, "crunched": 0, "end": 6223484, "filename": "/packages/caustics/caust13.png"}, {"audio": 0, "start": 6223484, "crunched": 0, "end": 6247648, "filename": "/packages/caustics/caust11.png"}, {"audio": 0, "start": 6247648, "crunched": 0, "end": 6272189, "filename": "/packages/caustics/caust18.png"}, {"audio": 0, "start": 6272189, "crunched": 0, "end": 6295395, "filename": "/packages/caustics/caust25.png"}, {"audio": 0, "start": 6295395, "crunched": 0, "end": 6320136, "filename": "/packages/caustics/caust12.png"}, {"audio": 0, "start": 6320136, "crunched": 0, "end": 6344003, "filename": "/packages/caustics/caust07.png"}, {"audio": 0, "start": 6344003, "crunched": 0, "end": 6368546, "filename": "/packages/caustics/caust31.png"}, {"audio": 0, "start": 6368546, "crunched": 0, "end": 6392725, "filename": "/packages/caustics/caust19.png"}, {"audio": 0, "start": 6392725, "crunched": 0, "end": 6416369, "filename": "/packages/caustics/caust27.png"}, {"audio": 0, "start": 6416369, "crunched": 0, "end": 6439538, "filename": "/packages/caustics/caust24.png"}, {"audio": 0, "start": 6439538, "crunched": 0, "end": 6462982, "filename": "/packages/caustics/caust22.png"}, {"audio": 0, "start": 6462982, "crunched": 0, "end": 6487144, "filename": "/packages/caustics/caust08.png"}, {"audio": 0, "start": 6487144, "crunched": 0, "end": 6510782, "filename": "/packages/caustics/caust21.png"}, {"audio": 0, "start": 6510782, "crunched": 0, "end": 6535834, "filename": "/packages/caustics/caust14.png"}, {"audio": 0, "start": 6535834, "crunched": 0, "end": 6559109, "filename": "/packages/caustics/caust23.png"}, {"audio": 0, "start": 6559109, "crunched": 0, "end": 6582610, "filename": "/packages/caustics/caust28.png"}, {"audio": 0, "start": 6582610, "crunched": 0, "end": 6605480, "filename": "/packages/caustics/caust05.png"}, {"audio": 0, "start": 6605480, "crunched": 0, "end": 6629034, "filename": "/packages/caustics/caust26.png"}, {"audio": 0, "start": 6629034, "crunched": 0, "end": 6652917, "filename": "/packages/caustics/caust09.png"}, {"audio": 0, "start": 6652917, "crunched": 0, "end": 6676115, "filename": "/packages/caustics/caust04.png"}, {"audio": 0, "start": 6676115, "crunched": 0, "end": 6699865, "filename": "/packages/caustics/caust29.png"}, {"audio": 0, "start": 6699865, "crunched": 0, "end": 6723189, "filename": "/packages/caustics/caust06.png"}, {"audio": 0, "start": 6723189, "crunched": 0, "end": 6747295, "filename": "/packages/caustics/caust20.png"}, {"audio": 0, "start": 6747295, "crunched": 0, "end": 6747353, "filename": "/packages/caustics/readme.txt"}, {"audio": 0, "start": 6747353, "crunched": 0, "end": 6771178, "filename": "/packages/caustics/caust10.png"}, {"audio": 0, "start": 6771178, "crunched": 0, "end": 6771421, "filename": "/packages/models/debris/md2.cfg"}, {"audio": 0, "start": 6771421, "crunched": 0, "end": 6786197, "filename": "/packages/models/debris/tris.md2"}, {"audio": 0, "start": 6786197, "crunched": 0, "end": 6978023, "filename": "/packages/models/debris/skin.png"}, {"audio": 0, "start": 6978023, "crunched": 0, "end": 6978161, "filename": "/packages/models/projectiles/grenade/iqm.cfg"}, {"audio": 0, "start": 6978161, "crunched": 0, "end": 6985880, "filename": "/packages/models/projectiles/rocket/normal.jpg"}, {"audio": 0, "start": 6985880, "crunched": 0, "end": 6986036, "filename": "/packages/models/projectiles/rocket/iqm.cfg"}, {"audio": 0, "start": 6986036, "crunched": 0, "end": 7006804, "filename": "/packages/models/projectiles/rocket/mask.jpg"}, {"audio": 0, "start": 7006804, "crunched": 0, "end": 7020041, "filename": "/packages/models/projectiles/rocket/skin.jpg"}, {"audio": 0, "start": 7020041, "crunched": 0, "end": 7023177, "filename": "/packages/models/projectiles/rocket/rocket.iqm"}, {"audio": 0, "start": 7023177, "crunched": 0, "end": 7023837, "filename": "/packages/models/projectiles/rocket/readme.txt"}, {"audio": 0, "start": 7023837, "crunched": 0, "end": 7023974, "filename": "/packages/brushes/gradient_128.png"}, {"audio": 0, "start": 7023974, "crunched": 0, "end": 7025181, "filename": "/packages/brushes/square_64_hard.png"}, {"audio": 0, "start": 7025181, "crunched": 0, "end": 7027545, "filename": "/packages/brushes/circle_128_solid.png"}, {"audio": 0, "start": 7027545, "crunched": 0, "end": 7027665, "filename": "/packages/brushes/gradient_32.png"}, {"audio": 0, "start": 7027665, "crunched": 0, "end": 7028756, "filename": "/packages/brushes/circle_16_soft.png"}, {"audio": 0, "start": 7028756, "crunched": 0, "end": 7029994, "filename": "/packages/brushes/circle_32_solid.png"}, {"audio": 0, "start": 7029994, "crunched": 0, "end": 7030123, "filename": "/packages/brushes/gradient_64.png"}, {"audio": 0, "start": 7030123, "crunched": 0, "end": 7033683, "filename": "/packages/brushes/circle_32_hard.png"}, {"audio": 0, "start": 7033683, "crunched": 0, "end": 7034664, "filename": "/packages/brushes/square_32_solid.png"}, {"audio": 0, "start": 7034664, "crunched": 0, "end": 7035656, "filename": "/packages/brushes/circle_8_soft.png"}, {"audio": 0, "start": 7035656, "crunched": 0, "end": 7035759, "filename": "/packages/brushes/gradient_16.png"}, {"audio": 0, "start": 7035759, "crunched": 0, "end": 7036881, "filename": "/packages/brushes/circle_16_hard.png"}, {"audio": 0, "start": 7036881, "crunched": 0, "end": 7038064, "filename": "/packages/brushes/square_32_hard.png"}, {"audio": 0, "start": 7038064, "crunched": 0, "end": 7039177, "filename": "/packages/brushes/circle_16_solid.png"}, {"audio": 0, "start": 7039177, "crunched": 0, "end": 7043509, "filename": "/packages/brushes/circle_64_hard.png"}, {"audio": 0, "start": 7043509, "crunched": 0, "end": 7046986, "filename": "/packages/brushes/circle_128_soft.png"}, {"audio": 0, "start": 7046986, "crunched": 0, "end": 7047959, "filename": "/packages/brushes/square_16_solid.png"}, {"audio": 0, "start": 7047959, "crunched": 0, "end": 7057594, "filename": "/packages/brushes/noise_128.png"}, {"audio": 0, "start": 7057594, "crunched": 0, "end": 7058879, "filename": "/packages/brushes/circle_32_soft.png"}, {"audio": 0, "start": 7058879, "crunched": 0, "end": 7062967, "filename": "/packages/brushes/circle_128_hard.png"}, {"audio": 0, "start": 7062967, "crunched": 0, "end": 7065257, "filename": "/packages/brushes/noise_64.png"}, {"audio": 0, "start": 7065257, "crunched": 0, "end": 7066841, "filename": "/packages/brushes/circle_64_solid.png"}, {"audio": 0, "start": 7066841, "crunched": 0, "end": 7067923, "filename": "/packages/brushes/square_16_hard.png"}, {"audio": 0, "start": 7067923, "crunched": 0, "end": 7068918, "filename": "/packages/brushes/circle_8_solid.png"}, {"audio": 0, "start": 7068918, "crunched": 0, "end": 7069914, "filename": "/packages/brushes/circle_8_hard.png"}, {"audio": 0, "start": 7069914, "crunched": 0, "end": 7069973, "filename": "/packages/brushes/readme.txt"}, {"audio": 0, "start": 7069973, "crunched": 0, "end": 7071787, "filename": "/packages/brushes/circle_64_soft.png"}, {"audio": 0, "start": 7071787, "crunched": 0, "end": 7072793, "filename": "/packages/brushes/square_64_solid.png"}, {"audio": 0, "start": 7072793, "crunched": 0, "end": 7094114, "filename": "/packages/hud/wasm.png"}, {"audio": 0, "start": 7094114, "crunched": 0, "end": 7237858, "filename": "/packages/hud/damage.png"}, {"audio": 0, "start": 7237858, "crunched": 0, "end": 7343259, "filename": "/packages/hud/items.png"}, {"audio": 0, "start": 7343259, "crunched": 0, "end": 7364564, "filename": "/packages/hud/js.png"}, {"audio": 0, "start": 7364564, "crunched": 0, "end": 7364635, "filename": "/packages/hud/readme.txt"}], "remote_package_size": 7364635, "package_uuid": "561ef076-3df7-42fa-a63c-cba01b2f721e"});

})();

