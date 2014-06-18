var Module = {
  desiredWidth: 640,
  desiredHeight: 480,
  autoexec: function() {
    Module.setStatus('');
  },
  postLoadWorld: function() {
    Module.tweakDetail();
    BananaBread.execute('sensitivity 10');
    BananaBread.execute('clearconsole');
  }
};

importScripts('game/gl-matrix.js');

importScripts('game/preload_base.js');
importScripts('game/preload_character.js');
importScripts('game/preload_low.js');

importScripts('game/setup_low.js');

importScripts('js/api.js');
importScripts('js/zee.js');

importScripts('bb.worker.js');

