Model.objLoad('model.obj');

Model.objSkin('trunk', 'trees_bark_002_col.jpg');
Model.objBumpmap('trunk', 'trees_bark_002_nor.jpg');
Model.objSpec('trunk', 'trees_bark_002_spec.jpg');

Model.objSkin('leaves', 'trees_leaves_003_col.png');

Model.alphatest(0.6);
Model.bb(4, 32);
Model.scale(3300);
Model.ellipseCollide(1);
Model.spec(7);

