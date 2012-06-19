
Module.setPlayerModels = function() {
  BananaBread.setPlayerModelInfo("snoutx10k", "snoutx10k", "snoutx10k", "snoutx10k/hudguns", 0, 0, 0, 0, 0, "snoutx10k", "snoutx10k", "snoutx10k", true);
};

Module.tweakDetail = function() {
  BananaBread.execute('waterreflect 1');
  BananaBread.execute('waterrefract 1');
  BananaBread.execute('glare 1');
  BananaBread.execute('maxdebris 25');
};

Module.loadDefaultMap = function() {
  BananaBread.execute('sleep 10 [ effic md ]');
};

