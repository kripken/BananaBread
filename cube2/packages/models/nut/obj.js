Model.objLoad('model.obj');

Model.objSkin('nut', 'props_pickups_apricot_diff.jpg');
Model.objBumpmap('nut', 'props_pickups_apricot_nor.jpg');
Model.objSpec('nut', 'props_pickups_apricot_spec.jpg');

Model.bb(4, 4);
Model.scale(20000);
Model.ambient(100);
Model.ellipseCollide(1);

