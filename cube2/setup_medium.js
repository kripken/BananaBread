
Module.setPlayerModels = function() {
  BananaBread.setPlayerModelInfo("snoutx10k", "snoutx10k", "snoutx10k", "snoutx10k/hudguns", 0, 0, 0, 0, 0, "snoutx10k", "snoutx10k", "snoutx10k", true);
};

Module.tweakDetail = function() {
  BananaBread.execute('maxdebris 17');
  BananaBread.execute('glare 1');
  BananaBread.execute('glarescale 8.0');
  BananaBread.execute('blurglare 7');
};

Module.loadDefaultMap = function() {
  BananaBread.execute('sleep 10 [ effic zoomin ]');
};

