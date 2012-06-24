
Module.setPlayerModels = function() {
  BananaBread.setPlayerModelInfo("snoutx10k", "snoutx10k", "snoutx10k", "snoutx10k/hudguns", 0, 0, 0, 0, 0, "snoutx10k", "snoutx10k", "snoutx10k", true);
};

Module.tweakDetail = function() {
  BananaBread.execute('maxdebris 17');
};

Module.loadDefaultMap = function() {
  BananaBread.execute('sleep 10 [ effic cyber1 ; sleep 1000 [ addbot 60 ] ]');
};

