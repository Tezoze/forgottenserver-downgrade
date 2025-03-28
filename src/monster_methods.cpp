#include "otpch.h"
#include "monster.h"
#include "game.h"
#include "scheduler.h"
#include "tasks.h"

extern Game g_game;
extern Scheduler g_scheduler;

// Define createSchedulerTask with proper TaskFunc signature
SchedulerTask* createSchedulerTask(uint32_t delay, TaskFunc&& f)
{
    return new SchedulerTask(delay, std::move(f));
}

void Monster::onAttackedCreatureKilled(Creature* target)
{
    if (target == this) {
        return;
    }

    // Reset aggression
    g_scheduler.addEvent(createSchedulerTask(1000, [this]() { 
        setAttackedCreature(nullptr); 
    }));
}

void Monster::onIdleStatus()
{
    if (getHealth() <= 0) {
        return;
    }

    updateIdleStatus();

    if (isMasterInRange || !isSummon()) {
        if (updateLookDirection()) {
            g_game.internalCreatureTurn(this, getDirection());
        }
    }
}

void Monster::onAttacked()
{
    // Monsters become aggressive when attacked
    if (isSummon()) {
        if (getMaster() && getMaster()->getAttackedCreature()) {
            if (!getMaster()->getAttackedCreature()->isRemoved() && 
                getMaster()->getAttackedCreature()->getHealth() > 0) {
                selectTarget(getMaster()->getAttackedCreature());
            }
        }
    }
}

void Monster::onAttackedCreatureDrainHealth(Creature* target, int32_t points)
{
    // Example implementation for when monster drains health from a target
    if (points > 0 && target) {
        // Heal 10% of damage done
        int32_t healed = points / 10;
        if (healed > 0) {
            g_game.addMagicEffect(getPosition(), CONST_ME_MAGIC_BLUE);
            g_game.addCreatureHealth(this);
        }
    }
}

bool Monster::onKilledCreature(Creature* target, [[maybe_unused]] bool lastHit)
{
    if (target->getMonster() && isFriend(target->getMonster())) {
        return false; // Don't get rewards for killing friends
    }

    // Notify master if monster is a summon
    if (isSummon() && getMaster()) {
        getMaster()->onAttackedCreatureKilled(target);
    }

    // Set the monster to move around randomly after killing its target
    if (attackedCreature == target) {
        attackedCreature = nullptr;
    }

    return true;
}

void Monster::onGainExperience(uint64_t gainExp, [[maybe_unused]] Creature* target)
{
    if (getMaster()) {
        // Distribute experience to the master
        gainExp = gainExp / 2;
        getMaster()->onGainExperience(gainExp, target);
    }
}

// Match the onWalk signature from Creature class
void Monster::onWalk(Direction& dir)
{
    if (getHealth() <= 0) {
        return;
    }

    if (walkDelay > 0) {
        if (attackTicks) {
            dir = DIRECTION_NONE;
        } else {
            walkDelay--;
            return;
        }
    }

    Creature::onWalk(dir);
} 