 /*
 * Copyright (C) 2013-2015 DeathCore <http://www.noffearrdeathproject.net/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* ScriptData
SDName: Silverpine_Forest
SD%Complete: 100
SDComment: Quest support: 435
SDCategory: Silverpine Forest
EndScriptData */

/* ContentData
npc_deathstalker_erland
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"
#include "Player.h"

/*######
## npc_astor_hadren
######*/

#define GOSSIP_HAH "You're Astor Hadren, right?"
#define GOSSIP_SAH "You've got something I need, Astor. And I'll be taking it now."

class npc_astor_hadren : public CreatureScript
{
public:
    npc_astor_hadren() : CreatureScript("npc_astor_hadren") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_astor_hadrenAI(pCreature);
    }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
    {
        pPlayer->PlayerTalkClass->ClearMenus();
        switch (uiAction)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SAH, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                pPlayer->SEND_GOSSIP_MENU(624, pCreature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 2:
                pPlayer->CLOSE_GOSSIP_MENU();
                pCreature->setFaction(21);
                if (pPlayer)
                    CAST_AI(npc_astor_hadren::npc_astor_hadrenAI, pCreature->AI())->AttackStart(pPlayer);
                break;
        }
        return true;
    }

    bool OnGossipHello(Player* pPlayer, Creature* pCreature)
    {
        if (pPlayer->GetQuestStatus(1886) == QUEST_STATUS_INCOMPLETE)
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HAH, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        pPlayer->SEND_GOSSIP_MENU(623, pCreature->GetGUID());

        return true;
    }

    struct npc_astor_hadrenAI : public ScriptedAI
    {
        npc_astor_hadrenAI(Creature *c) : ScriptedAI(c) {}

        void Reset()
        {
            me->setFaction(68);
        }

        void EnterCombat(Unit* /*who*/)
        {
        }

        void JustDied(Unit * /*who*/)
        {
            me->setFaction(68);
        }
    };
};

/*######
## npc_deathstalker_erland
######*/

enum eErland
{
    SAY_QUESTACCEPT     = 0,
    SAY_START           = 1,
    SAY_AGGRO           = 2,
    SAY_PROGRESS        = 3,
    SAY_LAST            = 4,

    SAY_RANE            = 0,
    SAY_RANE_ANSWER     = 5,
    SAY_MOVE_QUINN      = 6,

    SAY_QUINN           = 7,
    SAY_QUINN_ANSWER    = 0,
    SAY_BYE             = 8,

    QUEST_ESCORTING     = 435,
    NPC_RANE            = 1950,
    NPC_QUINN           = 1951
};

class npc_deathstalker_erland : public CreatureScript
{
public:
    npc_deathstalker_erland() : CreatureScript("npc_deathstalker_erland") { }

    struct npc_deathstalker_erlandAI : public npc_escortAI
    {
        npc_deathstalker_erlandAI(Creature* creature) : npc_escortAI(creature) {}

        void WaypointReached(uint32 waypointId)
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            switch (waypointId)
            {
                case 1:
                    Talk(SAY_START, player->GetGUID());
                    break;
                case 10:
                    Talk(SAY_PROGRESS);
                    break;
                case 13:
                    Talk(SAY_LAST, player->GetGUID());
                    player->GroupEventHappens(QUEST_ESCORTING, me);
                    break;
                case 15:
                    if (Creature* rane = me->FindNearestCreature(NPC_RANE, 20.0f))
                        rane->AI()->Talk(SAY_RANE);
                    break;
                case 16:
                    Talk(SAY_RANE_ANSWER);
                    break;
                case 17:
                    Talk(SAY_MOVE_QUINN);
                    break;
                case 24:
                    Talk(SAY_QUINN);
                    break;
                case 25:
                    if (Creature* quinn = me->FindNearestCreature(NPC_QUINN, 20.0f))
                        quinn->AI()->Talk(SAY_QUINN_ANSWER);
                    break;
                case 26:
                    Talk(SAY_BYE);
                    break;
            }
        }

        void Reset() {}

        void EnterCombat(Unit* who)
        {
            Talk(SAY_AGGRO, who->GetGUID());
        }
    };

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_ESCORTING)
        {
            creature->AI()->Talk(SAY_QUESTACCEPT, player->GetGUID());

            if (npc_escortAI* pEscortAI = CAST_AI(npc_deathstalker_erland::npc_deathstalker_erlandAI, creature->AI()))
                pEscortAI->Start(true, false, player->GetGUID());
        }

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_deathstalker_erlandAI(creature);
    }
};

/*######
## pyrewood_ambush
#######*/

#define QUEST_PYREWOOD_AMBUSH 452

#define NPCSAY_INIT "Get ready, they'll be arriving any minute..." //not blizzlike
#define NPCSAY_END "Thanks for your help!" //not blizzlike

static float PyrewoodSpawnPoints[3][4] =
{
    //pos_x   pos_y     pos_z    orien
    //outside
    /*
    {-400.85f, 1513.64f, 18.67f, 0},
    {-397.32f, 1514.12f, 18.67f, 0},
    {-397.44f, 1511.09f, 18.67f, 0},
    */
    //door
    {-396.17f, 1505.86f, 19.77f, 0},
    {-396.91f, 1505.77f, 19.77f, 0},
    {-397.94f, 1504.74f, 19.77f, 0},
};

#define WAIT_SECS 6000

class pyrewood_ambush : public CreatureScript
{
public:
    pyrewood_ambush() : CreatureScript("pyrewood_ambush") { }

    bool OnQuestAccept(Player *pPlayer, Creature *pCreature, const Quest *pQuest)
    {
        if (pQuest->GetQuestId() == QUEST_PYREWOOD_AMBUSH && !CAST_AI(pyrewood_ambush::pyrewood_ambushAI, pCreature->AI())->QuestInProgress)
        {
            CAST_AI(pyrewood_ambush::pyrewood_ambushAI, pCreature->AI())->QuestInProgress = true;
            CAST_AI(pyrewood_ambush::pyrewood_ambushAI, pCreature->AI())->Phase = 0;
            CAST_AI(pyrewood_ambush::pyrewood_ambushAI, pCreature->AI())->KillCount = 0;
            CAST_AI(pyrewood_ambush::pyrewood_ambushAI, pCreature->AI())->PlayerGUID = pPlayer->GetGUID();
        }

        return true;
    }

    CreatureAI* GetAI(Creature *pCreature) const
    {
        return new pyrewood_ambushAI (pCreature);
    }

    struct pyrewood_ambushAI : public ScriptedAI
    {
        pyrewood_ambushAI(Creature *c) : ScriptedAI(c), Summons(me)
        {
           QuestInProgress = false;
        }

        uint32 Phase;
        int8 KillCount;
        uint32 WaitTimer;
        uint64 PlayerGUID;
        SummonList Summons;

        bool QuestInProgress;

        void Reset()
        {
            WaitTimer = WAIT_SECS;

            if (!QuestInProgress) //fix reset values (see UpdateVictim)
            {
                Phase = 0;
                KillCount = 0;
                PlayerGUID = 0;
                Summons.DespawnAll();
            }
        }

        void EnterCombat(Unit * /*who*/){}

        void JustSummoned(Creature *pSummoned)
        {
            Summons.Summon(pSummoned);
            ++KillCount;
        }

        void SummonedCreatureDespawn(Creature *pSummoned)
        {
            Summons.Despawn(pSummoned);
            --KillCount;
        }

        void SummonCreatureWithRandomTarget(uint32 creatureId, int position)
        {
            if (Creature *pSummoned = me->SummonCreature(creatureId, PyrewoodSpawnPoints[position][0], PyrewoodSpawnPoints[position][1], PyrewoodSpawnPoints[position][2], PyrewoodSpawnPoints[position][3], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000))
            {
                Player *pPlayer = NULL;
                Unit *pTarget = NULL;
                if (PlayerGUID)
                {
                    pPlayer = Unit::GetPlayer(*me, PlayerGUID);
                    if (pPlayer)
                        pTarget = RAND((Unit*)me, (Unit*)pPlayer);
                } else
                    pTarget = me;

                if (pTarget)
                {
                    pSummoned->setFaction(168);
                    pSummoned->AddThreat(pTarget, 32.0f);
                    pSummoned->AI()->AttackStart(pTarget);
                }
            }
        }

        void JustDied(Unit * /*pKiller*/)
        {
            if (PlayerGUID)
                if (Player *pPlayer = Unit::GetPlayer(*me, PlayerGUID))
                    if (pPlayer->GetQuestStatus(QUEST_PYREWOOD_AMBUSH) == QUEST_STATUS_INCOMPLETE)
                        pPlayer->FailQuest(QUEST_PYREWOOD_AMBUSH);
        }

        void UpdateAI(const uint32 diff)
        {
            //sLog->outString("DEBUG: p(%i) k(%i) d(%u) W(%i)", Phase, KillCount, diff, WaitTimer);

            if (!QuestInProgress)
                return;

            if (KillCount && Phase < 6)
            {
                if (!UpdateVictim()) //reset() on target Despawn...
                    return;

                DoMeleeAttackIfReady();
                return;
            }

            switch (Phase)
            {
                case 0:
                    if (WaitTimer == WAIT_SECS)
                        me->MonsterSay(NPCSAY_INIT, LANG_UNIVERSAL, 0); //no blizzlike

                    if (WaitTimer <= diff)
                    {
                        WaitTimer -= diff;
                        return;
                    }
                    break;
                case 1:
                    SummonCreatureWithRandomTarget(2060, 1);
                    break;
                case 2:
                    SummonCreatureWithRandomTarget(2061, 2);
                    SummonCreatureWithRandomTarget(2062, 0);
                    break;
                case 3:
                    SummonCreatureWithRandomTarget(2063, 1);
                    SummonCreatureWithRandomTarget(2064, 2);
                    SummonCreatureWithRandomTarget(2065, 0);
                    break;
                case 4:
                    SummonCreatureWithRandomTarget(2066, 1);
                    SummonCreatureWithRandomTarget(2067, 0);
                    SummonCreatureWithRandomTarget(2068, 2);
                    break;
                case 5: //end
                    if (PlayerGUID)
                    {
                        if (Player *pPlayer = Unit::GetPlayer(*me, PlayerGUID))
                        {
                            me->MonsterSay(NPCSAY_END, LANG_UNIVERSAL, 0); //not blizzlike
                            pPlayer->GroupEventHappens(QUEST_PYREWOOD_AMBUSH, me);
                        }
                    }
                    QuestInProgress = false;
                    Reset();
                    break;
            }
            ++Phase; //prepare next phase
        }
    };
};


/*######
## AddSC
######*/

void AddSC_silverpine_forest()
{
    new npc_astor_hadren();
    new npc_deathstalker_erland();
    new pyrewood_ambush();
}
