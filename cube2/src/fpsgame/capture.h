// capture.h: client and server state for capture gamemode
#ifndef PARSEMESSAGES

#ifdef SERVMODE
struct captureservmode : servmode
#else
VARP(capturetether, 0, 1, 1);
VARP(autorepammo, 0, 1, 1);
VARP(basenumbers, 0, 0, 1);

struct captureclientmode : clientmode
#endif
{
    static const int CAPTURERADIUS = 64;
    static const int CAPTUREHEIGHT = 24;
    static const int OCCUPYBONUS = 1;
    static const int OCCUPYPOINTS = 1;
    static const int OCCUPYENEMYLIMIT = 28;
    static const int OCCUPYNEUTRALLIMIT = 14;
    static const int SCORESECS = 10;
    static const int AMMOSECS = 15;
    static const int REGENSECS = 1;
    static const int REGENHEALTH = 10;
    static const int REGENARMOUR = 10;
    static const int REGENAMMO = 20;
    static const int MAXAMMO = 5;
    static const int REPAMMODIST = 32;
    static const int RESPAWNSECS = 5;
    static const int MAXBASES = 100;

    struct baseinfo
    {
        vec o;
        string owner, enemy;
#ifndef SERVMODE
        vec ammopos;
        string name, info;
        entitylight light;
#endif
        int ammogroup, ammotype, ammo, owners, enemies, converted, capturetime;

        baseinfo() { reset(); }

        void noenemy()
        {
            enemy[0] = '\0';
            enemies = 0;
            converted = 0;
        }

        void reset()
        {
            noenemy();
            owner[0] = '\0';
            capturetime = -1;
            ammogroup = 0;
            ammotype = 0;
            ammo = 0;
            owners = 0;
        }

        bool enter(const char *team)
        {
            if(!strcmp(owner, team))
            {
                owners++;
                return false;
            }
            if(!enemies)
            {
                if(strcmp(enemy, team))
                {
                    converted = 0;
                    copystring(enemy, team);
                }
                enemies++;
                return true;
            }
            else if(strcmp(enemy, team)) return false;
            else enemies++;
            return false;
        }

        bool steal(const char *team)
        {
            return !enemies && strcmp(owner, team);
        }

        bool leave(const char *team)
        {
            if(!strcmp(owner, team) && owners > 0)
            {
                owners--;
                return false;
            }
            if(strcmp(enemy, team) || enemies <= 0) return false;
            enemies--;
            return !enemies;
        }

        int occupy(const char *team, int units)
        {
            if(strcmp(enemy, team)) return -1;
            converted += units;
            if(units<0)
            {
                if(converted<=0) noenemy();
                return -1;
            }
            else if(converted<(owner[0] ? int(OCCUPYENEMYLIMIT) : int(OCCUPYNEUTRALLIMIT))) return -1;
            if(owner[0]) { owner[0] = '\0'; converted = 0; copystring(enemy, team); return 0; }
            else { copystring(owner, team); ammo = 0; capturetime = 0; owners = enemies; noenemy(); return 1; }
        }

        bool addammo(int i)
        {
            if(ammo>=MAXAMMO) return false;
            ammo = min(ammo+i, int(MAXAMMO));
            return true;
        }

        bool takeammo(const char *team)
        {
            if(strcmp(owner, team) || ammo<=0) return false;
            ammo--;
            return true;
        }
    };

    vector<baseinfo> bases;

    struct score
    {
        string team;
        int total;
    };

    vector<score> scores;

    int captures;

    void resetbases()
    {
        bases.shrink(0);
        scores.shrink(0);
        captures = 0;
    }

    bool hidefrags() { return true; }

    int getteamscore(const char *team)
    {
        loopv(scores)
        {
            score &cs = scores[i];
            if(!strcmp(cs.team, team)) return cs.total;
        }
        return 0;
    }

    void getteamscores(vector<teamscore> &teamscores)
    {
        loopv(scores) teamscores.add(teamscore(scores[i].team, scores[i].total));
    }

    score &findscore(const char *team)
    {
        loopv(scores)
        {
            score &cs = scores[i];
            if(!strcmp(cs.team, team)) return cs;
        }
        score &cs = scores.add();
        copystring(cs.team, team);
        cs.total = 0;
        return cs;
    }

    void addbase(int ammotype, const vec &o)
    {
        if(bases.length() >= MAXBASES) return;
        baseinfo &b = bases.add();
        b.ammogroup = min(ammotype, 0);
        b.ammotype = ammotype > 0 ? ammotype : rnd(5)+1;
        b.o = o;

        if(b.ammogroup)
        {
            loopi(bases.length()-1) if(b.ammogroup == bases[i].ammogroup)
            {
                b.ammotype = bases[i].ammotype;
                return;
            }
            int uses[5] = { 0, 0, 0, 0, 0 };
            loopi(bases.length()-1) if(bases[i].ammogroup)
            {
                loopj(i) if(bases[j].ammogroup == bases[i].ammogroup) goto nextbase;
                uses[bases[i].ammotype-1]++;
                nextbase:;
            }
            int mintype = 0;
            loopi(5) if(uses[i] < uses[mintype]) mintype = i;
            int numavail = 0, avail[5];
            loopi(5) if(uses[i] == uses[mintype]) avail[numavail++] = i+1;
            b.ammotype = avail[rnd(numavail)];
        }
    }

    void initbase(int i, int ammotype, const char *owner, const char *enemy, int converted, int ammo)
    {
        if(!bases.inrange(i)) return;
        baseinfo &b = bases[i];
        b.ammotype = ammotype;
        copystring(b.owner, owner);
        copystring(b.enemy, enemy);
        b.converted = converted;
        b.ammo = ammo;
    }

    bool hasbases(const char *team)
    {
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(b.owner[0] && !strcmp(b.owner, team)) return true;
        }
        return false;
    }

    float disttoenemy(baseinfo &b)
    {
        float dist = 1e10f;
        loopv(bases)
        {
            baseinfo &e = bases[i];
            if(e.owner[0] && strcmp(b.owner, e.owner))
                dist = min(dist, b.o.dist(e.o));
        }
        return dist;
    }

    bool insidebase(const baseinfo &b, const vec &o)
    {
        float dx = (b.o.x-o.x), dy = (b.o.y-o.y), dz = (b.o.z-o.z);
        return dx*dx + dy*dy <= CAPTURERADIUS*CAPTURERADIUS && fabs(dz) <= CAPTUREHEIGHT;
    }

#ifndef SERVMODE
    static const int AMMOHEIGHT = 5;

    captureclientmode() : captures(0)
    {
    }

    void respawned(fpsent *d)
    {
    }

    void replenishammo()
    {
        if(!m_capture || m_regencapture) return;
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(b.ammotype>0 && b.ammotype<=I_CARTRIDGES-I_SHELLS+1 && insidebase(b, player1->feetpos()) && player1->hasmaxammo(b.ammotype-1+I_SHELLS)) return;
        }
        addmsg(N_REPAMMO, "rc", player1);
    }

    void receiveammo(fpsent *d, int type)
    {
        type += I_SHELLS-1;
        if(type<I_SHELLS || type>I_CARTRIDGES) return;
        entities::repammo(d, type, d==player1);
        int icon = itemstats[type-I_SHELLS].icon;
        if(icon >= 0) particle_icon(d->abovehead(), icon%4, icon/4, PART_HUD_ICON_GREY, 2000, 0xFFFFFF, 2.0f, -8);
    }

    void checkitems(fpsent *d)
    {
        if(m_regencapture || !autorepammo || d!=player1 || d->state!=CS_ALIVE) return;
        vec o = d->feetpos();
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(b.ammotype>0 && b.ammotype<=I_CARTRIDGES-I_SHELLS+1 && insidebase(b, d->feetpos()) && !strcmp(b.owner, d->team) && b.o.dist(o) < 12)
            {
                if(d->lastrepammo!=i)
                {
                    if(b.ammo > 0 && !player1->hasmaxammo(b.ammotype-1+I_SHELLS)) addmsg(N_REPAMMO, "rc", d);
                    d->lastrepammo = i;
                }
                return;
            }
        }
        d->lastrepammo = -1;
    }

    void rendertether(fpsent *d)
    {
        int oldbase = d->lastbase;
        d->lastbase = -1;
        vec pos(d->o.x, d->o.y, d->o.z + (d->aboveeye - d->eyeheight)/2);
        if(d->state==CS_ALIVE)
        {
            loopv(bases)
            {
                baseinfo &b = bases[i];
                if(!insidebase(b, d->feetpos()) || (strcmp(b.owner, d->team) && strcmp(b.enemy, d->team))) continue;
                if(d->lastbase < 0 && (lookupmaterial(d->feetpos())&MATF_CLIP) == MAT_GAMECLIP) break;
                particle_flare(pos, vec(b.ammopos.x, b.ammopos.y, b.ammopos.z - AMMOHEIGHT - 4.4f), 0, PART_LIGHTNING, strcmp(d->team, player1->team) ? 0xFF2222 : 0x2222FF, 1.0f);
                if(oldbase < 0)
                {
                    particle_fireball(pos, 4.8f, PART_EXPLOSION, 250, strcmp(d->team, player1->team) ? 0x802020 : 0x2020FF, 4.8f);
                    particle_splash(PART_SPARK, 50, 250, pos, strcmp(d->team, player1->team) ? 0x802020 : 0x2020FF, 0.24f);
                }
                d->lastbase = i;
            }
        }
        if(d->lastbase < 0 && oldbase >= 0)
        {
            particle_fireball(pos, 4.8f, PART_EXPLOSION, 250, strcmp(d->team, player1->team) ? 0x802020 : 0x2020FF, 4.8f);
            particle_splash(PART_SPARK, 50, 250, pos, strcmp(d->team, player1->team) ? 0x802020 : 0x2020FF, 0.24f);
        }
    }

    void preload()
    {
        static const char *basemodels[3] = { "base/neutral", "base/red", "base/blue" };
        loopi(3) preloadmodel(basemodels[i]);
    }

    void rendergame()
    {
        if(capturetether && canaddparticles())
        {
            loopv(players)
            {
                fpsent *d = players[i];
                if(d) rendertether(d);
            }
            rendertether(player1);
        }
        loopv(bases)
        {
            baseinfo &b = bases[i];
            const char *basename = b.owner[0] ? (strcmp(b.owner, player1->team) ? "base/red" : "base/blue") : "base/neutral";
            rendermodel(&b.light, basename, ANIM_MAPMODEL|ANIM_LOOP, b.o, 0, 0, MDL_SHADOW | MDL_CULL_VFC | MDL_CULL_OCCLUDED);
            float fradius = 1.0f, fheight = 0.5f;
            regular_particle_flame(PART_FLAME, vec(b.ammopos.x, b.ammopos.y, b.ammopos.z - 4.5f), fradius, fheight, b.owner[0] ? (strcmp(b.owner, player1->team) ? 0x802020 : 0x2020FF) : 0x208020, 3, 2.0f);
            //regular_particle_flame(PART_SMOKE, vec(b.ammopos.x, b.ammopos.y, b.ammopos.z - 4.5f + 4.0f*min(fradius, fheight)), fradius, fheight, 0x303020, 1, 4.0f, 100.0f, 2000.0f, -20);

//            particle_fireball(b.ammopos, 4.8f, PART_EXPLOSION, 0, b.owner[0] ? (strcmp(b.owner, player1->team) ? 0x802020 : 0x2020FF) : 0x208020, 4.8f);

            if(b.ammotype>0 && b.ammotype<=I_CARTRIDGES-I_SHELLS+1)
            {
                const char *ammoname = entities::entmdlname(I_SHELLS+b.ammotype-1);
                if(m_regencapture)
                {
                    vec height(0, 0, 0);
                    abovemodel(height, ammoname);
                    vec ammopos(b.ammopos);
                    ammopos.z -= height.z/2 + sinf(lastmillis/100.0f)/20;
                    rendermodel(&b.light, ammoname, ANIM_MAPMODEL|ANIM_LOOP, ammopos, lastmillis/10.0f, 0, MDL_SHADOW | MDL_CULL_VFC | MDL_CULL_OCCLUDED);
                }
                else loopj(b.ammo)
                {
                    float angle = 2*M_PI*(lastmillis/4000.0f + j/float(MAXAMMO));
                    vec ammopos(b.o);
                    ammopos.x += 10*cosf(angle);
                    ammopos.y += 10*sinf(angle);
                    ammopos.z += 4;
                    rendermodel(&b.light, entities::entmdlname(I_SHELLS+b.ammotype-1), ANIM_MAPMODEL|ANIM_LOOP, ammopos, 0, 0, MDL_SHADOW | MDL_CULL_VFC | MDL_CULL_OCCLUDED);
                }
            }

            int tcolor = 0x1EC850, mtype = -1, mcolor = 0xFFFFFF, mcolor2 = 0;
            if(b.owner[0])
            {
                bool isowner = !strcmp(b.owner, player1->team);
                if(b.enemy[0]) { mtype = PART_METER_VS; mcolor = 0xFF1932; mcolor2 = 0x3219FF; if(!isowner) swap(mcolor, mcolor2); }
                if(!b.name[0]) formatstring(b.info)("base %d: %s", i+1, b.owner);
                else if(basenumbers) formatstring(b.info)("%s (%d): %s", b.name, i+1, b.owner);
                else formatstring(b.info)("%s: %s", b.name, b.owner);
                tcolor = isowner ? 0x6496FF : 0xFF4B19;
            }
            else if(b.enemy[0])
            {
                if(!b.name[0]) formatstring(b.info)("base %d: %s", i+1, b.enemy);
                else if(basenumbers) formatstring(b.info)("%s (%d): %s", b.name, i+1, b.enemy);
                else formatstring(b.info)("%s: %s", b.name, b.enemy);
                if(strcmp(b.enemy, player1->team)) { tcolor = 0xFF4B19; mtype = PART_METER; mcolor = 0xFF1932; }
                else { tcolor = 0x6496FF; mtype = PART_METER; mcolor = 0x3219FF; }
            }
            else if(!b.name[0]) formatstring(b.info)("base %d", i+1);
            else if(basenumbers) formatstring(b.info)("%s (%d)", b.name, i+1);
            else copystring(b.info, b.name);

            vec above(b.ammopos);
            above.z += AMMOHEIGHT;
            if(b.info[0]) particle_text(above, b.info, PART_TEXT, 1, tcolor, 2.0f);
            if(mtype>=0)
            {
                above.z += 3.0f;
                particle_meter(above, b.converted/float((b.owner[0] ? int(OCCUPYENEMYLIMIT) : int(OCCUPYNEUTRALLIMIT))), mtype, 1, mcolor, mcolor2, 2.0f);
            }
        }
    }

    void drawblips(fpsent *d, float blipsize, int fw, int fh, int type, bool skipenemy = false)
    {
        float scale = calcradarscale();
        int blips = 0;
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(skipenemy && b.enemy[0]) continue;
            switch(type)
            {
                case 1: if(!b.owner[0] || strcmp(b.owner, player1->team)) continue; break;
                case 0: if(b.owner[0]) continue; break;
                case -1: if(!b.owner[0] || !strcmp(b.owner, player1->team)) continue; break;
                case -2: if(!b.enemy[0] || !strcmp(b.enemy, player1->team)) continue; break;
            }
            vec dir(d->o);
            dir.sub(b.o).div(scale);
            float dist = dir.magnitude2(), maxdist = 1 - 0.05f - blipsize;
            if(dist >= maxdist) dir.mul(maxdist/dist);
            dir.rotate_around_z(-camera1->yaw*RAD);
            if(basenumbers)
            {
                static string blip;
                formatstring(blip)("%d", i+1);
                int tw, th;
                text_bounds(blip, tw, th);
                draw_text(blip, int(0.5f*(dir.x*fw/blipsize - tw)), int(0.5f*(dir.y*fh/blipsize - th)));
            }
            else
            {
                if(!blips) glBegin(GL_QUADS);
                float x = 0.5f*(dir.x*fw/blipsize - fw), y = 0.5f*(dir.y*fh/blipsize - fh);
                glTexCoord2f(0.0f, 0.0f); glVertex2f(x,    y);
                glTexCoord2f(1.0f, 0.0f); glVertex2f(x+fw, y);
                glTexCoord2f(1.0f, 1.0f); glVertex2f(x+fw, y+fh);
                glTexCoord2f(0.0f, 1.0f); glVertex2f(x,    y+fh);
            }
            blips++;
        }
        if(blips && !basenumbers) glEnd();
    }

    int respawnwait(fpsent *d)
    {
        if(m_regencapture) return -1;
        return max(0, RESPAWNSECS-(lastmillis-d->lastpain)/1000);
    }

    int clipconsole(int w, int h)
    {
        return (h*(1 + 1 + 10))/(4*10);
    }

    void drawhud(fpsent *d, int w, int h)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        int s = 1800/4, x = 1800*w/h - s - s/10, y = s/10;
        glColor4f(1, 1, 1, minimapalpha);
        if(minimapalpha >= 1) glDisable(GL_BLEND);
        bindminimap();
        drawminimap(d, x, y, s);
        if(minimapalpha >= 1) glEnable(GL_BLEND);
        glColor3f(1, 1, 1);
        float margin = 0.04f, roffset = s*margin, rsize = s + 2*roffset;
        settexture("packages/hud/radar.png", 3);
        drawradar(x - roffset, y - roffset, rsize);
        #if 0
        settexture("packages/hud/compass.png", 3);
        glPushMatrix();
        glTranslatef(x - roffset + 0.5f*rsize, y - roffset + 0.5f*rsize, 0);
        glRotatef(camera1->yaw + 180, 0, 0, -1);
        drawradar(-0.5f*rsize, -0.5f*rsize, rsize);
        glPopMatrix();
        #endif
        bool showenemies = lastmillis%1000 >= 500;
        int fw = 1, fh = 1;
        if(basenumbers)
        {
            pushfont();
            setfont("digit_blue");
            text_bounds(" ", fw, fh);
        }
        else settexture("packages/hud/blip_blue.png", 3);
        glPushMatrix();
        glTranslatef(x + 0.5f*s, y + 0.5f*s, 0);
        float blipsize = basenumbers ? 0.1f : 0.05f;
        glScalef((s*blipsize)/fw, (s*blipsize)/fh, 1.0f);
        drawblips(d, blipsize, fw, fh, 1, showenemies);
        if(basenumbers) setfont("digit_grey");
        else settexture("packages/hud/blip_grey.png", 3);
        drawblips(d, blipsize, fw, fh, 0, showenemies);
        if(basenumbers) setfont("digit_red");
        else settexture("packages/hud/blip_red.png", 3);
        drawblips(d, blipsize, fw, fh, -1, showenemies);
        if(showenemies) drawblips(d, blipsize, fw, fh, -2);
        glPopMatrix();
        if(basenumbers) popfont();
        if(d->state == CS_DEAD)
        {
            int wait = respawnwait(d);
            if(wait>=0)
            {
                glPushMatrix();
                glScalef(2, 2, 1);
                bool flash = wait>0 && d==player1 && lastspawnattempt>=d->lastpain && lastmillis < lastspawnattempt+100;
                draw_textf("%s%d", (x+s/2)/2-(wait>=10 ? 28 : 16), (y+s/2)/2-32, flash ? "\f3" : "", wait);
                glPopMatrix();
            }
        }
    }

    void setup()
    {
        resetbases();
        loopv(entities::ents)
        {
            extentity *e = entities::ents[i];
            if(e->type!=BASE) continue;
            baseinfo &b = bases.add();
            b.o = e->o;
            b.ammopos = b.o;
            abovemodel(b.ammopos, "base/neutral");
            b.ammopos.z += AMMOHEIGHT-2;
            b.ammotype = e->attr1;
            defformatstring(alias)("base_%d", e->attr2);
            const char *name = getalias(alias);
            copystring(b.name, name);
            b.light = e->light;
        }
    }

    void senditems(packetbuf &p)
    {
        putint(p, N_BASES);
        putint(p, bases.length());
        loopv(bases)
        {
            baseinfo &b = bases[i];
            putint(p, b.ammotype);
            putint(p, int(b.o.x*DMF));
            putint(p, int(b.o.y*DMF));
            putint(p, int(b.o.z*DMF));
        }
    }

    void updatebase(int i, const char *owner, const char *enemy, int converted, int ammo)
    {
        if(!bases.inrange(i)) return;
        baseinfo &b = bases[i];
        if(owner[0])
        {
            if(strcmp(b.owner, owner))
            {
                if(!b.name[0]) conoutf(CON_GAMEINFO, "%s captured base %d", owner, i+1);
                else if(basenumbers) conoutf(CON_GAMEINFO, "%s captured %s (%d)", owner, b.name, i+1);
                else conoutf(CON_GAMEINFO, "%s captured %s", owner, b.name);
                if(!strcmp(owner, player1->team)) playsound(S_V_BASECAP);
            }
        }
        else if(b.owner[0])
        {
            if(!b.name[0]) conoutf(CON_GAMEINFO, "%s lost base %d", b.owner, i+1);
            else if(basenumbers) conoutf(CON_GAMEINFO, "%s lost %s (%d)", b.owner, b.name, i+1);
            else conoutf(CON_GAMEINFO, "%s lost %s", b.owner, b.name);
            if(!strcmp(b.owner, player1->team)) playsound(S_V_BASELOST);
        }
        if(strcmp(b.owner, owner)) particle_splash(PART_SPARK, 200, 250, b.ammopos, owner[0] ? (strcmp(owner, player1->team) ? 0x802020 : 0x2020FF) : 0x208020, 0.24f);
        copystring(b.owner, owner);
        copystring(b.enemy, enemy);
        b.converted = converted;
        if(ammo>b.ammo)
        {
            playsound(S_ITEMSPAWN, &b.o);
            int icon = b.ammotype>0 && b.ammotype<=I_CARTRIDGES-I_SHELLS+1 ? itemstats[b.ammotype-1].icon : -1;
            if(icon >= 0) particle_icon(vec(b.ammopos.x, b.ammopos.y, b.ammopos.z + AMMOHEIGHT + 1.0f), icon%4, icon/4, PART_HUD_ICON, 2000, 0xFFFFFF, 2.0f, -8);
        }
        b.ammo = ammo;
    }

    void setscore(int base, const char *team, int total)
    {
        findscore(team).total = total;
        if(total>=10000) conoutf(CON_GAMEINFO, "team %s captured all bases", team);
        else if(bases.inrange(base))
        {
            baseinfo &b = bases[base];
            if(!strcmp(b.owner, team))
            {
                defformatstring(msg)("%d", total);
                vec above(b.ammopos);
                above.z += AMMOHEIGHT+1.0f;
                particle_textcopy(above, msg, PART_TEXT, 2000, isteam(team, player1->team) ? 0x6496FF : 0xFF4B19, 4.0f, -8);
            }
        }
    }

    int closesttoenemy(const char *team, bool noattacked = false, bool farthest = false)
    {
        float bestdist = farthest ? -1e10f : 1e10f;
        int best = -1;
        int attackers = INT_MAX, attacked = -1;
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(!b.owner[0] || strcmp(b.owner, team)) continue;
            if(noattacked && b.enemy[0]) continue;
            float dist = disttoenemy(b);
            if(farthest ? dist > bestdist : dist < bestdist)
            {
                best = i;
                bestdist = dist;
            }
            else if(b.enemy[0] && b.enemies < attackers)
            {
                attacked = i;
                attackers = b.enemies;
            }
        }
        if(best < 0) return attacked;
        return best;
    }

    int pickteamspawn(const char *team)
    {
        int closest = closesttoenemy(team, true, m_regencapture);
        if(!m_regencapture && closest < 0) closest = closesttoenemy(team, false);
        if(closest < 0) return -1;
        baseinfo &b = bases[closest];

        float bestdist = 1e10f, altdist = 1e10f;
        int best = -1, alt = -1;
        loopv(entities::ents)
        {
            extentity *e = entities::ents[i];
            if(e->type!=PLAYERSTART || e->attr2) continue;
            float dist = e->o.dist(b.o);
            if(dist < bestdist)
            {
                alt = best;
                altdist = bestdist;
                best = i;
                bestdist = dist;
            }
            else if(dist < altdist)
            {
                alt = i;
                altdist = dist;
            }
        }
        return rnd(2) ? best : alt;
    }

    void pickspawn(fpsent *d)
    {
        findplayerspawn(d, pickteamspawn(d->team));
    }

    const char *prefixnextmap() { return "capture_"; }


	bool aicheck(fpsent *d, ai::aistate &b)
	{
		return false;
	}

	void aifind(fpsent *d, ai::aistate &b, vector<ai::interest> &interests)
	{
		vec pos = d->feetpos();
		loopvj(bases)
		{
			baseinfo &f = bases[j];
			static vector<int> targets; // build a list of others who are interested in this
			targets.setsize(0);
			ai::checkothers(targets, d, ai::AI_S_DEFEND, ai::AI_T_AFFINITY, j, true);
			fpsent *e = NULL;
			int regen = !m_regencapture || d->health >= 100 ? 0 : 1;
			if(m_regencapture)
			{
				int gun = f.ammotype-1+I_SHELLS;
				if(f.ammo > 0 && f.ammotype > 0 && f.ammotype <= I_CARTRIDGES-I_SHELLS+1 && !d->hasmaxammo(gun))
					regen = gun != d->ai->weappref ? 2 : 4;
			}
			loopi(numdynents()) if((e = (fpsent *)iterdynents(i)) && !e->ai && e->state == CS_ALIVE && isteam(d->team, e->team))
			{ // try to guess what non ai are doing
				vec ep = e->feetpos();
				if(targets.find(e->clientnum) < 0 && ep.squaredist(f.o) <= (CAPTURERADIUS*CAPTURERADIUS))
					targets.add(e->clientnum);
			}
			if((regen && f.owner[0] && !strcmp(f.owner, d->team)) || (targets.empty() && (!f.owner[0] || strcmp(f.owner, d->team) || f.enemy[0])))
			{
				ai::interest &n = interests.add();
				n.state = ai::AI_S_DEFEND;
				n.node = ai::closestwaypoint(f.o, ai::SIGHTMIN, false);
				n.target = j;
				n.targtype = ai::AI_T_AFFINITY;
				n.score = pos.squaredist(f.o)/(regen ? float(100*regen) : 1.f);
			}
		}
	}

	bool aidefend(fpsent *d, ai::aistate &b)
	{
		if(bases.inrange(b.target))
		{
			baseinfo &f = bases[b.target];
			bool regen = !m_regencapture || d->health >= 100 ? false : true;
			if(!regen && m_regencapture)
			{
				int gun = f.ammotype-1+I_SHELLS;
				if(f.ammo > 0 && f.ammotype > 0 && f.ammotype <= I_CARTRIDGES-I_SHELLS+1 && !d->hasmaxammo(gun))
					regen = true;
			}
			int walk = 0;
			if(!regen && !f.enemy[0] && f.owner[0] && !strcmp(f.owner, d->team))
			{
				static vector<int> targets; // build a list of others who are interested in this
				targets.setsize(0);
				ai::checkothers(targets, d, ai::AI_S_DEFEND, ai::AI_T_AFFINITY, b.target, true);
				fpsent *e = NULL;
				loopi(numdynents()) if((e = (fpsent *)iterdynents(i)) && !e->ai && e->state == CS_ALIVE && isteam(d->team, e->team))
				{ // try to guess what non ai are doing
					vec ep = e->feetpos();
					if(targets.find(e->clientnum) < 0 && (ep.squaredist(f.o) <= (CAPTURERADIUS*CAPTURERADIUS*4)))
						targets.add(e->clientnum);
				}
				if(!targets.empty())
				{
					if(lastmillis-b.millis >= (201-d->skill)*33)
					{
						d->ai->trywipe = true; // re-evaluate so as not to herd
						return true;
					}
					else walk = 2;
				}
				else walk = 1;
				b.millis = lastmillis;
			}
			return ai::defend(d, b, f.o, float(CAPTURERADIUS), float(CAPTURERADIUS*(2+(walk*2))), walk); // less wander than ctf
		}
		return false;
	}

	bool aipursue(fpsent *d, ai::aistate &b)
	{
		b.type = ai::AI_S_DEFEND;
		return aidefend(d, b);
	}
};

extern captureclientmode capturemode;
ICOMMAND(repammo, "", (), capturemode.replenishammo());
ICOMMAND(insidebases, "", (),
{
    vector<char> buf;
    if(m_capture && player1->state == CS_ALIVE) loopv(capturemode.bases)
    {
        if(capturemode.insidebase(capturemode.bases[i], player1->feetpos()))
        {
            if(buf.length()) buf.add(' ');
            defformatstring(basenum)("%d", i);
            buf.put(basenum, strlen(basenum));
        }
    }
    buf.add('\0');
    result(buf.getbuf());
}); 
    
#else
    bool notgotbases;

    captureservmode() : captures(0), notgotbases(false) {}

    void reset(bool empty)
    {
        resetbases();
        notgotbases = !empty;
    }

    void cleanup()
    {
        reset(false);
    }

    void setup()
    {
        reset(false);
        if(notgotitems || ments.empty()) return;
        loopv(ments)
        {
            entity &e = ments[i];
            if(e.type != BASE) continue;
            int ammotype = e.attr1;
            addbase(ammotype>=GUN_SG && ammotype<=GUN_PISTOL ? ammotype : min(ammotype, 0), e.o); 
        }
        notgotbases = false;
        sendbases();
        loopv(clients) if(clients[i]->state.state==CS_ALIVE) entergame(clients[i]);
    }

    void newmap()
    {
        reset(true);
    }

    void stealbase(int n, const char *team)
    {
        baseinfo &b = bases[n];
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->state.state==CS_ALIVE && ci->team[0] && !strcmp(ci->team, team) && insidebase(b, ci->state.o))
                b.enter(ci->team);
        }
        sendbaseinfo(n);
    }

    void replenishammo(clientinfo *ci)
    {
        if(m_noitems || notgotbases || ci->state.state!=CS_ALIVE || !ci->team[0]) return;
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(b.ammotype>0 && b.ammotype<=I_CARTRIDGES-I_SHELLS+1 && insidebase(b, ci->state.o) && !ci->state.hasmaxammo(b.ammotype-1+I_SHELLS) && b.takeammo(ci->team))
            {
                sendbaseinfo(i);
                sendf(-1, 1, "riii", N_REPAMMO, ci->clientnum, b.ammotype);
                ci->state.addammo(b.ammotype);
                break;
            }
        }
    }

    void movebases(const char *team, const vec &oldpos, bool oldclip, const vec &newpos, bool newclip)
    {
        if(!team[0] || gamemillis>=gamelimit) return;
        loopv(bases)
        {
            baseinfo &b = bases[i];
            bool leave = !oldclip && insidebase(b, oldpos),
                 enter = !newclip && insidebase(b, newpos);
            if(leave && !enter && b.leave(team)) sendbaseinfo(i);
            else if(enter && !leave && b.enter(team)) sendbaseinfo(i);
            else if(leave && enter && b.steal(team)) stealbase(i, team);
        }
    }

    void leavebases(const char *team, const vec &o)
    {
        movebases(team, o, false, vec(-1e10f, -1e10f, -1e10f), true);
    }

    void enterbases(const char *team, const vec &o)
    {
        movebases(team, vec(-1e10f, -1e10f, -1e10f), true, o, false);
    }

    void addscore(int base, const char *team, int n)
    {
        if(!n) return;
        score &cs = findscore(team);
        cs.total += n;
        sendf(-1, 1, "riisi", N_BASESCORE, base, team, cs.total);
    }

    void regenowners(baseinfo &b, int ticks)
    {
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->state.state==CS_ALIVE && ci->team[0] && !strcmp(ci->team, b.owner) && insidebase(b, ci->state.o))
            {
                bool notify = false;
                if(ci->state.health < ci->state.maxhealth)
                {
                    ci->state.health = min(ci->state.health + ticks*REGENHEALTH, ci->state.maxhealth);
                    notify = true;
                }
                if(ci->state.armour < itemstats[I_GREENARMOUR-I_SHELLS].max)
                {
                    ci->state.armour = min(ci->state.armour + ticks*REGENARMOUR, itemstats[I_GREENARMOUR-I_SHELLS].max);
                    notify = true;
                }
                if(b.ammotype>0)
                {
                    int ammotype = b.ammotype-1+I_SHELLS;
                    if(ammotype<=I_CARTRIDGES && !ci->state.hasmaxammo(ammotype))
                    {
                        ci->state.addammo(b.ammotype, ticks*REGENAMMO, 100);
                        notify = true;
                    }
                }
                if(notify)
                    sendf(-1, 1, "ri6", N_BASEREGEN, ci->clientnum, ci->state.health, ci->state.armour, b.ammotype, b.ammotype>0 ? ci->state.ammo[b.ammotype] : 0);
            }
        }
    }

    void update()
    {
        if(gamemillis>=gamelimit) return;
        endcheck();
        int t = gamemillis/1000 - (gamemillis-curtime)/1000;
        if(t<1) return;
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(b.enemy[0])
            {
                if(!b.owners || !b.enemies) b.occupy(b.enemy, OCCUPYBONUS*(b.enemies ? 1 : -1) + OCCUPYPOINTS*(b.enemies ? b.enemies : -(1+b.owners))*t);
                sendbaseinfo(i);
            }
            else if(b.owner[0])
            {
                b.capturetime += t;

                int score = b.capturetime/SCORESECS - (b.capturetime-t)/SCORESECS;
                if(score) addscore(i, b.owner, score);

                if(m_regencapture)
                {
                    int regen = b.capturetime/REGENSECS - (b.capturetime-t)/REGENSECS;
                    if(regen) regenowners(b, regen);
                }
                else
                {
                    int ammo = b.capturetime/AMMOSECS - (b.capturetime-t)/AMMOSECS;
                    if(ammo && b.addammo(ammo)) sendbaseinfo(i);
                }
            }
        }
    }

    void sendbaseinfo(int i)
    {
        baseinfo &b = bases[i];
        sendf(-1, 1, "riissii", N_BASEINFO, i, b.owner, b.enemy, b.enemy[0] ? b.converted : 0, b.owner[0] ? b.ammo : 0);
    }

    void sendbases()
    {
        packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
        initclient(NULL, p, false);
        sendpacket(-1, 1, p.finalize());
    }

    void initclient(clientinfo *ci, packetbuf &p, bool connecting)
    {
        if(connecting)
        {
            loopv(scores)
            {
                score &cs = scores[i];
                putint(p, N_BASESCORE);
                putint(p, -1);
                sendstring(cs.team, p);
                putint(p, cs.total);
            }
        }
        putint(p, N_BASES);
        putint(p, bases.length());
        loopv(bases)
        {
            baseinfo &b = bases[i];
            putint(p, min(max(b.ammotype, 1), I_CARTRIDGES+1));
            sendstring(b.owner, p);
            sendstring(b.enemy, p);
            putint(p, b.converted);
            putint(p, b.ammo);
        }
    }

    void endcheck()
    {
        const char *lastteam = NULL;

        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(b.owner[0])
            {
                if(!lastteam) lastteam = b.owner;
                else if(strcmp(lastteam, b.owner))
                {
                    lastteam = NULL;
                    break;
                }
            }
            else
            {
                lastteam = NULL;
                break;
            }
        }

        if(!lastteam) return;
        findscore(lastteam).total = 10000;
        sendf(-1, 1, "riisi", N_BASESCORE, -1, lastteam, 10000);
        startintermission();
    }

    void entergame(clientinfo *ci)
    {
        if(notgotbases || ci->state.state!=CS_ALIVE || ci->gameclip) return;
        enterbases(ci->team, ci->state.o);
    }

    void spawned(clientinfo *ci)
    {
        if(notgotbases || ci->gameclip) return;
        enterbases(ci->team, ci->state.o);
    }

    void leavegame(clientinfo *ci, bool disconnecting = false)
    {
        if(notgotbases || ci->state.state!=CS_ALIVE || ci->gameclip) return;
        leavebases(ci->team, ci->state.o);
    }

    void died(clientinfo *ci, clientinfo *actor)
    {
        if(notgotbases || ci->gameclip) return;
        leavebases(ci->team, ci->state.o);
    }

    void moved(clientinfo *ci, const vec &oldpos, bool oldclip, const vec &newpos, bool newclip)
    {
        if(notgotbases) return;
        movebases(ci->team, oldpos, oldclip, newpos, newclip);
    }

    void changeteam(clientinfo *ci, const char *oldteam, const char *newteam)
    {
        if(notgotbases || ci->gameclip) return;
        leavebases(oldteam, ci->state.o);
        enterbases(newteam, ci->state.o);
    }

    void parsebases(ucharbuf &p, bool commit)
    {
        int numbases = getint(p);
        loopi(numbases)
        {
            int ammotype = getint(p);
            vec o;
            loopk(3) o[k] = max(getint(p)/DMF, 0.0f);
            if(p.overread()) break;
            if(commit && notgotbases) addbase(ammotype>=GUN_SG && ammotype<=GUN_PISTOL ? ammotype : min(ammotype, 0), o);
        }
        if(commit && notgotbases)
        {
            notgotbases = false;
            sendbases();
            loopv(clients) if(clients[i]->state.state==CS_ALIVE) entergame(clients[i]);
        }
    }

    bool extinfoteam(const char *team, ucharbuf &p)
    {
        int numbases = 0;
        loopvj(bases) if(!strcmp(bases[j].owner, team)) numbases++;
        putint(p, numbases);
        loopvj(bases) if(!strcmp(bases[j].owner, team)) putint(p, j);
        return true;
    }
};

#endif

#elif SERVMODE

case N_BASES:
    if(smode==&capturemode) capturemode.parsebases(p, (ci->state.state!=CS_SPECTATOR || ci->privilege || ci->local) && !strcmp(ci->clientmap, smapname));
    break;

case N_REPAMMO:
    if((ci->state.state!=CS_SPECTATOR || ci->local || ci->privilege) && cq && smode==&capturemode) capturemode.replenishammo(cq);
    break;

#else

case N_BASEINFO:
{
    int base = getint(p);
    string owner, enemy;
    getstring(text, p);
    copystring(owner, text);
    getstring(text, p);
    copystring(enemy, text);
    int converted = getint(p), ammo = getint(p);
    if(m_capture) capturemode.updatebase(base, owner, enemy, converted, ammo);
    break;
}

case N_BASEREGEN:
{
    int rcn = getint(p), health = getint(p), armour = getint(p), ammotype = getint(p), ammo = getint(p);
    fpsent *regen = rcn==player1->clientnum ? player1 : getclient(rcn);
    if(regen && m_capture)
    {
        regen->health = health;
        regen->armour = armour;
        if(ammotype>=GUN_SG && ammotype<=GUN_PISTOL) regen->ammo[ammotype] = ammo;
    }
    break;
}

case N_BASES:
{
    int numbases = getint(p);
    loopi(numbases)
    {
        int ammotype = getint(p);
        string owner, enemy;
        getstring(text, p);
        copystring(owner, text);
        getstring(text, p);
        copystring(enemy, text);
        int converted = getint(p), ammo = getint(p);
        capturemode.initbase(i, ammotype, owner, enemy, converted, ammo);
    }
    break;
}

case N_BASESCORE:
{
    int base = getint(p);
    getstring(text, p);
    int total = getint(p);
    if(m_capture) capturemode.setscore(base, text, total);
    break;
}

case N_REPAMMO:
{
    int rcn = getint(p), ammotype = getint(p);
    fpsent *r = rcn==player1->clientnum ? player1 : getclient(rcn);
    if(r && m_capture) capturemode.receiveammo(r, ammotype);
    break;
}

#endif


