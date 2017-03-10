
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
Module['FS_createPath']('/packages/models/vwep', 'rifle', true, true);
Module['FS_createPath']('/packages/models/vwep', 'shotg', true, true);
Module['FS_createPath']('/packages/models', 'snoutx10k', true, true);
Module['FS_createPath']('/packages/models/snoutx10k', 'hudguns', true, true);
Module['FS_createPath']('/packages/models/snoutx10k/hudguns', 'chaing', true, true);
Module['FS_createPath']('/packages/models/snoutx10k/hudguns', 'gl', true, true);
Module['FS_createPath']('/packages/models/snoutx10k/hudguns', 'rocket', true, true);
Module['FS_createPath']('/packages/models/snoutx10k/hudguns', 'rifle', true, true);
Module['FS_createPath']('/packages/models/snoutx10k/hudguns', 'shotg', true, true);
Module['FS_createPath']('/packages/models', 'hudguns', true, true);
Module['FS_createPath']('/packages/models/hudguns', 'chaing', true, true);
Module['FS_createPath']('/packages/models/hudguns', 'gl', true, true);
Module['FS_createPath']('/packages/models/hudguns', 'rocket', true, true);
Module['FS_createPath']('/packages/models/hudguns', 'rifle', true, true);
Module['FS_createPath']('/packages/models/hudguns', 'shotg', true, true);

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
 loadPackage({"files": [{"audio": 0, "start": 0, "crunched": 0, "end": 94, "filename": "/packages/models/vwep/readme.txt"}, {"audio": 0, "start": 94, "crunched": 0, "end": 111014, "filename": "/packages/models/vwep/chaing/minigun_vwep.iqm"}, {"audio": 0, "start": 111014, "crunched": 0, "end": 111196, "filename": "/packages/models/vwep/chaing/iqm.cfg"}, {"audio": 0, "start": 111196, "crunched": 0, "end": 111368, "filename": "/packages/models/vwep/gl/iqm.cfg"}, {"audio": 0, "start": 111368, "crunched": 0, "end": 194968, "filename": "/packages/models/vwep/gl/gl_vwep.iqm"}, {"audio": 0, "start": 194968, "crunched": 0, "end": 195122, "filename": "/packages/models/vwep/rocket/iqm.cfg"}, {"audio": 0, "start": 195122, "crunched": 0, "end": 276010, "filename": "/packages/models/vwep/rocket/rl_vwep.iqm"}, {"audio": 0, "start": 276010, "crunched": 0, "end": 378090, "filename": "/packages/models/vwep/rifle/sniper_vwep.iqm"}, {"audio": 0, "start": 378090, "crunched": 0, "end": 378290, "filename": "/packages/models/vwep/rifle/iqm.cfg"}, {"audio": 0, "start": 378290, "crunched": 0, "end": 474074, "filename": "/packages/models/vwep/shotg/shotgun_vwep.iqm"}, {"audio": 0, "start": 474074, "crunched": 0, "end": 474277, "filename": "/packages/models/vwep/shotg/iqm.cfg"}, {"audio": 0, "start": 474277, "crunched": 0, "end": 481033, "filename": "/packages/models/snoutx10k/edit.md5anim.iqm"}, {"audio": 0, "start": 481033, "crunched": 0, "end": 487797, "filename": "/packages/models/snoutx10k/shotgun_idle.md5anim.iqm"}, {"audio": 0, "start": 487797, "crunched": 0, "end": 505193, "filename": "/packages/models/snoutx10k/win.md5anim.iqm"}, {"audio": 0, "start": 505193, "crunched": 0, "end": 516109, "filename": "/packages/models/snoutx10k/gl_shoot.md5anim.iqm"}, {"audio": 0, "start": 516109, "crunched": 0, "end": 742669, "filename": "/packages/models/snoutx10k/snoutx10k.iqm"}, {"audio": 0, "start": 742669, "crunched": 0, "end": 804961, "filename": "/packages/models/snoutx10k/forward.md5anim.iqm"}, {"audio": 0, "start": 804961, "crunched": 0, "end": 811717, "filename": "/packages/models/snoutx10k/rl_idle.md5anim.iqm"}, {"audio": 0, "start": 811717, "crunched": 0, "end": 823113, "filename": "/packages/models/snoutx10k/shoot.md5anim.iqm"}, {"audio": 0, "start": 823113, "crunched": 0, "end": 881515, "filename": "/packages/models/snoutx10k/lower.jpg"}, {"audio": 0, "start": 881515, "crunched": 0, "end": 888279, "filename": "/packages/models/snoutx10k/minigun_idle.md5anim.iqm"}, {"audio": 0, "start": 888279, "crunched": 0, "end": 888661, "filename": "/packages/models/snoutx10k/iqm.cfg"}, {"audio": 0, "start": 888661, "crunched": 0, "end": 903857, "filename": "/packages/models/snoutx10k/sink.md5anim.iqm"}, {"audio": 0, "start": 903857, "crunched": 0, "end": 907501, "filename": "/packages/models/snoutx10k/ragdoll.cfg"}, {"audio": 0, "start": 907501, "crunched": 0, "end": 918469, "filename": "/packages/models/snoutx10k/chainsaw_attack.md5anim.iqm"}, {"audio": 0, "start": 918469, "crunched": 0, "end": 929525, "filename": "/packages/models/snoutx10k/sniper_shoot.md5anim.iqm"}, {"audio": 0, "start": 929525, "crunched": 0, "end": 968517, "filename": "/packages/models/snoutx10k/upper_mask.jpg"}, {"audio": 0, "start": 968517, "crunched": 0, "end": 979389, "filename": "/packages/models/snoutx10k/rl_shoot.md5anim.iqm"}, {"audio": 0, "start": 979389, "crunched": 0, "end": 1002037, "filename": "/packages/models/snoutx10k/dying2.md5anim.iqm"}, {"audio": 0, "start": 1002037, "crunched": 0, "end": 1008793, "filename": "/packages/models/snoutx10k/jump.md5anim.iqm"}, {"audio": 0, "start": 1008793, "crunched": 0, "end": 1011822, "filename": "/packages/models/snoutx10k/anims.cfg"}, {"audio": 0, "start": 1011822, "crunched": 0, "end": 1018578, "filename": "/packages/models/snoutx10k/dead2.md5anim.iqm"}, {"audio": 0, "start": 1018578, "crunched": 0, "end": 1042650, "filename": "/packages/models/snoutx10k/dying.md5anim.iqm"}, {"audio": 0, "start": 1042650, "crunched": 0, "end": 1049414, "filename": "/packages/models/snoutx10k/chainsaw_idle.md5anim.iqm"}, {"audio": 0, "start": 1049414, "crunched": 0, "end": 1105691, "filename": "/packages/models/snoutx10k/upper_normals.jpg"}, {"audio": 0, "start": 1105691, "crunched": 0, "end": 1112447, "filename": "/packages/models/snoutx10k/lag.md5anim.iqm"}, {"audio": 0, "start": 1112447, "crunched": 0, "end": 1130363, "filename": "/packages/models/snoutx10k/pain.md5anim.iqm"}, {"audio": 0, "start": 1130363, "crunched": 0, "end": 1192655, "filename": "/packages/models/snoutx10k/left.md5anim.iqm"}, {"audio": 0, "start": 1192655, "crunched": 0, "end": 1210811, "filename": "/packages/models/snoutx10k/swim.md5anim.iqm"}, {"audio": 0, "start": 1210811, "crunched": 0, "end": 1221867, "filename": "/packages/models/snoutx10k/minigun_shoot.md5anim.iqm"}, {"audio": 0, "start": 1221867, "crunched": 0, "end": 1228631, "filename": "/packages/models/snoutx10k/sniper_idle.md5anim.iqm"}, {"audio": 0, "start": 1228631, "crunched": 0, "end": 1289296, "filename": "/packages/models/snoutx10k/upper.jpg"}, {"audio": 0, "start": 1289296, "crunched": 0, "end": 1300240, "filename": "/packages/models/snoutx10k/shotgun_shoot.md5anim.iqm"}, {"audio": 0, "start": 1300240, "crunched": 0, "end": 1358944, "filename": "/packages/models/snoutx10k/backward.md5anim.iqm"}, {"audio": 0, "start": 1358944, "crunched": 0, "end": 1427401, "filename": "/packages/models/snoutx10k/lower_normals.jpg"}, {"audio": 0, "start": 1427401, "crunched": 0, "end": 1434157, "filename": "/packages/models/snoutx10k/gl_idle.md5anim.iqm"}, {"audio": 0, "start": 1434157, "crunched": 0, "end": 1508181, "filename": "/packages/models/snoutx10k/idle.md5anim.iqm"}, {"audio": 0, "start": 1508181, "crunched": 0, "end": 1525257, "filename": "/packages/models/snoutx10k/pain2.md5anim.iqm"}, {"audio": 0, "start": 1525257, "crunched": 0, "end": 1585365, "filename": "/packages/models/snoutx10k/right.md5anim.iqm"}, {"audio": 0, "start": 1585365, "crunched": 0, "end": 1608813, "filename": "/packages/models/snoutx10k/punch.md5anim.iqm"}, {"audio": 0, "start": 1608813, "crunched": 0, "end": 1615569, "filename": "/packages/models/snoutx10k/dead.md5anim.iqm"}, {"audio": 0, "start": 1615569, "crunched": 0, "end": 1631805, "filename": "/packages/models/snoutx10k/lose.md5anim.iqm"}, {"audio": 0, "start": 1631805, "crunched": 0, "end": 1631899, "filename": "/packages/models/snoutx10k/readme.txt"}, {"audio": 0, "start": 1631899, "crunched": 0, "end": 1670321, "filename": "/packages/models/snoutx10k/lower_mask.jpg"}, {"audio": 0, "start": 1670321, "crunched": 0, "end": 1698677, "filename": "/packages/models/snoutx10k/taunt.md5anim.iqm"}, {"audio": 0, "start": 1698677, "crunched": 0, "end": 1699159, "filename": "/packages/models/snoutx10k/hudguns/iqm.cfg"}, {"audio": 0, "start": 1699159, "crunched": 0, "end": 1804956, "filename": "/packages/models/snoutx10k/hudguns/snout_hands.jpg"}, {"audio": 0, "start": 1804956, "crunched": 0, "end": 1942460, "filename": "/packages/models/snoutx10k/hudguns/snout_hands.iqm"}, {"audio": 0, "start": 1942460, "crunched": 0, "end": 2003543, "filename": "/packages/models/snoutx10k/hudguns/snout_hands_normals.jpg"}, {"audio": 0, "start": 2003543, "crunched": 0, "end": 2023800, "filename": "/packages/models/snoutx10k/hudguns/snout_hands_mask.jpg"}, {"audio": 0, "start": 2023800, "crunched": 0, "end": 2023929, "filename": "/packages/models/snoutx10k/hudguns/chaing/iqm.cfg"}, {"audio": 0, "start": 2023929, "crunched": 0, "end": 2024054, "filename": "/packages/models/snoutx10k/hudguns/gl/iqm.cfg"}, {"audio": 0, "start": 2024054, "crunched": 0, "end": 2024183, "filename": "/packages/models/snoutx10k/hudguns/rocket/iqm.cfg"}, {"audio": 0, "start": 2024183, "crunched": 0, "end": 2024311, "filename": "/packages/models/snoutx10k/hudguns/rifle/iqm.cfg"}, {"audio": 0, "start": 2024311, "crunched": 0, "end": 2024439, "filename": "/packages/models/snoutx10k/hudguns/shotg/iqm.cfg"}, {"audio": 0, "start": 2024439, "crunched": 0, "end": 2024533, "filename": "/packages/models/hudguns/readme.txt"}, {"audio": 0, "start": 2024533, "crunched": 0, "end": 2060600, "filename": "/packages/models/hudguns/chaing/m134_normals.jpg"}, {"audio": 0, "start": 2060600, "crunched": 0, "end": 2151224, "filename": "/packages/models/hudguns/chaing/chaing.iqm"}, {"audio": 0, "start": 2151224, "crunched": 0, "end": 2151791, "filename": "/packages/models/hudguns/chaing/iqm.cfg"}, {"audio": 0, "start": 2151791, "crunched": 0, "end": 2155191, "filename": "/packages/models/hudguns/chaing/hands_mg_shoot.iqm"}, {"audio": 0, "start": 2155191, "crunched": 0, "end": 2158259, "filename": "/packages/models/hudguns/chaing/hands_mg_idle.iqm"}, {"audio": 0, "start": 2158259, "crunched": 0, "end": 2158687, "filename": "/packages/models/hudguns/chaing/chaing_idle.iqm"}, {"audio": 0, "start": 2158687, "crunched": 0, "end": 2159159, "filename": "/packages/models/hudguns/chaing/chaing_shoot.iqm"}, {"audio": 0, "start": 2159159, "crunched": 0, "end": 2190748, "filename": "/packages/models/hudguns/chaing/m134_mask.jpg"}, {"audio": 0, "start": 2190748, "crunched": 0, "end": 2338811, "filename": "/packages/models/hudguns/chaing/m134.jpg"}, {"audio": 0, "start": 2338811, "crunched": 0, "end": 2339403, "filename": "/packages/models/hudguns/gl/gl_shoot.md5anim.iqm"}, {"audio": 0, "start": 2339403, "crunched": 0, "end": 2339886, "filename": "/packages/models/hudguns/gl/iqm.cfg"}, {"audio": 0, "start": 2339886, "crunched": 0, "end": 2357696, "filename": "/packages/models/hudguns/gl/gl_mask.jpg"}, {"audio": 0, "start": 2357696, "crunched": 0, "end": 2360764, "filename": "/packages/models/hudguns/gl/hands_gl_idle.md5anim.iqm"}, {"audio": 0, "start": 2360764, "crunched": 0, "end": 2424702, "filename": "/packages/models/hudguns/gl/gl.jpg"}, {"audio": 0, "start": 2424702, "crunched": 0, "end": 2460405, "filename": "/packages/models/hudguns/gl/gl_normals.jpg"}, {"audio": 0, "start": 2460405, "crunched": 0, "end": 2615277, "filename": "/packages/models/hudguns/gl/gl.iqm"}, {"audio": 0, "start": 2615277, "crunched": 0, "end": 2620025, "filename": "/packages/models/hudguns/gl/hands_gl_shoot.md5anim.iqm"}, {"audio": 0, "start": 2620025, "crunched": 0, "end": 2620533, "filename": "/packages/models/hudguns/gl/gl_idle.md5anim.iqm"}, {"audio": 0, "start": 2620533, "crunched": 0, "end": 2621041, "filename": "/packages/models/hudguns/rocket/rl_idle.md5anim.iqm"}, {"audio": 0, "start": 2621041, "crunched": 0, "end": 2621700, "filename": "/packages/models/hudguns/rocket/iqm.cfg"}, {"audio": 0, "start": 2621700, "crunched": 0, "end": 2622336, "filename": "/packages/models/hudguns/rocket/rl_shoot.md5anim.iqm"}, {"audio": 0, "start": 2622336, "crunched": 0, "end": 2625404, "filename": "/packages/models/hudguns/rocket/hands_rl_idle.md5anim.iqm"}, {"audio": 0, "start": 2625404, "crunched": 0, "end": 2651104, "filename": "/packages/models/hudguns/rocket/rl_mask.jpg"}, {"audio": 0, "start": 2651104, "crunched": 0, "end": 2756418, "filename": "/packages/models/hudguns/rocket/rl.jpg"}, {"audio": 0, "start": 2756418, "crunched": 0, "end": 2760158, "filename": "/packages/models/hudguns/rocket/hands_rl_shoot.md5anim.iqm"}, {"audio": 0, "start": 2760158, "crunched": 0, "end": 2866870, "filename": "/packages/models/hudguns/rocket/rl.iqm"}, {"audio": 0, "start": 2866870, "crunched": 0, "end": 2909938, "filename": "/packages/models/hudguns/rocket/rl_normals.jpg"}, {"audio": 0, "start": 2909938, "crunched": 0, "end": 2927766, "filename": "/packages/models/hudguns/rifle/hands_rifle_shoot.md5anim.iqm"}, {"audio": 0, "start": 2927766, "crunched": 0, "end": 3117470, "filename": "/packages/models/hudguns/rifle/rifle.iqm"}, {"audio": 0, "start": 3117470, "crunched": 0, "end": 3118065, "filename": "/packages/models/hudguns/rifle/iqm.cfg"}, {"audio": 0, "start": 3118065, "crunched": 0, "end": 3146527, "filename": "/packages/models/hudguns/rifle/sniper_mask.jpg"}, {"audio": 0, "start": 3146527, "crunched": 0, "end": 3147671, "filename": "/packages/models/hudguns/rifle/rifle_shoot.md5anim.iqm"}, {"audio": 0, "start": 3147671, "crunched": 0, "end": 3150739, "filename": "/packages/models/hudguns/rifle/hands_rifle_idle.md5anim.iqm"}, {"audio": 0, "start": 3150739, "crunched": 0, "end": 3196220, "filename": "/packages/models/hudguns/rifle/sniper_normals.jpg"}, {"audio": 0, "start": 3196220, "crunched": 0, "end": 3196816, "filename": "/packages/models/hudguns/rifle/rifle_idle.md5anim.iqm"}, {"audio": 0, "start": 3196816, "crunched": 0, "end": 3294374, "filename": "/packages/models/hudguns/rifle/sniper.jpg"}, {"audio": 0, "start": 3294374, "crunched": 0, "end": 3295154, "filename": "/packages/models/hudguns/shotg/shotgun_idle.md5anim.iqm"}, {"audio": 0, "start": 3295154, "crunched": 0, "end": 3326930, "filename": "/packages/models/hudguns/shotg/shotgun_mask.jpg"}, {"audio": 0, "start": 3326930, "crunched": 0, "end": 3327672, "filename": "/packages/models/hudguns/shotg/iqm.cfg"}, {"audio": 0, "start": 3327672, "crunched": 0, "end": 3330740, "filename": "/packages/models/hudguns/shotg/hands_shotgun_idle.md5anim.iqm"}, {"audio": 0, "start": 3330740, "crunched": 0, "end": 3433469, "filename": "/packages/models/hudguns/shotg/shotgun.jpg"}, {"audio": 0, "start": 3433469, "crunched": 0, "end": 3444945, "filename": "/packages/models/hudguns/shotg/hands_shotgun_shoot.md5anim.iqm"}, {"audio": 0, "start": 3444945, "crunched": 0, "end": 3446855, "filename": "/packages/models/hudguns/shotg/shotgun_shell_mask.jpg"}, {"audio": 0, "start": 3446855, "crunched": 0, "end": 3452553, "filename": "/packages/models/hudguns/shotg/shotgun_shell.jpg"}, {"audio": 0, "start": 3452553, "crunched": 0, "end": 3456273, "filename": "/packages/models/hudguns/shotg/shotgun_attack.md5anim.iqm"}, {"audio": 0, "start": 3456273, "crunched": 0, "end": 3503376, "filename": "/packages/models/hudguns/shotg/shotgun_normals.jpg"}, {"audio": 0, "start": 3503376, "crunched": 0, "end": 3505200, "filename": "/packages/models/hudguns/shotg/shotgun_shell_normals.jpg"}, {"audio": 0, "start": 3505200, "crunched": 0, "end": 3629792, "filename": "/packages/models/hudguns/shotg/shotgun.iqm"}], "remote_package_size": 3629792, "package_uuid": "90edb67a-b829-4948-b0f9-a74dcd2584e4"});

})();

