
Module.setPlayerModels = function() {
  BananaBread.setPlayerModelInfo("frankie", "frankie", "frankie", 0, "nada", 0, 0, 0, 0, "frankie", "frankie", "frankie", false);
};

Module.tweakDetail = function() {
  BananaBread.execute('fog 10000'); // disable fog
  BananaBread.execute('maxdebris 10');
  BananaBread.execute('waterreflect 1');
  BananaBread.execute('playasong = [ ]');
  // clear console and avoid messages right after load too
  BananaBread.execute('clearconsole');
  BananaBread.execute('sleep 10 [ clearconsole ]');
  BananaBread.execute('sleep 333 [ clearconsole ]');
  BananaBread.execute('sleep 1000 [ clearconsole ]');
  BananaBread.execute('sleep 3000 [ clearconsole ]');

  BananaBread.glare = 1;
  BananaBread.execute('glare 1');

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
    BananaBread.playSound(BananaBread.Utils.randomPick(['q009/glauncher.ogg', 'q009/glauncher2.ogg', 'q009/glauncher3.ogg']), LinearMath.vec3.create([x, y, z]));
    new BananaBread.Effects.Fireworks([{
      position: LinearMath.vec3.create([x, y, z+16]),
      velocity: LinearMath.vec3.create([30*(Math.random()-0.5), 30*(Math.random()-0.5), 160 + Math.random()*80]),
      msLeft: 1000.0
    }]);
    setTimeout(shootFirework, Math.random()*4000);
  }
  for (var i = 0; i < 3; i++) {
    shootFirework();
  }
};

Module.loadDefaultMap = function() {
  BananaBread.execute('sleep 10 [ effic fireworks ]');
};

function toggleGlare() {
  BananaBread.glare = 1 - BananaBread.glare;
  BananaBread.execute('glare ' + BananaBread.glare);
}

