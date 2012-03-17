//md5dir "frankie"
Model.md5Load("Frankie.md5mesh", "frankie");
Model.md5Skin("Body", "<compress:2>flyingsquirrel_skin_col.jpg", "<compress:2>flyingsquirrel_skin_spec.png"); // .9 .8
Model.md5Bumpmap("Body", "<compress:2>flyingsquirrel_skin_nor.jpg");
//md5envmap Body socksky/mars
Model.md5Skin("Tail", "<compress:2>frankie_tail2.png");
Model.md5Alphatest("Tail", 0);
Model.scale(6500);
Model.trans(0, 0, 0.25);
Model.yaw(90);
//mdlspec 175

Model.md5Tag("Eye.R", "tag_weapon");

Library.include("models/frankie/anims.js");

