struct fpsent;

#define MAXBOTS 32

enum { AI_NONE = 0, AI_BOT, AI_MAX };
#define isaitype(a) (a >= 0 && a <= AI_MAX-1)

namespace ai
{
    const int MAXWAYPOINTS = USHRT_MAX - 2;
    const int MAXWAYPOINTLINKS = 6;
    const int WAYPOINTRADIUS = 16;

    const float MINWPDIST       = 4.f;     // is on top of
    const float CLOSEDIST       = 32.f;    // is close
    const float FARDIST         = 128.f;   // too far to remap close
    const float JUMPMIN         = 4.f;     // decides to jump
    const float JUMPMAX         = 32.f;    // max jump
    const float SIGHTMIN        = 64.f;    // minimum line of sight
    const float SIGHTMAX        = 1024.f;  // maximum line of sight
    const float VIEWMIN         = 90.f;    // minimum field of view
    const float VIEWMAX         = 180.f;   // maximum field of view

    struct waypoint
    {
        vec o;
        float curscore, estscore;
		int weight;
        ushort route, prev;
        ushort links[MAXWAYPOINTLINKS];

        waypoint() {}
        waypoint(const vec &o, int weight = 0) : o(o), weight(weight), route(0) { memset(links, 0, sizeof(links)); }

        int score() const { return int(curscore) + int(estscore); }

        int find(int wp)
		{
			loopi(MAXWAYPOINTLINKS) if(links[i] == wp) return i;
			return -1;
		}

		bool haslinks() { return links[0]!=0; }
    };
    extern vector<waypoint> waypoints;

    static inline bool iswaypoint(int n)
    {
        return n > 0 && n < waypoints.length();
    }

    extern int showwaypoints, dropwaypoints;
    extern int closestwaypoint(const vec &pos, float mindist, bool links, fpsent *d = NULL);
    extern void findwaypointswithin(const vec &pos, float mindist, float maxdist, vector<int> &results);
	extern void inferwaypoints(fpsent *d, const vec &o, const vec &v, float mindist = ai::CLOSEDIST);

    struct avoidset
    {
        struct obstacle
        {
            void *owner;
            int numwaypoints;
            float above;

            obstacle(void *owner, float above = -1) : owner(owner), numwaypoints(0), above(above) {}
        };

        vector<obstacle> obstacles;
        vector<int> waypoints;

        void clear()
        {
            obstacles.setsize(0);
            waypoints.setsize(0);
        }

        void add(void *owner, float above)
        {
            obstacles.add(obstacle(owner, above));
        }

        void add(void *owner, float above, int wp)
        {
            if(obstacles.empty() || owner != obstacles.last().owner) add(owner, above);
            obstacles.last().numwaypoints++;
            waypoints.add(wp);
        }

		void add(avoidset &avoid)
		{
			waypoints.put(avoid.waypoints.getbuf(), avoid.waypoints.length());
			loopv(avoid.obstacles)
			{
				obstacle &o = avoid.obstacles[i];
				if(obstacles.empty() || o.owner != obstacles.last().owner) add(o.owner, o.above);
				obstacles.last().numwaypoints += o.numwaypoints;
			}
		}

        void avoidnear(void *owner, float above, const vec &pos, float limit);

        #define loopavoid(v, d, body) \
            if(!(v).obstacles.empty()) \
            { \
                int cur = 0; \
                loopv((v).obstacles) \
                { \
                    const ai::avoidset::obstacle &ob = (v).obstacles[i]; \
                    int next = cur + ob.numwaypoints; \
                    if(ob.owner != d) \
                    { \
                        for(; cur < next; cur++) \
                        { \
                            int wp = (v).waypoints[cur]; \
                            body; \
                        } \
                    } \
                    cur = next; \
                } \
            }

        bool find(int n, fpsent *d) const
        {
            loopavoid(*this, d, { if(wp == n) return true; });
            return false;
        }

        int remap(fpsent *d, int n, vec &pos, bool retry = false);
    };

    extern bool route(fpsent *d, int node, int goal, vector<int> &route, const avoidset &obstacles, int retries = 0);
    extern void navigate();
    extern void clearwaypoints(bool full = false);
    extern void seedwaypoints();
    extern void loadwaypoints(bool force = false, const char *mname = NULL);
    extern void savewaypoints(bool force = false, const char *mname = NULL);

    // ai state information for the owner client
    enum
    {
        AI_S_WAIT = 0,      // waiting for next command
        AI_S_DEFEND,        // defend goal target
        AI_S_PURSUE,        // pursue goal target
        AI_S_INTEREST,      // interest in goal entity
        AI_S_MAX
    };

    enum
    {
        AI_T_NODE,
        AI_T_PLAYER,
        AI_T_AFFINITY,
        AI_T_ENTITY,
        AI_T_MAX
    };

    struct interest
    {
        int state, node, target, targtype;
        float score;
        interest() : state(-1), node(-1), target(-1), targtype(-1), score(0.f) {}
        ~interest() {}
    };

    struct aistate
    {
        int type, millis, targtype, target, idle;
        bool override;

        aistate(int m, int t, int r = -1, int v = -1) : type(t), millis(m), targtype(r), target(v)
        {
            reset();
        }
        ~aistate() {}

        void reset()
        {
            idle = 0;
            override = false;
        }
    };

    const int NUMPREVNODES = 6;

    struct aiinfo
    {
        vector<aistate> state;
        vector<int> route;
        vec target, spot;
        int enemy, enemyseen, enemymillis, weappref, prevnodes[NUMPREVNODES], targnode, targlast, targtime, targseq,
            lastrun, lasthunt, lastaction, lastcheck, jumpseed, jumprand, blocktime, huntseq, blockseq, lastaimrnd;
        float targyaw, targpitch, views[3], aimrnd[3];
        bool dontmove, becareful, tryreset, trywipe;

        aiinfo()
        {
            clearsetup();
            reset();
            loopk(3) views[k] = 0.f;
        }
        ~aiinfo() {}

		void clearsetup()
		{
         	weappref = GUN_PISTOL;
            spot = target = vec(0, 0, 0);
            lastaction = lasthunt = lastcheck = enemyseen = enemymillis = blocktime = huntseq = blockseq = targtime = targseq = lastaimrnd = 0;
            lastrun = jumpseed = lastmillis;
            jumprand = lastmillis+5000;
            targnode = targlast = enemy = -1;
		}

		void clear(bool prev = false)
		{
            if(prev) memset(prevnodes, -1, sizeof(prevnodes));
            route.setsize(0);
		}

        void wipe(bool prev = false)
        {
            clear(prev);
            state.setsize(0);
            addstate(AI_S_WAIT);
            trywipe = false;
        }

        void clean(bool tryit = false)
        {
            if(!tryit) becareful = dontmove = false;
            targyaw = rnd(360);
            targpitch = 0.f;
            tryreset = tryit;
        }

        void reset(bool tryit = false) { wipe(); clean(tryit); }

        bool hasprevnode(int n) const
        {
            loopi(NUMPREVNODES) if(prevnodes[i] == n) return true;
            return false;
        }

        void addprevnode(int n)
        {
            if(prevnodes[0] != n)
            {
                memmove(&prevnodes[1], prevnodes, sizeof(prevnodes) - sizeof(prevnodes[0]));
                prevnodes[0] = n;
            }
        }

        aistate &addstate(int t, int r = -1, int v = -1)
        {
            return state.add(aistate(lastmillis, t, r, v));
        }

        void removestate(int index = -1)
        {
            if(index < 0) state.pop();
            else if(state.inrange(index)) state.remove(index);
            if(!state.length()) addstate(AI_S_WAIT);
        }

        aistate &getstate(int idx = -1)
        {
            if(state.inrange(idx)) return state[idx];
            return state.last();
        }

		aistate &switchstate(aistate &b, int t, int r = -1, int v = -1)
		{
			if((b.type == t && b.targtype == r) || (b.type == AI_S_INTEREST && b.targtype == AI_T_NODE))
			{
				b.millis = lastmillis;
				b.target = v;
				b.reset();
				return b;
			}
			return addstate(t, r, v);
		}
    };

	extern avoidset obstacles;
    extern vec aitarget;

    extern float viewdist(int x = 101);
    extern float viewfieldx(int x = 101);
    extern float viewfieldy(int x = 101);
    extern bool targetable(fpsent *d, fpsent *e);
    extern bool cansee(fpsent *d, vec &x, vec &y, vec &targ = aitarget);

    extern void init(fpsent *d, int at, int on, int sk, int bn, int pm, const char *name, const char *team);
    extern void update();
    extern void avoid();
    extern void think(fpsent *d, bool run);

    extern bool badhealth(fpsent *d);
    extern bool checkothers(vector<int> &targets, fpsent *d = NULL, int state = -1, int targtype = -1, int target = -1, bool teams = false);
    extern bool makeroute(fpsent *d, aistate &b, int node, bool changed = true, int retries = 0);
    extern bool makeroute(fpsent *d, aistate &b, const vec &pos, bool changed = true, int retries = 0);
    extern bool randomnode(fpsent *d, aistate &b, const vec &pos, float guard = SIGHTMIN, float wander = SIGHTMAX);
    extern bool randomnode(fpsent *d, aistate &b, float guard = SIGHTMIN, float wander = SIGHTMAX);
    extern bool violence(fpsent *d, aistate &b, fpsent *e, int pursue = 0);
    extern bool patrol(fpsent *d, aistate &b, const vec &pos, float guard = SIGHTMIN, float wander = SIGHTMAX, int walk = 1, bool retry = false);
    extern bool defend(fpsent *d, aistate &b, const vec &pos, float guard = SIGHTMIN, float wander = SIGHTMAX, int walk = 1);
    extern void assist(fpsent *d, aistate &b, vector<interest> &interests, bool all = false, bool force = false);
    extern bool parseinterests(fpsent *d, aistate &b, vector<interest> &interests, bool override = false, bool ignore = false);

	extern void spawned(fpsent *d);
	extern void damaged(fpsent *d, fpsent *e);
	extern void killed(fpsent *d, fpsent *e);
	extern void itemspawned(int ent);

    extern void render();
}


