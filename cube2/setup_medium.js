
Module.setPlayerModels = function() {
  BananaBread.setPlayerModelInfo("snoutx10k", "snoutx10k", "snoutx10k", "snoutx10k/hudguns", 0, 0, 0, 0, 0, "snoutx10k", "snoutx10k", "snoutx10k", true);
};

Module.autoexec = function() {
  BananaBread.execute('aniso 4');
};

Module.tweakDetail = function() {
  BananaBread.execute('maxdebris 17');
  BananaBread.execute('waterreflect 1');
  BananaBread.execute('waterrefract 1');

  new CameraPath({
    steps: [{
      position: LinearMath.vec3.create([3201.224, 2654.501, 1547.068]),
      orientation: LinearMath.vec3.create([179.0898, 6.818282, 0.0])
    }, {
      position: LinearMath.vec3.create([3202.201, 2669.732, 1615.869]),
      orientation: LinearMath.vec3.create([179.5442, -11.81821, 0.0])
    }, {
      position: LinearMath.vec3.create([3086.916, 2422.177, 1659.728]),
      orientation: LinearMath.vec3.create([235.9077, -1.363664, 0.0])
    }, {
      position: LinearMath.vec3.create([3493.206, 2556.504, 1648.553]),
      orientation: LinearMath.vec3.create([181.817, -0.4545656, 0.0])
    }, {
      position: LinearMath.vec3.create([3558.436, 2322.658, 1611.323]),
      orientation: LinearMath.vec3.create([162.7262, 30.45452, 0.0])
    }, {
      position: LinearMath.vec3.create([3466.318, 2026.867, 1601.999]),
      orientation: LinearMath.vec3.create([2.271917, 4.999986, 0.0])
    }, {
      position: LinearMath.vec3.create([3353.81, 2176.516, 1561.096]),
      orientation: LinearMath.vec3.create([86.81731, -6.363651, 0.0])
    }, {
      position: LinearMath.vec3.create([3285.891, 2175.981, 1551.777]),
      orientation: LinearMath.vec3.create([15.90823, 0.454531, 0.0])
    }, {
      position: LinearMath.vec3.create([3252.219, 2234.031, 1553.28]),
      orientation: LinearMath.vec3.create([178.1809, 30.45453, 0.0])
    }, {
      position: LinearMath.vec3.create([3259.742, 2174.635, 1598.427]),
      orientation: LinearMath.vec3.create([145.0909, 6.818192, 0.0])
    }, {
      position: LinearMath.vec3.create([3251.713, 2123.673, 1614.099]),
      orientation: LinearMath.vec3.create([62.09092, -7.636365, 0.0])
    }, {
      position: LinearMath.vec3.create([3245.892, 2112.35, 1615.566]),
      orientation: LinearMath.vec3.create([33.99992, -5.818166, 0.0])
    }, {
      position: LinearMath.vec3.create([3243.335, 2111.635, 1633.693]),
      orientation: LinearMath.vec3.create([10, -40.636374, 0.0])
    }, {
      position: LinearMath.vec3.create([3160.278, 2084.209, 1624.748]),
      orientation: LinearMath.vec3.create([2.72652, 10.90908, 0.0])
    }, {
      position: LinearMath.vec3.create([3155.242, 2223.584, 1684.467]),
      orientation: LinearMath.vec3.create([-4.8173, 5.454529, 0.0])
    }, {
      position: LinearMath.vec3.create([3224.862, 2348.666, 1679.801]),
      orientation: LinearMath.vec3.create([-79.8169, -0.4545625, 0.0])
    }, {
      position: LinearMath.vec3.create([3378.123, 2286.918, 1555.412]),
      orientation: LinearMath.vec3.create([-102.1805, -30.90909, 0.0])
    }, {
      position: LinearMath.vec3.create([3573.424, 2285.129, 1574.543]),
      orientation: LinearMath.vec3.create([-56.545, 6.818174, 0.0])
    }, {
      position: LinearMath.vec3.create([3848.007, 2393.765, 1607.863]),
      orientation: LinearMath.vec3.create([-150.9084, -4.090919, 0.0])
    }, {
      position: LinearMath.vec3.create([4330.775, 2420.04, 1731.584]),
      orientation: LinearMath.vec3.create([-254.3631, -7.272726, 0.0])
    }, {
      position: LinearMath.vec3.create([4530.775, 2425.04, 1800.584]),
      orientation: LinearMath.vec3.create([-275.3631, -9.272726, 0.0])
    }],
    timeScale: 4.33,
    sigma: 0.75
  }).execute();
};

Module.loadDefaultMap = function() {
  BananaBread.execute('sleep 10 [ effic two_towers ]');
};

