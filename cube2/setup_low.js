
Module['setPlayerModels'] = function() {
  var setPlayerModelInfo = Module['cwrap']('_ZN4game18setplayermodelinfoEPKcS1_S1_S1_S1_S1_S1_S1_S1_S1_S1_S1_b', null,
    ['string', 'string', 'string', 'string', 'string', 'string', 'string', 'string', 'string', 'string', 'string', 'string', 'number']);

  setPlayerModelInfo("frankie", "frankie", "frankie", 0, "nada", 0, 0, 0, 0, "frankie", "frankie", "frankie", false);
}

