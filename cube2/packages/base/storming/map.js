// (C) 2009 Alon 'Kripken' Zakai

Library.include('library/1_3/');

Library.include('library/' + Global.LIBRARY_VERSION + '/__CorePatches');
Library.include('library/' + Global.LIBRARY_VERSION + '/Plugins');
Library.include('library/' + Global.LIBRARY_VERSION + '/Firing');
Library.include('library/' + Global.LIBRARY_VERSION + '/Health');
Library.include('library/' + Global.LIBRARY_VERSION + '/World');
Library.include('library/' + Global.LIBRARY_VERSION + '/GameManager');
Library.include('library/' + Global.LIBRARY_VERSION + '/modes/CTF');
Library.include('library/' + Global.LIBRARY_VERSION + '/AutoTargeting');
Library.include('library/' + Global.LIBRARY_VERSION + '/MultipartRendering');
Library.include('library/' + Global.LIBRARY_VERSION + '/guns/Insta');
Library.include('library/' + Global.LIBRARY_VERSION + '/guns/Stunball');
Library.include('library/' + Global.LIBRARY_VERSION + '/guns/Rocket');
Library.include('library/' + Global.LIBRARY_VERSION + '/guns/Shotgun');
Library.include('library/' + Global.LIBRARY_VERSION + '/guns/Chaingun');
Library.include('library/' + Global.LIBRARY_VERSION + '/mapelements/Teleporters');
Library.include('library/' + Global.LIBRARY_VERSION + '/mapelements/Cannons');
Library.include('library/' + Global.LIBRARY_VERSION + '/Editing');
Library.include('library/' + Global.LIBRARY_VERSION + '/CustomEffect');

//// Setup physics

var BULLET = 0;

if (BULLET) {
    Library.include('library/' + Global.LIBRARY_VERSION + '/Physics');
    Projectiles.serverside = false;
    Physics.Engine.create('bullet', true);
    physicsPlugins = [
        Physics.Engine.objectPlugin,
        Physics.Engine.playerPlugin,
    ];
} else {
    physicsPlugins = [];
}

//// Materials

Map.materialReset();

Map.texture("water", "materials/GK_Water_01_cc.jpg", 0, 0, 0, 2);    // water surface
Map.texture("1", "materials/GK_Water_F_01_cc.jpg", 0, 0, 0, 2);     // waterfall
Map.texture("1", "materials/GK_Water_03_nm.jpg", 0, 0, 0, 2);        // water normals
Map.texture("1", "materials/GK_Water_01_dudv.jpg", 0, 0, 0, 2);     // water distortion
Map.texture("1", "materials/GK_Water_F_01_nm.jpg", 0, 0, 0, 2);    // waterfall normals
Map.texture("1", "materials/GK_Water_F_01_dudv.jpg", 0, 0, 0, 2); // waterfall distortion

Map.texture("lava", "materials/GK_Lava_01_cc.jpg", 0, 0, 0, 4);    // lava surface
Map.texture("1", "materials/GK_Lava_01_cc.jpg", 0, 0, 0, 4);     // lavafall
Map.texture("1", "materials/GK_Lava_01_nm.jpg", 0, 0, 0, 4);        // lava normals

//// Textures

Map.textureReset();
Map.texture("0", "freeseamless/1.jpg"); // Reserved - dummy
Map.texture("water", "golgotha/watr1.jpg");

// Gor

Map.setShader("bumpspecmapparallaxworld");

//for (var i = 1; i <= 4; i++) {
forEach([3,4], function(i) {
    Map.texture("0", "gor/cr_env_tex_00" + i + "_cc.jpg");
    Map.texture("n", "gor/cr_env_tex_00" + i + "_nm.jpg");
    Map.texture("s", "gor/cr_env_tex_00" + i + "_sc.jpg");
    Map.texture("z", "gor/cr_env_tex_00" + i + "_hm.jpg");
});
//Map.autograss("yo_frankie/plants_grass_006_col_2.png");

//for (var i = 5; i <= 7; i++) {
forEach([5,7], function(i) {
    Map.texture("0", "gor/cr_env_tex_00" + i + "_cc.jpg", 0, 0, 0, 2);
    Map.texture("n", "gor/cr_env_tex_00" + i + "_nm.jpg", 0, 0, 0, 2);
    Map.texture("s", "gor/cr_env_tex_00" + i + "_sc.jpg", 0, 0, 0, 2);
    Map.texture("z", "gor/cr_env_tex_00" + i + "_hm.jpg", 0, 0, 0, 2);
});


// Yo Frankie!

Map.setShader("bumpspecmapworld");

Map.texture("0", "yo_frankie/generic_darkRock_col.jpg");
Map.texture("n", "yo_frankie/generic_darkRock_nor.jpg");
Map.texture("s", "yo_frankie/generic_darkRock_spec.jpg");
//Map.texture("s", "yo_frankie/generic_redrock_spec.jpg");

Map.texture("0", "yo_frankie/generic_redrock_col.jpg");
Map.texture("n", "yo_frankie/generic_redrock_nor.jpg");
Map.texture("s", "yo_frankie/generic_redrock_spec.jpg");

Map.texture("0", "yo_frankie/stone_cliff_tile_001_col.jpg");
Map.texture("n", "yo_frankie/stone_cliff_tile_001_nor.jpg");
Map.texture("s", "yo_frankie/stone_cliff_tile_001_spec.jpg");

Map.texture("0", "yo_frankie/trees_bark_001_col.jpg");
Map.texture("n", "yo_frankie/trees_bark_001_nor.jpg");
Map.texture("s", "yo_frankie/trees_bark_001_spec.jpg");

Map.texture("0", "yo_frankie/trees_bark_002_col.jpg");
Map.texture("n", "yo_frankie/trees_bark_002_nor.jpg");
Map.texture("s", "yo_frankie/trees_bark_002_spec.jpg");

Map.setShader("stdworld");

Map.texture("0", "yo_frankie/generic_dirt1_col.jpg");
Map.texture("0", "yo_frankie/generic_dirt2_col.jpg");
Map.texture("0", "yo_frankie/generic_grass1_col.jpg");
Map.autograss("yo_frankie/plants_grass_006_col_2.png");
Map.texture("0", "yo_frankie/generic_grass4_col.jpg");
Map.texture("0", "yo_frankie/generic_grayrock1_col.jpg");
Map.texture("0", "yo_frankie/generic_lava.jpg");
Map.texture("0", "yo_frankie/generic_lightRock_col.jpg");

// Freeseamless

Map.setShader("bumpspecmapworld");

Map.texture("0", "freeseamless/1.jpg");
Map.texture("n", "freeseamless/1_nor.jpg");
Map.texture("s", "freeseamless/1_spec.jpg");

// Golgotha

Map.setShader("stdworld");

Map.texture("0", "golgotha/snow1.jpg");

// Map settings

Map.fogColor(0, 0, 0);
MAX_FOG = 750;
Map.fog(MAX_FOG);
Map.loadSky("skyboxes/philo/sky3");
Map.skylight(100, 100, 100);
Map.ambient(20);
Map.shadowmapAmbient("0x3C3C3C");
Map.shadowmapAngle(300);

//// Player class

registerEntityClass(
    bakePlugins(
        Player,
        [
            Firing.plugins.protocol,
            Firing.plugins.player,
            Health.plugin,
            GameManager.playerPlugin,
            Projectiles.plugin,
            StunballVictimPlugin,
            Chaingun.plugin,
            {
                _class: "GamePlayer",

                init: function() {
                    this.modelName = 'stromar';
                    this.gunIndexes = [playerChaingun, playerInstaGun, playerRocketGun, playerShotgun];
                    this.currGunIndex = playerChaingun;
                },

                activate: function() {
                    this.movementSpeed = 80;
                },
            }
        ].concat(physicsPlugins)
    )
);

playerChaingun = Firing.registerGun(new Chaingun(), 'Chaingun', 'packages/hud/gui_gk_Icon_w01.png');
playerInstaGun = Firing.registerGun(new InstaGun(), 'Sniper Rifle', 'packages/hud/gui_gk_Icon_w03.png');
playerRocketGun = Firing.registerGun(new RocketGun(), 'Rocket Launcher', 'packages/hud/gui_gk_Icon_w02.png');
playerShotgun = Firing.registerGun(new Shotgun(), 'Shotgun', 'packages/hud/gui_gk_Icon_w04.png');

// Autocannons

function makeCannon(_name, gunClass, additionalPlugins) {
    additionalPlugins = defaultValue(additionalPlugins, []);

    var CannonGun = bakePlugins(gunClass, [CannonGunPlugin]);
    var cannonGun = new CannonGun();
    Firing.registerGun(cannonGun);
    var plugins = [AutoTargetingPlugin, MultipartRenderingPlugin, MultipartRenderingAutotargetingPlugin, CannonPlugin, Firing.plugins.protocol, BotFiringPlugin, { _class: _name, gun: cannonGun }];
    return registerEntityClass( bakePlugins(Mapmodel, plugins.concat(additionalPlugins)) );
}

makeCannon('Autoturrent', StunballGun, [Projectiles.plugin, StunballBotPlugin]);



//// Tower area trigger, for effects

registerEntityClass(AreaTrigger.extend({
    _class: "TowerArea",

    // Unlike most static entities, we need this to 'act' each turn, so the occupancy will be handled
    shouldAct: true,

    occupied: new StateBoolean(),

    // Server

    init: function(uniqueId, kwargs) {
        this._super(uniqueId, kwargs);

        this.occupied = false;
    },

    activate: function(kwargs) {
        this._super(kwargs);

        this.connect('onModify_occupied', this.onChangeOccupance);

        this.sinceOccupied = 10.0; // As if it's been a long time since we've been occupied
    },

    act: function(seconds) {
        if (this.sinceOccupied >= 3.0 && this.occupied) {
//            log(WARNING, "Too much time since occupied: " + this.sinceOccupied);
            this.occupied = false; // It's been a while since someone was here; mark as empty
        }

        this.sinceOccupied += seconds;
//        log(WARNING, "Time since occupied: " + this.sinceOccupied);
    },

    onChangeOccupance: function(value) {
        if (value === true) {
//            log(WARNING, "Erasing sinceOccupied counter");
            this.sinceOccupied = 0.0; // Restart counter, there is someone here
        }
    },

    // Client

    clientActivate: function(kwargs) {
        this._super(kwargs);

        this.sinceUpdate = 10.0;

        this.effectInterval = 0.15;
        this.effectTimer = new RepeatingTimer(this.effectInterval, true);
        this.soundTimer = new RepeatingTimer(3.25);
        this.soundTimer.prime(); // Be ready for next time

        this.onTowerPlayers = {};
    },

    clientOnCollision: function(collider) {
        this.onTowerPlayers[collider.uniqueId] = true;

        // Each player manages themselves, for purposes of occupying the tower
        if (collider != getPlayerEntity()) {
            return;
        }

        if (this.sinceUpdate >= 1.0) { // Send updates at most 1/sec
            this.occupied = true;
            this.sinceUpdate = 0.0;
        }
    },

    clientAct: function(seconds) {
        this.sinceUpdate += seconds;

        if (this.occupied) {
            this.showEffect(seconds);
        } else {
            this.soundTimer.prime(); // Be ready for next time
            this.onTowerPlayers = {}; // No one is on the tower
        }
    },

    showEffect: function(seconds) {
        // Acquire links to particle WorldMarker markers, if none yet
        if (this.powerPositions === undefined) {
            var powerMarkers = getEntitiesByTag('tower_power');
            if (powerMarkers.length !== 2) {
                log(ERROR, "Wrong number of power markers: " + powerMarkers.length);
            }

            // Create nice copies of these values, for speed
            this.powerPositions = [ powerMarkers[0].position.copy(), powerMarkers[1].position.copy() ];
            this.powerCenter = this.powerPositions[0].addNew(this.powerPositions[1]).mul(0.5);
        }

        // Show effect

        Effect.fireball(PARTICLE.EXPLOSION_NO_GLARE, this.powerCenter, 12, 0);

        var NUM_RAYS = 10;//3;

        if (this.effectTimer.tick(seconds)) {

            Effect.splash(PARTICLE.SPARK, 10, this.effectInterval*1.1, this.powerPositions[0], 0xFFEECC, 1.0, 70, 1);
            Effect.splash(PARTICLE.SPARK, 10, this.effectInterval*1.1, this.powerPositions[1], 0xFFEECC, 1.0, 70, 1);

            for (var i = 0; i < NUM_RAYS; i++) {
                var offset = (Math.random()-0.5)*22.5;
                var centerPos = this.powerCenter.copy();
                centerPos.z += offset;
                var leftPos = this.powerPositions[0].copy();
                leftPos.z += 0;//offset/12;
                var rightPos = this.powerPositions[1].copy();
                rightPos.z += 0;//offset/12;

                var color;
                switch (integer(Math.random()*3)) {
                    case 0: color = 0xFF8844; break;
                    case 1: color = 0xFF6633; break;
                    case 2: color = 0xFF4422; break;
                    case 3: color = 0xFF2211; break;
                    default: color = 0xFF0000;
                }

                Effect.lightning(leftPos, centerPos, this.effectInterval*3, color);
                Effect.lightning(rightPos, centerPos, this.effectInterval*3, color);
            }
        }

        if (this.soundTimer.tick(seconds)) {
            // Play sound
            Sound.play("olpc/NilsVanOttorloo/Musical_Glasses8.wav", this.powerCenter);
        }
    }
}));

//! Tower area is a singleton, and this finds it (might need to check more than once, as
//! tower area might arrive after other entities
function findTowerArea() {
    if (Global.towerAreaSingleton === undefined) {
        Global.towerAreaSingleton = getEntityByTag('tower_area');
    }
}


//// Collision area for catapult

CatapultTrigger = ResettableAreaTrigger.extend({
    _class: "CatapultTrigger",

    // Unlike most static entities, we need this to 'act' each turn, so the fires will be handled
    shouldAct: true,

    showEffectFor: new StateInteger({ clientSet: true }),

    clientActivate: function(kwargs) {
        this._super(kwargs);

        this.fires = getEntitiesByTag('catapult_fire');
        this.maximizeFire();

        this.connect('client_onModify_showEffectFor', this.onShowEffectFor);
    },

    clientAct: function(seconds) {
        this._super(seconds);

        if (!this.resettingTimer) {
            this.resettingTimer = 0;
        }

        this.resettingTimer += seconds;

        if (this.resettingTimer >= 4.0 && !this.readyToTrigger) {
            this.reset();
            this.resettingTimer = 0;
            this.maximizeFire();
        }
    },

    clientOnTrigger: function(collider) {
        collider.position.z += 5; // Prevent colliding with catapult itself
        if (!BULLET) {
            collider.falling = [-90, 0, 440]; // Hurl collider up and away
        } else {
            collider.velocity = [-600, 0, 440]; // Handle air friction as well
        }
        // Show effect for this client instantly, for others using network protocol
        this.showEffectFor = collider.uniqueId;
        Effect.clientDamage(5, 20);

        this.resettingTimer = 0;

        this.minimizeFire();
    },

    maximizeFire: function() {
        forEach(this.fires, function(fireEntity) {
            CAPI.FAST_setAttr2(fireEntity, 150); // Big fire radius
            CAPI.FAST_setAttr3(fireEntity, 66); // Big fire radius
        });
    },

    minimizeFire: function() {
        forEach(this.fires, function(fireEntity) {
            CAPI.FAST_setAttr2(fireEntity, 1); // Big fire radius
            CAPI.FAST_setAttr3(fireEntity, 1); // Big fire radius
        });
    },

    //! Shows an explosion + sound
    showEffect: function(entity) {
        Effect.fireball(PARTICLE.EXPLOSION, this.position, 45);
        Sound.play("yo_frankie/DeathFlash.wav", this.position);
    },

    //! Called when a remote client wants to show an effect
    onShowEffectFor: function(uniqueId) {
        entity = getEntity(uniqueId);
        if (entity === null) {
            return; // Entity has either not been created yet, or destroyed meanwhile
        }
        this.showEffect(entity);
    }
});

registerEntityClass(CatapultTrigger);


//MeleeWeaponShootAction = ShootAction.extend({
//    secondsLeft: 0.5,
//    timeAfterShot: 0.25,
//    animation: ANIM_ATTACK3,
//});


//// Application

ApplicationManager.setApplicationClass(Application.extend({
    _class: "GameApplication",

    getPcClass: function() {
        return "GamePlayer";
    },

    // Replace this with appropriate behaviour for when a player falls of the map
    clientOnEntityOffMap: Health.dieIfOffMap,

    getScoreboardText: GameManager.getScoreboardText,

    clientClick: Firing.clientClick,
}));

//// Load permanent entities

GameManager.setup([
    AutoTargeting.managerPlugin,
    GameManager.managerPlugins.eventList,
    Projectiles.plugin,
    ParallelActionsPlugin,
]);

if (Global.SERVER) { // Run this only on the server - not the clients
    var entities = CAPI.readFile("./entities.json");
    loadEntities(entities);

    GameManager.getSingleton().registerTeams([
        {
            _name: 'red',
            setup: function(player) {
                player.defaultModelName = 'stromar';
            },
            flagModelName: 'flag/red',
        },
        {
            _name: 'blue',
            setup: function(player) {
                player.defaultModelName = 'stromar';
            },
            flagModelName: 'flag/blue',
        },
    ]);
}

Map.preloadModel('stromar');

if (Global.CLIENT) {
    Global.queuedActions.push(function() {
        CustomEffect.Rain.start({
            frequency: 0.05,
            spawnAtOnce: 190,
            maxAmount: 1000,
            speed: 1000,
            size: 30,
            radius: 200,
            dropColor: 0x1233A0,
            splashColor: 0xCCDDFF,
        });
    });
}

