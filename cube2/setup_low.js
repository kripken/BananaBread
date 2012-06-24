
Module.setPlayerModels = function() {
  BananaBread.setPlayerModelInfo("frankie", "frankie", "frankie", 0, "nada", 0, 0, 0, 0, "frankie", "frankie", "frankie", false);
};

Module.tweakDetail = function() {
  BananaBread.execute('fog 10000'); // disable fog
  BananaBread.execute('maxdebris 10');
};

Module.loadDefaultMap = function() {
  BananaBread.execute('sleep 10 [ effic md2 ; sleep 15000 [ addbot 50 ] ]');
};

