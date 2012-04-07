#ifndef PARSEMESSAGES

#define collectteambase(s) (!strcmp(s, "good") ? 1 : (!strcmp(s, "evil") ? 2 : 0))
#define collectbaseteam(i) (i==1 ? "good" : (i==2 ? "evil" : NULL))

#ifdef SERVMODE
struct collectservmode : servmode
#else
struct collectclientmode : clientmode
#endif
{
    static const int BASERADIUS = 16;
    static const int BASEHEIGHT = 16;
    static const int MAXBASES = 20;
    static const int TOKENRADIUS = 16;
    static const int TOKENLIMIT = 5;
    static const int TOKENDIST = 16;
    static const int SCORELIMIT = 50;

    struct base
    {
        int id, team;
        vec o;
#ifndef SERVMODE
        vec tokenpos;
        string info;
        entitylight light;
#endif
    
        base() { reset(); }

        void reset()
        {
            o = vec(0, 0, 0);
            team = 0;
        }
    };

    struct token
    {
        int id, team, droptime;
        vec o;
#ifdef SERVMODE
        int yaw, dropper;
#else
        entitylight light;
#endif

        token() { reset(); }

        void reset()
        {
            o = vec(0, 0, 0);
            team = 0;
#ifdef SERVMODE
            dropper = -1;
#endif
            droptime = 0;
        }
    };

    vector<base> bases;
    int scores[2];
    vector<token> tokens;
#ifdef SERVMODE
    int nexttoken;
#endif

    void resetbases()
    {
        bases.shrink(0);
        tokens.shrink(0);
        loopk(2) scores[k] = 0;
        tokens.shrink(0);
#ifdef SERVMODE
        nexttoken = 0;
#endif
    }

#ifdef SERVMODE
    bool addbase(int i, const vec &o, int team)
#else
    bool addbase(int i, const vec &o, int team)
#endif
    {
        if(i<0 || i>=MAXBASES) return false;
        while(bases.length()<=i) bases.add();
        base &b = bases[i];
        b.reset();
        b.id = i;
        b.team = team;
        b.o = o;
        return true;
    }

    token *findtoken(int id)
    {
        loopv(tokens) if(tokens[i].id == id) return &tokens[i];
        return NULL;
    }

#ifdef SERVMODE
    token &droptoken(const vec &o, int yaw, int team, int droptime, int dropper)
#else
    token &droptoken(int id, const vec &o, int team, int droptime)
#endif
    {
        token &t = tokens.add();
        t.o = o;
        t.team = team;
        t.droptime = droptime;
#ifdef SERVMODE
        if(++nexttoken < 0) nexttoken = 1;
        t.id = nexttoken;
        t.dropper = dropper;
        t.yaw = yaw;
#else
        t.id = id;
#endif
        return t;
    }

    bool removetoken(int id)
    {
        loopv(tokens) if(tokens[i].id == id)
        {
            tokens.removeunordered(i);
            return true;
        }
        return false;
    }    

    int totalscore(int team)
    {
        return team >= 1 && team <= 2 ? scores[team-1] : 0;
    }

    int setscore(int team, int score)
    {
        if(team >= 1 && team <= 2) return scores[team-1] = score;
        return 0;
    }

    int addscore(int team, int score)
    {
        if(team >= 1 && team <= 2) return scores[team-1] += score;
        return 0;
    }

    bool hidefrags() { return true; }

    int getteamscore(const char *team)
    {
        return totalscore(collectteambase(team));
    }

    void getteamscores(vector<teamscore> &tscores)
    {
        loopk(2) if(scores[k]) tscores.add(teamscore(collectbaseteam(k+1), scores[k]));
    }

    bool insidebase(const base &b, const vec &o)
    {
        float dx = (b.o.x-o.x), dy = (b.o.y-o.y), dz = (b.o.z-o.z);
        return dx*dx + dy*dy <= BASERADIUS*BASERADIUS && fabs(dz) <= BASEHEIGHT;
    }

#ifdef SERVMODE
    static const int EXPIRETOKENTIME = 10000;

    bool notgotbases;

    collectservmode() : notgotbases(false) {}

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
            if(e.type != FLAG || e.attr2 < 1 || e.attr2 > 2) continue;
            if(!addbase(bases.length(), e.o, e.attr2)) break;
        }
        notgotbases = false;
    }

    void newmap()
    {
        reset(true);
    }

#if 0
    void losetokens(clientinfo *ci)
    {
        if(notgotbases || ci->state.tokens <= 0) return;
        sendf(-1, 1, "ri2", N_LOSETOKENS, ci->clientnum); 
        ci->state.tokens = 0;
    }    
#endif

    void droptokens(clientinfo *ci, bool penalty = false)
    {
        if(notgotbases) return;
        int team = collectteambase(ci->team), totalenemy = penalty ? 0 : ci->state.tokens, totalfriendly = 1, expired = 0;
        packetbuf p(300, ENET_PACKET_FLAG_RELIABLE);
        loopvrev(tokens)
        {
            token &t = tokens[i];
            if(t.dropper == ci->clientnum && (t.team == team ? ++totalfriendly > TOKENLIMIT : ++totalenemy > TOKENLIMIT))
            {
                if(!expired) putint(p, N_EXPIRETOKENS);
                expired++;
                putint(p, t.id);
                tokens.removeunordered(i);        
            }
        }
        if(expired) putint(p, -1);
        putint(p, N_DROPTOKENS);
        putint(p, ci->clientnum);
        putint(p, int(ci->state.o.x*DMF));
        putint(p, int(ci->state.o.y*DMF));
        putint(p, int(ci->state.o.z*DMF));
        int numdrops = 1 + (penalty ? 0 : ci->state.tokens), yaw = rnd(360);
        loopi(numdrops)
        {
            token &t = droptoken(ci->state.o, yaw + (i*360)/numdrops, !i ? team : -team, lastmillis, ci->clientnum); 
            putint(p, t.id);
            putint(p, t.team);
            putint(p, t.yaw);
        }
        putint(p, -1);
        sendpacket(-1, 1, p.finalize());
        ci->state.tokens = 0;
    }
 
    void leavegame(clientinfo *ci, bool disconnecting = false)
    {
        ci->state.tokens = 0;
    }

    void died(clientinfo *ci, clientinfo *actor)
    {
        droptokens(ci, !actor || isteam(actor->team, ci->team));
    }

    bool canchangeteam(clientinfo *ci, const char *oldteam, const char *newteam)
    {
        return collectteambase(newteam) > 0;
    }

    void changeteam(clientinfo *ci, const char *oldteam, const char *newteam)
    {
    }

    void deposittokens(clientinfo *ci, int i)
    {
        if(notgotbases || !bases.inrange(i) || ci->state.state!=CS_ALIVE || !ci->team[0] || ci->state.tokens <= 0) return;
        base &b = bases[i];
        if(!collectbaseteam(b.team)) return;
        int team = collectteambase(ci->team);
        if(b.team==team) return;
        ci->state.flags += ci->state.tokens;
        int score = addscore(team, ci->state.tokens);
        sendf(-1, 1, "ri7", N_DEPOSITTOKENS, ci->clientnum, i, ci->state.tokens, team, score, ci->state.flags);
        ci->state.tokens = 0;
        if(score >= SCORELIMIT) startintermission();
    }

    void taketoken(clientinfo *ci, int id)
    {
        if(notgotbases || ci->state.state!=CS_ALIVE || !ci->team[0]) return;
        token *t = findtoken(id);
        if(!t) return;
        int team = collectteambase(ci->team);
        if(t->team != team && (t->team > 0 || -t->team == team) && ci->state.tokens < TOKENLIMIT) ci->state.tokens++;
        sendf(-1, 1, "ri4", N_TAKETOKEN, ci->clientnum, id, ci->state.tokens);  
    }

    void update()
    {
        if(gamemillis>=gamelimit || notgotbases) return;
        vector<int> resets;
        loopvrev(tokens)
        {
            token &t = tokens[i];
            if(lastmillis - t.droptime >= EXPIRETOKENTIME)
            {
                resets.add(t.id);
                tokens.removeunordered(i);
            }
        }
        if(resets.length())
            sendf(-1, 1, "rivi", N_EXPIRETOKENS, resets.length(), resets.getbuf(), -1);
    }

    void initclient(clientinfo *ci, packetbuf &p, bool connecting)
    {
        putint(p, N_INITTOKENS);
        loopk(2) putint(p, scores[k]);
        putint(p, tokens.length());
        loopv(tokens)
        {
            token &t = tokens[i];
            putint(p, t.id);
            putint(p, t.team);
            putint(p, t.yaw);
            putint(p, int(t.o.x*DMF));
            putint(p, int(t.o.y*DMF));
            putint(p, int(t.o.z*DMF));
        }
        loopv(clients) if(clients[i]->state.state == CS_ALIVE && clients[i]->state.tokens > 0)
        {
            putint(p, clients[i]->clientnum);
            putint(p, clients[i]->state.tokens);
        }
        putint(p, -1);
    }

    void parsebases(ucharbuf &p, bool commit)
    {
        int numbases = getint(p);
        loopi(numbases)
        {
            int team = getint(p);
            vec o;
            loopk(3) o[k] = max(getint(p)/DMF, 0.0f);
            if(p.overread()) break;
            if(commit && notgotbases)
            {
                addbase(i, o, team);
            }
        }
        if(commit && notgotbases)
        {
            notgotbases = false;
        }
    }
};
#else
    static const int TOKENHEIGHT = 5;
    static const int RESPAWNSECS = 5;

    void preload()
    {
        preloadmodel("base/red");
        preloadmodel("base/blue");
        preloadmodel("skull/red");
        preloadmodel("skull/blue");
    }

    void drawblip(fpsent *d, float x, float y, float s, const vec &pos)
    {
        float scale = calcradarscale();
        vec dir = d->o;
        dir.sub(pos).div(scale);
        float size = 0.05f,
              xoffset = -size,
              yoffset = -size,
              dist = dir.magnitude2(), maxdist = 1 - 0.05f - 0.05f;
        if(dist >= maxdist) dir.mul(maxdist/dist);
        dir.rotate_around_z(-camera1->yaw*RAD);
        drawradar(x + s*0.5f*(1.0f + dir.x + xoffset), y + s*0.5f*(1.0f + dir.y + yoffset), size*s);
    }

    void drawblip(fpsent *d, float x, float y, float s, int i)
    {
        base &b = bases[i];
        settexture(b.team==collectteambase(player1->team) ? "packages/hud/blip_blue.png" : "packages/hud/blip_red.png", 3);
        drawblip(d, x, y, s, b.o);
    }

    int clipconsole(int w, int h)
    {
        return (h*(1 + 1 + 10))/(4*10);
    }

    void drawhud(fpsent *d, int w, int h)
    {
        if(d->state == CS_ALIVE && d->tokens > 0)
        {
            int x = HICON_X + 3*HICON_STEP + (d->quadmillis ? HICON_SIZE + HICON_SPACE : 0);
            glPushMatrix();
            glScalef(2, 2, 1);
            draw_textf("%d", (x + HICON_SIZE + HICON_SPACE)/2, HICON_TEXTY/2, d->tokens);
            glPopMatrix();
            drawicon(HICON_TOKEN, x, HICON_Y);
        }

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
        loopv(bases)
        {
            base &b = bases[i];
            if(!collectbaseteam(b.team)) continue;
            drawblip(d, x, y, s, i);
        }
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

    void rendergame()
    {
        int team = collectteambase(player1->team);
        vec theight(0, 0, 0);
        abovemodel(theight, "skull/red");
        loopv(bases)
        {
            base &b = bases[i];
            const char *basename = b.team==team ? "base/blue" : "base/red";
            rendermodel(&b.light, basename, ANIM_MAPMODEL|ANIM_LOOP, b.o, 0, 0, MDL_SHADOW | MDL_CULL_VFC | MDL_CULL_OCCLUDED);
            float fradius = 1.0f, fheight = 0.5f;
            regular_particle_flame(PART_FLAME, vec(b.tokenpos.x, b.tokenpos.y, b.tokenpos.z - 4.5f), fradius, fheight, b.team==team ? 0x2020FF : 0x802020, 3, 2.0f);
            vec tokenpos(b.tokenpos);
            tokenpos.z -= theight.z/2 + sinf(lastmillis/100.0f)/20;
            rendermodel(&b.light, b.team==team ? "skull/blue" : "skull/red", ANIM_MAPMODEL|ANIM_LOOP, tokenpos, lastmillis/10.0f, 0, MDL_SHADOW | MDL_CULL_VFC | MDL_CULL_OCCLUDED);
            formatstring(b.info)("%d", totalscore(b.team));
            vec above(b.tokenpos);
            above.z += TOKENHEIGHT;
            if(b.info[0]) particle_text(above, b.info, PART_TEXT, 1, b.team==team ? 0x6496FF : 0xFF4B19, 2.0f);
        }
        loopv(tokens)
        {
            token &t = tokens[i];
            vec p = t.o;
            p.z += 1+sinf(lastmillis/100.0+t.o.x+t.o.y)/20;
            rendermodel(&t.light, t.team == team || (t.team < 0 && -t.team != team) ? "skull/blue" : "skull/red", ANIM_MAPMODEL|ANIM_LOOP, p, lastmillis/10.0f, 0, MDL_SHADOW | MDL_CULL_VFC | MDL_CULL_DIST | MDL_CULL_OCCLUDED);
        }
        fpsent *exclude = isthirdperson() ? NULL : hudplayer();
        loopv(players)
        {
            fpsent *d = players[i];
            if(d->state != CS_ALIVE || d->tokens <= 0 || d == exclude) continue;
            vec pos = d->abovehead().add(vec(0, 0, 1));
            entitylight light;
            lightreaching(pos, light.color, light.dir, true);
            int dteam = collectteambase(d->team);
            loopj(d->tokens)
            {
                rendermodel(&light, dteam != team ? "skull/blue" : "skull/red", ANIM_MAPMODEL|ANIM_LOOP, pos, d->yaw+90, 0, MDL_SHADOW | MDL_CULL_VFC | MDL_CULL_DIST | MDL_CULL_OCCLUDED);
                pos.z += TOKENHEIGHT + 1;
            }
        }        
    }

    void setup()
    {
        resetbases();
        vector<extentity *> radarents;
        loopv(entities::ents)
        {
            extentity *e = entities::ents[i];
            if(e->type!=FLAG) continue;
            if(e->attr2<1 || e->attr2>2) continue;
            int index = bases.length();
            if(!addbase(index, e->o, e->attr2)) continue;
            base &b = bases[index];
            b.tokenpos = b.o;
            abovemodel(b.tokenpos, "base/blue");
            b.tokenpos.z += TOKENHEIGHT-2;
            b.light = e->light;
        }
    }

    void senditems(packetbuf &p)
    {
        putint(p, N_INITTOKENS);
        putint(p, bases.length());
        loopv(bases)
        {
            base &b = bases[i];
            putint(p, b.team);
            loopk(3) putint(p, int(b.o[k]*DMF));
        }
    }

    vec movetoken(const vec &o, int yaw)
    {
        static struct dropent : physent
        {
            dropent()
            {
                type = ENT_CAMERA;
                collidetype = COLLIDE_AABB;
            }
        } d;
        d.o = o;
        d.o.z += 4;
        d.radius = d.xradius = d.yradius = 4;
        d.eyeheight = d.aboveeye = 4;
        vecfromyawpitch(yaw, 0, 1, 0, d.vel);
        d.o.add(vec(d.vel).mul(4));
        movecamera(&d, d.vel, TOKENDIST-4, 1);
        if(!droptofloor(d.o, 4, 4)) return vec(-1, -1, -1);
        return d.o;
    }
        
    void parsetokens(ucharbuf &p, bool commit)
    {
        loopk(2)
        {
            int score = getint(p);
            if(commit) scores[k] = score;
        }
        int numtokens = getint(p);
        loopi(numtokens)
        {
            int id = getint(p), team = getint(p), yaw = getint(p);
            vec o;
            loopk(3) o[k] = getint(p)/DMF;
            if(p.overread()) break;
            o = movetoken(o, yaw);
            if(o.z >= 0) droptoken(id, o, team, lastmillis);
        }
        for(;;)
        {
            int cn = getint(p);
            if(cn < 0) break;
            int tokens = getint(p);
            if(p.overread()) break;
            fpsent *d = cn == player1->clientnum ? player1 : newclient(cn);
            if(d) d->tokens = tokens;
        }
    }

    void expiretoken(int id)
    {
        token *t = findtoken(id);
        if(!t) return;
        playsound(S_ITEMAMMO, &t->o);
        removetoken(id);
    }

    void taketoken(fpsent *d, int id, int total)
    {
        int team = collectteambase(d->team);
        token *t = findtoken(id);
        if(t) 
        {
            playsound(t->team == team || (t->team < 0 && -t->team != team) ? S_ITEMAMMO : S_ITEMHEALTH, d!=player1 ? &d->o : NULL);
            removetoken(id);
        }
        d->tokens = total;
    }
        
    void droptoken(fpsent *d, int id, const vec &o, int team, int yaw, int n)
    {
        vec pos = movetoken(o, yaw);
        if(pos.z < 0) return;
        token &t = droptoken(id, pos, team, lastmillis);
        lightreaching(vec(o).add(vec(0, 0, TOKENHEIGHT)), t.light.color, t.light.dir, true); 
        if(!n) playsound(S_ITEMSPAWN, &d->o);
    }

    void deposittokens(fpsent *d, int i, int deposited, int team, int score, int flags)
    {
        if(bases.inrange(i))
        {
            base &b = bases[i];
            playsound(S_FLAGSCORE, d != player1 ? &b.tokenpos : NULL);
        }
        d->tokens = 0;
        d->flags = flags;
        setscore(team, score);
    }

    void checkitems(fpsent *d)
    {
        if(d->state!=CS_ALIVE) return;
        vec o = d->feetpos();
        if(d->tokens > 0) 
        {
            int team = collectteambase(d->team);
            loopv(bases)
            {
                base &b = bases[i];
                if(!collectbaseteam(b.team) || b.team == team) continue;
                if(insidebase(b, o))
                {
                    addmsg(N_DEPOSITTOKENS, "rci", d, i);
                    d->tokens = 0;
                }
            }
        }
        if(d->tokens < TOKENLIMIT) loopv(tokens)
        {
            token &t = tokens[i];
            if(o.dist(t.o) < TOKENRADIUS && d->lastcollect.dist(t.o) >= TOKENRADIUS && (lookupmaterial(o)&MATF_CLIP) != MAT_GAMECLIP && (lookupmaterial(t.o)&MATF_CLIP) != MAT_GAMECLIP)
                addmsg(N_TAKETOKEN, "rci", d, t.id);
        }
        d->lastcollect = o;
    }

    int respawnwait(fpsent *d)
    {
        return max(0, RESPAWNSECS-(lastmillis-d->lastpain)/1000);
    }

    void pickspawn(fpsent *d)
    {
        findplayerspawn(d, -1, collectteambase(d->team));
    }

    const char *prefixnextmap() { return "ctf_"; }

    bool aicheck(fpsent *d, ai::aistate &b)
    {
        if(ai::badhealth(d)) return false;
        int team = collectteambase(d->team), best = -1;
        float bestdist = 1e16f;
        if(d->tokens > 0)
        {
            loopv(bases)
            {
                base &b = bases[i];
                if(b.team == team) continue;
                float dist = d->o.dist(b.o);
                if(best < 0 || dist < bestdist) { best = i; bestdist = dist; }
            }
            if(best < 0 || !ai::makeroute(d, b, bases[best].o)) return false;
            d->ai->switchstate(b, ai::AI_S_PURSUE, ai::AI_T_AFFINITY, -(best+1));
        }
        else
        {
            loopv(tokens)
            {
                token &t = tokens[i];
                float dist = d->o.dist(t.o)/(t.team != team && (t.team > 0 || -t.team == team) ? 10.0f : 1.0f);
                if(best < 0 || dist < bestdist) { best = i; bestdist = dist; } 
            }
            if(best < 0 || !ai::makeroute(d, b, tokens[best].o)) return false;
            d->ai->switchstate(b, ai::AI_S_PURSUE, ai::AI_T_AFFINITY, tokens[best].id);
        }
        return true;
    }

    void aifind(fpsent *d, ai::aistate &b, vector<ai::interest> &interests)
    {
        vec pos = d->feetpos();
        int team = collectteambase(d->team);
        if(d->tokens > 0)
        {
            loopv(bases)
            {
                base &b = bases[i];
                if(b.team == team) continue;
                ai::interest &n = interests.add();
                n.state = ai::AI_S_PURSUE;
                n.node = ai::closestwaypoint(b.o, ai::SIGHTMIN, true);
                n.target = -(i+1);
                n.targtype = ai::AI_T_AFFINITY;
                n.score = pos.squaredist(b.o)/(d->tokens > 2 ? 1e3f : 1e2f);
            }
        }
        if(d->tokens < TOKENLIMIT) loopv(tokens)
        {
            token &t = tokens[i];
            ai::interest &n = interests.add();
            n.state = ai::AI_S_PURSUE;
            n.node = ai::closestwaypoint(t.o, ai::SIGHTMIN, true);
            n.target = t.id;
            n.targtype = ai::AI_T_AFFINITY;
            n.score = pos.squaredist(t.o)/(t.team != team && (t.team > 0 || -t.team == team) ? 10.0f : 1.0f);
        } 
    }
            
    bool aipursue(fpsent *d, ai::aistate &b)
    {
        if(b.target < 0)
        {
            if(d->tokens <= 0 || !bases.inrange(-(b.target+1))) return false;
            base &g = bases[-(b.target+1)];
            if(g.team == collectteambase(d->team)) return false;
            return ai::makeroute(d, b, g.o);
        }
        else if(b.target > 0)
        {
            token *t = findtoken(b.target);
            if(t) return ai::makeroute(d, b, t->o);
        }
        return false;
    }
};

#endif

#elif SERVMODE

case N_INITTOKENS:
    if(smode==&collectmode) collectmode.parsebases(p, (ci->state.state!=CS_SPECTATOR || ci->privilege || ci->local) && !strcmp(ci->clientmap, smapname));
    break;

case N_TAKETOKEN:
{
    int id = getint(p);
    if((ci->state.state!=CS_SPECTATOR || ci->local || ci->privilege) && cq && smode==&collectmode) collectmode.taketoken(cq, id);
    break;
}

case N_DEPOSITTOKENS:
{
    int id = getint(p);
    if((ci->state.state!=CS_SPECTATOR || ci->local || ci->privilege) && cq && smode==&collectmode) collectmode.deposittokens(cq, id);
    break;
}

#else

case N_INITTOKENS:
    collectmode.parsetokens(p, m_collect);
    break;

case N_TAKETOKEN:
{
    int ocn = getint(p), id = getint(p), total = getint(p);
    fpsent *o = ocn==player1->clientnum ? player1 : newclient(ocn);
    if(o && m_collect) collectmode.taketoken(o, id, total);
    break;
}

case N_EXPIRETOKENS:
    for(;;)
    {
        int id = getint(p);
        if(p.overread() || id < 0) break;
        if(m_collect) collectmode.expiretoken(id);
    }
    break;

case N_DROPTOKENS:
{
    int ocn = getint(p);
    fpsent *o = ocn==player1->clientnum ? player1 : newclient(ocn);
    vec droploc;
    loopk(3) droploc[k] = getint(p)/DMF;
    for(int n = 0;; n++)
    {
        int id = getint(p);
        if(id < 0) break;
        int team = getint(p), yaw = getint(p);
        if(p.overread()) break;
        if(o && m_collect) collectmode.droptoken(d, id, droploc, team, yaw, n);
    }
    break;
}

case N_DEPOSITTOKENS:
{
    int ocn = getint(p), base = getint(p), deposited = getint(p), team = getint(p), score = getint(p), flags = getint(p);
    fpsent *o = ocn==player1->clientnum ? player1 : newclient(ocn);
    if(o && m_collect) collectmode.deposittokens(o, base, deposited, team, score, flags);
    break;
}

#endif

