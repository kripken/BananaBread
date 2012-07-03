
Module.setPlayerModels = function() {
  BananaBread.setPlayerModelInfo("frankie", "frankie", "frankie", 0, "nada", 0, 0, 0, 0, "frankie", "frankie", "frankie", false);
};

Module.tweakDetail = function() {
  BananaBread.execute('fog 10000'); // disable fog
  BananaBread.execute('maxdebris 10');
  BananaBread.execute('waterreflect 1');
  BananaBread.execute('playasong = [ ]');

  new CameraPath({
    steps: [{
      position: LinearMath.vec3.create([443, 697, 539]),
      orientation: LinearMath.vec3.create([190, -8, 0])
    }, {
      position: LinearMath.vec3.create([512, 670, 550]),
      orientation: LinearMath.vec3.create([182, 20, 0])
    }, {
      position: LinearMath.vec3.create([612, 705, 536]),
      orientation: LinearMath.vec3.create([190, 4, 0])
    }, {
      position: LinearMath.vec3.create([500, 730, 520]),
      orientation: LinearMath.vec3.create([200, 15, 0])
    }],
    timeScale: 5,
    loop: true,
    uncancellable: true
  }).execute();

  function shootFirework() {
    var x = 530, y = 530, z = 526;
    BananaBread.playSound('q009/explosion.ogg', LinearMath.vec3.create([x, y, z]));
    new BananaBread.Effects.Fireworks([{
      position: LinearMath.vec3.create([x, y, z+16]),
      velocity: LinearMath.vec3.create([30*(Math.random()-0.5), 30*(Math.random()-0.5), 160 + Math.random()*80]),
      msLeft: 3000.0,
      childMinZ: 686 + 10,
      minVelocityZ: -10,
      childMsLeft: 600,
      size: 1.0,
      color: 0xffffff
    }]);
    setTimeout(shootFirework, Math.random()*3500);
  }
  shootFirework();
  shootFirework();
};

Module.loadDefaultMap = function() {
  BananaBread.execute('sleep 10 [ effic fireworks ]');
};

