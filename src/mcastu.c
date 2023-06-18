/* NetHack 3.6	mcastu.c	$NHDT-Date: 1567418129 2019/09/02 09:55:29 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.55 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2011. */
/* NetHack may be freely redistributed.  See license for details. */

/* JNetHack Copyright */
/* (c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000  */
/* For 3.4-, Copyright (c) SHIRAKATA Kentaro, 2002-2023            */
/* JNetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* monster mage spells */
enum mcast_mage_spells {
    MGC_PSI_BOLT = 0,
    MGC_CURE_SELF,
    MGC_HASTE_SELF,
    MGC_STUN_YOU,
    MGC_DISAPPEAR,
    MGC_WEAKEN_YOU,
    MGC_DESTRY_ARMR,
    MGC_CURSE_ITEMS,
    MGC_AGGRAVATION,
    MGC_SUMMON_MONS,
    MGC_CLONE_WIZ,
    MGC_DEATH_TOUCH
};

/* monster cleric spells */
enum mcast_cleric_spells {
    CLC_OPEN_WOUNDS = 0,
    CLC_CURE_SELF,
    CLC_CONFUSE_YOU,
    CLC_PARALYZE,
    CLC_BLIND_YOU,
    CLC_INSECTS,
    CLC_CURSE_ITEMS,
    CLC_LIGHTNING,
    CLC_FIRE_PILLAR,
    CLC_GEYSER
};

STATIC_DCL void FDECL(cursetxt, (struct monst *, BOOLEAN_P));
STATIC_DCL int FDECL(choose_magic_spell, (int));
STATIC_DCL int FDECL(choose_clerical_spell, (int));
STATIC_DCL int FDECL(m_cure_self, (struct monst *, int));
STATIC_DCL void FDECL(cast_wizard_spell, (struct monst *, int, int));
STATIC_DCL void FDECL(cast_cleric_spell, (struct monst *, int, int));
STATIC_DCL boolean FDECL(is_undirected_spell, (unsigned int, int));
STATIC_DCL boolean
FDECL(spell_would_be_useless, (struct monst *, unsigned int, int));

extern const char *const flash_types[]; /* from zap.c */

/* feedback when frustrated monster couldn't cast a spell */
STATIC_OVL
void
cursetxt(mtmp, undirected)
struct monst *mtmp;
boolean undirected;
{
    if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)) {
        const char *point_msg; /* spellcasting monsters are impolite */

        if (undirected)
/*JP
            point_msg = "all around, then curses";
*/
            point_msg = "�������ʂ�";
        else if ((Invis && !perceives(mtmp->data)
                  && (mtmp->mux != u.ux || mtmp->muy != u.uy))
                 || is_obj_mappear(&youmonst, STRANGE_OBJECT)
                 || u.uundetected)
/*JP
            point_msg = "and curses in your general direction";
*/
            point_msg = "���Ȃ��̂��邠�����";
        else if (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy))
/*JP
            point_msg = "and curses at your displaced image";
*/
            point_msg = "���Ȃ��̌��e��";
        else
/*JP
            point_msg = "at you, then curses";
*/
            point_msg = "���Ȃ���";

/*JP
        pline("%s points %s.", Monnam(mtmp), point_msg);
*/
        pline("%s��%s�w�����C�􂢂��������D", Monnam(mtmp), point_msg);
    } else if ((!(moves % 4) || !rn2(4))) {
        if (!Deaf)
#if 0 /*JP*/
            Norep("You hear a mumbled curse.");   /* Deaf-aware */
#else
            /*JP:TODO:Deaf�Ή�*/
            Norep("�􂢂̌��t���Ԃ₭���𕷂����D");
#endif
    }
}

/* convert a level based random selection into a specific mage spell;
   inappropriate choices will be screened out by spell_would_be_useless() */
STATIC_OVL int
choose_magic_spell(spellval)
int spellval;
{
    /* for 3.4.3 and earlier, val greater than 22 selected the default spell
     */
    while (spellval > 24 && rn2(25))
        spellval = rn2(spellval);

    switch (spellval) {
    case 24:
    case 23:
        if (Antimagic || Hallucination)
            return MGC_PSI_BOLT;
        /*FALLTHRU*/
    case 22:
    case 21:
    case 20:
        return MGC_DEATH_TOUCH;
    case 19:
    case 18:
        return MGC_CLONE_WIZ;
    case 17:
    case 16:
    case 15:
        return MGC_SUMMON_MONS;
    case 14:
    case 13:
        return MGC_AGGRAVATION;
    case 12:
    case 11:
    case 10:
        return MGC_CURSE_ITEMS;
    case 9:
    case 8:
        return MGC_DESTRY_ARMR;
    case 7:
    case 6:
        return MGC_WEAKEN_YOU;
    case 5:
    case 4:
        return MGC_DISAPPEAR;
    case 3:
        return MGC_STUN_YOU;
    case 2:
        return MGC_HASTE_SELF;
    case 1:
        return MGC_CURE_SELF;
    case 0:
    default:
        return MGC_PSI_BOLT;
    }
}

/* convert a level based random selection into a specific cleric spell */
STATIC_OVL int
choose_clerical_spell(spellnum)
int spellnum;
{
    /* for 3.4.3 and earlier, num greater than 13 selected the default spell
     */
    while (spellnum > 15 && rn2(16))
        spellnum = rn2(spellnum);

    switch (spellnum) {
    case 15:
    case 14:
        if (rn2(3))
            return CLC_OPEN_WOUNDS;
        /*FALLTHRU*/
    case 13:
        return CLC_GEYSER;
    case 12:
        return CLC_FIRE_PILLAR;
    case 11:
        return CLC_LIGHTNING;
    case 10:
    case 9:
        return CLC_CURSE_ITEMS;
    case 8:
        return CLC_INSECTS;
    case 7:
    case 6:
        return CLC_BLIND_YOU;
    case 5:
    case 4:
        return CLC_PARALYZE;
    case 3:
    case 2:
        return CLC_CONFUSE_YOU;
    case 1:
        return CLC_CURE_SELF;
    case 0:
    default:
        return CLC_OPEN_WOUNDS;
    }
}

/* return values:
 * 1: successful spell
 * 0: unsuccessful spell
 */
int
castmu(mtmp, mattk, thinks_it_foundyou, foundyou)
register struct monst *mtmp;
register struct attack *mattk;
boolean thinks_it_foundyou;
boolean foundyou;
{
    int dmg, ml = mtmp->m_lev;
    int ret;
    int spellnum = 0;

    /* Three cases:
     * -- monster is attacking you.  Search for a useful spell.
     * -- monster thinks it's attacking you.  Search for a useful spell,
     *    without checking for undirected.  If the spell found is directed,
     *    it fails with cursetxt() and loss of mspec_used.
     * -- monster isn't trying to attack.  Select a spell once.  Don't keep
     *    searching; if that spell is not useful (or if it's directed),
     *    return and do something else.
     * Since most spells are directed, this means that a monster that isn't
     * attacking casts spells only a small portion of the time that an
     * attacking monster does.
     */
    if ((mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) && ml) {
        int cnt = 40;

        do {
            spellnum = rn2(ml);
            if (mattk->adtyp == AD_SPEL)
                spellnum = choose_magic_spell(spellnum);
            else
                spellnum = choose_clerical_spell(spellnum);
            /* not trying to attack?  don't allow directed spells */
            if (!thinks_it_foundyou) {
                if (!is_undirected_spell(mattk->adtyp, spellnum)
                    || spell_would_be_useless(mtmp, mattk->adtyp, spellnum)) {
                    if (foundyou)
                        impossible(
                       "spellcasting monster found you and doesn't know it?");
                    return 0;
                }
                break;
            }
        } while (--cnt > 0
                 && spell_would_be_useless(mtmp, mattk->adtyp, spellnum));
        if (cnt == 0)
            return 0;
    }

    /* monster unable to cast spells? */
    if (mtmp->mcan || mtmp->mspec_used || !ml) {
        cursetxt(mtmp, is_undirected_spell(mattk->adtyp, spellnum));
        return (0);
    }

    if (mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) {
        mtmp->mspec_used = 10 - mtmp->m_lev;
        if (mtmp->mspec_used < 2)
            mtmp->mspec_used = 2;
    }

    /* monster can cast spells, but is casting a directed spell at the
       wrong place?  If so, give a message, and return.  Do this *after*
       penalizing mspec_used. */
    if (!foundyou && thinks_it_foundyou
        && !is_undirected_spell(mattk->adtyp, spellnum)) {
#if 0 /*JP:T*/
        pline("%s casts a spell at %s!",
              canseemon(mtmp) ? Monnam(mtmp) : "Something",
              levl[mtmp->mux][mtmp->muy].typ == WATER ? "empty water"
                                                      : "thin air");
#else
        pline("%s�͉����Ȃ�%s�ɖ��@���������I",
              canseemon(mtmp) ? Monnam(mtmp) : "���҂�",
              levl[mtmp->mux][mtmp->muy].typ == WATER ? "����"
                                                      : "���");
#endif
        return (0);
    }

    nomul(0);
    if (rn2(ml * 10) < (mtmp->mconf ? 100 : 20)) { /* fumbled attack */
        if (canseemon(mtmp) && !Deaf)
/*JP
            pline_The("air crackles around %s.", mon_nam(mtmp));
*/
            pline("%s�̉��̋�C���p�`�p�`�������ĂĂ���D", mon_nam(mtmp));
        return (0);
    }
    if (canspotmon(mtmp) || !is_undirected_spell(mattk->adtyp, spellnum)) {
#if 0 /*JP*/
        pline("%s casts a spell%s!",
              canspotmon(mtmp) ? Monnam(mtmp) : "Something",
              is_undirected_spell(mattk->adtyp, spellnum)
                  ? ""
                  : (Invis && !perceives(mtmp->data)
                     && (mtmp->mux != u.ux || mtmp->muy != u.uy))
                        ? " at a spot near you"
                        : (Displaced
                           && (mtmp->mux != u.ux || mtmp->muy != u.uy))
                              ? " at your displaced image"
                              : " at you");
#else
        const char *who = (canspotmon(mtmp) ? Monnam(mtmp) : "���҂�");
        if(is_undirected_spell(mattk->adtyp, spellnum)){
            pline("%s�͎������������I", who);
        } else {
            pline("%s�͂��Ȃ�%s�ɖ��@���������I",
                  who,
                  (Invisible && !perceives(mtmp->data) && 
                   (mtmp->mux != u.ux || mtmp->muy != u.uy)) ?
                  "�̂�������" :
                  (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy)) ?
                  "�̌��e" :
                  "");
        }
#endif
    }

    /*
     * As these are spells, the damage is related to the level
     * of the monster casting the spell.
     */
    if (!foundyou) {
        dmg = 0;
        if (mattk->adtyp != AD_SPEL && mattk->adtyp != AD_CLRC) {
            impossible(
              "%s casting non-hand-to-hand version of hand-to-hand spell %d?",
                       Monnam(mtmp), mattk->adtyp);
            return (0);
        }
    } else if (mattk->damd)
        dmg = d((int) ((ml / 2) + mattk->damn), (int) mattk->damd);
    else
        dmg = d((int) ((ml / 2) + 1), 6);
    if (Half_spell_damage)
        dmg = (dmg + 1) / 2;

    ret = 1;

    switch (mattk->adtyp) {
    case AD_FIRE:
/*JP
        pline("You're enveloped in flames.");
*/
        You("���ɂ܂ꂽ�D");
        if (Fire_resistance) {
            shieldeff(u.ux, u.uy);
/*JP
            pline("But you resist the effects.");
*/
            pline("�������C���Ȃ��͉e�����󂯂Ȃ��D");
            dmg = 0;
        }
        burn_away_slime();
        break;
    case AD_COLD:
/*JP
        pline("You're covered in frost.");
*/
        You("�X�ɕ���ꂽ�D");
        if (Cold_resistance) {
            shieldeff(u.ux, u.uy);
/*JP
            pline("But you resist the effects.");
*/
            pline("�������C���Ȃ��͉e�����󂯂Ȃ��D");
            dmg = 0;
        }
        break;
    case AD_MAGM:
/*JP
        You("are hit by a shower of missiles!");
*/
        You("���@�̖����������I");
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
/*JP
            pline_The("missiles bounce off!");
*/
            pline("���@�̖�͔��˂����I");
            dmg = 0;
        } else
            dmg = d((int) mtmp->m_lev / 2 + 1, 6);
        break;
    case AD_SPEL: /* wizard spell */
    case AD_CLRC: /* clerical spell */
    {
        if (mattk->adtyp == AD_SPEL)
            cast_wizard_spell(mtmp, dmg, spellnum);
        else
            cast_cleric_spell(mtmp, dmg, spellnum);
        dmg = 0; /* done by the spell casting functions */
        break;
    }
    }
    if (dmg)
        mdamageu(mtmp, dmg);
    return (ret);
}

STATIC_OVL int
m_cure_self(mtmp, dmg)
struct monst *mtmp;
int dmg;
{
    if (mtmp->mhp < mtmp->mhpmax) {
        if (canseemon(mtmp))
/*JP
            pline("%s looks better.", Monnam(mtmp));
*/
            pline("%s�͋C�����悭�Ȃ����悤���D", Monnam(mtmp));
        /* note: player healing does 6d4; this used to do 1d8 */
        if ((mtmp->mhp += d(3, 6)) > mtmp->mhpmax)
            mtmp->mhp = mtmp->mhpmax;
        dmg = 0;
    }
    return dmg;
}

/* monster wizard and cleric spellcasting functions */
/*
   If dmg is zero, then the monster is not casting at you.
   If the monster is intentionally not casting at you, we have previously
   called spell_would_be_useless() and spellnum should always be a valid
   undirected spell.
   If you modify either of these, be sure to change is_undirected_spell()
   and spell_would_be_useless().
 */
STATIC_OVL
void
cast_wizard_spell(mtmp, dmg, spellnum)
struct monst *mtmp;
int dmg;
int spellnum;
{
    if (dmg == 0 && !is_undirected_spell(AD_SPEL, spellnum)) {
        impossible("cast directed wizard spell (%d) with dmg=0?", spellnum);
        return;
    }

    switch (spellnum) {
    case MGC_DEATH_TOUCH:
/*JP
        pline("Oh no, %s's using the touch of death!", mhe(mtmp));
*/
        pline("�Ȃ�Ă��������C%s�͎��̐鍐���g���Ă���I", mhe(mtmp));
        if (nonliving(youmonst.data) || is_demon(youmonst.data)) {
/*JP
            You("seem no deader than before.");
*/
            You("����ȏ㎀�˂Ȃ��悤���D");
        } else if (!Antimagic && rn2(mtmp->m_lev) > 12) {
            if (Hallucination) {
/*JP
                You("have an out of body experience.");
*/
                You("�H�̗��E��̌������D");
            } else {
                killer.format = KILLED_BY_AN;
/*JP
                Strcpy(killer.name, "touch of death");
*/
                Strcpy(killer.name, "���̐鍐��");
                done(DIED);
            }
        } else {
            if (Antimagic)
                shieldeff(u.ux, u.uy);
/*JP
            pline("Lucky for you, it didn't work!");
*/
            pline("�^�̂悢���ƂɂȂ�Ƃ��Ȃ������I");
        }
        dmg = 0;
        break;
    case MGC_CLONE_WIZ:
        if (mtmp->iswiz && context.no_of_wizards == 1) {
/*JP
            pline("Double Trouble...");
*/
            pline("��d�ꂾ�D�D�D");
            clonewiz();
            dmg = 0;
        } else
            impossible("bad wizard cloning?");
        break;
    case MGC_SUMMON_MONS: {
#if 0 /*JP*/
        int count;

        count = nasty(mtmp); /* summon something nasty */
#else
        nasty(mtmp); /* summon something nasty */
#endif
        if (mtmp->iswiz) {
/*JP
            verbalize("Destroy the thief, my pet%s!", plur(count));
*/
            verbalize("�������E���I�䂪���l��I");
        } else {
#if 0 /*JP*/
            const char *mappear = (count == 1) ? "A monster appears"
                                               : "Monsters appear";
#endif

            /* messages not quite right if plural monsters created but
               only a single monster is seen */
            if (Invis && !perceives(mtmp->data)
                && (mtmp->mux != u.ux || mtmp->muy != u.uy))
#if 0 /*JP:T*/
                pline("%s around a spot near you!", mappear);
#else
                pline("���������Ȃ��̂������΂Ɍ��ꂽ�I");
#endif
            else if (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy))
#if 0 /*JP:T*/
                pline("%s around your displaced image!", mappear);
#else
                pline("���������Ȃ��̌��e�̂������΂Ɍ��ꂽ�I");
#endif
            else
#if 0 /*JP:T*/
                pline("%s from nowhere!", mappear);
#else
                pline("�������ǂ�����Ƃ��Ȃ����ꂽ�I");
#endif
        }
        dmg = 0;
        break;
    }
    case MGC_AGGRAVATION:
/*JP
        You_feel("that monsters are aware of your presence.");
*/
        You_feel("�������������Ȃ��̑��݂ɋC�t�����悤�ȋC�������D");
        aggravate();
        dmg = 0;
        break;
    case MGC_CURSE_ITEMS:
/*JP
        You_feel("as if you need some help.");
*/
        You_feel("�������K�v�ȋC�������D");
        rndcurse();
        dmg = 0;
        break;
    case MGC_DESTRY_ARMR:
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
/*JP
            pline("A field of force surrounds you!");
*/
            pline("�s�v�c�ȗ͂����Ȃ����Ƃ�܂����I");
        } else if (!destroy_arm(some_armor(&youmonst))) {
/*JP
            Your("skin itches.");
*/
            You("���Y���Y�����D");
        }
        dmg = 0;
        break;
    case MGC_WEAKEN_YOU: /* drain strength */
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
/*JP
            You_feel("momentarily weakened.");
*/
            You_feel("��u�キ�Ȃ����悤�ȋC�������D");
        } else {
/*JP
            You("suddenly feel weaker!");
*/
            You("�ˑR�キ�Ȃ����悤�ȋC�������D");
            dmg = mtmp->m_lev - 6;
            if (Half_spell_damage)
                dmg = (dmg + 1) / 2;
            losestr(rnd(dmg));
            if (u.uhp < 1)
                done_in_by(mtmp, DIED);
        }
        dmg = 0;
        break;
    case MGC_DISAPPEAR: /* makes self invisible */
        if (!mtmp->minvis && !mtmp->invis_blkd) {
            if (canseemon(mtmp))
#if 0 /*JP:T*/
                pline("%s suddenly %s!", Monnam(mtmp),
                      !See_invisible ? "disappears" : "becomes transparent");
#else
                pline("%s�͓ˑR%s�I", Monnam(mtmp),
                      !See_invisible ? "������" : "�����ɂȂ���");
#endif
            mon_set_minvis(mtmp);
            if (cansee(mtmp->mx, mtmp->my) && !canspotmon(mtmp))
                map_invisible(mtmp->mx, mtmp->my);
            dmg = 0;
        } else
            impossible("no reason for monster to cast disappear spell?");
        break;
    case MGC_STUN_YOU:
        if (Antimagic || Free_action) {
            shieldeff(u.ux, u.uy);
            if (!Stunned)
/*JP
                You_feel("momentarily disoriented.");
*/
                You("��u�������o���������D");
            make_stunned(1L, FALSE);
        } else {
/*JP
            You(Stunned ? "struggle to keep your balance." : "reel...");
*/
            You(Stunned ? "�o�����X����낤�Ƃ��������D" : "���߂����D�D�D");
            dmg = d(ACURR(A_DEX) < 12 ? 6 : 4, 4);
            if (Half_spell_damage)
                dmg = (dmg + 1) / 2;
            make_stunned((HStun & TIMEOUT) + (long) dmg, FALSE);
        }
        dmg = 0;
        break;
    case MGC_HASTE_SELF:
        mon_adjust_speed(mtmp, 1, (struct obj *) 0);
        dmg = 0;
        break;
    case MGC_CURE_SELF:
        dmg = m_cure_self(mtmp, dmg);
        break;
    case MGC_PSI_BOLT:
        /* prior to 3.4.0 Antimagic was setting the damage to 1--this
           made the spell virtually harmless to players with magic res. */
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            dmg = (dmg + 1) / 2;
        }
        if (dmg <= 5)
/*JP
            You("get a slight %sache.", body_part(HEAD));
*/
            You("�������%s�ɂ������D",body_part(HEAD));
        else if (dmg <= 10)
/*JP
            Your("brain is on fire!");
*/
            You("�{��ɂ܂ꂽ�I");
        else if (dmg <= 20)
/*JP
            Your("%s suddenly aches painfully!", body_part(HEAD));
*/
            You("�ˑR%s�ɂɂ�����ꂽ�I", body_part(HEAD));
        else
/*JP
            Your("%s suddenly aches very painfully!", body_part(HEAD));
*/
            You("�ˑR������%s�ɂɂ�����ꂽ�I", body_part(HEAD));
        break;
    default:
        impossible("mcastu: invalid magic spell (%d)", spellnum);
        dmg = 0;
        break;
    }

    if (dmg)
        mdamageu(mtmp, dmg);
}

STATIC_OVL
void
cast_cleric_spell(mtmp, dmg, spellnum)
struct monst *mtmp;
int dmg;
int spellnum;
{
    if (dmg == 0 && !is_undirected_spell(AD_CLRC, spellnum)) {
        impossible("cast directed cleric spell (%d) with dmg=0?", spellnum);
        return;
    }

    switch (spellnum) {
    case CLC_GEYSER:
        /* this is physical damage (force not heat),
         * not magical damage or fire damage
         */
/*JP
        pline("A sudden geyser slams into you from nowhere!");
*/
        pline("�������ǂ�����Ƃ��Ȃ�����Ă��Ȃ���ł������I");
        dmg = d(8, 6);
        if (Half_physical_damage)
            dmg = (dmg + 1) / 2;
        break;
    case CLC_FIRE_PILLAR:
/*JP
        pline("A pillar of fire strikes all around you!");
*/
        pline("���Ȃ��̎���ɉΒ����������I");
        if (Fire_resistance) {
            shieldeff(u.ux, u.uy);
            dmg = 0;
        } else
            dmg = d(8, 6);
        if (Half_spell_damage)
            dmg = (dmg + 1) / 2;
        burn_away_slime();
        (void) burnarmor(&youmonst);
        destroy_item(SCROLL_CLASS, AD_FIRE);
        destroy_item(POTION_CLASS, AD_FIRE);
        destroy_item(SPBOOK_CLASS, AD_FIRE);
        (void) burn_floor_objects(u.ux, u.uy, TRUE, FALSE);
        break;
    case CLC_LIGHTNING: {
        boolean reflects;

/*JP
        pline("A bolt of lightning strikes down at you from above!");
*/
        pline("���Ȃ��̐^�ォ���Ȃ��~�蒍�����I");
/*JP
        reflects = ureflects("It bounces off your %s%s.", "");
*/
        reflects = ureflects("����͂��Ȃ���%s%s�Œ��˕Ԃ����D", "");
        if (reflects || Shock_resistance) {
            shieldeff(u.ux, u.uy);
            dmg = 0;
            if (reflects)
                break;
        } else
            dmg = d(8, 6);
        if (Half_spell_damage)
            dmg = (dmg + 1) / 2;
        destroy_item(WAND_CLASS, AD_ELEC);
        destroy_item(RING_CLASS, AD_ELEC);
        (void) flashburn((long) rnd(100));
        break;
    }
    case CLC_CURSE_ITEMS:
/*JP
        You_feel("as if you need some help.");
*/
        You_feel("�������K�v�ȋC�������D");
        rndcurse();
        dmg = 0;
        break;
    case CLC_INSECTS: {
        /* Try for insects, and if there are none
           left, go for (sticks to) snakes.  -3. */
        struct permonst *pm = mkclass(S_ANT, 0);
        struct monst *mtmp2 = (struct monst *) 0;
        char let = (pm ? S_ANT : S_SNAKE);
        boolean success = FALSE, seecaster;
        int i, quan, oldseen, newseen;
        coord bypos;
        const char *fmt;

        oldseen = monster_census(TRUE);
        quan = (mtmp->m_lev < 2) ? 1 : rnd((int) mtmp->m_lev / 2);
        if (quan < 3)
            quan = 3;
        for (i = 0; i <= quan; i++) {
            if (!enexto(&bypos, mtmp->mux, mtmp->muy, mtmp->data))
                break;
            if ((pm = mkclass(let, 0)) != 0
                && (mtmp2 = makemon(pm, bypos.x, bypos.y, MM_ANGRY)) != 0) {
                success = TRUE;
                mtmp2->msleeping = mtmp2->mpeaceful = mtmp2->mtame = 0;
                set_malign(mtmp2);
            }
        }
        newseen = monster_census(TRUE);

        /* not canspotmon(), which includes unseen things sensed via warning
         */
        seecaster = canseemon(mtmp) || tp_sensemon(mtmp) || Detect_monsters;

        fmt = 0;
        if (!seecaster) {
            char *arg; /* [not const: upstart(N==1 ? an() : makeplural())] */
/*JP
            const char *what = (let == S_SNAKE) ? "snake" : "insect";
*/
            const char *what = (let == S_SNAKE) ? "�w�r" : "��";

            if (newseen <= oldseen || Unaware) {
                /* unseen caster fails or summons unseen critters,
                   or unconscious hero ("You dream that you hear...") */
/*JP
                You_hear("someone summoning %s.", makeplural(what));
*/
                You_hear("�N����%s���������Ă���̂𕷂����D", what);
            } else {
                /* unseen caster summoned seen critter(s) */
                arg = (newseen == oldseen + 1) ? an(what) : makeplural(what);
                if (!Deaf)
#if 0 /*JP:T*/
                    You_hear("someone summoning something, and %s %s.", arg,
                             vtense(arg, "appear"));
#else
                    You_hear("�N������������������̂𕷂����C������%s�����ꂽ�D",
                             arg);
#endif
                else
/*JP
                    pline("%s %s.", upstart(arg), vtense(arg, "appear"));
*/
                    pline("%s�����ꂽ�D", arg);
            }

        /* seen caster, possibly producing unseen--or just one--critters;
           hero is told what the caster is doing and doesn't necessarily
           observe complete accuracy of that caster's results (in other
           words, no need to fuss with visibility or singularization;
           player is told what's happening even if hero is unconscious) */
        } else if (!success)
/*JP
            fmt = "%s casts at a clump of sticks, but nothing happens.";
*/
            fmt = "%s�͖_�؂�ɖ��@�����������C�Ȃɂ�������Ȃ������D";
        else if (let == S_SNAKE)
/*JP
            fmt = "%s transforms a clump of sticks into snakes!";
*/
            fmt = "%s�͖_�؂���w�r�ɕς����I";
        else if (Invis && !perceives(mtmp->data)
                 && (mtmp->mux != u.ux || mtmp->muy != u.uy))
/*JP
            fmt = "%s summons insects around a spot near you!";
*/
            fmt = "%s�͒������Ȃ��̂������΂ɏ��������I";
        else if (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy))
/*JP
            fmt = "%s summons insects around your displaced image!";
*/
            fmt = "%s�͒������Ȃ��̌��e�̎���ɏ��������I";
        else
/*JP
            fmt = "%s summons insects!";
*/
            fmt = "%s�͒������������I";
        if (fmt)
            pline(fmt, Monnam(mtmp));

        dmg = 0;
        break;
    }
    case CLC_BLIND_YOU:
        /* note: resists_blnd() doesn't apply here */
        if (!Blinded) {
#if 0 /*JP:T*/
            int num_eyes = eyecount(youmonst.data);
            pline("Scales cover your %s!", (num_eyes == 1)
                                               ? body_part(EYE)
                                               : makeplural(body_part(EYE)));
#else
            pline("�؂����Ȃ���%s�𕢂����I", body_part(EYE));
#endif
            make_blinded(Half_spell_damage ? 100L : 200L, FALSE);
            if (!Blind)
                Your1(vision_clears);
            dmg = 0;
        } else
            impossible("no reason for monster to cast blindness spell?");
        break;
    case CLC_PARALYZE:
        if (Antimagic || Free_action) {
            shieldeff(u.ux, u.uy);
            if (multi >= 0)
/*JP
                You("stiffen briefly.");
*/
                You("��u�d�������D");
            nomul(-1);
/*JP
            multi_reason = "paralyzed by a monster";
*/
            multi_reason = "�����ɖ�Ⴢ�����ꂽ����";
        } else {
            if (multi >= 0)
/*JP
                You("are frozen in place!");
*/
                You("���̏�œ����Ȃ��Ȃ����I");
            dmg = 4 + (int) mtmp->m_lev;
            if (Half_spell_damage)
                dmg = (dmg + 1) / 2;
            nomul(-dmg);
/*JP
            multi_reason = "paralyzed by a monster";
*/
            multi_reason = "�����ɖ�Ⴢ�����ꂽ����";
        }
        nomovemsg = 0;
        dmg = 0;
        break;
    case CLC_CONFUSE_YOU:
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
/*JP
            You_feel("momentarily dizzy.");
*/
            You("��u�߂܂��������D");
        } else {
            boolean oldprop = !!Confusion;

            dmg = (int) mtmp->m_lev;
            if (Half_spell_damage)
                dmg = (dmg + 1) / 2;
            make_confused(HConfusion + dmg, TRUE);
            if (Hallucination)
/*JP
                You_feel("%s!", oldprop ? "trippier" : "trippy");
*/
                You("%s�ւ�ւ�ɂȂ����I", oldprop ? "������" : "");
            else
/*JP
                You_feel("%sconfused!", oldprop ? "more " : "");
*/
                You("%s���������I", oldprop ? "������" : "");
        }
        dmg = 0;
        break;
    case CLC_CURE_SELF:
        dmg = m_cure_self(mtmp, dmg);
        break;
    case CLC_OPEN_WOUNDS:
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            dmg = (dmg + 1) / 2;
        }
        if (dmg <= 5)
/*JP
            Your("skin itches badly for a moment.");
*/
            Your("�畆�͈�u�C���Y���Y���Ƃ����D");
        else if (dmg <= 10)
/*JP
            pline("Wounds appear on your body!");
*/
            pline("�������Ȃ��̑̂ɏo�����I");
        else if (dmg <= 20)
/*JP
            pline("Severe wounds appear on your body!");
*/
            pline("�Ђǂ��������Ȃ��̑̂ɏo�����I");
        else
/*JP
            Your("body is covered with painful wounds!");
*/
            pline("�̂������炯�ɂȂ����I");
        break;
    default:
        impossible("mcastu: invalid clerical spell (%d)", spellnum);
        dmg = 0;
        break;
    }

    if (dmg)
        mdamageu(mtmp, dmg);
}

STATIC_DCL
boolean
is_undirected_spell(adtyp, spellnum)
unsigned int adtyp;
int spellnum;
{
    if (adtyp == AD_SPEL) {
        switch (spellnum) {
        case MGC_CLONE_WIZ:
        case MGC_SUMMON_MONS:
        case MGC_AGGRAVATION:
        case MGC_DISAPPEAR:
        case MGC_HASTE_SELF:
        case MGC_CURE_SELF:
            return TRUE;
        default:
            break;
        }
    } else if (adtyp == AD_CLRC) {
        switch (spellnum) {
        case CLC_INSECTS:
        case CLC_CURE_SELF:
            return TRUE;
        default:
            break;
        }
    }
    return FALSE;
}

/* Some spells are useless under some circumstances. */
STATIC_DCL
boolean
spell_would_be_useless(mtmp, adtyp, spellnum)
struct monst *mtmp;
unsigned int adtyp;
int spellnum;
{
    /* Some spells don't require the player to really be there and can be cast
     * by the monster when you're invisible, yet still shouldn't be cast when
     * the monster doesn't even think you're there.
     * This check isn't quite right because it always uses your real position.
     * We really want something like "if the monster could see mux, muy".
     */
    boolean mcouldseeu = couldsee(mtmp->mx, mtmp->my);

    if (adtyp == AD_SPEL) {
        /* aggravate monsters, etc. won't be cast by peaceful monsters */
        if (mtmp->mpeaceful
            && (spellnum == MGC_AGGRAVATION || spellnum == MGC_SUMMON_MONS
                || spellnum == MGC_CLONE_WIZ))
            return TRUE;
        /* haste self when already fast */
        if (mtmp->permspeed == MFAST && spellnum == MGC_HASTE_SELF)
            return TRUE;
        /* invisibility when already invisible */
        if ((mtmp->minvis || mtmp->invis_blkd) && spellnum == MGC_DISAPPEAR)
            return TRUE;
        /* peaceful monster won't cast invisibility if you can't see
           invisible,
           same as when monsters drink potions of invisibility.  This doesn't
           really make a lot of sense, but lets the player avoid hitting
           peaceful monsters by mistake */
        if (mtmp->mpeaceful && !See_invisible && spellnum == MGC_DISAPPEAR)
            return TRUE;
        /* healing when already healed */
        if (mtmp->mhp == mtmp->mhpmax && spellnum == MGC_CURE_SELF)
            return TRUE;
        /* don't summon monsters if it doesn't think you're around */
        if (!mcouldseeu && (spellnum == MGC_SUMMON_MONS
                            || (!mtmp->iswiz && spellnum == MGC_CLONE_WIZ)))
            return TRUE;
        if ((!mtmp->iswiz || context.no_of_wizards > 1)
            && spellnum == MGC_CLONE_WIZ)
            return TRUE;
        /* aggravation (global wakeup) when everyone is already active */
        if (spellnum == MGC_AGGRAVATION) {
            /* if nothing needs to be awakened then this spell is useless
               but caster might not realize that [chance to pick it then
               must be very small otherwise caller's many retry attempts
               will eventually end up picking it too often] */
            if (!has_aggravatables(mtmp))
                return rn2(100) ? TRUE : FALSE;
        }
    } else if (adtyp == AD_CLRC) {
        /* summon insects/sticks to snakes won't be cast by peaceful monsters
         */
        if (mtmp->mpeaceful && spellnum == CLC_INSECTS)
            return TRUE;
        /* healing when already healed */
        if (mtmp->mhp == mtmp->mhpmax && spellnum == CLC_CURE_SELF)
            return TRUE;
        /* don't summon insects if it doesn't think you're around */
        if (!mcouldseeu && spellnum == CLC_INSECTS)
            return TRUE;
        /* blindness spell on blinded player */
        if (Blinded && spellnum == CLC_BLIND_YOU)
            return TRUE;
    }
    return FALSE;
}

/* convert 1..10 to 0..9; add 10 for second group (spell casting) */
#define ad_to_typ(k) (10 + (int) k - 1)

/* monster uses spell (ranged) */
int
buzzmu(mtmp, mattk)
register struct monst *mtmp;
register struct attack *mattk;
{
    /* don't print constant stream of curse messages for 'normal'
       spellcasting monsters at range */
    if (mattk->adtyp > AD_SPC2)
        return (0);

    if (mtmp->mcan) {
        cursetxt(mtmp, FALSE);
        return (0);
    }
    if (lined_up(mtmp) && rn2(3)) {
        nomul(0);
        if (mattk->adtyp && (mattk->adtyp < 11)) { /* no cf unsigned >0 */
            if (canseemon(mtmp))
#if 0 /*JP:T*/
                pline("%s zaps you with a %s!", Monnam(mtmp),
                      flash_types[ad_to_typ(mattk->adtyp)]);
#else
                pline("%s��%s�����Ȃ��Ɍ����ĕ������D", Monnam(mtmp),
                      flash_types[ad_to_typ(mattk->adtyp)]);
#endif
            buzz(-ad_to_typ(mattk->adtyp), (int) mattk->damn, mtmp->mx,
                 mtmp->my, sgn(tbx), sgn(tby));
        } else
            impossible("Monster spell %d cast", mattk->adtyp - 1);
    }
    return (1);
}

/*mcastu.c*/
