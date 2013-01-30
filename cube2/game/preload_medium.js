
(function() {


  if (typeof Module == 'undefined') Module = {};
  if (!Module['preRun']) Module['preRun'] = [];
  Module["preRun"].push(function() {


function assert(check, msg) {
  if (!check) throw msg + new Error().stack;
}
Module['FS_createPath']('/', 'packages', true, true);
Module['FS_createPath']('/packages', 'base', true, true);
Module['FS_createPath']('/packages', 'gk', true, true);
Module['FS_createPath']('/packages/gk', 'fantasy', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'castell_plaster_gk_v01', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'iron_intersection_gk_v01', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'castell_wall_gk_v02', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'castell_wall_gk_v03', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'castell_wall_trim_gk_v01', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'iron_plates_gk_v01', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'iron_trim_gk_v01', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'wooden_planks_gk_v01', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'rock_formation_gk_v02', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'castell_wall_gk_v01', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'rock_formation_gk_v01', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'wooden_roof_tiles_gk_v01', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'stone_ground_gk_v01', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'stone_ground_tiles_gk_v01', true, true);
Module['FS_createPath']('/packages/gk/fantasy', 'skyfantasyJPG', true, true);

    function DataRequest() {}
    DataRequest.prototype = {
      requests: {},
      open: function(mode, name) {
        this.requests[name] = this;
      },
      send: function() {}
    };
  
    var filePreload0 = new DataRequest();
    filePreload0.open('GET', 'packages/base/two_towers.ogz', true);
    filePreload0.responseType = 'arraybuffer';
    filePreload0.onload = function() {
      var arrayBuffer = filePreload0.response;
      assert(arrayBuffer, 'Loading file packages/base/two_towers.ogz failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/base', 'two_towers.ogz', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/base/two_towers.ogz');

      });
    };
    Module['addRunDependency']('fp packages/base/two_towers.ogz');
    filePreload0.send(null);

    var filePreload1 = new DataRequest();
    filePreload1.open('GET', 'packages/base/two_towers.cfg', true);
    filePreload1.responseType = 'arraybuffer';
    filePreload1.onload = function() {
      var arrayBuffer = filePreload1.response;
      assert(arrayBuffer, 'Loading file packages/base/two_towers.cfg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/base', 'two_towers.cfg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/base/two_towers.cfg');

      });
    };
    Module['addRunDependency']('fp packages/base/two_towers.cfg');
    filePreload1.send(null);

    var filePreload2 = new DataRequest();
    filePreload2.open('GET', 'packages/base/two_towers.wpt', true);
    filePreload2.responseType = 'arraybuffer';
    filePreload2.onload = function() {
      var arrayBuffer = filePreload2.response;
      assert(arrayBuffer, 'Loading file packages/base/two_towers.wpt failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/base', 'two_towers.wpt', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/base/two_towers.wpt');

      });
    };
    Module['addRunDependency']('fp packages/base/two_towers.wpt');
    filePreload2.send(null);

    var filePreload3 = new DataRequest();
    filePreload3.open('GET', 'packages/gk/fantasy/castell_plaster_gk_v01/package.cfg', true);
    filePreload3.responseType = 'arraybuffer';
    filePreload3.onload = function() {
      var arrayBuffer = filePreload3.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/castell_plaster_gk_v01/package.cfg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/castell_plaster_gk_v01', 'package.cfg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/castell_plaster_gk_v01/package.cfg');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/castell_plaster_gk_v01/package.cfg');
    filePreload3.send(null);

    var filePreload4 = new DataRequest();
    filePreload4.open('GET', 'packages/gk/fantasy/iron_intersection_gk_v01/package.cfg', true);
    filePreload4.responseType = 'arraybuffer';
    filePreload4.onload = function() {
      var arrayBuffer = filePreload4.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/iron_intersection_gk_v01/package.cfg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/iron_intersection_gk_v01', 'package.cfg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/iron_intersection_gk_v01/package.cfg');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/iron_intersection_gk_v01/package.cfg');
    filePreload4.send(null);

    var filePreload5 = new DataRequest();
    filePreload5.open('GET', 'packages/gk/fantasy/castell_wall_gk_v02/package.cfg', true);
    filePreload5.responseType = 'arraybuffer';
    filePreload5.onload = function() {
      var arrayBuffer = filePreload5.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/castell_wall_gk_v02/package.cfg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/castell_wall_gk_v02', 'package.cfg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/castell_wall_gk_v02/package.cfg');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/castell_wall_gk_v02/package.cfg');
    filePreload5.send(null);

    var filePreload6 = new DataRequest();
    filePreload6.open('GET', 'packages/gk/fantasy/castell_wall_gk_v03/package.cfg', true);
    filePreload6.responseType = 'arraybuffer';
    filePreload6.onload = function() {
      var arrayBuffer = filePreload6.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/castell_wall_gk_v03/package.cfg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/castell_wall_gk_v03', 'package.cfg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/castell_wall_gk_v03/package.cfg');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/castell_wall_gk_v03/package.cfg');
    filePreload6.send(null);

    var filePreload7 = new DataRequest();
    filePreload7.open('GET', 'packages/gk/fantasy/castell_wall_trim_gk_v01/package.cfg', true);
    filePreload7.responseType = 'arraybuffer';
    filePreload7.onload = function() {
      var arrayBuffer = filePreload7.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/castell_wall_trim_gk_v01/package.cfg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/castell_wall_trim_gk_v01', 'package.cfg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/castell_wall_trim_gk_v01/package.cfg');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/castell_wall_trim_gk_v01/package.cfg');
    filePreload7.send(null);

    var filePreload8 = new DataRequest();
    filePreload8.open('GET', 'packages/gk/fantasy/iron_plates_gk_v01/package.cfg', true);
    filePreload8.responseType = 'arraybuffer';
    filePreload8.onload = function() {
      var arrayBuffer = filePreload8.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/iron_plates_gk_v01/package.cfg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/iron_plates_gk_v01', 'package.cfg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/iron_plates_gk_v01/package.cfg');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/iron_plates_gk_v01/package.cfg');
    filePreload8.send(null);

    var filePreload9 = new DataRequest();
    filePreload9.open('GET', 'packages/gk/fantasy/iron_trim_gk_v01/package.cfg', true);
    filePreload9.responseType = 'arraybuffer';
    filePreload9.onload = function() {
      var arrayBuffer = filePreload9.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/iron_trim_gk_v01/package.cfg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/iron_trim_gk_v01', 'package.cfg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/iron_trim_gk_v01/package.cfg');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/iron_trim_gk_v01/package.cfg');
    filePreload9.send(null);

    var filePreload10 = new DataRequest();
    filePreload10.open('GET', 'packages/gk/fantasy/wooden_planks_gk_v01/package.cfg', true);
    filePreload10.responseType = 'arraybuffer';
    filePreload10.onload = function() {
      var arrayBuffer = filePreload10.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/wooden_planks_gk_v01/package.cfg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/wooden_planks_gk_v01', 'package.cfg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/wooden_planks_gk_v01/package.cfg');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/wooden_planks_gk_v01/package.cfg');
    filePreload10.send(null);

    var filePreload11 = new DataRequest();
    filePreload11.open('GET', 'packages/gk/fantasy/rock_formation_gk_v02/package.cfg', true);
    filePreload11.responseType = 'arraybuffer';
    filePreload11.onload = function() {
      var arrayBuffer = filePreload11.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/rock_formation_gk_v02/package.cfg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/rock_formation_gk_v02', 'package.cfg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/rock_formation_gk_v02/package.cfg');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/rock_formation_gk_v02/package.cfg');
    filePreload11.send(null);

    var filePreload12 = new DataRequest();
    filePreload12.open('GET', 'packages/gk/fantasy/package.cfg', true);
    filePreload12.responseType = 'arraybuffer';
    filePreload12.onload = function() {
      var arrayBuffer = filePreload12.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/package.cfg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy', 'package.cfg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/package.cfg');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/package.cfg');
    filePreload12.send(null);

    var filePreload13 = new DataRequest();
    filePreload13.open('GET', 'packages/gk/fantasy/castell_wall_gk_v01/package.cfg', true);
    filePreload13.responseType = 'arraybuffer';
    filePreload13.onload = function() {
      var arrayBuffer = filePreload13.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/castell_wall_gk_v01/package.cfg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/castell_wall_gk_v01', 'package.cfg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/castell_wall_gk_v01/package.cfg');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/castell_wall_gk_v01/package.cfg');
    filePreload13.send(null);

    var filePreload14 = new DataRequest();
    filePreload14.open('GET', 'packages/gk/fantasy/rock_formation_gk_v01/package.cfg', true);
    filePreload14.responseType = 'arraybuffer';
    filePreload14.onload = function() {
      var arrayBuffer = filePreload14.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/rock_formation_gk_v01/package.cfg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/rock_formation_gk_v01', 'package.cfg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/rock_formation_gk_v01/package.cfg');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/rock_formation_gk_v01/package.cfg');
    filePreload14.send(null);

    var filePreload15 = new DataRequest();
    filePreload15.open('GET', 'packages/gk/fantasy/wooden_roof_tiles_gk_v01/package.cfg', true);
    filePreload15.responseType = 'arraybuffer';
    filePreload15.onload = function() {
      var arrayBuffer = filePreload15.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/wooden_roof_tiles_gk_v01/package.cfg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/wooden_roof_tiles_gk_v01', 'package.cfg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/wooden_roof_tiles_gk_v01/package.cfg');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/wooden_roof_tiles_gk_v01/package.cfg');
    filePreload15.send(null);

    var filePreload16 = new DataRequest();
    filePreload16.open('GET', 'packages/gk/fantasy/stone_ground_gk_v01/package.cfg', true);
    filePreload16.responseType = 'arraybuffer';
    filePreload16.onload = function() {
      var arrayBuffer = filePreload16.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/stone_ground_gk_v01/package.cfg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/stone_ground_gk_v01', 'package.cfg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/stone_ground_gk_v01/package.cfg');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/stone_ground_gk_v01/package.cfg');
    filePreload16.send(null);

    var filePreload17 = new DataRequest();
    filePreload17.open('GET', 'packages/gk/fantasy/stone_ground_tiles_gk_v01/package.cfg', true);
    filePreload17.responseType = 'arraybuffer';
    filePreload17.onload = function() {
      var arrayBuffer = filePreload17.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/stone_ground_tiles_gk_v01/package.cfg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/stone_ground_tiles_gk_v01', 'package.cfg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/stone_ground_tiles_gk_v01/package.cfg');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/stone_ground_tiles_gk_v01/package.cfg');
    filePreload17.send(null);

    var filePreload18 = new DataRequest();
    filePreload18.open('GET', 'packages/gk/fantasy/rock_formation_gk_v01/rock_formation_gk_v01_cc.dds', true);
    filePreload18.responseType = 'arraybuffer';
    filePreload18.onload = function() {
      var arrayBuffer = filePreload18.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/rock_formation_gk_v01/rock_formation_gk_v01_cc.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/rock_formation_gk_v01', 'rock_formation_gk_v01_cc.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/rock_formation_gk_v01/rock_formation_gk_v01_cc.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/rock_formation_gk_v01/rock_formation_gk_v01_cc.dds');
    filePreload18.send(null);

    var filePreload19 = new DataRequest();
    filePreload19.open('GET', 'packages/gk/fantasy/rock_formation_gk_v01/rock_formation_gk_v01_nm.dds', true);
    filePreload19.responseType = 'arraybuffer';
    filePreload19.onload = function() {
      var arrayBuffer = filePreload19.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/rock_formation_gk_v01/rock_formation_gk_v01_nm.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/rock_formation_gk_v01', 'rock_formation_gk_v01_nm.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/rock_formation_gk_v01/rock_formation_gk_v01_nm.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/rock_formation_gk_v01/rock_formation_gk_v01_nm.dds');
    filePreload19.send(null);

    var filePreload20 = new DataRequest();
    filePreload20.open('GET', 'packages/gk/fantasy/rock_formation_gk_v02/rock_formation_gk_v02_cc.dds', true);
    filePreload20.responseType = 'arraybuffer';
    filePreload20.onload = function() {
      var arrayBuffer = filePreload20.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/rock_formation_gk_v02/rock_formation_gk_v02_cc.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/rock_formation_gk_v02', 'rock_formation_gk_v02_cc.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/rock_formation_gk_v02/rock_formation_gk_v02_cc.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/rock_formation_gk_v02/rock_formation_gk_v02_cc.dds');
    filePreload20.send(null);

    var filePreload21 = new DataRequest();
    filePreload21.open('GET', 'packages/gk/fantasy/rock_formation_gk_v02/rock_formation_gk_v02_nm.dds', true);
    filePreload21.responseType = 'arraybuffer';
    filePreload21.onload = function() {
      var arrayBuffer = filePreload21.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/rock_formation_gk_v02/rock_formation_gk_v02_nm.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/rock_formation_gk_v02', 'rock_formation_gk_v02_nm.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/rock_formation_gk_v02/rock_formation_gk_v02_nm.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/rock_formation_gk_v02/rock_formation_gk_v02_nm.dds');
    filePreload21.send(null);

    var filePreload22 = new DataRequest();
    filePreload22.open('GET', 'packages/gk/fantasy/castell_wall_gk_v01/castell_wall_gk_v01_cc.dds', true);
    filePreload22.responseType = 'arraybuffer';
    filePreload22.onload = function() {
      var arrayBuffer = filePreload22.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/castell_wall_gk_v01/castell_wall_gk_v01_cc.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/castell_wall_gk_v01', 'castell_wall_gk_v01_cc.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/castell_wall_gk_v01/castell_wall_gk_v01_cc.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/castell_wall_gk_v01/castell_wall_gk_v01_cc.dds');
    filePreload22.send(null);

    var filePreload23 = new DataRequest();
    filePreload23.open('GET', 'packages/gk/fantasy/castell_wall_gk_v01/castell_wall_gk_v01_nm.dds', true);
    filePreload23.responseType = 'arraybuffer';
    filePreload23.onload = function() {
      var arrayBuffer = filePreload23.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/castell_wall_gk_v01/castell_wall_gk_v01_nm.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/castell_wall_gk_v01', 'castell_wall_gk_v01_nm.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/castell_wall_gk_v01/castell_wall_gk_v01_nm.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/castell_wall_gk_v01/castell_wall_gk_v01_nm.dds');
    filePreload23.send(null);

    var filePreload24 = new DataRequest();
    filePreload24.open('GET', 'packages/gk/fantasy/castell_wall_gk_v02/castell_wall_gk_v02_cc.dds', true);
    filePreload24.responseType = 'arraybuffer';
    filePreload24.onload = function() {
      var arrayBuffer = filePreload24.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/castell_wall_gk_v02/castell_wall_gk_v02_cc.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/castell_wall_gk_v02', 'castell_wall_gk_v02_cc.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/castell_wall_gk_v02/castell_wall_gk_v02_cc.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/castell_wall_gk_v02/castell_wall_gk_v02_cc.dds');
    filePreload24.send(null);

    var filePreload25 = new DataRequest();
    filePreload25.open('GET', 'packages/gk/fantasy/castell_wall_gk_v02/castell_wall_gk_v02_nm.dds', true);
    filePreload25.responseType = 'arraybuffer';
    filePreload25.onload = function() {
      var arrayBuffer = filePreload25.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/castell_wall_gk_v02/castell_wall_gk_v02_nm.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/castell_wall_gk_v02', 'castell_wall_gk_v02_nm.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/castell_wall_gk_v02/castell_wall_gk_v02_nm.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/castell_wall_gk_v02/castell_wall_gk_v02_nm.dds');
    filePreload25.send(null);

    var filePreload26 = new DataRequest();
    filePreload26.open('GET', 'packages/gk/fantasy/castell_wall_trim_gk_v01/castell_wall_trim_gk_v01_cc.dds', true);
    filePreload26.responseType = 'arraybuffer';
    filePreload26.onload = function() {
      var arrayBuffer = filePreload26.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/castell_wall_trim_gk_v01/castell_wall_trim_gk_v01_cc.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/castell_wall_trim_gk_v01', 'castell_wall_trim_gk_v01_cc.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/castell_wall_trim_gk_v01/castell_wall_trim_gk_v01_cc.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/castell_wall_trim_gk_v01/castell_wall_trim_gk_v01_cc.dds');
    filePreload26.send(null);

    var filePreload27 = new DataRequest();
    filePreload27.open('GET', 'packages/gk/fantasy/castell_wall_trim_gk_v01/castell_wall_trim_gk_v01_nm.dds', true);
    filePreload27.responseType = 'arraybuffer';
    filePreload27.onload = function() {
      var arrayBuffer = filePreload27.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/castell_wall_trim_gk_v01/castell_wall_trim_gk_v01_nm.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/castell_wall_trim_gk_v01', 'castell_wall_trim_gk_v01_nm.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/castell_wall_trim_gk_v01/castell_wall_trim_gk_v01_nm.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/castell_wall_trim_gk_v01/castell_wall_trim_gk_v01_nm.dds');
    filePreload27.send(null);

    var filePreload28 = new DataRequest();
    filePreload28.open('GET', 'packages/gk/fantasy/stone_ground_gk_v01/stone_ground_gk_v01_cc.dds', true);
    filePreload28.responseType = 'arraybuffer';
    filePreload28.onload = function() {
      var arrayBuffer = filePreload28.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/stone_ground_gk_v01/stone_ground_gk_v01_cc.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/stone_ground_gk_v01', 'stone_ground_gk_v01_cc.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/stone_ground_gk_v01/stone_ground_gk_v01_cc.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/stone_ground_gk_v01/stone_ground_gk_v01_cc.dds');
    filePreload28.send(null);

    var filePreload29 = new DataRequest();
    filePreload29.open('GET', 'packages/gk/fantasy/stone_ground_gk_v01/stone_ground_gk_v01_nm.dds', true);
    filePreload29.responseType = 'arraybuffer';
    filePreload29.onload = function() {
      var arrayBuffer = filePreload29.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/stone_ground_gk_v01/stone_ground_gk_v01_nm.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/stone_ground_gk_v01', 'stone_ground_gk_v01_nm.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/stone_ground_gk_v01/stone_ground_gk_v01_nm.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/stone_ground_gk_v01/stone_ground_gk_v01_nm.dds');
    filePreload29.send(null);

    var filePreload30 = new DataRequest();
    filePreload30.open('GET', 'packages/gk/fantasy/stone_ground_tiles_gk_v01/stone_ground_tiles_gk_v01_cc.dds', true);
    filePreload30.responseType = 'arraybuffer';
    filePreload30.onload = function() {
      var arrayBuffer = filePreload30.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/stone_ground_tiles_gk_v01/stone_ground_tiles_gk_v01_cc.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/stone_ground_tiles_gk_v01', 'stone_ground_tiles_gk_v01_cc.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/stone_ground_tiles_gk_v01/stone_ground_tiles_gk_v01_cc.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/stone_ground_tiles_gk_v01/stone_ground_tiles_gk_v01_cc.dds');
    filePreload30.send(null);

    var filePreload31 = new DataRequest();
    filePreload31.open('GET', 'packages/gk/fantasy/stone_ground_tiles_gk_v01/stone_ground_tiles_gk_v01_nm.dds', true);
    filePreload31.responseType = 'arraybuffer';
    filePreload31.onload = function() {
      var arrayBuffer = filePreload31.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/stone_ground_tiles_gk_v01/stone_ground_tiles_gk_v01_nm.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/stone_ground_tiles_gk_v01', 'stone_ground_tiles_gk_v01_nm.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/stone_ground_tiles_gk_v01/stone_ground_tiles_gk_v01_nm.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/stone_ground_tiles_gk_v01/stone_ground_tiles_gk_v01_nm.dds');
    filePreload31.send(null);

    var filePreload32 = new DataRequest();
    filePreload32.open('GET', 'packages/gk/fantasy/wooden_roof_tiles_gk_v01/wooden_roof_tiles_gk_v01_cc.dds', true);
    filePreload32.responseType = 'arraybuffer';
    filePreload32.onload = function() {
      var arrayBuffer = filePreload32.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/wooden_roof_tiles_gk_v01/wooden_roof_tiles_gk_v01_cc.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/wooden_roof_tiles_gk_v01', 'wooden_roof_tiles_gk_v01_cc.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/wooden_roof_tiles_gk_v01/wooden_roof_tiles_gk_v01_cc.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/wooden_roof_tiles_gk_v01/wooden_roof_tiles_gk_v01_cc.dds');
    filePreload32.send(null);

    var filePreload33 = new DataRequest();
    filePreload33.open('GET', 'packages/gk/fantasy/wooden_roof_tiles_gk_v01/wooden_roof_tiles_gk_v01_nm.dds', true);
    filePreload33.responseType = 'arraybuffer';
    filePreload33.onload = function() {
      var arrayBuffer = filePreload33.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/wooden_roof_tiles_gk_v01/wooden_roof_tiles_gk_v01_nm.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/wooden_roof_tiles_gk_v01', 'wooden_roof_tiles_gk_v01_nm.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/wooden_roof_tiles_gk_v01/wooden_roof_tiles_gk_v01_nm.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/wooden_roof_tiles_gk_v01/wooden_roof_tiles_gk_v01_nm.dds');
    filePreload33.send(null);

    var filePreload34 = new DataRequest();
    filePreload34.open('GET', 'packages/gk/fantasy/wooden_planks_gk_v01/wooden_planks_gk_v01_cc.dds', true);
    filePreload34.responseType = 'arraybuffer';
    filePreload34.onload = function() {
      var arrayBuffer = filePreload34.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/wooden_planks_gk_v01/wooden_planks_gk_v01_cc.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/wooden_planks_gk_v01', 'wooden_planks_gk_v01_cc.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/wooden_planks_gk_v01/wooden_planks_gk_v01_cc.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/wooden_planks_gk_v01/wooden_planks_gk_v01_cc.dds');
    filePreload34.send(null);

    var filePreload35 = new DataRequest();
    filePreload35.open('GET', 'packages/gk/fantasy/wooden_planks_gk_v01/wooden_planks_gk_v01_nm.dds', true);
    filePreload35.responseType = 'arraybuffer';
    filePreload35.onload = function() {
      var arrayBuffer = filePreload35.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/wooden_planks_gk_v01/wooden_planks_gk_v01_nm.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/wooden_planks_gk_v01', 'wooden_planks_gk_v01_nm.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/wooden_planks_gk_v01/wooden_planks_gk_v01_nm.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/wooden_planks_gk_v01/wooden_planks_gk_v01_nm.dds');
    filePreload35.send(null);

    var filePreload36 = new DataRequest();
    filePreload36.open('GET', 'packages/gk/fantasy/castell_plaster_gk_v01/castell_plaster_gk_v01_cc.dds', true);
    filePreload36.responseType = 'arraybuffer';
    filePreload36.onload = function() {
      var arrayBuffer = filePreload36.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/castell_plaster_gk_v01/castell_plaster_gk_v01_cc.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/castell_plaster_gk_v01', 'castell_plaster_gk_v01_cc.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/castell_plaster_gk_v01/castell_plaster_gk_v01_cc.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/castell_plaster_gk_v01/castell_plaster_gk_v01_cc.dds');
    filePreload36.send(null);

    var filePreload37 = new DataRequest();
    filePreload37.open('GET', 'packages/gk/fantasy/castell_plaster_gk_v01/castell_plaster_gk_v01_nm.dds', true);
    filePreload37.responseType = 'arraybuffer';
    filePreload37.onload = function() {
      var arrayBuffer = filePreload37.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/castell_plaster_gk_v01/castell_plaster_gk_v01_nm.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/castell_plaster_gk_v01', 'castell_plaster_gk_v01_nm.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/castell_plaster_gk_v01/castell_plaster_gk_v01_nm.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/castell_plaster_gk_v01/castell_plaster_gk_v01_nm.dds');
    filePreload37.send(null);

    var filePreload38 = new DataRequest();
    filePreload38.open('GET', 'packages/gk/fantasy/iron_plates_gk_v01/iron_plates_gk_v01_cc.dds', true);
    filePreload38.responseType = 'arraybuffer';
    filePreload38.onload = function() {
      var arrayBuffer = filePreload38.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/iron_plates_gk_v01/iron_plates_gk_v01_cc.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/iron_plates_gk_v01', 'iron_plates_gk_v01_cc.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/iron_plates_gk_v01/iron_plates_gk_v01_cc.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/iron_plates_gk_v01/iron_plates_gk_v01_cc.dds');
    filePreload38.send(null);

    var filePreload39 = new DataRequest();
    filePreload39.open('GET', 'packages/gk/fantasy/iron_plates_gk_v01/iron_plates_gk_v01_nm.dds', true);
    filePreload39.responseType = 'arraybuffer';
    filePreload39.onload = function() {
      var arrayBuffer = filePreload39.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/iron_plates_gk_v01/iron_plates_gk_v01_nm.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/iron_plates_gk_v01', 'iron_plates_gk_v01_nm.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/iron_plates_gk_v01/iron_plates_gk_v01_nm.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/iron_plates_gk_v01/iron_plates_gk_v01_nm.dds');
    filePreload39.send(null);

    var filePreload40 = new DataRequest();
    filePreload40.open('GET', 'packages/gk/fantasy/iron_trim_gk_v01/iron_trim_gk_v01_cc.dds', true);
    filePreload40.responseType = 'arraybuffer';
    filePreload40.onload = function() {
      var arrayBuffer = filePreload40.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/iron_trim_gk_v01/iron_trim_gk_v01_cc.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/iron_trim_gk_v01', 'iron_trim_gk_v01_cc.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/iron_trim_gk_v01/iron_trim_gk_v01_cc.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/iron_trim_gk_v01/iron_trim_gk_v01_cc.dds');
    filePreload40.send(null);

    var filePreload41 = new DataRequest();
    filePreload41.open('GET', 'packages/gk/fantasy/iron_trim_gk_v01/iron_trim_gk_v01_nm.dds', true);
    filePreload41.responseType = 'arraybuffer';
    filePreload41.onload = function() {
      var arrayBuffer = filePreload41.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/iron_trim_gk_v01/iron_trim_gk_v01_nm.dds failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/iron_trim_gk_v01', 'iron_trim_gk_v01_nm.dds', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/iron_trim_gk_v01/iron_trim_gk_v01_nm.dds');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/iron_trim_gk_v01/iron_trim_gk_v01_nm.dds');
    filePreload41.send(null);

    var filePreload42 = new DataRequest();
    filePreload42.open('GET', 'packages/gk/fantasy/skyfantasyJPG/skyfantasy_up.jpg', true);
    filePreload42.responseType = 'arraybuffer';
    filePreload42.onload = function() {
      var arrayBuffer = filePreload42.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/skyfantasyJPG/skyfantasy_up.jpg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/skyfantasyJPG', 'skyfantasy_up.jpg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/skyfantasyJPG/skyfantasy_up.jpg');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/skyfantasyJPG/skyfantasy_up.jpg');
    filePreload42.send(null);

    var filePreload43 = new DataRequest();
    filePreload43.open('GET', 'packages/gk/fantasy/skyfantasyJPG/skyfantasy_bk.jpg', true);
    filePreload43.responseType = 'arraybuffer';
    filePreload43.onload = function() {
      var arrayBuffer = filePreload43.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/skyfantasyJPG/skyfantasy_bk.jpg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/skyfantasyJPG', 'skyfantasy_bk.jpg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/skyfantasyJPG/skyfantasy_bk.jpg');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/skyfantasyJPG/skyfantasy_bk.jpg');
    filePreload43.send(null);

    var filePreload44 = new DataRequest();
    filePreload44.open('GET', 'packages/gk/fantasy/skyfantasyJPG/skyfantasy_dn.jpg', true);
    filePreload44.responseType = 'arraybuffer';
    filePreload44.onload = function() {
      var arrayBuffer = filePreload44.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/skyfantasyJPG/skyfantasy_dn.jpg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/skyfantasyJPG', 'skyfantasy_dn.jpg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/skyfantasyJPG/skyfantasy_dn.jpg');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/skyfantasyJPG/skyfantasy_dn.jpg');
    filePreload44.send(null);

    var filePreload45 = new DataRequest();
    filePreload45.open('GET', 'packages/gk/fantasy/skyfantasyJPG/skyfantasy_rt.jpg', true);
    filePreload45.responseType = 'arraybuffer';
    filePreload45.onload = function() {
      var arrayBuffer = filePreload45.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/skyfantasyJPG/skyfantasy_rt.jpg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/skyfantasyJPG', 'skyfantasy_rt.jpg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/skyfantasyJPG/skyfantasy_rt.jpg');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/skyfantasyJPG/skyfantasy_rt.jpg');
    filePreload45.send(null);

    var filePreload46 = new DataRequest();
    filePreload46.open('GET', 'packages/gk/fantasy/skyfantasyJPG/skyfantasy_lf.jpg', true);
    filePreload46.responseType = 'arraybuffer';
    filePreload46.onload = function() {
      var arrayBuffer = filePreload46.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/skyfantasyJPG/skyfantasy_lf.jpg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/skyfantasyJPG', 'skyfantasy_lf.jpg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/skyfantasyJPG/skyfantasy_lf.jpg');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/skyfantasyJPG/skyfantasy_lf.jpg');
    filePreload46.send(null);

    var filePreload47 = new DataRequest();
    filePreload47.open('GET', 'packages/gk/fantasy/skyfantasyJPG/skyfantasy_ft.jpg', true);
    filePreload47.responseType = 'arraybuffer';
    filePreload47.onload = function() {
      var arrayBuffer = filePreload47.response;
      assert(arrayBuffer, 'Loading file packages/gk/fantasy/skyfantasyJPG/skyfantasy_ft.jpg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/fantasy/skyfantasyJPG', 'skyfantasy_ft.jpg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/fantasy/skyfantasyJPG/skyfantasy_ft.jpg');

      });
    };
    Module['addRunDependency']('fp packages/gk/fantasy/skyfantasyJPG/skyfantasy_ft.jpg');
    filePreload47.send(null);

    if (!Module.expectedDataFileDownloads) {
      Module.expectedDataFileDownloads = 0;
      Module.finishedDataFileDownloads = 0;
    }
    Module.expectedDataFileDownloads++;

    var dataFile = new XMLHttpRequest();
    dataFile.onprogress = function(event) {
      var url = 'medium.data';
      if (event.loaded && event.total) {
        if (!dataFile.addedTotal) {
          dataFile.addedTotal = true;
          if (!Module.dataFileDownloads) Module.dataFileDownloads = {};
          Module.dataFileDownloads[url] = {
            loaded: event.loaded,
            total: event.total
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
        Module['setStatus']('Downloading data... (' + loaded + '/' + total + ')');
      } else if (!Module.dataFileDownloads) {
        Module['setStatus']('Downloading data...');
      }
    }
    dataFile.open('GET', 'medium.data', true);
    dataFile.responseType = 'arraybuffer';
    dataFile.onload = function() {
      Module.finishedDataFileDownloads++;
      var arrayBuffer = dataFile.response;
      assert(arrayBuffer, 'Loading data file failed.');
      var byteArray = new Uint8Array(arrayBuffer);
      var curr;
      
        curr = DataRequest.prototype.requests['packages/base/two_towers.ogz'];
        curr.response = byteArray.subarray(0,4195600);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/base/two_towers.cfg'];
        curr.response = byteArray.subarray(4195600,4196688);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/base/two_towers.wpt'];
        curr.response = byteArray.subarray(4196688,4213217);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/castell_plaster_gk_v01/package.cfg'];
        curr.response = byteArray.subarray(4213217,4213664);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/iron_intersection_gk_v01/package.cfg'];
        curr.response = byteArray.subarray(4213664,4214123);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/castell_wall_gk_v02/package.cfg'];
        curr.response = byteArray.subarray(4214123,4214842);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/castell_wall_gk_v03/package.cfg'];
        curr.response = byteArray.subarray(4214842,4215561);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/castell_wall_trim_gk_v01/package.cfg'];
        curr.response = byteArray.subarray(4215561,4216360);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/iron_plates_gk_v01/package.cfg'];
        curr.response = byteArray.subarray(4216360,4217062);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/iron_trim_gk_v01/package.cfg'];
        curr.response = byteArray.subarray(4217062,4217734);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/wooden_planks_gk_v01/package.cfg'];
        curr.response = byteArray.subarray(4217734,4218468);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/rock_formation_gk_v02/package.cfg'];
        curr.response = byteArray.subarray(4218468,4218907);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/package.cfg'];
        curr.response = byteArray.subarray(4218907,4219331);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/castell_wall_gk_v01/package.cfg'];
        curr.response = byteArray.subarray(4219331,4220050);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/rock_formation_gk_v01/package.cfg'];
        curr.response = byteArray.subarray(4220050,4220489);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/wooden_roof_tiles_gk_v01/package.cfg'];
        curr.response = byteArray.subarray(4220489,4221287);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/stone_ground_gk_v01/package.cfg'];
        curr.response = byteArray.subarray(4221287,4221710);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/stone_ground_tiles_gk_v01/package.cfg'];
        curr.response = byteArray.subarray(4221710,4222181);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/rock_formation_gk_v01/rock_formation_gk_v01_cc.dds'];
        curr.response = byteArray.subarray(4222181,4571861);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/rock_formation_gk_v01/rock_formation_gk_v01_nm.dds'];
        curr.response = byteArray.subarray(4571861,4659397);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/rock_formation_gk_v02/rock_formation_gk_v02_cc.dds'];
        curr.response = byteArray.subarray(4659397,5009077);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/rock_formation_gk_v02/rock_formation_gk_v02_nm.dds'];
        curr.response = byteArray.subarray(5009077,5096613);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/castell_wall_gk_v01/castell_wall_gk_v01_cc.dds'];
        curr.response = byteArray.subarray(5096613,5184149);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/castell_wall_gk_v01/castell_wall_gk_v01_nm.dds'];
        curr.response = byteArray.subarray(5184149,5271685);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/castell_wall_gk_v02/castell_wall_gk_v02_cc.dds'];
        curr.response = byteArray.subarray(5271685,5359221);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/castell_wall_gk_v02/castell_wall_gk_v02_nm.dds'];
        curr.response = byteArray.subarray(5359221,5446757);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/castell_wall_trim_gk_v01/castell_wall_trim_gk_v01_cc.dds'];
        curr.response = byteArray.subarray(5446757,5490677);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/castell_wall_trim_gk_v01/castell_wall_trim_gk_v01_nm.dds'];
        curr.response = byteArray.subarray(5490677,5534597);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/stone_ground_gk_v01/stone_ground_gk_v01_cc.dds'];
        curr.response = byteArray.subarray(5534597,5884277);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/stone_ground_gk_v01/stone_ground_gk_v01_nm.dds'];
        curr.response = byteArray.subarray(5884277,6233957);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/stone_ground_tiles_gk_v01/stone_ground_tiles_gk_v01_cc.dds'];
        curr.response = byteArray.subarray(6233957,6583637);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/stone_ground_tiles_gk_v01/stone_ground_tiles_gk_v01_nm.dds'];
        curr.response = byteArray.subarray(6583637,6671173);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/wooden_roof_tiles_gk_v01/wooden_roof_tiles_gk_v01_cc.dds'];
        curr.response = byteArray.subarray(6671173,7020853);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/wooden_roof_tiles_gk_v01/wooden_roof_tiles_gk_v01_nm.dds'];
        curr.response = byteArray.subarray(7020853,7108389);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/wooden_planks_gk_v01/wooden_planks_gk_v01_cc.dds'];
        curr.response = byteArray.subarray(7108389,7195925);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/wooden_planks_gk_v01/wooden_planks_gk_v01_nm.dds'];
        curr.response = byteArray.subarray(7195925,7283461);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/castell_plaster_gk_v01/castell_plaster_gk_v01_cc.dds'];
        curr.response = byteArray.subarray(7283461,7370997);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/castell_plaster_gk_v01/castell_plaster_gk_v01_nm.dds'];
        curr.response = byteArray.subarray(7370997,7458533);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/iron_plates_gk_v01/iron_plates_gk_v01_cc.dds'];
        curr.response = byteArray.subarray(7458533,7633461);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/iron_plates_gk_v01/iron_plates_gk_v01_nm.dds'];
        curr.response = byteArray.subarray(7633461,7808389);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/iron_trim_gk_v01/iron_trim_gk_v01_cc.dds'];
        curr.response = byteArray.subarray(7808389,7852309);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/iron_trim_gk_v01/iron_trim_gk_v01_nm.dds'];
        curr.response = byteArray.subarray(7852309,7896229);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/skyfantasyJPG/skyfantasy_up.jpg'];
        curr.response = byteArray.subarray(7896229,7979070);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/skyfantasyJPG/skyfantasy_bk.jpg'];
        curr.response = byteArray.subarray(7979070,8135516);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/skyfantasyJPG/skyfantasy_dn.jpg'];
        curr.response = byteArray.subarray(8135516,8320967);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/skyfantasyJPG/skyfantasy_rt.jpg'];
        curr.response = byteArray.subarray(8320967,8470406);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/skyfantasyJPG/skyfantasy_lf.jpg'];
        curr.response = byteArray.subarray(8470406,8619468);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/fantasy/skyfantasyJPG/skyfantasy_ft.jpg'];
        curr.response = byteArray.subarray(8619468,8773303);
        curr.onload();
                Module['removeRunDependency']('datafile_medium.data');

    };
    Module['addRunDependency']('datafile_medium.data');
    dataFile.send(null);
    if (Module['setStatus']) Module['setStatus']('Downloading...');
  
  });


})();

