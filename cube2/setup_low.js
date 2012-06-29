
Module.setPlayerModels = function() {
  BananaBread.setPlayerModelInfo("frankie", "frankie", "frankie", 0, "nada", 0, 0, 0, 0, "frankie", "frankie", "frankie", false);
};

Module.tweakDetail = function() {
  BananaBread.execute('fog 10000'); // disable fog
  BananaBread.execute('maxdebris 10');

  var startPosition = LinearMath.vec3.create([660.6883, 423.9656, 578.6837]);
  var endPosition = LinearMath.vec3.create([215.5453, 437.4472, 715.4446]);
  var startOrientation = LinearMath.vec3.create([7.273222, 18.63635, 0.0]);
  var endOrientation = LinearMath.vec3.create([-30.4548, -49.09093, 0.0]);
  var startTime = Date.now();
  var position = LinearMath.vec3.create();
  var orientation = LinearMath.vec3.create();

  function moveCamera() {
    var factor = (Date.now() - startTime)/(15*1000);
    if (factor > 1) return;
    LinearMath.vec3.lerp(startPosition, endPosition, factor, position);
    LinearMath.vec3.lerp(startOrientation, endOrientation, factor, orientation);
    BananaBread.forceCamera(position, orientation);
    Module.requestAnimationFrame(moveCamera);
  }
  Module.requestAnimationFrame(moveCamera);
};

Module.loadDefaultMap = function() {
  BananaBread.execute('sleep 10 [ effic md2 ; sleep 20000 [ addbot 50 ] ]');
};

