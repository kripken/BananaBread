
Module.setPlayerModels = function() {
  //BananaBread.setPlayerModelInfo("frankie", "frankie", "frankie", 0, "nada", 0, 0, 0, 0, "frankie", "frankie", "frankie", false);
  BananaBread.setPlayerModelInfo("snoutx10k", "snoutx10k", "snoutx10k", "snoutx10k/hudguns", 0, 0, 0, 0, 0, "snoutx10k", "snoutx10k", "snoutx10k", true);
};

/*
Module.tweakDetail = function() {
  BananaBread.execute('fog 10000'); // disable fog
  BananaBread.execute('maxdebris 10');
};
*/

Module.loadDefaultMap = function() {
  BananaBread.execute('sleep 10 [ effic sb2 ]');
};

Module.autoexec = function() {
  if(true === Module['join']) {
    console.log('connecting to host');
    BananaBread.execute('connect');
  }
};
