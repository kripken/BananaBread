
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
    var PACKAGE_NAME = 'character.data';
    var REMOTE_PACKAGE_BASE = 'character.data';
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
Module['FS_createPath']('/', 'packages', true, true);
Module['FS_createPath']('/packages', 'models', true, true);
Module['FS_createPath']('/packages/models', 'vwep', true, true);
Module['FS_createPath']('/packages/models/vwep', 'chaing', true, true);
Module['FS_createPath']('/packages/models/vwep', 'gl', true, true);
Module['FS_createPath']('/packages/models/vwep', 'rocket', true, true);
Module['FS_createPath']('/packages/models/vwep', 'shotg', true, true);
Module['FS_createPath']('/packages/models/vwep', 'rifle', true, true);
Module['FS_createPath']('/packages/models', 'snoutx10k', true, true);
Module['FS_createPath']('/packages/models/snoutx10k', 'hudguns', true, true);
Module['FS_createPath']('/packages/models/snoutx10k/hudguns', 'chaing', true, true);
Module['FS_createPath']('/packages/models/snoutx10k/hudguns', 'gl', true, true);
Module['FS_createPath']('/packages/models/snoutx10k/hudguns', 'rocket', true, true);
Module['FS_createPath']('/packages/models/snoutx10k/hudguns', 'shotg', true, true);
Module['FS_createPath']('/packages/models/snoutx10k/hudguns', 'rifle', true, true);
Module['FS_createPath']('/packages/models', 'hudguns', true, true);
Module['FS_createPath']('/packages/models/hudguns', 'chaing', true, true);
Module['FS_createPath']('/packages/models/hudguns', 'gl', true, true);
Module['FS_createPath']('/packages/models/hudguns', 'rocket', true, true);
Module['FS_createPath']('/packages/models/hudguns', 'shotg', true, true);
Module['FS_createPath']('/packages/models/hudguns', 'rifle', true, true);

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
              Module['removeRunDependency']('datafile_character.data');

    };
    Module['addRunDependency']('datafile_character.data');
  
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
 loadPackage({"files": [{"audio": 0, "start": 0, "crunched": 0, "end": 94, "filename": "/packages/models/vwep/readme.txt"}, {"audio": 0, "start": 94, "crunched": 0, "end": 276, "filename": "/packages/models/vwep/chaing/iqm.cfg"}, {"audio": 0, "start": 276, "crunched": 0, "end": 111196, "filename": "/packages/models/vwep/chaing/minigun_vwep.iqm"}, {"audio": 0, "start": 111196, "crunched": 0, "end": 194796, "filename": "/packages/models/vwep/gl/gl_vwep.iqm"}, {"audio": 0, "start": 194796, "crunched": 0, "end": 194968, "filename": "/packages/models/vwep/gl/iqm.cfg"}, {"audio": 0, "start": 194968, "crunched": 0, "end": 195122, "filename": "/packages/models/vwep/rocket/iqm.cfg"}, {"audio": 0, "start": 195122, "crunched": 0, "end": 276010, "filename": "/packages/models/vwep/rocket/rl_vwep.iqm"}, {"audio": 0, "start": 276010, "crunched": 0, "end": 371794, "filename": "/packages/models/vwep/shotg/shotgun_vwep.iqm"}, {"audio": 0, "start": 371794, "crunched": 0, "end": 371997, "filename": "/packages/models/vwep/shotg/iqm.cfg"}, {"audio": 0, "start": 371997, "crunched": 0, "end": 372197, "filename": "/packages/models/vwep/rifle/iqm.cfg"}, {"audio": 0, "start": 372197, "crunched": 0, "end": 474277, "filename": "/packages/models/vwep/rifle/sniper_vwep.iqm"}, {"audio": 0, "start": 474277, "crunched": 0, "end": 512699, "filename": "/packages/models/snoutx10k/lower_mask.jpg"}, {"audio": 0, "start": 512699, "crunched": 0, "end": 572807, "filename": "/packages/models/snoutx10k/right.md5anim.iqm"}, {"audio": 0, "start": 572807, "crunched": 0, "end": 579563, "filename": "/packages/models/snoutx10k/gl_idle.md5anim.iqm"}, {"audio": 0, "start": 579563, "crunched": 0, "end": 586319, "filename": "/packages/models/snoutx10k/edit.md5anim.iqm"}, {"audio": 0, "start": 586319, "crunched": 0, "end": 644721, "filename": "/packages/models/snoutx10k/lower.jpg"}, {"audio": 0, "start": 644721, "crunched": 0, "end": 700998, "filename": "/packages/models/snoutx10k/upper_normals.jpg"}, {"audio": 0, "start": 700998, "crunched": 0, "end": 719154, "filename": "/packages/models/snoutx10k/swim.md5anim.iqm"}, {"audio": 0, "start": 719154, "crunched": 0, "end": 730210, "filename": "/packages/models/snoutx10k/sniper_shoot.md5anim.iqm"}, {"audio": 0, "start": 730210, "crunched": 0, "end": 741082, "filename": "/packages/models/snoutx10k/rl_shoot.md5anim.iqm"}, {"audio": 0, "start": 741082, "crunched": 0, "end": 741464, "filename": "/packages/models/snoutx10k/iqm.cfg"}, {"audio": 0, "start": 741464, "crunched": 0, "end": 757700, "filename": "/packages/models/snoutx10k/lose.md5anim.iqm"}, {"audio": 0, "start": 757700, "crunched": 0, "end": 775616, "filename": "/packages/models/snoutx10k/pain.md5anim.iqm"}, {"audio": 0, "start": 775616, "crunched": 0, "end": 792692, "filename": "/packages/models/snoutx10k/pain2.md5anim.iqm"}, {"audio": 0, "start": 792692, "crunched": 0, "end": 854984, "filename": "/packages/models/snoutx10k/left.md5anim.iqm"}, {"audio": 0, "start": 854984, "crunched": 0, "end": 913688, "filename": "/packages/models/snoutx10k/backward.md5anim.iqm"}, {"audio": 0, "start": 913688, "crunched": 0, "end": 982145, "filename": "/packages/models/snoutx10k/lower_normals.jpg"}, {"audio": 0, "start": 982145, "crunched": 0, "end": 993541, "filename": "/packages/models/snoutx10k/shoot.md5anim.iqm"}, {"audio": 0, "start": 993541, "crunched": 0, "end": 1004597, "filename": "/packages/models/snoutx10k/minigun_shoot.md5anim.iqm"}, {"audio": 0, "start": 1004597, "crunched": 0, "end": 1007626, "filename": "/packages/models/snoutx10k/anims.cfg"}, {"audio": 0, "start": 1007626, "crunched": 0, "end": 1014382, "filename": "/packages/models/snoutx10k/dead2.md5anim.iqm"}, {"audio": 0, "start": 1014382, "crunched": 0, "end": 1037030, "filename": "/packages/models/snoutx10k/dying2.md5anim.iqm"}, {"audio": 0, "start": 1037030, "crunched": 0, "end": 1043794, "filename": "/packages/models/snoutx10k/minigun_idle.md5anim.iqm"}, {"audio": 0, "start": 1043794, "crunched": 0, "end": 1058990, "filename": "/packages/models/snoutx10k/sink.md5anim.iqm"}, {"audio": 0, "start": 1058990, "crunched": 0, "end": 1062634, "filename": "/packages/models/snoutx10k/ragdoll.cfg"}, {"audio": 0, "start": 1062634, "crunched": 0, "end": 1073550, "filename": "/packages/models/snoutx10k/gl_shoot.md5anim.iqm"}, {"audio": 0, "start": 1073550, "crunched": 0, "end": 1080306, "filename": "/packages/models/snoutx10k/rl_idle.md5anim.iqm"}, {"audio": 0, "start": 1080306, "crunched": 0, "end": 1142598, "filename": "/packages/models/snoutx10k/forward.md5anim.iqm"}, {"audio": 0, "start": 1142598, "crunched": 0, "end": 1153542, "filename": "/packages/models/snoutx10k/shotgun_shoot.md5anim.iqm"}, {"audio": 0, "start": 1153542, "crunched": 0, "end": 1160298, "filename": "/packages/models/snoutx10k/dead.md5anim.iqm"}, {"audio": 0, "start": 1160298, "crunched": 0, "end": 1160392, "filename": "/packages/models/snoutx10k/readme.txt"}, {"audio": 0, "start": 1160392, "crunched": 0, "end": 1177788, "filename": "/packages/models/snoutx10k/win.md5anim.iqm"}, {"audio": 0, "start": 1177788, "crunched": 0, "end": 1184544, "filename": "/packages/models/snoutx10k/lag.md5anim.iqm"}, {"audio": 0, "start": 1184544, "crunched": 0, "end": 1207992, "filename": "/packages/models/snoutx10k/punch.md5anim.iqm"}, {"audio": 0, "start": 1207992, "crunched": 0, "end": 1232064, "filename": "/packages/models/snoutx10k/dying.md5anim.iqm"}, {"audio": 0, "start": 1232064, "crunched": 0, "end": 1238828, "filename": "/packages/models/snoutx10k/sniper_idle.md5anim.iqm"}, {"audio": 0, "start": 1238828, "crunched": 0, "end": 1299493, "filename": "/packages/models/snoutx10k/upper.jpg"}, {"audio": 0, "start": 1299493, "crunched": 0, "end": 1306249, "filename": "/packages/models/snoutx10k/jump.md5anim.iqm"}, {"audio": 0, "start": 1306249, "crunched": 0, "end": 1313013, "filename": "/packages/models/snoutx10k/shotgun_idle.md5anim.iqm"}, {"audio": 0, "start": 1313013, "crunched": 0, "end": 1352005, "filename": "/packages/models/snoutx10k/upper_mask.jpg"}, {"audio": 0, "start": 1352005, "crunched": 0, "end": 1426029, "filename": "/packages/models/snoutx10k/idle.md5anim.iqm"}, {"audio": 0, "start": 1426029, "crunched": 0, "end": 1436997, "filename": "/packages/models/snoutx10k/chainsaw_attack.md5anim.iqm"}, {"audio": 0, "start": 1436997, "crunched": 0, "end": 1465353, "filename": "/packages/models/snoutx10k/taunt.md5anim.iqm"}, {"audio": 0, "start": 1465353, "crunched": 0, "end": 1691913, "filename": "/packages/models/snoutx10k/snoutx10k.iqm"}, {"audio": 0, "start": 1691913, "crunched": 0, "end": 1698677, "filename": "/packages/models/snoutx10k/chainsaw_idle.md5anim.iqm"}, {"audio": 0, "start": 1698677, "crunched": 0, "end": 1718934, "filename": "/packages/models/snoutx10k/hudguns/snout_hands_mask.jpg"}, {"audio": 0, "start": 1718934, "crunched": 0, "end": 1719416, "filename": "/packages/models/snoutx10k/hudguns/iqm.cfg"}, {"audio": 0, "start": 1719416, "crunched": 0, "end": 1780499, "filename": "/packages/models/snoutx10k/hudguns/snout_hands_normals.jpg"}, {"audio": 0, "start": 1780499, "crunched": 0, "end": 1886296, "filename": "/packages/models/snoutx10k/hudguns/snout_hands.jpg"}, {"audio": 0, "start": 1886296, "crunched": 0, "end": 2023800, "filename": "/packages/models/snoutx10k/hudguns/snout_hands.iqm"}, {"audio": 0, "start": 2023800, "crunched": 0, "end": 2023929, "filename": "/packages/models/snoutx10k/hudguns/chaing/iqm.cfg"}, {"audio": 0, "start": 2023929, "crunched": 0, "end": 2024054, "filename": "/packages/models/snoutx10k/hudguns/gl/iqm.cfg"}, {"audio": 0, "start": 2024054, "crunched": 0, "end": 2024183, "filename": "/packages/models/snoutx10k/hudguns/rocket/iqm.cfg"}, {"audio": 0, "start": 2024183, "crunched": 0, "end": 2024311, "filename": "/packages/models/snoutx10k/hudguns/shotg/iqm.cfg"}, {"audio": 0, "start": 2024311, "crunched": 0, "end": 2024439, "filename": "/packages/models/snoutx10k/hudguns/rifle/iqm.cfg"}, {"audio": 0, "start": 2024439, "crunched": 0, "end": 2024533, "filename": "/packages/models/hudguns/readme.txt"}, {"audio": 0, "start": 2024533, "crunched": 0, "end": 2060600, "filename": "/packages/models/hudguns/chaing/m134_normals.jpg"}, {"audio": 0, "start": 2060600, "crunched": 0, "end": 2061072, "filename": "/packages/models/hudguns/chaing/chaing_shoot.iqm"}, {"audio": 0, "start": 2061072, "crunched": 0, "end": 2061639, "filename": "/packages/models/hudguns/chaing/iqm.cfg"}, {"audio": 0, "start": 2061639, "crunched": 0, "end": 2093228, "filename": "/packages/models/hudguns/chaing/m134_mask.jpg"}, {"audio": 0, "start": 2093228, "crunched": 0, "end": 2241291, "filename": "/packages/models/hudguns/chaing/m134.jpg"}, {"audio": 0, "start": 2241291, "crunched": 0, "end": 2241719, "filename": "/packages/models/hudguns/chaing/chaing_idle.iqm"}, {"audio": 0, "start": 2241719, "crunched": 0, "end": 2244787, "filename": "/packages/models/hudguns/chaing/hands_mg_idle.iqm"}, {"audio": 0, "start": 2244787, "crunched": 0, "end": 2335411, "filename": "/packages/models/hudguns/chaing/chaing.iqm"}, {"audio": 0, "start": 2335411, "crunched": 0, "end": 2338811, "filename": "/packages/models/hudguns/chaing/hands_mg_shoot.iqm"}, {"audio": 0, "start": 2338811, "crunched": 0, "end": 2339319, "filename": "/packages/models/hudguns/gl/gl_idle.md5anim.iqm"}, {"audio": 0, "start": 2339319, "crunched": 0, "end": 2494191, "filename": "/packages/models/hudguns/gl/gl.iqm"}, {"audio": 0, "start": 2494191, "crunched": 0, "end": 2498939, "filename": "/packages/models/hudguns/gl/hands_gl_shoot.md5anim.iqm"}, {"audio": 0, "start": 2498939, "crunched": 0, "end": 2562877, "filename": "/packages/models/hudguns/gl/gl.jpg"}, {"audio": 0, "start": 2562877, "crunched": 0, "end": 2565945, "filename": "/packages/models/hudguns/gl/hands_gl_idle.md5anim.iqm"}, {"audio": 0, "start": 2565945, "crunched": 0, "end": 2566428, "filename": "/packages/models/hudguns/gl/iqm.cfg"}, {"audio": 0, "start": 2566428, "crunched": 0, "end": 2567020, "filename": "/packages/models/hudguns/gl/gl_shoot.md5anim.iqm"}, {"audio": 0, "start": 2567020, "crunched": 0, "end": 2602723, "filename": "/packages/models/hudguns/gl/gl_normals.jpg"}, {"audio": 0, "start": 2602723, "crunched": 0, "end": 2620533, "filename": "/packages/models/hudguns/gl/gl_mask.jpg"}, {"audio": 0, "start": 2620533, "crunched": 0, "end": 2624273, "filename": "/packages/models/hudguns/rocket/hands_rl_shoot.md5anim.iqm"}, {"audio": 0, "start": 2624273, "crunched": 0, "end": 2649973, "filename": "/packages/models/hudguns/rocket/rl_mask.jpg"}, {"audio": 0, "start": 2649973, "crunched": 0, "end": 2650609, "filename": "/packages/models/hudguns/rocket/rl_shoot.md5anim.iqm"}, {"audio": 0, "start": 2650609, "crunched": 0, "end": 2651268, "filename": "/packages/models/hudguns/rocket/iqm.cfg"}, {"audio": 0, "start": 2651268, "crunched": 0, "end": 2651776, "filename": "/packages/models/hudguns/rocket/rl_idle.md5anim.iqm"}, {"audio": 0, "start": 2651776, "crunched": 0, "end": 2694844, "filename": "/packages/models/hudguns/rocket/rl_normals.jpg"}, {"audio": 0, "start": 2694844, "crunched": 0, "end": 2801556, "filename": "/packages/models/hudguns/rocket/rl.iqm"}, {"audio": 0, "start": 2801556, "crunched": 0, "end": 2804624, "filename": "/packages/models/hudguns/rocket/hands_rl_idle.md5anim.iqm"}, {"audio": 0, "start": 2804624, "crunched": 0, "end": 2909938, "filename": "/packages/models/hudguns/rocket/rl.jpg"}, {"audio": 0, "start": 2909938, "crunched": 0, "end": 2913006, "filename": "/packages/models/hudguns/shotg/hands_shotgun_idle.md5anim.iqm"}, {"audio": 0, "start": 2913006, "crunched": 0, "end": 2913748, "filename": "/packages/models/hudguns/shotg/iqm.cfg"}, {"audio": 0, "start": 2913748, "crunched": 0, "end": 3016477, "filename": "/packages/models/hudguns/shotg/shotgun.jpg"}, {"audio": 0, "start": 3016477, "crunched": 0, "end": 3018387, "filename": "/packages/models/hudguns/shotg/shotgun_shell_mask.jpg"}, {"audio": 0, "start": 3018387, "crunched": 0, "end": 3024085, "filename": "/packages/models/hudguns/shotg/shotgun_shell.jpg"}, {"audio": 0, "start": 3024085, "crunched": 0, "end": 3055861, "filename": "/packages/models/hudguns/shotg/shotgun_mask.jpg"}, {"audio": 0, "start": 3055861, "crunched": 0, "end": 3059581, "filename": "/packages/models/hudguns/shotg/shotgun_attack.md5anim.iqm"}, {"audio": 0, "start": 3059581, "crunched": 0, "end": 3184173, "filename": "/packages/models/hudguns/shotg/shotgun.iqm"}, {"audio": 0, "start": 3184173, "crunched": 0, "end": 3184953, "filename": "/packages/models/hudguns/shotg/shotgun_idle.md5anim.iqm"}, {"audio": 0, "start": 3184953, "crunched": 0, "end": 3232056, "filename": "/packages/models/hudguns/shotg/shotgun_normals.jpg"}, {"audio": 0, "start": 3232056, "crunched": 0, "end": 3233880, "filename": "/packages/models/hudguns/shotg/shotgun_shell_normals.jpg"}, {"audio": 0, "start": 3233880, "crunched": 0, "end": 3245356, "filename": "/packages/models/hudguns/shotg/hands_shotgun_shoot.md5anim.iqm"}, {"audio": 0, "start": 3245356, "crunched": 0, "end": 3290837, "filename": "/packages/models/hudguns/rifle/sniper_normals.jpg"}, {"audio": 0, "start": 3290837, "crunched": 0, "end": 3388395, "filename": "/packages/models/hudguns/rifle/sniper.jpg"}, {"audio": 0, "start": 3388395, "crunched": 0, "end": 3388990, "filename": "/packages/models/hudguns/rifle/iqm.cfg"}, {"audio": 0, "start": 3388990, "crunched": 0, "end": 3406818, "filename": "/packages/models/hudguns/rifle/hands_rifle_shoot.md5anim.iqm"}, {"audio": 0, "start": 3406818, "crunched": 0, "end": 3596522, "filename": "/packages/models/hudguns/rifle/rifle.iqm"}, {"audio": 0, "start": 3596522, "crunched": 0, "end": 3597118, "filename": "/packages/models/hudguns/rifle/rifle_idle.md5anim.iqm"}, {"audio": 0, "start": 3597118, "crunched": 0, "end": 3625580, "filename": "/packages/models/hudguns/rifle/sniper_mask.jpg"}, {"audio": 0, "start": 3625580, "crunched": 0, "end": 3626724, "filename": "/packages/models/hudguns/rifle/rifle_shoot.md5anim.iqm"}, {"audio": 0, "start": 3626724, "crunched": 0, "end": 3629792, "filename": "/packages/models/hudguns/rifle/hands_rifle_idle.md5anim.iqm"}], "remote_package_size": 3629792, "package_uuid": "dfbcbaa7-361b-4c20-b3d2-858a1c779d87"});

})();

