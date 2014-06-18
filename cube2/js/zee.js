// TODO: use this in game-setup.js
// Pre-unzip ogz files, we can do this in parallel in a worker during preload

(function() {
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

  if (!Module.preloadPlugins) Module.preloadPlugins = [];

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

