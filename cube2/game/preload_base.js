
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
 loadPackage({"files": [{"audio": 0, "start": 0, "crunched": 0, "end": 47850, "filename": "/data/menus.cfg"}, {"audio": 0, "start": 47850, "crunched": 0, "end": 52734, "filename": "/data/guioverlay.png"}, {"audio": 0, "start": 52734, "crunched": 0, "end": 66286, "filename": "/data/background_decal.png"}, {"audio": 0, "start": 66286, "crunched": 0, "end": 69197, "filename": "/data/sounds.cfg"}, {"audio": 0, "start": 69197, "crunched": 0, "end": 70419, "filename": "/data/default_map_settings.cfg"}, {"audio": 0, "start": 70419, "crunched": 0, "end": 73702, "filename": "/data/hit.png"}, {"audio": 0, "start": 73702, "crunched": 0, "end": 73838, "filename": "/data/default_map_models.cfg"}, {"audio": 0, "start": 73838, "crunched": 0, "end": 77121, "filename": "/data/crosshair.png"}, {"audio": 0, "start": 77121, "crunched": 0, "end": 80433, "filename": "/data/teammate.png"}, {"audio": 0, "start": 80433, "crunched": 0, "end": 97995, "filename": "/data/background.png"}, {"audio": 0, "start": 97995, "crunched": 0, "end": 187625, "filename": "/data/stdshader.cfg"}, {"audio": 0, "start": 187625, "crunched": 0, "end": 191871, "filename": "/data/guiskin.png"}, {"audio": 0, "start": 191871, "crunched": 0, "end": 322077, "filename": "/data/logo.png"}, {"audio": 0, "start": 322077, "crunched": 0, "end": 324909, "filename": "/data/guislider.png"}, {"audio": 0, "start": 324909, "crunched": 0, "end": 327316, "filename": "/data/keymap.cfg"}, {"audio": 0, "start": 327316, "crunched": 0, "end": 332200, "filename": "/data/mapshot_frame.png"}, {"audio": 0, "start": 332200, "crunched": 0, "end": 336131, "filename": "/data/guicursor.png"}, {"audio": 0, "start": 336131, "crunched": 0, "end": 339114, "filename": "/data/loading_bar.png"}, {"audio": 0, "start": 339114, "crunched": 0, "end": 423601, "filename": "/data/glsl.cfg"}, {"audio": 0, "start": 423601, "crunched": 0, "end": 427282, "filename": "/data/loading_frame.png"}, {"audio": 0, "start": 427282, "crunched": 0, "end": 428295, "filename": "/data/stdlib.cfg"}, {"audio": 0, "start": 428295, "crunched": 0, "end": 432061, "filename": "/data/game_fps.cfg"}, {"audio": 0, "start": 432061, "crunched": 0, "end": 432220, "filename": "/data/background_detail.png"}, {"audio": 0, "start": 432220, "crunched": 0, "end": 440724, "filename": "/data/stdedit.cfg"}, {"audio": 0, "start": 440724, "crunched": 0, "end": 440796, "filename": "/data/font.cfg"}, {"audio": 0, "start": 440796, "crunched": 0, "end": 446313, "filename": "/data/brush.cfg"}, {"audio": 0, "start": 446313, "crunched": 0, "end": 454478, "filename": "/data/game_rpg.cfg"}, {"audio": 0, "start": 454478, "crunched": 0, "end": 461695, "filename": "/data/defaults.cfg"}, {"audio": 0, "start": 461695, "crunched": 0, "end": 617698, "filename": "/packages/textures/water.jpg"}, {"audio": 0, "start": 617698, "crunched": 0, "end": 872871, "filename": "/packages/textures/waterdudv.jpg"}, {"audio": 0, "start": 872871, "crunched": 0, "end": 875907, "filename": "/packages/textures/notexture.png"}, {"audio": 0, "start": 875907, "crunched": 0, "end": 1053469, "filename": "/packages/textures/waterfalln.jpg"}, {"audio": 0, "start": 1053469, "crunched": 0, "end": 1090663, "filename": "/packages/textures/waterfall.jpg"}, {"audio": 0, "start": 1090663, "crunched": 0, "end": 1091334, "filename": "/packages/textures/readme.txt"}, {"audio": 0, "start": 1091334, "crunched": 0, "end": 1333504, "filename": "/packages/textures/waterfalldudv.jpg"}, {"audio": 0, "start": 1333504, "crunched": 0, "end": 1683327, "filename": "/packages/textures/watern.jpg"}, {"audio": 0, "start": 1683327, "crunched": 0, "end": 1769451, "filename": "/packages/fonts/font.png"}, {"audio": 0, "start": 1769451, "crunched": 0, "end": 1771693, "filename": "/packages/fonts/default.cfg"}, {"audio": 0, "start": 1771693, "crunched": 0, "end": 1776418, "filename": "/packages/fonts/font_readme.txt"}, {"audio": 0, "start": 1776418, "crunched": 0, "end": 1789796, "filename": "/packages/icons/info.jpg"}, {"audio": 0, "start": 1789796, "crunched": 0, "end": 1801906, "filename": "/packages/icons/arrow_fw.jpg"}, {"audio": 0, "start": 1801906, "crunched": 0, "end": 1817040, "filename": "/packages/icons/frankie.jpg"}, {"audio": 0, "start": 1817040, "crunched": 0, "end": 1835716, "filename": "/packages/icons/server.jpg"}, {"audio": 0, "start": 1835716, "crunched": 0, "end": 1848996, "filename": "/packages/icons/radio_on.jpg"}, {"audio": 0, "start": 1848996, "crunched": 0, "end": 1867199, "filename": "/packages/icons/checkbox_on.jpg"}, {"audio": 0, "start": 1867199, "crunched": 0, "end": 1880094, "filename": "/packages/icons/cube.jpg"}, {"audio": 0, "start": 1880094, "crunched": 0, "end": 1893151, "filename": "/packages/icons/exit.jpg"}, {"audio": 0, "start": 1893151, "crunched": 0, "end": 1909591, "filename": "/packages/icons/checkbox_off.jpg"}, {"audio": 0, "start": 1909591, "crunched": 0, "end": 1922659, "filename": "/packages/icons/chat.jpg"}, {"audio": 0, "start": 1922659, "crunched": 0, "end": 1926748, "filename": "/packages/icons/menu.png"}, {"audio": 0, "start": 1926748, "crunched": 0, "end": 1926845, "filename": "/packages/icons/readme.txt"}, {"audio": 0, "start": 1926845, "crunched": 0, "end": 1940341, "filename": "/packages/icons/snoutx10k.jpg"}, {"audio": 0, "start": 1940341, "crunched": 0, "end": 1959069, "filename": "/packages/icons/radio_off.jpg"}, {"audio": 0, "start": 1959069, "crunched": 0, "end": 1970731, "filename": "/packages/icons/arrow_bw.jpg"}, {"audio": 0, "start": 1970731, "crunched": 0, "end": 1989038, "filename": "/packages/icons/action.jpg"}, {"audio": 0, "start": 1989038, "crunched": 0, "end": 2007030, "filename": "/packages/icons/menu.jpg"}, {"audio": 0, "start": 2007030, "crunched": 0, "end": 2020534, "filename": "/packages/icons/hand.jpg"}, {"audio": 0, "start": 2020534, "crunched": 0, "end": 2078396, "filename": "/packages/particles/lightning.jpg"}, {"audio": 0, "start": 2078396, "crunched": 0, "end": 2082908, "filename": "/packages/particles/smoke.png"}, {"audio": 0, "start": 2082908, "crunched": 0, "end": 2084713, "filename": "/packages/particles/spark.png"}, {"audio": 0, "start": 2084713, "crunched": 0, "end": 2146865, "filename": "/packages/particles/ball2.png"}, {"audio": 0, "start": 2146865, "crunched": 0, "end": 2166370, "filename": "/packages/particles/circle.png"}, {"audio": 0, "start": 2166370, "crunched": 0, "end": 2185392, "filename": "/packages/particles/muzzleflash2.jpg"}, {"audio": 0, "start": 2185392, "crunched": 0, "end": 2201018, "filename": "/packages/particles/blood.png"}, {"audio": 0, "start": 2201018, "crunched": 0, "end": 2208433, "filename": "/packages/particles/steam.png"}, {"audio": 0, "start": 2208433, "crunched": 0, "end": 2941912, "filename": "/packages/particles/explosion.png"}, {"audio": 0, "start": 2941912, "crunched": 0, "end": 2942156, "filename": "/packages/particles/readme.txt"}, {"audio": 0, "start": 2942156, "crunched": 0, "end": 2945054, "filename": "/packages/particles/base.png"}, {"audio": 0, "start": 2945054, "crunched": 0, "end": 2947321, "filename": "/packages/particles/blob.png"}, {"audio": 0, "start": 2947321, "crunched": 0, "end": 3004485, "filename": "/packages/particles/bullet.png"}, {"audio": 0, "start": 3004485, "crunched": 0, "end": 3024386, "filename": "/packages/particles/muzzleflash1.jpg"}, {"audio": 0, "start": 3024386, "crunched": 0, "end": 3094578, "filename": "/packages/particles/flames.png"}, {"audio": 0, "start": 3094578, "crunched": 0, "end": 3094823, "filename": "/packages/particles/readme.txt~"}, {"audio": 0, "start": 3094823, "crunched": 0, "end": 3095684, "filename": "/packages/particles/flare.jpg"}, {"audio": 0, "start": 3095684, "crunched": 0, "end": 3115822, "filename": "/packages/particles/muzzleflash3.jpg"}, {"audio": 0, "start": 3115822, "crunched": 0, "end": 3441722, "filename": "/packages/particles/lensflares.png"}, {"audio": 0, "start": 3441722, "crunched": 0, "end": 3481558, "filename": "/packages/particles/scorch.png"}, {"audio": 0, "start": 3481558, "crunched": 0, "end": 3535490, "filename": "/packages/particles/ball1.png"}, {"audio": 1, "start": 3535490, "crunched": 0, "end": 3560960, "filename": "/packages/sounds/aard/pain1.wav"}, {"audio": 1, "start": 3560960, "crunched": 0, "end": 3570674, "filename": "/packages/sounds/aard/die1.wav"}, {"audio": 1, "start": 3570674, "crunched": 0, "end": 3582036, "filename": "/packages/sounds/aard/land.wav"}, {"audio": 1, "start": 3582036, "crunched": 0, "end": 3585730, "filename": "/packages/sounds/aard/grunt2.wav"}, {"audio": 1, "start": 3585730, "crunched": 0, "end": 3587434, "filename": "/packages/sounds/aard/tak.wav"}, {"audio": 1, "start": 3587434, "crunched": 0, "end": 3594080, "filename": "/packages/sounds/aard/weapload.wav"}, {"audio": 1, "start": 3594080, "crunched": 0, "end": 3605486, "filename": "/packages/sounds/aard/grunt1.wav"}, {"audio": 1, "start": 3605486, "crunched": 0, "end": 3614836, "filename": "/packages/sounds/aard/pain3.wav"}, {"audio": 1, "start": 3614836, "crunched": 0, "end": 3624246, "filename": "/packages/sounds/aard/pain2.wav"}, {"audio": 1, "start": 3624246, "crunched": 0, "end": 3634898, "filename": "/packages/sounds/aard/die2.wav"}, {"audio": 1, "start": 3634898, "crunched": 0, "end": 3642858, "filename": "/packages/sounds/aard/pain5.wav"}, {"audio": 1, "start": 3642858, "crunched": 0, "end": 3650838, "filename": "/packages/sounds/aard/pain4.wav"}, {"audio": 1, "start": 3650838, "crunched": 0, "end": 3662700, "filename": "/packages/sounds/aard/bang.wav"}, {"audio": 1, "start": 3662700, "crunched": 0, "end": 3675014, "filename": "/packages/sounds/aard/itempick.wav"}, {"audio": 1, "start": 3675014, "crunched": 0, "end": 3682680, "filename": "/packages/sounds/aard/pain6.wav"}, {"audio": 1, "start": 3682680, "crunched": 0, "end": 3686738, "filename": "/packages/sounds/aard/outofammo.wav"}, {"audio": 1, "start": 3686738, "crunched": 0, "end": 3690870, "filename": "/packages/sounds/aard/jump.wav"}, {"audio": 1, "start": 3690870, "crunched": 0, "end": 3718757, "filename": "/packages/sounds/q009/minigun.ogg"}, {"audio": 1, "start": 3718757, "crunched": 0, "end": 3843155, "filename": "/packages/sounds/q009/shotgun3.ogg"}, {"audio": 1, "start": 3843155, "crunched": 0, "end": 3901854, "filename": "/packages/sounds/q009/rlauncher2.ogg"}, {"audio": 1, "start": 3901854, "crunched": 0, "end": 4024537, "filename": "/packages/sounds/q009/rifle3.ogg"}, {"audio": 1, "start": 4024537, "crunched": 0, "end": 4050710, "filename": "/packages/sounds/q009/teleport.ogg"}, {"audio": 1, "start": 4050710, "crunched": 0, "end": 4184496, "filename": "/packages/sounds/q009/ren.ogg"}, {"audio": 1, "start": 4184496, "crunched": 0, "end": 4207824, "filename": "/packages/sounds/q009/minigun2.ogg"}, {"audio": 1, "start": 4207824, "crunched": 0, "end": 4226715, "filename": "/packages/sounds/q009/jumppad.ogg"}, {"audio": 1, "start": 4226715, "crunched": 0, "end": 4260402, "filename": "/packages/sounds/q009/glauncher.ogg"}, {"audio": 1, "start": 4260402, "crunched": 0, "end": 4280845, "filename": "/packages/sounds/q009/weapswitch.ogg"}, {"audio": 1, "start": 4280845, "crunched": 0, "end": 4310827, "filename": "/packages/sounds/q009/explosion.ogg"}, {"audio": 1, "start": 4310827, "crunched": 0, "end": 4435047, "filename": "/packages/sounds/q009/rifle2.ogg"}, {"audio": 1, "start": 4435047, "crunched": 0, "end": 4560127, "filename": "/packages/sounds/q009/shotgun.ogg"}, {"audio": 1, "start": 4560127, "crunched": 0, "end": 4586383, "filename": "/packages/sounds/q009/minigun3.ogg"}, {"audio": 1, "start": 4586383, "crunched": 0, "end": 4689449, "filename": "/packages/sounds/q009/ren2.ogg"}, {"audio": 1, "start": 4689449, "crunched": 0, "end": 4818486, "filename": "/packages/sounds/q009/rifle.ogg"}, {"audio": 1, "start": 4818486, "crunched": 0, "end": 4934925, "filename": "/packages/sounds/q009/ren3.ogg"}, {"audio": 1, "start": 4934925, "crunched": 0, "end": 4992862, "filename": "/packages/sounds/q009/rlauncher.ogg"}, {"audio": 1, "start": 4992862, "crunched": 0, "end": 5025484, "filename": "/packages/sounds/q009/quaddamage_out.ogg"}, {"audio": 1, "start": 5025484, "crunched": 0, "end": 5043359, "filename": "/packages/sounds/q009/outofammo.ogg"}, {"audio": 1, "start": 5043359, "crunched": 0, "end": 5169461, "filename": "/packages/sounds/q009/shotgun2.ogg"}, {"audio": 1, "start": 5169461, "crunched": 0, "end": 5196361, "filename": "/packages/sounds/q009/pistol3.ogg"}, {"audio": 0, "start": 5196361, "crunched": 0, "end": 5215801, "filename": "/packages/sounds/q009/license.txt"}, {"audio": 0, "start": 5215801, "crunched": 0, "end": 5217117, "filename": "/packages/sounds/q009/readme.txt"}, {"audio": 1, "start": 5217117, "crunched": 0, "end": 5244825, "filename": "/packages/sounds/q009/quaddamage_shoot.ogg"}, {"audio": 1, "start": 5244825, "crunched": 0, "end": 5302470, "filename": "/packages/sounds/q009/rlauncher3.ogg"}, {"audio": 1, "start": 5302470, "crunched": 0, "end": 5330852, "filename": "/packages/sounds/q009/pistol2.ogg"}, {"audio": 1, "start": 5330852, "crunched": 0, "end": 5366294, "filename": "/packages/sounds/q009/glauncher2.ogg"}, {"audio": 1, "start": 5366294, "crunched": 0, "end": 5399522, "filename": "/packages/sounds/q009/glauncher3.ogg"}, {"audio": 1, "start": 5399522, "crunched": 0, "end": 5427916, "filename": "/packages/sounds/q009/pistol.ogg"}, {"audio": 1, "start": 5427916, "crunched": 0, "end": 5447525, "filename": "/packages/sounds/yo_frankie/amb_waterdrip_2.ogg"}, {"audio": 0, "start": 5447525, "crunched": 0, "end": 5448155, "filename": "/packages/sounds/yo_frankie/readme.txt"}, {"audio": 1, "start": 5448155, "crunched": 0, "end": 5455568, "filename": "/packages/sounds/yo_frankie/sfx_interact.ogg"}, {"audio": 1, "start": 5455568, "crunched": 0, "end": 5479473, "filename": "/packages/sounds/yo_frankie/watersplash2.ogg"}, {"audio": 0, "start": 5479473, "crunched": 0, "end": 5654377, "filename": "/packages/gk/lava/lava_cc.dds"}, {"audio": 0, "start": 5654377, "crunched": 0, "end": 6004057, "filename": "/packages/gk/lava/lava_nm.dds"}, {"audio": 0, "start": 6004057, "crunched": 0, "end": 6028219, "filename": "/packages/caustics/caust08.png"}, {"audio": 0, "start": 6028219, "crunched": 0, "end": 6052698, "filename": "/packages/caustics/caust17.png"}, {"audio": 0, "start": 6052698, "crunched": 0, "end": 6077884, "filename": "/packages/caustics/caust13.png"}, {"audio": 0, "start": 6077884, "crunched": 0, "end": 6101208, "filename": "/packages/caustics/caust06.png"}, {"audio": 0, "start": 6101208, "crunched": 0, "end": 6124783, "filename": "/packages/caustics/caust03.png"}, {"audio": 0, "start": 6124783, "crunched": 0, "end": 6147653, "filename": "/packages/caustics/caust05.png"}, {"audio": 0, "start": 6147653, "crunched": 0, "end": 6171832, "filename": "/packages/caustics/caust19.png"}, {"audio": 0, "start": 6171832, "crunched": 0, "end": 6195107, "filename": "/packages/caustics/caust23.png"}, {"audio": 0, "start": 6195107, "crunched": 0, "end": 6218276, "filename": "/packages/caustics/caust24.png"}, {"audio": 0, "start": 6218276, "crunched": 0, "end": 6241482, "filename": "/packages/caustics/caust25.png"}, {"audio": 0, "start": 6241482, "crunched": 0, "end": 6264983, "filename": "/packages/caustics/caust28.png"}, {"audio": 0, "start": 6264983, "crunched": 0, "end": 6288537, "filename": "/packages/caustics/caust26.png"}, {"audio": 0, "start": 6288537, "crunched": 0, "end": 6313278, "filename": "/packages/caustics/caust12.png"}, {"audio": 0, "start": 6313278, "crunched": 0, "end": 6336476, "filename": "/packages/caustics/caust04.png"}, {"audio": 0, "start": 6336476, "crunched": 0, "end": 6360343, "filename": "/packages/caustics/caust07.png"}, {"audio": 0, "start": 6360343, "crunched": 0, "end": 6384886, "filename": "/packages/caustics/caust31.png"}, {"audio": 0, "start": 6384886, "crunched": 0, "end": 6409335, "filename": "/packages/caustics/caust15.png"}, {"audio": 0, "start": 6409335, "crunched": 0, "end": 6434387, "filename": "/packages/caustics/caust14.png"}, {"audio": 0, "start": 6434387, "crunched": 0, "end": 6458137, "filename": "/packages/caustics/caust29.png"}, {"audio": 0, "start": 6458137, "crunched": 0, "end": 6482301, "filename": "/packages/caustics/caust11.png"}, {"audio": 0, "start": 6482301, "crunched": 0, "end": 6506555, "filename": "/packages/caustics/caust30.png"}, {"audio": 0, "start": 6506555, "crunched": 0, "end": 6531096, "filename": "/packages/caustics/caust18.png"}, {"audio": 0, "start": 6531096, "crunched": 0, "end": 6531154, "filename": "/packages/caustics/readme.txt"}, {"audio": 0, "start": 6531154, "crunched": 0, "end": 6555037, "filename": "/packages/caustics/caust09.png"}, {"audio": 0, "start": 6555037, "crunched": 0, "end": 6578862, "filename": "/packages/caustics/caust10.png"}, {"audio": 0, "start": 6578862, "crunched": 0, "end": 6602306, "filename": "/packages/caustics/caust22.png"}, {"audio": 0, "start": 6602306, "crunched": 0, "end": 6626798, "filename": "/packages/caustics/caust01.png"}, {"audio": 0, "start": 6626798, "crunched": 0, "end": 6651317, "filename": "/packages/caustics/caust00.png"}, {"audio": 0, "start": 6651317, "crunched": 0, "end": 6675423, "filename": "/packages/caustics/caust20.png"}, {"audio": 0, "start": 6675423, "crunched": 0, "end": 6699780, "filename": "/packages/caustics/caust16.png"}, {"audio": 0, "start": 6699780, "crunched": 0, "end": 6723424, "filename": "/packages/caustics/caust27.png"}, {"audio": 0, "start": 6723424, "crunched": 0, "end": 6747540, "filename": "/packages/caustics/caust02.png"}, {"audio": 0, "start": 6747540, "crunched": 0, "end": 6771178, "filename": "/packages/caustics/caust21.png"}, {"audio": 0, "start": 6771178, "crunched": 0, "end": 6785954, "filename": "/packages/models/debris/tris.md2"}, {"audio": 0, "start": 6785954, "crunched": 0, "end": 6786197, "filename": "/packages/models/debris/md2.cfg"}, {"audio": 0, "start": 6786197, "crunched": 0, "end": 6978023, "filename": "/packages/models/debris/skin.png"}, {"audio": 0, "start": 6978023, "crunched": 0, "end": 6978161, "filename": "/packages/models/projectiles/grenade/iqm.cfg"}, {"audio": 0, "start": 6978161, "crunched": 0, "end": 6978317, "filename": "/packages/models/projectiles/rocket/iqm.cfg"}, {"audio": 0, "start": 6978317, "crunched": 0, "end": 6991554, "filename": "/packages/models/projectiles/rocket/skin.jpg"}, {"audio": 0, "start": 6991554, "crunched": 0, "end": 6994690, "filename": "/packages/models/projectiles/rocket/rocket.iqm"}, {"audio": 0, "start": 6994690, "crunched": 0, "end": 7002409, "filename": "/packages/models/projectiles/rocket/normal.jpg"}, {"audio": 0, "start": 7002409, "crunched": 0, "end": 7003069, "filename": "/packages/models/projectiles/rocket/readme.txt"}, {"audio": 0, "start": 7003069, "crunched": 0, "end": 7023837, "filename": "/packages/models/projectiles/rocket/mask.jpg"}, {"audio": 0, "start": 7023837, "crunched": 0, "end": 7023974, "filename": "/packages/brushes/gradient_128.png"}, {"audio": 0, "start": 7023974, "crunched": 0, "end": 7024970, "filename": "/packages/brushes/circle_8_hard.png"}, {"audio": 0, "start": 7024970, "crunched": 0, "end": 7026208, "filename": "/packages/brushes/circle_32_solid.png"}, {"audio": 0, "start": 7026208, "crunched": 0, "end": 7030540, "filename": "/packages/brushes/circle_64_hard.png"}, {"audio": 0, "start": 7030540, "crunched": 0, "end": 7031747, "filename": "/packages/brushes/square_64_hard.png"}, {"audio": 0, "start": 7031747, "crunched": 0, "end": 7032829, "filename": "/packages/brushes/square_16_hard.png"}, {"audio": 0, "start": 7032829, "crunched": 0, "end": 7033802, "filename": "/packages/brushes/square_16_solid.png"}, {"audio": 0, "start": 7033802, "crunched": 0, "end": 7034783, "filename": "/packages/brushes/square_32_solid.png"}, {"audio": 0, "start": 7034783, "crunched": 0, "end": 7036068, "filename": "/packages/brushes/circle_32_soft.png"}, {"audio": 0, "start": 7036068, "crunched": 0, "end": 7037063, "filename": "/packages/brushes/circle_8_solid.png"}, {"audio": 0, "start": 7037063, "crunched": 0, "end": 7038877, "filename": "/packages/brushes/circle_64_soft.png"}, {"audio": 0, "start": 7038877, "crunched": 0, "end": 7039990, "filename": "/packages/brushes/circle_16_solid.png"}, {"audio": 0, "start": 7039990, "crunched": 0, "end": 7042354, "filename": "/packages/brushes/circle_128_solid.png"}, {"audio": 0, "start": 7042354, "crunched": 0, "end": 7051989, "filename": "/packages/brushes/noise_128.png"}, {"audio": 0, "start": 7051989, "crunched": 0, "end": 7052981, "filename": "/packages/brushes/circle_8_soft.png"}, {"audio": 0, "start": 7052981, "crunched": 0, "end": 7054164, "filename": "/packages/brushes/square_32_hard.png"}, {"audio": 0, "start": 7054164, "crunched": 0, "end": 7055286, "filename": "/packages/brushes/circle_16_hard.png"}, {"audio": 0, "start": 7055286, "crunched": 0, "end": 7058846, "filename": "/packages/brushes/circle_32_hard.png"}, {"audio": 0, "start": 7058846, "crunched": 0, "end": 7062323, "filename": "/packages/brushes/circle_128_soft.png"}, {"audio": 0, "start": 7062323, "crunched": 0, "end": 7062452, "filename": "/packages/brushes/gradient_64.png"}, {"audio": 0, "start": 7062452, "crunched": 0, "end": 7062572, "filename": "/packages/brushes/gradient_32.png"}, {"audio": 0, "start": 7062572, "crunched": 0, "end": 7063578, "filename": "/packages/brushes/square_64_solid.png"}, {"audio": 0, "start": 7063578, "crunched": 0, "end": 7064669, "filename": "/packages/brushes/circle_16_soft.png"}, {"audio": 0, "start": 7064669, "crunched": 0, "end": 7068757, "filename": "/packages/brushes/circle_128_hard.png"}, {"audio": 0, "start": 7068757, "crunched": 0, "end": 7068816, "filename": "/packages/brushes/readme.txt"}, {"audio": 0, "start": 7068816, "crunched": 0, "end": 7071106, "filename": "/packages/brushes/noise_64.png"}, {"audio": 0, "start": 7071106, "crunched": 0, "end": 7072690, "filename": "/packages/brushes/circle_64_solid.png"}, {"audio": 0, "start": 7072690, "crunched": 0, "end": 7072793, "filename": "/packages/brushes/gradient_16.png"}, {"audio": 0, "start": 7072793, "crunched": 0, "end": 7216537, "filename": "/packages/hud/damage.png"}, {"audio": 0, "start": 7216537, "crunched": 0, "end": 7237842, "filename": "/packages/hud/js.png"}, {"audio": 0, "start": 7237842, "crunched": 0, "end": 7237913, "filename": "/packages/hud/readme.txt"}, {"audio": 0, "start": 7237913, "crunched": 0, "end": 7259234, "filename": "/packages/hud/wasm.png"}, {"audio": 0, "start": 7259234, "crunched": 0, "end": 7364635, "filename": "/packages/hud/items.png"}], "remote_package_size": 7364635, "package_uuid": "fb1fb70a-b340-4288-a7e3-152063c95b5d"});

})();

