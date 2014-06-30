// Nested Workers shim (based on emscripten headless.js)

if (typeof Worker === 'undefined') {
  Worker = function(workerPath) {
    var thisWorker = this;
    var xhr = new XMLHttpRequest();
    xhr.open('GET', workerPath, false);
    xhr.onload = function() {
      var workerCode = xhr.responseText;
      workerCode = workerCode.replace(/Module/g, 'zzModuleyy' + (Worker.id++)). // prevent collision with the global Module object. Note that this becomes global, so we need unique ids
                              //replace(/Date.now/g, 'Recorder.dnow'). // recorded values are just for the "main thread" - workers were not recorded, and should not consume
                              //replace(/performance.now/g, 'Recorder.pnow').
                              //replace(/Math.random/g, 'Recorder.random').
                              replace(/\nonmessage = /, '\nvar onmessage = '); // workers commonly do "onmessage = ", we need to varify that to sandbox
      function print(x) {
        console.log(x);
      }
      function printErr(x) {
        console.error(x);
      }

      console.log('loading fake worker ' + workerPath);
      eval(workerCode); // will implement onmessage()

      function duplicateJSON(json) {
        function handleTypedArrays(key, value) {
          if (value && value.toString && value.toString().substring(0, 8) == '[object ' && value.length && value.byteLength) {
            return Array.prototype.slice.call(value);
          }
          return value;
        }
        return JSON.parse(JSON.stringify(json, handleTypedArrays))
      }
      thisWorker.terminate = function(){};
      thisWorker.postMessage = function(msg) {
        msg.messageId = Worker.messageId++;
        console.log('main thread sending message ' + msg.messageId + ' to worker ' + workerPath);
        setTimeout(function() {
          console.log('worker ' + workerPath + ' receiving message ' + msg.messageId);
          onmessage({ data: duplicateJSON(msg) });
        });
      };
      var postMessage = function(msg) {
        msg.messageId = Worker.messageId++;
        console.log('worker ' + workerPath + ' sending message ' + msg.messageId);
        setTimeout(function() {
          console.log('main thread receiving message ' + msg.messageId + ' from ' + workerPath);
          thisWorker.onmessage({ data: duplicateJSON(msg) });
        });
      };
    };
    xhr.send(null);
  };
  Worker.id = 0;
  Worker.messageId = 0;
}

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

