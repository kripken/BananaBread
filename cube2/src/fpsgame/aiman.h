// server-side ai manager
namespace aiman
{
    bool dorefresh = false;
    VARN(serverbotlimit, botlimit, 0, 8, MAXBOTS);
    VARN(serverbotbalance, botbalance, 0, 1, 1);

    void calcteams(vector<teamscore> &teams)
    {
        const char *defaults[2] = { "good", "evil" };
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->state.state==CS_SPECTATOR || !ci->team[0]) continue;
            teamscore *t = NULL;
            loopvj(teams) if(!strcmp(teams[j].team, ci->team)) { t = &teams[j]; break; }
            if(t) t->score++;
            else teams.add(teamscore(ci->team, 1));
        }
        teams.sort(teamscore::compare);
        if(teams.length() < int(sizeof(defaults)/sizeof(defaults[0])))
        {
            loopi(sizeof(defaults)/sizeof(defaults[0]))
            {
                loopvj(teams) if(!strcmp(teams[j].team, defaults[i])) goto nextteam;
                teams.add(teamscore(defaults[i], 0));
            nextteam:;
            }
        }
    }

    void balanceteams()
    {
        vector<teamscore> teams;
        calcteams(teams);
        vector<clientinfo *> reassign;
        loopv(bots) if(bots[i]) reassign.add(bots[i]);
        while(reassign.length() && teams.length() && teams[0].score > teams.last().score + 1)
        {
            teamscore &t = teams.last();
            clientinfo *bot = NULL;
            loopv(reassign) if(reassign[i] && !strcmp(reassign[i]->team, teams[0].team))
            {
                bot = reassign.removeunordered(i);
                teams[0].score--;
                t.score++;
                for(int j = teams.length() - 2; j >= 0; j--)
                {
                    if(teams[j].score >= teams[j+1].score) break;
                    swap(teams[j], teams[j+1]);
                }
                break;
            }
            if(bot)
            {
                if(smode && bot->state.state==CS_ALIVE) smode->changeteam(bot, bot->team, t.team);
                copystring(bot->team, t.team, MAXTEAMLEN+1);
                sendf(-1, 1, "riisi", N_SETTEAM, bot->clientnum, bot->team, 0);
            }
            else teams.remove(0, 1);
        }
    }

    const char *chooseteam()
    {
        vector<teamscore> teams;
        calcteams(teams);
        return teams.length() ? teams.last().team : "";
    }

    static inline bool validaiclient(clientinfo *ci)
    {
        return ci->clientnum >= 0 && ci->state.aitype == AI_NONE && (ci->state.state!=CS_SPECTATOR || ci->local || ci->privilege);
    }

	clientinfo *findaiclient(clientinfo *exclude = NULL)
	{
        clientinfo *least = NULL;
		loopv(clients)
		{
			clientinfo *ci = clients[i];
			if(!validaiclient(ci) || ci==exclude) continue;
            if(!least || ci->bots.length() < least->bots.length()) least = ci;
		}
        return least;
	}

	bool addai(int skill, int limit)
	{
		int numai = 0, cn = -1, maxai = limit >= 0 ? min(limit, MAXBOTS) : MAXBOTS;
		loopv(bots)
        {
            clientinfo *ci = bots[i];
            if(!ci || ci->ownernum < 0) { if(cn < 0) cn = i; continue; }
			numai++;
		}
		if(numai >= maxai) return false;
        if(bots.inrange(cn))
        {
            clientinfo *ci = bots[cn];
            if(ci)
            { // reuse a slot that was going to removed

                clientinfo *owner = findaiclient();
                ci->ownernum = owner ? owner->clientnum : -1;
                ci->aireinit = 2;
                dorefresh = true;
                return true;
            }
        }
        else { cn = bots.length(); bots.add(NULL); }
        const char *team = m_teammode ? chooseteam() : "";
        if(!bots[cn]) bots[cn] = new clientinfo;
        clientinfo *ci = bots[cn];
		ci->clientnum = MAXCLIENTS + cn;
		ci->state.aitype = AI_BOT;
        clientinfo *owner = findaiclient();
		ci->ownernum = owner ? owner->clientnum : -1;
        if(owner) owner->bots.add(ci);
        ci->state.skill = skill <= 0 ? rnd(50) + 51 : clamp(skill, 1, 101);
	    clients.add(ci);
		ci->state.lasttimeplayed = lastmillis;
		copystring(ci->name, "bot", MAXNAMELEN+1);
		ci->state.state = CS_DEAD;
        copystring(ci->team, team, MAXTEAMLEN+1);
        ci->playermodel = rnd(128);
		ci->aireinit = 2;
		ci->connected = true;
        dorefresh = true;
		return true;
	}

	void deleteai(clientinfo *ci)
	{
        int cn = ci->clientnum - MAXCLIENTS;
        if(!bots.inrange(cn)) return;
        if(smode) smode->leavegame(ci, true);
        sendf(-1, 1, "ri2", N_CDIS, ci->clientnum);
        clientinfo *owner = (clientinfo *)getclientinfo(ci->ownernum);
        if(owner) owner->bots.removeobj(ci);
        clients.removeobj(ci);
        DELETEP(bots[cn]);
		dorefresh = true;
	}

	bool deleteai()
	{
        loopvrev(bots) if(bots[i] && bots[i]->ownernum >= 0)
        {
			deleteai(bots[i]);
			return true;
		}
		return false;
	}

	void reinitai(clientinfo *ci)
	{
		if(ci->ownernum < 0) deleteai(ci);
		else if(ci->aireinit >= 1)
		{
			sendf(-1, 1, "ri6ss", N_INITAI, ci->clientnum, ci->ownernum, ci->state.aitype, ci->state.skill, ci->playermodel, ci->name, ci->team);
			if(ci->aireinit == 2)
            {
                ci->reassign();
                if(ci->state.state==CS_ALIVE) sendspawn(ci);
                else sendresume(ci);
            }
			ci->aireinit = 0;
		}
	}

	void shiftai(clientinfo *ci, clientinfo *owner)
	{
        clientinfo *prevowner = (clientinfo *)getclientinfo(ci->ownernum);
        if(prevowner) prevowner->bots.removeobj(ci);
		if(!owner) { ci->aireinit = 0; ci->ownernum = -1; }
		else { ci->aireinit = 2; ci->ownernum = owner->clientnum; owner->bots.add(ci); }
        dorefresh = true;
	}

	void removeai(clientinfo *ci)
	{ // either schedules a removal, or someone else to assign to

		loopvrev(ci->bots) shiftai(ci->bots[i], findaiclient(ci));
	}

	bool reassignai()
	{
        clientinfo *hi = NULL, *lo = NULL;
		loopv(clients)
		{
			clientinfo *ci = clients[i];
			if(!validaiclient(ci)) continue;
            if(!lo || ci->bots.length() < lo->bots.length()) lo = ci;
            if(!hi || ci->bots.length() > hi->bots.length()) hi = ci;
		}
		if(hi && lo && hi->bots.length() - lo->bots.length() > 1)
		{
			loopvrev(hi->bots)
			{
				shiftai(hi->bots[i], lo);
				return true;
			}
		}
		return false;
	}


	void checksetup()
	{
	    if(m_teammode && botbalance) balanceteams();
		loopvrev(bots) if(bots[i]) reinitai(bots[i]);
	}

	void clearai()
	{ // clear and remove all ai immediately
        loopvrev(bots) if(bots[i]) deleteai(bots[i]);
	}

	void checkai()
	{
        if(!dorefresh) return;
        dorefresh = false;
        if(m_botmode && numclients(-1, false, true))
		{
			checksetup();
			while(reassignai());
		}
		else clearai();
	}

	void reqadd(clientinfo *ci, int skill)
	{
        if(!ci->local && !ci->privilege) return;
        if(!addai(skill, !ci->local && ci->privilege < PRIV_ADMIN ? botlimit : -1)) sendf(ci->clientnum, 1, "ris", N_SERVMSG, "failed to create or assign bot");
	}

	void reqdel(clientinfo *ci)
	{
        if(!ci->local && !ci->privilege) return;
        if(!deleteai()) sendf(ci->clientnum, 1, "ris", N_SERVMSG, "failed to remove any bots");
	}

    void setbotlimit(clientinfo *ci, int limit)
    {
        if(ci && !ci->local && ci->privilege < PRIV_ADMIN) return;
        botlimit = clamp(limit, 0, MAXBOTS);
        dorefresh = true;
        defformatstring(msg)("bot limit is now %d", botlimit);
        sendservmsg(msg);
    }

    void setbotbalance(clientinfo *ci, bool balance)
    {
        if(ci && !ci->local && !ci->privilege) return;
        botbalance = balance ? 1 : 0;
        dorefresh = true;
        defformatstring(msg)("bot team balancing is now %s", botbalance ? "enabled" : "disabled");
        sendservmsg(msg);
    }


    void changemap()
    {
        dorefresh = true;
        loopv(clients) if(clients[i]->local || clients[i]->privilege) return;
        if(!botbalance) setbotbalance(NULL, true);
    }

    void addclient(clientinfo *ci)
    {
        if(ci->state.aitype == AI_NONE) dorefresh = true;
    }

    void changeteam(clientinfo *ci)
    {
        if(ci->state.aitype == AI_NONE) dorefresh = true;
    }
}
