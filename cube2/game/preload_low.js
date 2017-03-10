
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
    var PACKAGE_NAME = 'low.data';
    var REMOTE_PACKAGE_BASE = 'low.data';
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
Module['FS_createPath']('/packages', 'base', true, true);
Module['FS_createPath']('/packages', 'models', true, true);
Module['FS_createPath']('/packages/models', 'ffflag', true, true);
Module['FS_createPath']('/packages/models', 'ffpit', true, true);
Module['FS_createPath']('/packages', 'gk', true, true);
Module['FS_createPath']('/packages/gk', 'fantasy', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'skyfantasyJPG', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'rock_formation_gk_v01', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'iron_plates_gk_v01', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'iron_trim_gk_v01', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'castell_wall_trim_gk_v01', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'rock_formation_gk_v02', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'iron_intersection_gk_v01', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'castell_wall_gk_v02', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'wooden_roof_tiles_gk_v01', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'stone_ground_tiles_gk_v01', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'castell_wall_gk_v01', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'castell_wall_gk_v03', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'stone_ground_gk_v01', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'wooden_planks_gk_v01', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'castell_plaster_gk_v01', true, true);

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
              Module['removeRunDependency']('datafile_low.data');

    };
    Module['addRunDependency']('datafile_low.data');
  
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
 loadPackage({"files": [{"audio": 0, "start": 0, "crunched": 0, "end": 554042, "filename": "/packages/base/colos.ogz"}, {"audio": 0, "start": 554042, "crunched": 0, "end": 555197, "filename": "/packages/base/colos.cfg"}, {"audio": 0, "start": 555197, "crunched": 0, "end": 566661, "filename": "/packages/base/colos.wpt"}, {"audio": 0, "start": 566661, "crunched": 0, "end": 566913, "filename": "/packages/models/ffflag/md5.cfg"}, {"audio": 0, "start": 566913, "crunched": 0, "end": 1965169, "filename": "/packages/models/ffflag/ffflag_sc.dds"}, {"audio": 0, "start": 1965169, "crunched": 0, "end": 3363425, "filename": "/packages/models/ffflag/ffflag_cc.dds"}, {"audio": 0, "start": 3363425, "crunched": 0, "end": 4761681, "filename": "/packages/models/ffflag/ffflag_nm.dds"}, {"audio": 0, "start": 4761681, "crunched": 0, "end": 4785102, "filename": "/packages/models/ffflag/ffflag.md5mesh"}, {"audio": 0, "start": 4785102, "crunched": 0, "end": 4876857, "filename": "/packages/models/ffflag/ffflag.md5anim"}, {"audio": 0, "start": 4876857, "crunched": 0, "end": 4877137, "filename": "/packages/models/ffpit/md5.cfg"}, {"audio": 0, "start": 4877137, "crunched": 0, "end": 5226817, "filename": "/packages/models/ffpit/ffpit_01_gk_sc.dds"}, {"audio": 0, "start": 5226817, "crunched": 0, "end": 5576497, "filename": "/packages/models/ffpit/ffpit_01_gk_cc.dds"}, {"audio": 0, "start": 5576497, "crunched": 0, "end": 5926177, "filename": "/packages/models/ffpit/ffpit_01_gk_nm.dds"}, {"audio": 0, "start": 5926177, "crunched": 0, "end": 5971374, "filename": "/packages/models/ffpit/ffpit.md5mesh"}, {"audio": 0, "start": 5971374, "crunched": 0, "end": 6120813, "filename": "/packages/gk/fantasy/skyfantasyJPG/skyfantasy_rt.jpg"}, {"audio": 0, "start": 6120813, "crunched": 0, "end": 6203654, "filename": "/packages/gk/fantasy/skyfantasyJPG/skyfantasy_up.jpg"}, {"audio": 0, "start": 6203654, "crunched": 0, "end": 6360100, "filename": "/packages/gk/fantasy/skyfantasyJPG/skyfantasy_bk.jpg"}, {"audio": 0, "start": 6360100, "crunched": 0, "end": 6513935, "filename": "/packages/gk/fantasy/skyfantasyJPG/skyfantasy_ft.jpg"}, {"audio": 0, "start": 6513935, "crunched": 0, "end": 6699386, "filename": "/packages/gk/fantasy/skyfantasyJPG/skyfantasy_dn.jpg"}, {"audio": 0, "start": 6699386, "crunched": 0, "end": 6848448, "filename": "/packages/gk/fantasy/skyfantasyJPG/skyfantasy_lf.jpg"}, {"audio": 0, "start": 6848448, "crunched": 0, "end": 6848887, "filename": "/packages/gk/fantasy/rock_formation_gk_v01/package.cfg"}, {"audio": 0, "start": 6848887, "crunched": 0, "end": 6849589, "filename": "/packages/gk/fantasy/iron_plates_gk_v01/package.cfg"}, {"audio": 0, "start": 6849589, "crunched": 0, "end": 6850261, "filename": "/packages/gk/fantasy/iron_trim_gk_v01/package.cfg"}, {"audio": 0, "start": 6850261, "crunched": 0, "end": 6850685, "filename": "/packages/gk/fantasy/package.cfg"}, {"audio": 0, "start": 6850685, "crunched": 0, "end": 6851484, "filename": "/packages/gk/fantasy/castell_wall_trim_gk_v01/package.cfg"}, {"audio": 0, "start": 6851484, "crunched": 0, "end": 6851923, "filename": "/packages/gk/fantasy/rock_formation_gk_v02/package.cfg"}, {"audio": 0, "start": 6851923, "crunched": 0, "end": 6852382, "filename": "/packages/gk/fantasy/iron_intersection_gk_v01/package.cfg"}, {"audio": 0, "start": 6852382, "crunched": 0, "end": 6853101, "filename": "/packages/gk/fantasy/castell_wall_gk_v02/package.cfg"}, {"audio": 0, "start": 6853101, "crunched": 0, "end": 6853899, "filename": "/packages/gk/fantasy/wooden_roof_tiles_gk_v01/package.cfg"}, {"audio": 0, "start": 6853899, "crunched": 0, "end": 6854370, "filename": "/packages/gk/fantasy/stone_ground_tiles_gk_v01/package.cfg"}, {"audio": 0, "start": 6854370, "crunched": 0, "end": 6855089, "filename": "/packages/gk/fantasy/castell_wall_gk_v01/package.cfg"}, {"audio": 0, "start": 6855089, "crunched": 0, "end": 6855808, "filename": "/packages/gk/fantasy/castell_wall_gk_v03/package.cfg"}, {"audio": 0, "start": 6855808, "crunched": 0, "end": 6856231, "filename": "/packages/gk/fantasy/stone_ground_gk_v01/package.cfg"}, {"audio": 0, "start": 6856231, "crunched": 0, "end": 6856965, "filename": "/packages/gk/fantasy/wooden_planks_gk_v01/package.cfg"}, {"audio": 0, "start": 6856965, "crunched": 0, "end": 6857412, "filename": "/packages/gk/fantasy/castell_plaster_gk_v01/package.cfg"}, {"audio": 0, "start": 6857412, "crunched": 0, "end": 7207092, "filename": "/packages/gk/fantasy/rock_formation_gk_v01/rock_formation_gk_v01_cc.dds"}, {"audio": 0, "start": 7207092, "crunched": 0, "end": 7294628, "filename": "/packages/gk/fantasy/rock_formation_gk_v01/rock_formation_gk_v01_nm.dds"}, {"audio": 0, "start": 7294628, "crunched": 0, "end": 7382164, "filename": "/packages/gk/fantasy/castell_wall_gk_v01/castell_wall_gk_v01_cc.dds"}, {"audio": 0, "start": 7382164, "crunched": 0, "end": 7469700, "filename": "/packages/gk/fantasy/castell_wall_gk_v01/castell_wall_gk_v01_nm.dds"}, {"audio": 0, "start": 7469700, "crunched": 0, "end": 7557236, "filename": "/packages/gk/fantasy/castell_wall_gk_v02/castell_wall_gk_v02_cc.dds"}, {"audio": 0, "start": 7557236, "crunched": 0, "end": 7644772, "filename": "/packages/gk/fantasy/castell_wall_gk_v02/castell_wall_gk_v02_nm.dds"}, {"audio": 0, "start": 7644772, "crunched": 0, "end": 7688692, "filename": "/packages/gk/fantasy/castell_wall_trim_gk_v01/castell_wall_trim_gk_v01_cc.dds"}, {"audio": 0, "start": 7688692, "crunched": 0, "end": 7732612, "filename": "/packages/gk/fantasy/castell_wall_trim_gk_v01/castell_wall_trim_gk_v01_nm.dds"}, {"audio": 0, "start": 7732612, "crunched": 0, "end": 8082292, "filename": "/packages/gk/fantasy/stone_ground_tiles_gk_v01/stone_ground_tiles_gk_v01_cc.dds"}, {"audio": 0, "start": 8082292, "crunched": 0, "end": 8169828, "filename": "/packages/gk/fantasy/stone_ground_tiles_gk_v01/stone_ground_tiles_gk_v01_nm.dds"}, {"audio": 0, "start": 8169828, "crunched": 0, "end": 8257364, "filename": "/packages/gk/fantasy/wooden_planks_gk_v01/wooden_planks_gk_v01_cc.dds"}, {"audio": 0, "start": 8257364, "crunched": 0, "end": 8344900, "filename": "/packages/gk/fantasy/wooden_planks_gk_v01/wooden_planks_gk_v01_nm.dds"}, {"audio": 0, "start": 8344900, "crunched": 0, "end": 8432436, "filename": "/packages/gk/fantasy/castell_plaster_gk_v01/castell_plaster_gk_v01_cc.dds"}, {"audio": 0, "start": 8432436, "crunched": 0, "end": 8519972, "filename": "/packages/gk/fantasy/castell_plaster_gk_v01/castell_plaster_gk_v01_nm.dds"}, {"audio": 0, "start": 8519972, "crunched": 0, "end": 8694900, "filename": "/packages/gk/fantasy/iron_plates_gk_v01/iron_plates_gk_v01_cc.dds"}, {"audio": 0, "start": 8694900, "crunched": 0, "end": 8869828, "filename": "/packages/gk/fantasy/iron_plates_gk_v01/iron_plates_gk_v01_nm.dds"}, {"audio": 0, "start": 8869828, "crunched": 0, "end": 8913748, "filename": "/packages/gk/fantasy/iron_trim_gk_v01/iron_trim_gk_v01_cc.dds"}, {"audio": 0, "start": 8913748, "crunched": 0, "end": 8957668, "filename": "/packages/gk/fantasy/iron_trim_gk_v01/iron_trim_gk_v01_nm.dds"}, {"audio": 0, "start": 8957668, "crunched": 0, "end": 8979668, "filename": "/packages/gk/fantasy/iron_intersection_gk_v01/iron_intersection_gk_v01_cc.dds"}, {"audio": 0, "start": 8979668, "crunched": 0, "end": 9067204, "filename": "/packages/gk/fantasy/iron_intersection_gk_v01/iron_intersection_gk_v01_nm.dds"}], "remote_package_size": 9067204, "package_uuid": "b76b7734-b4b1-450d-9784-6adc4d27b624"});

})();

