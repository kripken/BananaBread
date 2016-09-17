
//
// To run with proxy to worker:
//  * Change makefile to have --proxy-to-worker
//  * Build the client
//  * Paste this file's contents at the top of the generated bb.worker.js
//  * Run mini-worker.html
//

var Module = { postRun: [] };
Module.desiredWidth = 640;
Module.desiredHeight = 480;
Module.autoexec = function() {
  Module.setStatus('');
};
Module.postLoadWorld = function() {
  Module.tweakDetail();
  BananaBread.execute('sensitivity 10');
  BananaBread.execute('clearconsole');
};

importScripts("js/api.js");
importScripts("js/zee.js");
importScripts("game/gl-matrix.js");
importScripts("game/setup_low.js");
importScripts("game/preload_base.js");
importScripts("game/preload_character.js");
importScripts("game/preload_low.js");

