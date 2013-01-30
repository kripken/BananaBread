
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
Module['FS_createPath']('/packages/gk', 'cyber', true, true);
Module['FS_createPath']('/packages/gk/cyber', 'cyberskydark', true, true);
Module['FS_createPath']('/packages/gk/cyber', 'plain_colors', true, true);
Module['FS_createPath']('/packages/gk/cyber', 'cybersky', true, true);
Module['FS_createPath']('/packages/gk/cyber', 'glow_colors', true, true);

    function DataRequest() {}
    DataRequest.prototype = {
      requests: {},
      open: function(mode, name) {
        this.requests[name] = this;
      },
      send: function() {}
    };
  
    var filePreload0 = new DataRequest();
    filePreload0.open('GET', 'packages/base/cyber1.ogz', true);
    filePreload0.responseType = 'arraybuffer';
    filePreload0.onload = function() {
      var arrayBuffer = filePreload0.response;
      assert(arrayBuffer, 'Loading file packages/base/cyber1.ogz failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/base', 'cyber1.ogz', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/base/cyber1.ogz');

      });
    };
    Module['addRunDependency']('fp packages/base/cyber1.ogz');
    filePreload0.send(null);

    var filePreload1 = new DataRequest();
    filePreload1.open('GET', 'packages/base/cyber1.cfg', true);
    filePreload1.responseType = 'arraybuffer';
    filePreload1.onload = function() {
      var arrayBuffer = filePreload1.response;
      assert(arrayBuffer, 'Loading file packages/base/cyber1.cfg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/base', 'cyber1.cfg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/base/cyber1.cfg');

      });
    };
    Module['addRunDependency']('fp packages/base/cyber1.cfg');
    filePreload1.send(null);

    var filePreload2 = new DataRequest();
    filePreload2.open('GET', 'packages/base/cyber1.wpt', true);
    filePreload2.responseType = 'arraybuffer';
    filePreload2.onload = function() {
      var arrayBuffer = filePreload2.response;
      assert(arrayBuffer, 'Loading file packages/base/cyber1.wpt failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/base', 'cyber1.wpt', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/base/cyber1.wpt');

      });
    };
    Module['addRunDependency']('fp packages/base/cyber1.wpt');
    filePreload2.send(null);

    var filePreload3 = new DataRequest();
    filePreload3.open('GET', 'packages/gk/cyber/cyberskydark/gkskycyber_rt.png', true);
    filePreload3.responseType = 'arraybuffer';
    filePreload3.onload = function() {
      var arrayBuffer = filePreload3.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/cyberskydark/gkskycyber_rt.png failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/cyberskydark', 'gkskycyber_rt.png', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/cyberskydark/gkskycyber_rt.png');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/cyberskydark/gkskycyber_rt.png');
    filePreload3.send(null);

    var filePreload4 = new DataRequest();
    filePreload4.open('GET', 'packages/gk/cyber/cyberskydark/gkskycyber_lf.png', true);
    filePreload4.responseType = 'arraybuffer';
    filePreload4.onload = function() {
      var arrayBuffer = filePreload4.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/cyberskydark/gkskycyber_lf.png failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/cyberskydark', 'gkskycyber_lf.png', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/cyberskydark/gkskycyber_lf.png');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/cyberskydark/gkskycyber_lf.png');
    filePreload4.send(null);

    var filePreload5 = new DataRequest();
    filePreload5.open('GET', 'packages/gk/cyber/cyberskydark/gkskycyber_ft.png', true);
    filePreload5.responseType = 'arraybuffer';
    filePreload5.onload = function() {
      var arrayBuffer = filePreload5.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/cyberskydark/gkskycyber_ft.png failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/cyberskydark', 'gkskycyber_ft.png', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/cyberskydark/gkskycyber_ft.png');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/cyberskydark/gkskycyber_ft.png');
    filePreload5.send(null);

    var filePreload6 = new DataRequest();
    filePreload6.open('GET', 'packages/gk/cyber/cyberskydark/gkskycyber_bk.png', true);
    filePreload6.responseType = 'arraybuffer';
    filePreload6.onload = function() {
      var arrayBuffer = filePreload6.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/cyberskydark/gkskycyber_bk.png failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/cyberskydark', 'gkskycyber_bk.png', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/cyberskydark/gkskycyber_bk.png');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/cyberskydark/gkskycyber_bk.png');
    filePreload6.send(null);

    var filePreload7 = new DataRequest();
    filePreload7.open('GET', 'packages/gk/cyber/cyberskydark/gkskycyber_up.png', true);
    filePreload7.responseType = 'arraybuffer';
    filePreload7.onload = function() {
      var arrayBuffer = filePreload7.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/cyberskydark/gkskycyber_up.png failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/cyberskydark', 'gkskycyber_up.png', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/cyberskydark/gkskycyber_up.png');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/cyberskydark/gkskycyber_up.png');
    filePreload7.send(null);

    var filePreload8 = new DataRequest();
    filePreload8.open('GET', 'packages/gk/cyber/cyberskydark/gkskycyber_dn.png', true);
    filePreload8.responseType = 'arraybuffer';
    filePreload8.onload = function() {
      var arrayBuffer = filePreload8.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/cyberskydark/gkskycyber_dn.png failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/cyberskydark', 'gkskycyber_dn.png', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/cyberskydark/gkskycyber_dn.png');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/cyberskydark/gkskycyber_dn.png');
    filePreload8.send(null);

    var filePreload9 = new DataRequest();
    filePreload9.open('GET', 'packages/gk/cyber/plain_colors/plain_color_05_cc.png', true);
    filePreload9.responseType = 'arraybuffer';
    filePreload9.onload = function() {
      var arrayBuffer = filePreload9.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/plain_colors/plain_color_05_cc.png failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/plain_colors', 'plain_color_05_cc.png', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/plain_colors/plain_color_05_cc.png');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/plain_colors/plain_color_05_cc.png');
    filePreload9.send(null);

    var filePreload10 = new DataRequest();
    filePreload10.open('GET', 'packages/gk/cyber/plain_colors/plain_color_nm.png', true);
    filePreload10.responseType = 'arraybuffer';
    filePreload10.onload = function() {
      var arrayBuffer = filePreload10.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/plain_colors/plain_color_nm.png failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/plain_colors', 'plain_color_nm.png', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/plain_colors/plain_color_nm.png');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/plain_colors/plain_color_nm.png');
    filePreload10.send(null);

    var filePreload11 = new DataRequest();
    filePreload11.open('GET', 'packages/gk/cyber/plain_colors/plain_color_04_cc.png', true);
    filePreload11.responseType = 'arraybuffer';
    filePreload11.onload = function() {
      var arrayBuffer = filePreload11.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/plain_colors/plain_color_04_cc.png failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/plain_colors', 'plain_color_04_cc.png', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/plain_colors/plain_color_04_cc.png');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/plain_colors/plain_color_04_cc.png');
    filePreload11.send(null);

    var filePreload12 = new DataRequest();
    filePreload12.open('GET', 'packages/gk/cyber/plain_colors/plain_color_03_cc.png', true);
    filePreload12.responseType = 'arraybuffer';
    filePreload12.onload = function() {
      var arrayBuffer = filePreload12.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/plain_colors/plain_color_03_cc.png failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/plain_colors', 'plain_color_03_cc.png', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/plain_colors/plain_color_03_cc.png');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/plain_colors/plain_color_03_cc.png');
    filePreload12.send(null);

    var filePreload13 = new DataRequest();
    filePreload13.open('GET', 'packages/gk/cyber/plain_colors/plain_color_01_cc.png', true);
    filePreload13.responseType = 'arraybuffer';
    filePreload13.onload = function() {
      var arrayBuffer = filePreload13.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/plain_colors/plain_color_01_cc.png failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/plain_colors', 'plain_color_01_cc.png', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/plain_colors/plain_color_01_cc.png');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/plain_colors/plain_color_01_cc.png');
    filePreload13.send(null);

    var filePreload14 = new DataRequest();
    filePreload14.open('GET', 'packages/gk/cyber/plain_colors/Thumbs.db', true);
    filePreload14.responseType = 'arraybuffer';
    filePreload14.onload = function() {
      var arrayBuffer = filePreload14.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/plain_colors/Thumbs.db failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/plain_colors', 'Thumbs.db', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/plain_colors/Thumbs.db');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/plain_colors/Thumbs.db');
    filePreload14.send(null);

    var filePreload15 = new DataRequest();
    filePreload15.open('GET', 'packages/gk/cyber/plain_colors/package.cfg', true);
    filePreload15.responseType = 'arraybuffer';
    filePreload15.onload = function() {
      var arrayBuffer = filePreload15.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/plain_colors/package.cfg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/plain_colors', 'package.cfg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/plain_colors/package.cfg');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/plain_colors/package.cfg');
    filePreload15.send(null);

    var filePreload16 = new DataRequest();
    filePreload16.open('GET', 'packages/gk/cyber/plain_colors/plain_color_01_sc.png', true);
    filePreload16.responseType = 'arraybuffer';
    filePreload16.onload = function() {
      var arrayBuffer = filePreload16.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/plain_colors/plain_color_01_sc.png failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/plain_colors', 'plain_color_01_sc.png', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/plain_colors/plain_color_01_sc.png');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/plain_colors/plain_color_01_sc.png');
    filePreload16.send(null);

    var filePreload17 = new DataRequest();
    filePreload17.open('GET', 'packages/gk/cyber/plain_colors/plain_color_02_cc.png', true);
    filePreload17.responseType = 'arraybuffer';
    filePreload17.onload = function() {
      var arrayBuffer = filePreload17.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/plain_colors/plain_color_02_cc.png failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/plain_colors', 'plain_color_02_cc.png', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/plain_colors/plain_color_02_cc.png');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/plain_colors/plain_color_02_cc.png');
    filePreload17.send(null);

    var filePreload18 = new DataRequest();
    filePreload18.open('GET', 'packages/gk/cyber/cybersky/gkskycyber_bk.jpg', true);
    filePreload18.responseType = 'arraybuffer';
    filePreload18.onload = function() {
      var arrayBuffer = filePreload18.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/cybersky/gkskycyber_bk.jpg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/cybersky', 'gkskycyber_bk.jpg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/cybersky/gkskycyber_bk.jpg');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/cybersky/gkskycyber_bk.jpg');
    filePreload18.send(null);

    var filePreload19 = new DataRequest();
    filePreload19.open('GET', 'packages/gk/cyber/cybersky/gkskycyber_rt.jpg', true);
    filePreload19.responseType = 'arraybuffer';
    filePreload19.onload = function() {
      var arrayBuffer = filePreload19.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/cybersky/gkskycyber_rt.jpg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/cybersky', 'gkskycyber_rt.jpg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/cybersky/gkskycyber_rt.jpg');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/cybersky/gkskycyber_rt.jpg');
    filePreload19.send(null);

    var filePreload20 = new DataRequest();
    filePreload20.open('GET', 'packages/gk/cyber/cybersky/gkskycyber_dn.jpg', true);
    filePreload20.responseType = 'arraybuffer';
    filePreload20.onload = function() {
      var arrayBuffer = filePreload20.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/cybersky/gkskycyber_dn.jpg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/cybersky', 'gkskycyber_dn.jpg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/cybersky/gkskycyber_dn.jpg');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/cybersky/gkskycyber_dn.jpg');
    filePreload20.send(null);

    var filePreload21 = new DataRequest();
    filePreload21.open('GET', 'packages/gk/cyber/cybersky/gkskycyber_lf.jpg', true);
    filePreload21.responseType = 'arraybuffer';
    filePreload21.onload = function() {
      var arrayBuffer = filePreload21.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/cybersky/gkskycyber_lf.jpg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/cybersky', 'gkskycyber_lf.jpg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/cybersky/gkskycyber_lf.jpg');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/cybersky/gkskycyber_lf.jpg');
    filePreload21.send(null);

    var filePreload22 = new DataRequest();
    filePreload22.open('GET', 'packages/gk/cyber/cybersky/gkskycyber_ft.jpg', true);
    filePreload22.responseType = 'arraybuffer';
    filePreload22.onload = function() {
      var arrayBuffer = filePreload22.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/cybersky/gkskycyber_ft.jpg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/cybersky', 'gkskycyber_ft.jpg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/cybersky/gkskycyber_ft.jpg');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/cybersky/gkskycyber_ft.jpg');
    filePreload22.send(null);

    var filePreload23 = new DataRequest();
    filePreload23.open('GET', 'packages/gk/cyber/cybersky/gkskycyber_up.jpg', true);
    filePreload23.responseType = 'arraybuffer';
    filePreload23.onload = function() {
      var arrayBuffer = filePreload23.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/cybersky/gkskycyber_up.jpg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/cybersky', 'gkskycyber_up.jpg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/cybersky/gkskycyber_up.jpg');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/cybersky/gkskycyber_up.jpg');
    filePreload23.send(null);

    var filePreload24 = new DataRequest();
    filePreload24.open('GET', 'packages/gk/cyber/glow_colors/glow_color_box_01_gc.png', true);
    filePreload24.responseType = 'arraybuffer';
    filePreload24.onload = function() {
      var arrayBuffer = filePreload24.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/glow_colors/glow_color_box_01_gc.png failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/glow_colors', 'glow_color_box_01_gc.png', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/glow_colors/glow_color_box_01_gc.png');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/glow_colors/glow_color_box_01_gc.png');
    filePreload24.send(null);

    var filePreload25 = new DataRequest();
    filePreload25.open('GET', 'packages/gk/cyber/glow_colors/glow_color_05.png', true);
    filePreload25.responseType = 'arraybuffer';
    filePreload25.onload = function() {
      var arrayBuffer = filePreload25.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/glow_colors/glow_color_05.png failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/glow_colors', 'glow_color_05.png', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/glow_colors/glow_color_05.png');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/glow_colors/glow_color_05.png');
    filePreload25.send(null);

    var filePreload26 = new DataRequest();
    filePreload26.open('GET', 'packages/gk/cyber/glow_colors/glow_color_02.png', true);
    filePreload26.responseType = 'arraybuffer';
    filePreload26.onload = function() {
      var arrayBuffer = filePreload26.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/glow_colors/glow_color_02.png failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/glow_colors', 'glow_color_02.png', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/glow_colors/glow_color_02.png');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/glow_colors/glow_color_02.png');
    filePreload26.send(null);

    var filePreload27 = new DataRequest();
    filePreload27.open('GET', 'packages/gk/cyber/glow_colors/glow_color_box_01_cc.png', true);
    filePreload27.responseType = 'arraybuffer';
    filePreload27.onload = function() {
      var arrayBuffer = filePreload27.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/glow_colors/glow_color_box_01_cc.png failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/glow_colors', 'glow_color_box_01_cc.png', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/glow_colors/glow_color_box_01_cc.png');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/glow_colors/glow_color_box_01_cc.png');
    filePreload27.send(null);

    var filePreload28 = new DataRequest();
    filePreload28.open('GET', 'packages/gk/cyber/glow_colors/Thumbs.db', true);
    filePreload28.responseType = 'arraybuffer';
    filePreload28.onload = function() {
      var arrayBuffer = filePreload28.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/glow_colors/Thumbs.db failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/glow_colors', 'Thumbs.db', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/glow_colors/Thumbs.db');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/glow_colors/Thumbs.db');
    filePreload28.send(null);

    var filePreload29 = new DataRequest();
    filePreload29.open('GET', 'packages/gk/cyber/glow_colors/glow_color_01.png', true);
    filePreload29.responseType = 'arraybuffer';
    filePreload29.onload = function() {
      var arrayBuffer = filePreload29.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/glow_colors/glow_color_01.png failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/glow_colors', 'glow_color_01.png', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/glow_colors/glow_color_01.png');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/glow_colors/glow_color_01.png');
    filePreload29.send(null);

    var filePreload30 = new DataRequest();
    filePreload30.open('GET', 'packages/gk/cyber/glow_colors/package.cfg', true);
    filePreload30.responseType = 'arraybuffer';
    filePreload30.onload = function() {
      var arrayBuffer = filePreload30.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/glow_colors/package.cfg failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/glow_colors', 'package.cfg', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/glow_colors/package.cfg');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/glow_colors/package.cfg');
    filePreload30.send(null);

    var filePreload31 = new DataRequest();
    filePreload31.open('GET', 'packages/gk/cyber/glow_colors/glow_color_nm.png', true);
    filePreload31.responseType = 'arraybuffer';
    filePreload31.onload = function() {
      var arrayBuffer = filePreload31.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/glow_colors/glow_color_nm.png failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/glow_colors', 'glow_color_nm.png', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/glow_colors/glow_color_nm.png');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/glow_colors/glow_color_nm.png');
    filePreload31.send(null);

    var filePreload32 = new DataRequest();
    filePreload32.open('GET', 'packages/gk/cyber/glow_colors/glow_color_04.png', true);
    filePreload32.responseType = 'arraybuffer';
    filePreload32.onload = function() {
      var arrayBuffer = filePreload32.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/glow_colors/glow_color_04.png failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/glow_colors', 'glow_color_04.png', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/glow_colors/glow_color_04.png');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/glow_colors/glow_color_04.png');
    filePreload32.send(null);

    var filePreload33 = new DataRequest();
    filePreload33.open('GET', 'packages/gk/cyber/glow_colors/glow_color_03.png', true);
    filePreload33.responseType = 'arraybuffer';
    filePreload33.onload = function() {
      var arrayBuffer = filePreload33.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/glow_colors/glow_color_03.png failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/glow_colors', 'glow_color_03.png', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/glow_colors/glow_color_03.png');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/glow_colors/glow_color_03.png');
    filePreload33.send(null);

    var filePreload34 = new DataRequest();
    filePreload34.open('GET', 'packages/gk/cyber/glow_colors/glow_color_01_sc.png', true);
    filePreload34.responseType = 'arraybuffer';
    filePreload34.onload = function() {
      var arrayBuffer = filePreload34.response;
      assert(arrayBuffer, 'Loading file packages/gk/cyber/glow_colors/glow_color_01_sc.png failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      
      Module['FS_createPreloadedFile']('/packages/gk/cyber/glow_colors', 'glow_color_01_sc.png', byteArray, true, true, function() {
        Module['removeRunDependency']('fp packages/gk/cyber/glow_colors/glow_color_01_sc.png');

      });
    };
    Module['addRunDependency']('fp packages/gk/cyber/glow_colors/glow_color_01_sc.png');
    filePreload34.send(null);

    if (!Module.expectedDataFileDownloads) {
      Module.expectedDataFileDownloads = 0;
      Module.finishedDataFileDownloads = 0;
    }
    Module.expectedDataFileDownloads++;

    var dataFile = new XMLHttpRequest();
    dataFile.onprogress = function(event) {
      var url = 'four.data';
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
    dataFile.open('GET', 'four.data', true);
    dataFile.responseType = 'arraybuffer';
    dataFile.onload = function() {
      Module.finishedDataFileDownloads++;
      var arrayBuffer = dataFile.response;
      assert(arrayBuffer, 'Loading data file failed.');
      var byteArray = new Uint8Array(arrayBuffer);
      var curr;
      
        curr = DataRequest.prototype.requests['packages/base/cyber1.ogz'];
        curr.response = byteArray.subarray(0,497714);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/base/cyber1.cfg'];
        curr.response = byteArray.subarray(497714,498018);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/base/cyber1.wpt'];
        curr.response = byteArray.subarray(498018,516314);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/cyberskydark/gkskycyber_rt.png'];
        curr.response = byteArray.subarray(516314,1333188);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/cyberskydark/gkskycyber_lf.png'];
        curr.response = byteArray.subarray(1333188,2163910);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/cyberskydark/gkskycyber_ft.png'];
        curr.response = byteArray.subarray(2163910,2993125);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/cyberskydark/gkskycyber_bk.png'];
        curr.response = byteArray.subarray(2993125,3830872);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/cyberskydark/gkskycyber_up.png'];
        curr.response = byteArray.subarray(3830872,4571848);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/cyberskydark/gkskycyber_dn.png'];
        curr.response = byteArray.subarray(4571848,5317397);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/plain_colors/plain_color_05_cc.png'];
        curr.response = byteArray.subarray(5317397,5320257);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/plain_colors/plain_color_nm.png'];
        curr.response = byteArray.subarray(5320257,5323114);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/plain_colors/plain_color_04_cc.png'];
        curr.response = byteArray.subarray(5323114,5325975);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/plain_colors/plain_color_03_cc.png'];
        curr.response = byteArray.subarray(5325975,5328835);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/plain_colors/plain_color_01_cc.png'];
        curr.response = byteArray.subarray(5328835,5331694);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/plain_colors/Thumbs.db'];
        curr.response = byteArray.subarray(5331694,5335278);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/plain_colors/package.cfg'];
        curr.response = byteArray.subarray(5335278,5336829);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/plain_colors/plain_color_01_sc.png'];
        curr.response = byteArray.subarray(5336829,5344691);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/plain_colors/plain_color_02_cc.png'];
        curr.response = byteArray.subarray(5344691,5347551);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/cybersky/gkskycyber_bk.jpg'];
        curr.response = byteArray.subarray(5347551,5534063);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/cybersky/gkskycyber_rt.jpg'];
        curr.response = byteArray.subarray(5534063,5719745);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/cybersky/gkskycyber_dn.jpg'];
        curr.response = byteArray.subarray(5719745,5878322);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/cybersky/gkskycyber_lf.jpg'];
        curr.response = byteArray.subarray(5878322,6063973);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/cybersky/gkskycyber_ft.jpg'];
        curr.response = byteArray.subarray(6063973,6247407);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/cybersky/gkskycyber_up.jpg'];
        curr.response = byteArray.subarray(6247407,6421821);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/glow_colors/glow_color_box_01_gc.png'];
        curr.response = byteArray.subarray(6421821,6427194);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/glow_colors/glow_color_05.png'];
        curr.response = byteArray.subarray(6427194,6430052);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/glow_colors/glow_color_02.png'];
        curr.response = byteArray.subarray(6430052,6432912);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/glow_colors/glow_color_box_01_cc.png'];
        curr.response = byteArray.subarray(6432912,6442489);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/glow_colors/Thumbs.db'];
        curr.response = byteArray.subarray(6442489,6448633);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/glow_colors/glow_color_01.png'];
        curr.response = byteArray.subarray(6448633,6451494);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/glow_colors/package.cfg'];
        curr.response = byteArray.subarray(6451494,6453247);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/glow_colors/glow_color_nm.png'];
        curr.response = byteArray.subarray(6453247,6456104);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/glow_colors/glow_color_04.png'];
        curr.response = byteArray.subarray(6456104,6458963);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/glow_colors/glow_color_03.png'];
        curr.response = byteArray.subarray(6458963,6461822);
        curr.onload();
      
        curr = DataRequest.prototype.requests['packages/gk/cyber/glow_colors/glow_color_01_sc.png'];
        curr.response = byteArray.subarray(6461822,6469684);
        curr.onload();
                Module['removeRunDependency']('datafile_four.data');

    };
    Module['addRunDependency']('datafile_four.data');
    dataFile.send(null);
    if (Module['setStatus']) Module['setStatus']('Downloading...');
  
  });


})();

