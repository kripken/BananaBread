
Module.setPlayerModels = function() {
  BananaBread.setPlayerModelInfo("snoutx10k", "snoutx10k", "snoutx10k", "snoutx10k/hudguns", 0, 0, 0, 0, 0, "snoutx10k", "snoutx10k", "snoutx10k", true);
};

Module.autoexec = function() {
  BananaBread.execute('aniso 16');
};

Module.tweakDetail = function() {
  BananaBread.execute('maxdebris 25');
  BananaBread.execute('glare 1');
  BananaBread.execute('glarescale 1.75');
  BananaBread.execute('blurglare 7');
  BananaBread.execute('waterreflect 1');
  BananaBread.execute('waterrefract 1');
};

Module.loadDefaultMap = function() {
  BananaBread.execute('sleep 10 [ effic zoom ]');
};

