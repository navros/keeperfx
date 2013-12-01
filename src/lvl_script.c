/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_script.c
 *     Level script commands support.
 * @par Purpose:
 *     Load, recognize and maintain the level script.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     12 Feb 2009 - 24 Feb 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "lvl_script.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_sound.h"
#include "bflib_math.h"
#include "bflib_guibtns.h"

#include "front_simple.h"
#include "config.h"
#include "config_terrain.h"
#include "config_trapdoor.h"
#include "config_rules.h"
#include "config_lenses.h"
#include "config_magic.h"
#include "config_creature.h"
#include "gui_soundmsgs.h"
#include "frontmenu_ingame_tabs.h"
#include "player_instances.h"
#include "player_data.h"
#include "player_utils.h"
#include "thing_factory.h"
#include "thing_physics.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "thing_stats.h"
#include "creature_states.h"
#include "creature_states_hero.h"
#include "creature_groups.h"
#include "room_library.h"
#include "lvl_filesdk1.h"
#include "game_merge.h"
#include "dungeon_data.h"
#include "game_legacy.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
const struct CommandDesc command_desc[] = {
  {"CREATE_PARTY",                 "A       ", Cmd_CREATE_PARTY},
  {"ADD_TO_PARTY",                 "AANNAN  ", Cmd_ADD_TO_PARTY},
  {"ADD_PARTY_TO_LEVEL",           "AAAN    ", Cmd_ADD_PARTY_TO_LEVEL},
  {"ADD_CREATURE_TO_LEVEL",        "AAANNN  ", Cmd_ADD_CREATURE_TO_LEVEL},
  {"IF",                           "AAAN    ", Cmd_IF},
  {"IF_ACTION_POINT",              "NA      ", Cmd_IF_ACTION_POINT},
  {"ENDIF",                        "        ", Cmd_ENDIF},
  {"SET_HATE",                     "NNN     ", Cmd_SET_HATE},
  {"SET_GENERATE_SPEED",           "N       ", Cmd_SET_GENERATE_SPEED},
  {"REM",                          "        ", Cmd_REM},
  {"START_MONEY",                  "AN      ", Cmd_START_MONEY},
  {"ROOM_AVAILABLE",               "AANN    ", Cmd_ROOM_AVAILABLE},
  {"CREATURE_AVAILABLE",           "AANN    ", Cmd_CREATURE_AVAILABLE},
  {"MAGIC_AVAILABLE",              "AANN    ", Cmd_MAGIC_AVAILABLE},
  {"TRAP_AVAILABLE",               "AANN    ", Cmd_TRAP_AVAILABLE},
  {"RESEARCH",                     "AAAN    ", Cmd_RESEARCH},
  {"RESEARCH_ORDER",               "AAAN    ", Cmd_RESEARCH_ORDER},
  {"COMPUTER_PLAYER",              "AN      ", Cmd_COMPUTER_PLAYER},
  {"SET_TIMER",                    "AA      ", Cmd_SET_TIMER},
  {"ADD_TUNNELLER_TO_LEVEL",       "AAANNN  ", Cmd_ADD_TUNNELLER_TO_LEVEL},
  {"WIN_GAME",                     "        ", Cmd_WIN_GAME},
  {"LOSE_GAME",                    "        ", Cmd_LOSE_GAME},
  {"SET_FLAG",                     "AAN     ", Cmd_SET_FLAG},
  {"MAX_CREATURES",                "AN      ", Cmd_MAX_CREATURES},
  {"NEXT_COMMAND_REUSABLE",        "        ", Cmd_NEXT_COMMAND_REUSABLE},
  {"DOOR_AVAILABLE",               "AANN    ", Cmd_DOOR_AVAILABLE},
  {"DISPLAY_OBJECTIVE",            "NA      ", Cmd_DISPLAY_OBJECTIVE},
  {"DISPLAY_OBJECTIVE_WITH_POS",   "NNN     ", Cmd_DISPLAY_OBJECTIVE_WITH_POS},
  {"DISPLAY_INFORMATION",          "NA      ", Cmd_DISPLAY_INFORMATION},
  {"DISPLAY_INFORMATION_WITH_POS", "NNN     ", Cmd_DISPLAY_INFORMATION_WITH_POS},
  {"ADD_TUNNELLER_PARTY_TO_LEVEL", "AAAANNN ", Cmd_ADD_TUNNELLER_PARTY_TO_LEVEL},
  {"ADD_CREATURE_TO_POOL",         "AN      ", Cmd_ADD_CREATURE_TO_POOL},
  {"RESET_ACTION_POINT",           "N       ", Cmd_RESET_ACTION_POINT},
  {"SET_CREATURE_MAX_LEVEL",       "AAN     ", Cmd_SET_CREATURE_MAX_LEVEL},
  {"SET_MUSIC",                    "N       ", Cmd_SET_MUSIC},
  {"TUTORIAL_FLASH_BUTTON",        "NN      ", Cmd_TUTORIAL_FLASH_BUTTON},
  {"SET_CREATURE_STRENGTH",        "AN      ", Cmd_SET_CREATURE_STRENGTH},
  {"SET_CREATURE_HEALTH",          "AN      ", Cmd_SET_CREATURE_HEALTH},
  {"SET_CREATURE_ARMOUR",          "AN      ", Cmd_SET_CREATURE_ARMOUR},
  {"SET_CREATURE_FEAR_WOUNDED",    "AN      ", Cmd_SET_CREATURE_FEAR_WOUNDED},
  {"SET_CREATURE_FEAR_STRONGER",   "AN      ", Cmd_SET_CREATURE_FEAR_STRONGER},
  {"IF_AVAILABLE",                 "AAAN    ", Cmd_IF_AVAILABLE},
  {"SET_COMPUTER_GLOBALS",         "ANNNNNN ", Cmd_SET_COMPUTER_GLOBALS},
  {"SET_COMPUTER_CHECKS",          "AANNNNN ", Cmd_SET_COMPUTER_CHECKS},
  {"SET_COMPUTER_EVENT",           "AANN    ", Cmd_SET_COMPUTER_EVENT},
  {"SET_COMPUTER_PROCESS",         "AANNNNN ", Cmd_SET_COMPUTER_PROCESS},
  {"ALLY_PLAYERS",                 "AAN     ", Cmd_ALLY_PLAYERS},
  {"DEAD_CREATURES_RETURN_TO_POOL","N       ", Cmd_DEAD_CREATURES_RETURN_TO_POOL},
  {"BONUS_LEVEL_TIME",             "N       ", Cmd_BONUS_LEVEL_TIME},
  {"QUICK_OBJECTIVE",              "NAA     ", Cmd_QUICK_OBJECTIVE},
  {"QUICK_INFORMATION",            "NAA     ", Cmd_QUICK_INFORMATION},
  {"QUICK_OBJECTIVE_WITH_POS",     "NANN    ", Cmd_QUICK_OBJECTIVE_WITH_POS},
  {"QUICK_INFORMATION_WITH_POS",   "NANN    ", Cmd_QUICK_INFORMATION_WITH_POS},
  {"SWAP_CREATURE",                "AA      ", Cmd_SWAP_CREATURE},
  {"PRINT",                        "A       ", Cmd_PRINT},
  {"MESSAGE",                      "A       ", Cmd_MESSAGE},
  {"PLAY_MESSAGE",                 "AAN     ", Cmd_PLAY_MESSAGE},
  {"ADD_GOLD_TO_PLAYER",           "AN      ", Cmd_ADD_GOLD_TO_PLAYER},
  {"SET_CREATURE_TENDENCIES",      "AAN     ", Cmd_SET_CREATURE_TENDENCIES},
  {"REVEAL_MAP_RECT",              "ANNNN   ", Cmd_REVEAL_MAP_RECT},
  {"REVEAL_MAP_LOCATION",          "ANN     ", Cmd_REVEAL_MAP_LOCATION},
  {"LEVEL_VERSION",                "N       ", Cmd_LEVEL_VERSION},
  {"KILL_CREATURE",                "AAAN    ", Cmd_KILL_CREATURE},
  {NULL,                           "        ", Cmd_NONE},
};

const struct CommandDesc dk1_command_desc[] = {
  {"CREATE_PARTY",                 "A       ", Cmd_CREATE_PARTY},
  {"ADD_TO_PARTY",                 "AANNAN  ", Cmd_ADD_TO_PARTY},
  {"ADD_PARTY_TO_LEVEL",           "AAAN    ", Cmd_ADD_PARTY_TO_LEVEL},
  {"ADD_CREATURE_TO_LEVEL",        "AAANNN  ", Cmd_ADD_CREATURE_TO_LEVEL},
  {"IF",                           "AAAN    ", Cmd_IF},
  {"IF_ACTION_POINT",              "NA      ", Cmd_IF_ACTION_POINT},
  {"ENDIF",                        "        ", Cmd_ENDIF},
  {"SET_HATE",                     "NNN     ", Cmd_SET_HATE},
  {"SET_GENERATE_SPEED",           "N       ", Cmd_SET_GENERATE_SPEED},
  {"REM",                          "        ", Cmd_REM},
  {"START_MONEY",                  "AN      ", Cmd_START_MONEY},
  {"ROOM_AVAILABLE",               "AANN    ", Cmd_ROOM_AVAILABLE},
  {"CREATURE_AVAILABLE",           "AANN    ", Cmd_CREATURE_AVAILABLE},
  {"MAGIC_AVAILABLE",              "AANN    ", Cmd_MAGIC_AVAILABLE},
  {"TRAP_AVAILABLE",               "AANN    ", Cmd_TRAP_AVAILABLE},
  {"RESEARCH",                     "AAAN    ", Cmd_RESEARCH_ORDER},
  {"COMPUTER_PLAYER",              "AN      ", Cmd_COMPUTER_PLAYER},
  {"SET_TIMER",                    "AA      ", Cmd_SET_TIMER},
  {"ADD_TUNNELLER_TO_LEVEL",       "AAANNN  ", Cmd_ADD_TUNNELLER_TO_LEVEL},
  {"WIN_GAME",                     "        ", Cmd_WIN_GAME},
  {"LOSE_GAME",                    "        ", Cmd_LOSE_GAME},
  {"SET_FLAG",                     "AAN     ", Cmd_SET_FLAG},
  {"MAX_CREATURES",                "AN      ", Cmd_MAX_CREATURES},
  {"NEXT_COMMAND_REUSABLE",        "        ", Cmd_NEXT_COMMAND_REUSABLE},
  {"DOOR_AVAILABLE",               "AANN    ", Cmd_DOOR_AVAILABLE},
  {"DISPLAY_OBJECTIVE",            "NA      ", Cmd_DISPLAY_OBJECTIVE},
  {"DISPLAY_OBJECTIVE_WITH_POS",   "NNN     ", Cmd_DISPLAY_OBJECTIVE_WITH_POS},
  {"DISPLAY_INFORMATION",          "N       ", Cmd_DISPLAY_INFORMATION},
  {"DISPLAY_INFORMATION_WITH_POS", "NNN     ", Cmd_DISPLAY_INFORMATION_WITH_POS},
  {"ADD_TUNNELLER_PARTY_TO_LEVEL", "AAAANNN ", Cmd_ADD_TUNNELLER_PARTY_TO_LEVEL},
  {"ADD_CREATURE_TO_POOL",         "AN      ", Cmd_ADD_CREATURE_TO_POOL},
  {"RESET_ACTION_POINT",           "N       ", Cmd_RESET_ACTION_POINT},
  {"SET_CREATURE_MAX_LEVEL",       "AAN     ", Cmd_SET_CREATURE_MAX_LEVEL},
  {"SET_MUSIC",                    "N       ", Cmd_SET_MUSIC},
  {"TUTORIAL_FLASH_BUTTON",        "NN      ", Cmd_TUTORIAL_FLASH_BUTTON},
  {"SET_CREATURE_STRENGTH",        "AN      ", Cmd_SET_CREATURE_STRENGTH},
  {"SET_CREATURE_HEALTH",          "AN      ", Cmd_SET_CREATURE_HEALTH},
  {"SET_CREATURE_ARMOUR",          "AN      ", Cmd_SET_CREATURE_ARMOUR},
  {"SET_CREATURE_FEAR",            "AN      ", Cmd_SET_CREATURE_FEAR_WOUNDED},
  {"IF_AVAILABLE",                 "AAAN    ", Cmd_IF_AVAILABLE},
  {"SET_COMPUTER_GLOBALS",         "ANNNNNN ", Cmd_SET_COMPUTER_GLOBALS},
  {"SET_COMPUTER_CHECKS",          "AANNNNN ", Cmd_SET_COMPUTER_CHECKS},
  {"SET_COMPUTER_EVENT",           "AANN    ", Cmd_SET_COMPUTER_EVENT},
  {"SET_COMPUTER_PROCESS",         "AANNNNN ", Cmd_SET_COMPUTER_PROCESS},
  {"ALLY_PLAYERS",                 "AA      ", Cmd_ALLY_PLAYERS},
  {"DEAD_CREATURES_RETURN_TO_POOL","N       ", Cmd_DEAD_CREATURES_RETURN_TO_POOL},
  {"BONUS_LEVEL_TIME",             "N       ", Cmd_BONUS_LEVEL_TIME},
  {"QUICK_OBJECTIVE",              "NAA     ", Cmd_QUICK_OBJECTIVE},
  {"QUICK_INFORMATION",            "NA      ", Cmd_QUICK_INFORMATION},
  {"SWAP_CREATURE",                "AA      ", Cmd_SWAP_CREATURE},
  {"PRINT",                        "A       ", Cmd_PRINT},
  {"MESSAGE",                      "A       ", Cmd_MESSAGE},
  {"LEVEL_VERSION",                "N       ", Cmd_LEVEL_VERSION},
  {NULL,                           "        ", Cmd_NONE},
};

const struct NamedCommand newcrtr_desc[] = {
  {"NEW_CREATURE_A",   1},
  {"NEW_CREATURE_B",   2},
  {NULL,               0},
};

const struct NamedCommand player_desc[] = {
  {"PLAYER0",          0},
  {"PLAYER1",          1},
  {"PLAYER2",          2},
  {"PLAYER3",          3},
  {"PLAYER_GOOD",      4},
  {"ALL_PLAYERS",      ALL_PLAYERS},
  {NULL,               0},
};

const struct NamedCommand variable_desc[] = {
  {"MONEY",                       SVar_MONEY},
  {"GAME_TURN",                   SVar_GAME_TURN},
  {"BREAK_IN",                    SVar_BREAK_IN},
//{"CREATURE_NUM",                SVar_CREATURE_NUM},
  {"TOTAL_IMPS",                  SVar_TOTAL_IMPS},
  {"TOTAL_CREATURES",             SVar_TOTAL_CREATURES},
  {"TOTAL_RESEARCH",              SVar_TOTAL_RESEARCH},
  {"TOTAL_DOORS",                 SVar_TOTAL_DOORS},
  {"TOTAL_AREA",                  SVar_TOTAL_AREA},
  {"TOTAL_CREATURES_LEFT",        SVar_TOTAL_CREATURES_LEFT},
  {"CREATURES_ANNOYED",           SVar_CREATURES_ANNOYED},
  {"BATTLES_LOST",                SVar_BATTLES_LOST},
  {"BATTLES_WON",                 SVar_BATTLES_WON},
  {"ROOMS_DESTROYED",             SVar_ROOMS_DESTROYED},
  {"SPELLS_STOLEN",               SVar_SPELLS_STOLEN},
  {"TIMES_BROKEN_INTO",           SVar_TIMES_BROKEN_INTO},
  {"GOLD_POTS_STOLEN",            SVar_GOLD_POTS_STOLEN},
//  {"TIMER",                     SVar_TIMER},
  {"DUNGEON_DESTROYED",           SVar_DUNGEON_DESTROYED},
  {"TOTAL_GOLD_MINED",            SVar_TOTAL_GOLD_MINED},
//  {"FLAG",                      SVar_FLAG},
//  {"ROOM",                      SVar_ROOM_SLABS},
  {"DOORS_DESTROYED",             SVar_DOORS_DESTROYED},
  {"CREATURES_SCAVENGED_LOST",    SVar_CREATURES_SCAVENGED_LOST},
  {"CREATURES_SCAVENGED_GAINED",  SVar_CREATURES_SCAVENGED_GAINED},
  {"ALL_DUNGEONS_DESTROYED",      SVar_ALL_DUNGEONS_DESTROYED},
//  {"DOOR",                      SVar_DOOR_NUM},
  {NULL,                           0},
};

const struct NamedCommand comparison_desc[] = {
  {"==",     MOp_EQUAL},
  {"!=",     MOp_NOT_EQUAL},
  {"<",      MOp_SMALLER},
  {">",      MOp_GREATER},
  {"<=",     MOp_SMALLER_EQ},
  {">=",     MOp_GREATER_EQ},
  {NULL,     0},
};

const struct NamedCommand head_for_desc[] = {
  {"ACTION_POINT",         MLoc_ACTIONPOINT},
  {"DUNGEON",              MLoc_PLAYERSDUNGEON},
  {"DUNGEON_HEART",        MLoc_PLAYERSHEART},
  {"APPROPIATE_DUNGEON",   MLoc_APPROPRTDUNGEON},
  {NULL,                   0},
};

const struct NamedCommand timer_desc[] = {
  {"TIMER0", 0},
  {"TIMER1", 1},
  {"TIMER2", 2},
  {"TIMER3", 3},
  {"TIMER4", 4},
  {"TIMER5", 5},
  {"TIMER6", 6},
  {"TIMER7", 7},
  {NULL,     0},
};

const struct NamedCommand flag_desc[] = {
  {"FLAG0",  0},
  {"FLAG1",  1},
  {"FLAG2",  2},
  {"FLAG3",  3},
  {"FLAG4",  4},
  {"FLAG5",  5},
  {"FLAG6",  6},
  {"FLAG7",  7},
  {NULL,     0},
};

const struct NamedCommand hero_objective_desc[] = {
  {"STEAL_GOLD",           CHeroTsk_StealGold},
  {"STEAL_SPELLS",         CHeroTsk_StealSpells},
  {"ATTACK_ENEMIES",       CHeroTsk_AttackEnemies},
  {"ATTACK_DUNGEON_HEART", CHeroTsk_AttackDnHeart},
  {"ATTACK_ROOMS",         CHeroTsk_AttackRooms},
  {"DEFEND_PARTY",         CHeroTsk_DefendParty},
  {NULL,                   0},
};

const struct NamedCommand msgtype_desc[] = {
  {"SPEECH",           1},
  {"SOUND",            2},
  {NULL,               0},
};

const struct NamedCommand tendency_desc[] = {
  {"IMPRISON",         1},
  {"FLEE",             2},
  {NULL,               0},
};

const struct NamedCommand creature_select_criteria_desc[] = {
  {"MOST_EXPERIENCED",     CSelCrit_MostExperienced},
  {"LEAST_EXPERIENCED",    CSelCrit_LeastExperienced},
  {"NEAR_OWN_HEART",       CSelCrit_NearOwnHeart},
  {"NEAR_ENEMY_HEART",     CSelCrit_NearEnemyHeart},
  {"ON_ENEMY_GROUND",      CSelCrit_OnEnemyGround},
  {NULL,                   0},
};

/******************************************************************************/
DLLIMPORT void _DK_command_if_available(char *plrname, char *varib_name, char *operatr, long value);
DLLIMPORT void _DK_command_set_computer_globals(char *plrname, long apt_idx, long tngclass, long tngmodel, long tngowner, long random_factor, long a6);
DLLIMPORT void _DK_command_set_computer_checks(char *plrname, char *chkname, long apt_idx, long tngclass, long tngmodel, long tngowner, long random_factor);
DLLIMPORT void _DK_command_set_computer_events(char *plrname, char *evntname, long apt_idx, long tngclass);
DLLIMPORT void _DK_command_set_computer_process(char *plrname, char *procname, long apt_idx, long tngclass, long tngmodel, long tngowner, long random_factor);
DLLIMPORT void _DK_command_display_objective(long msg_num, char *plrname, long tngclass, long tngmodel);
DLLIMPORT void _DK_command_add_tunneller_party_to_level(char *plrname, char *prtname, char *apt_num, char *objectv, long target, char crtr_level, unsigned long carried_gold);
DLLIMPORT long _DK_script_support_setup_player_as_computer_keeper(unsigned char plyridx, long tngclass);
DLLIMPORT void _DK_command_research(char *plrname, char *trg_type, char *trg_name, unsigned long val);
DLLIMPORT void _DK_command_if_action_point(long apt_idx, char *plrname);
DLLIMPORT void _DK_command_add_tunneller_to_level(char *plrname, char *dst_place, char *objectv, long target, unsigned char crtr_level, unsigned long carried_gold);
DLLIMPORT void _DK_command_add_to_party(char *prtname, char *crtr_name, long crtr_level, long carried_gold, char *objectv, long countdown);
DLLIMPORT void _DK_command_add_party_to_level(char *plrname, char *prtname, char *dst_place, long ncopies);
DLLIMPORT void _DK_command_add_creature_to_level(char *plrname, char *crtr_name, char *dst_place, long ncopies, long crtr_level, long carried_gold);
DLLIMPORT void _DK_command_if(char *plrname, char *varib_name, char *operatr, long value);
DLLIMPORT void _DK_command_add_value(unsigned long var_index, unsigned long val1, long val2, long val3, long val4);
DLLIMPORT struct CommandDesc *_DK_get_next_word(char **line, char *params, unsigned char *line_end);
DLLIMPORT long _DK_scan_line(char *line);
DLLIMPORT short _DK_load_script(long lvl_num);
DLLIMPORT void _DK_script_process_value(unsigned long var_index, unsigned long val1, long val2, long val3, long val4);
DLLIMPORT struct Thing *_DK_script_process_new_tunneller(unsigned char apt_idx, long tngclass, unsigned char tngmodel, long tngowner, unsigned char random_factor, unsigned long a6);
DLLIMPORT struct Thing *_DK_script_process_new_party(struct Party *party, unsigned char tngclass, long tngmodel, long tngowner);
DLLIMPORT struct Thing *_DK_script_create_new_creature(unsigned char apt_idx, long tngclass, long tngmodel, long tngowner, long random_factor);
DLLIMPORT long _DK_get_condition_value(char plyr_idx, unsigned char valtype, unsigned char tngmodel);
DLLIMPORT void _DK_script_process_new_tunneller_party(unsigned char apt_idx, long tngclass, long tngmodel, unsigned char tngowner, long random_factor, unsigned char a6, unsigned long a7);
DLLIMPORT long _DK_script_support_create_thing_at_hero_door(long apt_idx, unsigned char tngclass, unsigned char tngmodel, unsigned char tngowner, unsigned char random_factor);
DLLIMPORT long _DK_script_support_create_thing_at_action_point(long apt_idx, unsigned char tngclass, unsigned char tngmodel, unsigned char tngowner, unsigned char random_factor);
DLLIMPORT long _DK_script_support_create_creature_at_dungeon_heart(unsigned char apt_idx, unsigned char tngclass, unsigned char tngmodel);
DLLIMPORT long _DK_script_support_send_tunneller_to_action_point(struct Thing *thing, long tngclass);
DLLIMPORT long _DK_script_support_send_tunneller_to_dungeon(struct Thing *thing, unsigned char tngclass);
DLLIMPORT long _DK_script_support_send_tunneller_to_dungeon_heart(struct Thing *thing, unsigned char tngclass);
DLLIMPORT long _DK_script_support_send_tunneller_to_appropriate_dungeon(struct Thing *thing);
/******************************************************************************/
/**
 * Reads word from 'line' into 'param'. Sets if 'line_end' was reached.
 */
const struct CommandDesc *get_next_word(char **line, char *param, unsigned char *line_end)
{
  const struct CommandDesc *cmnd_desc;
  long rnd_min,rnd_max;
  unsigned int pos;
  char chr;
  int i;
  SCRIPTDBG(12,"Starting");
  cmnd_desc = NULL;
  // Find start of an item to read
  pos = 0;
  param[pos] = '\0';
  while (1)
  {
    chr = **line;
    // number
    if ((isalnum(chr)) || (chr == '-'))
        break;
    // operator
    if ((chr == '\"') || (chr == '=') || (chr == '!') || (chr == '<') || (chr == '>'))
        break;
    // end of line
    if ((chr == '\r') || (chr == '\n') || (chr == '\0'))
    {
        (*line_end) = true;
        return NULL;
    }
    (*line)++;
  }

  chr = **line;
  // Text string
  if (isalpha(chr))
  {
    // Read the parameter
    while (isalnum(chr) || (chr == '_'))
    {
      param[pos] = chr;
      pos++;
      (*line)++;
      chr = **line;
      if (pos+1 >= MAX_TEXT_LENGTH) break;
    }
    param[pos] = '\0';
    strupr(param);
    // Check if it's a command
    i = 0;
    cmnd_desc = NULL;
    if (level_file_version > 0)
    {
      while (command_desc[i].textptr != NULL)
      {
        if (strcmp(param, command_desc[i].textptr) == 0)
        {
          cmnd_desc = &command_desc[i];
          break;
        }
        i++;
      }
    } else
    {
      while (dk1_command_desc[i].textptr != NULL)
      {
        if (strcmp(param, dk1_command_desc[i].textptr) == 0)
        {
          cmnd_desc = &dk1_command_desc[i];
          break;
        }
        i++;
      }
    }
    // Support of the RANDOM function
    if (strcmp(param, "RANDOM") == 0)
    {
      // Get the minimum random number
      // skip some chars at start
      pos = 0;
      chr = **line;
      while ((!isdigit(chr)) && (chr != '-'))
      {
        if ((chr == '\r') || (chr == '\n') || (chr == '\0'))
        {
          SCRPTERRLOG("Invalid first argument for RANDOM command");
          (*line_end) = true;
          return NULL;
        }
        (*line)++;
        chr = **line;
      }
      // copy the number as string
      if (chr == '-')
      {
        param[pos] = chr;
        pos++;
        (*line)++;
      }
      chr = **line;
      while ( isdigit(chr) )
      {
        param[pos] = chr;
        pos++;
        (*line)++;
        chr = **line;
      }
      param[pos] = '\0';
      rnd_min = atol(param);
      // Get the maximum random number
      // skip some chars at start
      pos = 0;
      chr = **line;
      while ((!isdigit(chr)) && (chr != '-'))
      {
        if ((chr == '\r') || (chr == '\n') || (chr == '\0'))
        {
          SCRPTERRLOG("Invalid second argument for RANDOM command");
          (*line_end) = true;
          return NULL;
        }
        (*line)++;
        chr = **line;
      }
      // copy the number as string
      if (chr == '-')
      {
        param[pos] = chr;
        pos++;
        (*line)++;
      }
      chr = **line;
      while ( isdigit(chr) )
      {
        param[pos] = chr;
        pos++;
        (*line)++;
        chr = **line;
      }
      param[pos] = '\0';
      rnd_max = atol(param);
      // Prepare the value
      itoa((rand() % (rnd_max-rnd_min+1)) + rnd_min, param, 10);
      pos = 16; // we won't have numbers greater than 16 chars
    }
  } else
  // Number string
  if (isdigit(chr) || (chr == '-'))
  {
      if (chr == '-')
      {
        param[pos] = chr;
        pos++;
        (*line)++;
      }
      chr = **line;
      if (!isdigit(chr))
      {
        SCRPTERRLOG("Unexpected '-' not followed by a number");
        return NULL;
      }
      while ( isdigit(chr) )
      {
        param[pos] = chr;
        pos++;
        (*line)++;
        chr = **line;
        if (pos+1 >= MAX_TEXT_LENGTH) break;
      }
  } else
  // Multiword string taken into quotes
  if (chr == '\"')
  {
      (*line)++;
      chr = **line;
      while ((chr != '\0') && (chr != '\n') && (chr != '\r'))
      {
        if (chr == '\"')
        {
          (*line)++;
          break;
        }
        param[pos] = chr;
        pos++;
        (*line)++;
        chr = **line;
        if (pos+1 >= MAX_TEXT_LENGTH) break;
    }
  } else
  // Other cases - only operators are left
  {
      param[pos] = chr;
      pos++;
      (*line)++;
      switch (chr)
      {
      case '!':
          chr = **line;
          if (chr != '=')
          {
            SCRPTERRLOG("Expected '=' after '!'");
            return NULL;
          }
          param[pos] = chr;
          pos++;
          (*line)++;
          break;
      case '>':
      case '<':
          chr = **line;
          if (chr == '=')
          {
            param[pos] = chr;
            pos++;
            (*line)++;
          }
          break;
      case '=':
          chr = **line;
          if (chr != '=')
          {
            SCRPTERRLOG("Expected '=' after '='");
            return 0;
          }
          param[pos] = chr;
          pos++;
          (*line)++;
          break;
      default:
          break;
      }
  }
  chr = **line;
  if ((chr == '\0') || (chr == '\r')  || (chr == '\n'))
    *line_end = true;
  param[pos] = '\0';
  return cmnd_desc;
}

const char *script_get_command_name(long cmnd_index)
{
  long i;
  i = 0;
  while (command_desc[i].textptr != NULL)
  {
    if (command_desc[i].index == cmnd_index)
      return command_desc[i].textptr;
    i++;
  }
  return NULL;
}

/**
 * Returns if the command is 'preloaded'. Preloaded commands are initialized
 * before the whole level data is loaded.
 */
TbBool script_is_preloaded_command(long cmnd_index)
{
  switch (cmnd_index)
  {
  case Cmd_SWAP_CREATURE:
  case Cmd_LEVEL_VERSION:
      return true;
  default:
      return false;
  }
}

#define get_players_range(plrname, plr_start, plr_end) get_players_range_f(plrname, plr_start, plr_end, __func__, text_line_number)
long get_players_range_f(char *plrname, int *plr_start, int *plr_end, const char *func_name, long ln_num)
{
  long plr_id;
  plr_id = get_rid(player_desc, plrname);
  if (plr_id == -1)
  {
    ERRORMSG("%s(line %lu): Invalid player name, '%s'",func_name,ln_num, plrname);
    *plr_start = 0;
    *plr_end = 0;
    return -1;
  }
  if (plr_id == ALL_PLAYERS)
  {
    *plr_start = 0;
    *plr_end = PLAYERS_COUNT;
    return plr_id;
  }
  if (plr_id < PLAYERS_COUNT)
  {
    *plr_start = plr_id;
    *plr_end = (*plr_start) + 1;
    return plr_id;
  }
  ERRORMSG("%s(line %lu): Player '%s' out of range",func_name,ln_num, plrname);
  *plr_start = 0;
  *plr_end = 0;
  return -1;
}

#define get_player_id(plrname, plr_id) get_player_id_f(plrname, plr_id, __func__, text_line_number)
TbBool get_player_id_f(char *plrname, long *plr_id, const char *func_name, long ln_num)
{
  *plr_id = get_rid(player_desc, plrname);
  if (*plr_id == -1)
  {
    ERRORMSG("%s(line %lu): Invalid player name, '%s'",func_name,ln_num, plrname);
    return false;
  }
  return true;
}

unsigned short get_map_location_type(TbMapLocation location)
{
  return location & 0x0F;
}

unsigned long get_map_location_longval(TbMapLocation location)
{
  return (location >> 4);
}

unsigned long get_map_location_plyrval(TbMapLocation location)
{
  return (location >> 12);
}

unsigned short get_map_location_plyridx(TbMapLocation location)
{
  return (location >> 4) & 0xFF;
}

/**
 * Returns location id for 1-param location from script.
 * @param locname
 * @param location
 * @return
 * @see get_map_heading_id()
 */
#define get_map_location_id(locname, location) get_map_location_id_f(locname, location, __func__, text_line_number)
TbBool get_map_location_id_f(const char *locname, TbMapLocation *location, const char *func_name, long ln_num)
{
    struct Thing *thing;
    long i;
    // If there's no locname, then coordinates are set directly as (x,y)
    if (locname == NULL)
    {
      *location = MLoc_NONE;
      return true;
    }
    // Player name means the location of player's Dungeon Heart
    i = get_rid(player_desc, locname);
    if (i != -1)
    {
      if (i != ALL_PLAYERS) {
          if (!player_has_heart(i)) {
              WARNMSG("%s(line %lu): Target player %d has no heart",func_name,ln_num, (int)i);
          }
          *location = ((unsigned long)i << 4) | MLoc_PLAYERSHEART;
      } else {
          *location = MLoc_NONE;
      }
      return true;
    }
    // Creature name means location of such creature belonging to player0
    i = get_rid(creature_desc, locname);
    if (i != -1)
    {
        *location = ((unsigned long)i << 12) | MLoc_CREATUREKIND;
        return true;
    }
    // Room name means location of such room belonging to player0
    i = get_rid(room_desc, locname);
    if (i != -1)
    {
        *location = ((unsigned long)i << 12) | MLoc_ROOMKIND;
        return true;
    }
    i = atol(locname);
    // Negative number means Hero Gate
    if (i < 0)
    {
        long n;
        n = -i;
        thing = find_hero_gate_of_number(n);
        if (thing_is_invalid(thing))
        {
            ERRORMSG("%s(line %lu): Non-existing Hero Door, no %d",func_name,ln_num,(int)-i);
            *location = MLoc_NONE;
            return false;
        }
        *location = (((unsigned long)n) << 4) | MLoc_HEROGATE;
    } else
    // Positive number means Action Point
    if (i > 0)
    {
        long n;
        n = action_point_number_to_index(i);
        if (!action_point_exists_idx(n))
        {
            ERRORMSG("%s(line %lu): Non-existing Action Point, no %d",func_name,ln_num,(int)i);
            *location = MLoc_NONE;
            return false;
        }
        // Set to action point number
        *location = (((unsigned long)n) << 4) | MLoc_ACTIONPOINT;
    } else
    // Zero is an error; reset to no location
    {
      ERRORMSG("%s(line %lu): Invalid LOCATION = '%s'",func_name,ln_num, locname);
      *location = MLoc_NONE;
    }
    return true;
}

/**
 * Returns location id for 2-param tunneler heading from script.
 * @param headname
 * @param target
 * @param location
 * @return
 * @see get_map_location_id()
 */
#define get_map_heading_id(headname, target, location) get_map_heading_id_f(headname, target, location, __func__, text_line_number)
TbBool get_map_heading_id_f(const char *headname, long target, TbMapLocation *location, const char *func_name, long ln_num)
{
    long head_id;
    long n;
    // If there's no headname, then there's an error
    if (headname == NULL)
    {
        SCRPTERRLOG("No heading objective");
        *location = MLoc_NONE;
        return false;
    }
    head_id = get_rid(head_for_desc, headname);
    if (head_id == -1)
    {
        SCRPTERRLOG("Unhandled heading objective, '%s'", headname);
        *location = MLoc_NONE;
        return false;
    }
    // Check if the target place exists, and set 'location'
    // Note that we only need to support enum items which are in head_for_desc[].
    switch (head_id)
    {
    case MLoc_ACTIONPOINT:
        n = action_point_number_to_index(target);
        *location = ((unsigned long)n << 4) | head_id;
        if (!action_point_exists_idx(n)) {
            SCRPTWRNLOG("Target action point no %d doesn't exist", (int)target);
        }
        return true;
    case MLoc_PLAYERSDUNGEON:
    case MLoc_PLAYERSHEART:
        *location = ((unsigned long)target << 4) | head_id;
        if (!player_has_heart(target)) {
            SCRPTWRNLOG("Target player %d has no heart", (int)target);
        }
        return true;
    case MLoc_APPROPRTDUNGEON:
        *location = (0) | head_id; // This option has no 'target' value
        return true;
    default:
        *location = MLoc_NONE;
        SCRPTWRNLOG("Unsupported Heading objective %d", (int)head_id);
        break;
    }
    return false;
}

TbBool script_support_setup_player_as_computer_keeper(unsigned short plyridx, long comp_model)
{
    struct PlayerInfo *player;
    player = get_player(plyridx);
    if (player_invalid(player)) {
        SCRPTWRNLOG("Tried to set up invalid player %d",(int)plyridx);
        return false;
    }
    player->field_0 |= 0x01;
    player->id_number = plyridx;
    player->field_2C = 1;
    player->field_0 |= 0x40;
    init_player_start(player, false);
    if (!setup_a_computer_player(plyridx, comp_model)) {
        player->field_0 &= ~0x40;
        player->field_0 &= ~0x01;
        return false;
    }
    return true;
}

TbBool script_support_setup_player_as_zombie_keeper(unsigned short plyridx)
{
    struct PlayerInfo *player;
    SYNCDBG(8,"Starting for player %d",(int)plyridx);
    player = get_player(plyridx);
    if (player_invalid(player)) {
        SCRPTWRNLOG("Tried to set up invalid player %d",(int)plyridx);
        return false;
    }
    player->field_0 &= ~0x01; // mark as non-existing
    player->id_number = plyridx;
    player->field_2C = 0;
    player->field_0 &= ~0x40;
    init_player_start(player, false);
    return true;
}

void command_create_party(char *prtname)
{
    if (script_current_condition != -1)
    {
        SCRPTWRNLOG("Party '%s' defined inside conditional statement",prtname);
    }
    create_party(prtname);
}

long pop_condition(void)
{
  if (script_current_condition == -1)
  {
    SCRPTERRLOG("unexpected ENDIF");
    return -1;
  }
  if ( condition_stack_pos )
  {
    condition_stack_pos--;
    script_current_condition = condition_stack[condition_stack_pos];
  } else
  {
    script_current_condition = -1;
  }
  return script_current_condition;
}

void command_add_to_party(char *prtname, char *crtr_name, long crtr_level, long carried_gold, char *objectv, long countdown)
{
    long crtr_id, objctv_id;
    if ((crtr_level < 1) || (crtr_level > CREATURE_MAX_LEVEL))
    {
      SCRPTERRLOG("Invalid Creature Level parameter; %ld not in range (%d,%d)",crtr_level,1,CREATURE_MAX_LEVEL);
      return;
    }
    crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
      SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
      return;
    }
    objctv_id = get_rid(hero_objective_desc, objectv);
    if (objctv_id == -1)
    {
      SCRPTERRLOG("Unknown party member objective, '%s'", objectv);
      return;
    }
  //SCRPTLOG("Party '%s' member kind %d, level %d",prtname,crtr_id,crtr_level);
    if (script_current_condition != -1)
    {
      SCRPTWRNLOG("Party '%s' member added inside conditional statement",prtname);
    }
    add_member_to_party_name(prtname, crtr_id, crtr_level, carried_gold, objctv_id, countdown);
}

void command_tutorial_flash_button(long btn_id, long duration)
{
  command_add_value(Cmd_TUTORIAL_FLASH_BUTTON, 0, btn_id, duration, 0);
}

void command_add_party_to_level(char *plrname, char *prtname, char *locname, long ncopies)
{
    struct PartyTrigger *pr_trig;
    struct Party *party;
    TbMapLocation location;
    long plr_id,prty_id;
    if (ncopies < 1)
    {
        SCRPTERRLOG("Invalid NUMBER parameter");
        return;
    }
    if (game.script.party_triggers_num >= PARTY_TRIGGERS_COUNT)
    {
        SCRPTERRLOG("Too many ADD_CREATURE commands in script");
        return;
    }
    // Recognize player
    if (!get_player_id(plrname, &plr_id))
        return;
    // Recognize place where party is created
    if (!get_map_location_id(locname, &location))
        return;
    // Recognize party name
    prty_id = get_party_index_of_name(prtname);
    if (prty_id < 0)
    {
        SCRPTERRLOG("Party of requested name, '%s', is not defined",prtname);
        return;
    }
    if ((script_current_condition < 0) && (next_command_reusable == 0))
    {
        party = &game.script.creature_partys[prty_id];
        script_process_new_party(party, plr_id, location, ncopies);
    } else
    {
        pr_trig = &game.script.party_triggers[game.script.party_triggers_num%PARTY_TRIGGERS_COUNT];
        set_flag_byte(&(pr_trig->flags), TrgF_REUSABLE, next_command_reusable);
        set_flag_byte(&(pr_trig->flags), TrgF_DISABLED, false);
        pr_trig->plyr_idx = plr_id;
        pr_trig->creatr_id = -prty_id;
        pr_trig->location = location;
        pr_trig->ncopies = ncopies;
        pr_trig->condit_idx = script_current_condition;
        game.script.party_triggers_num++;
    }
}

void command_add_creature_to_level(char *plrname, char *crtr_name, char *locname, long ncopies, long crtr_level, long carried_gold)
{
    struct PartyTrigger *pr_trig;
    TbMapLocation location;
    long plr_id,crtr_id;
    if ((crtr_level < 1) || (crtr_level > CREATURE_MAX_LEVEL))
    {
        SCRPTERRLOG("Invalid CREATURE LEVEL parameter");
        return;
    }
    if ((ncopies <= 0) || (ncopies >= CREATURES_COUNT))
    {
        SCRPTERRLOG("Invalid number of creatures to add");
        return;
    }
    if (game.script.party_triggers_num >= PARTY_TRIGGERS_COUNT)
    {
        SCRPTERRLOG("Too many ADD_CREATURE commands in script");
        return;
    }
    crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
    }
    // Recognize player
    if (!get_player_id(plrname, &plr_id))
        return;
    // Recognize place where party is created
    if (!get_map_location_id(locname, &location))
        return;
    if (script_current_condition < 0)
    {
        script_process_new_creatures(plr_id, crtr_id, location, ncopies, carried_gold, crtr_level-1);
    } else
    {
        pr_trig = &game.script.party_triggers[game.script.party_triggers_num%PARTY_TRIGGERS_COUNT];
        set_flag_byte(&(pr_trig->flags), TrgF_REUSABLE, next_command_reusable);
        set_flag_byte(&(pr_trig->flags), TrgF_DISABLED, false);
        pr_trig->plyr_idx = plr_id;
        pr_trig->creatr_id = crtr_id;
        pr_trig->crtr_level = crtr_level-1;
        pr_trig->carried_gold = carried_gold;
        pr_trig->location = location;
        pr_trig->ncopies = ncopies;
        pr_trig->condit_idx = script_current_condition;
        game.script.party_triggers_num++;
    }
}

void command_add_condition(long plr_id, long opertr_id, long varib_type, long varib_id, long value)
{
    struct Condition *condt;
    condt = &game.script.conditions[game.script.conditions_num];
    condt->condit_idx = script_current_condition;
    condt->plyr_idx = plr_id;
    condt->variabl_type = varib_type;
    condt->variabl_idx = varib_id;
    condt->operation = opertr_id;
    condt->rvalue = value;
    if (condition_stack_pos >= CONDITIONS_COUNT)
    {
        game.script.conditions_num++;
        SCRPTWRNLOG("Conditions too deep in script");
        return;
    }
    if (script_current_condition >= 0)
    {
        condition_stack[condition_stack_pos] = script_current_condition;
        condition_stack_pos++;
    }
    script_current_condition = game.script.conditions_num;
    game.script.conditions_num++;
}

void command_if(char *plrname, char *varib_name, char *operatr, long value)
{
    long plr_id,opertr_id;
    long varib_type,varib_id;
    if (game.script.conditions_num >= CONDITIONS_COUNT)
    {
      SCRPTERRLOG("Too many (over %d) conditions in script", CONDITIONS_COUNT);
      return;
    }
    // Recognize player
    if (!get_player_id(plrname, &plr_id))
      return;
    // Recognize variable
    varib_type = get_id(variable_desc, varib_name);
    if (varib_type == -1)
      varib_id = -1;
    else
      varib_id = 0;
    if (varib_id == -1)
    {
      varib_id = get_id(creature_desc, varib_name);
      varib_type = SVar_CREATURE_NUM;
    }
    if (varib_id == -1)
    {
      varib_id = get_id(room_desc, varib_name);
      varib_type = SVar_ROOM_SLABS;
    }
    if (varib_id == -1)
    {
      varib_id = get_id(timer_desc, varib_name);
      varib_type = SVar_TIMER;
    }
    if (varib_id == -1)
    {
      varib_id = get_id(flag_desc, varib_name);
      varib_type = SVar_FLAG;
    }
    if (varib_id == -1)
    {
      varib_id = get_id(door_desc, varib_name);
      varib_type = SVar_DOOR_NUM;
    }
    if (varib_id == -1)
    {
      SCRPTERRLOG("Unknown variable name, '%s'", varib_name);
      return;
    }
    // Recognize comparison
    opertr_id = get_id(comparison_desc, operatr);
    if (opertr_id == -1)
    {
      SCRPTERRLOG("Unknown comparison name, '%s'", operatr);
      return;
    }
    // Add the condition to script structure
    command_add_condition(plr_id, opertr_id, varib_type, varib_id, value);
}

struct ScriptValue *allocate_script_value(void)
{
  struct ScriptValue *value;
  if (game.script.values_num < 0)
    return NULL;
  if (game.script.values_num >= SCRIPT_VALUES_COUNT)
    return NULL;
  value = &game.script.values[game.script.values_num];
  game.script.values_num++;
  return value;
}

void command_add_value(unsigned long var_index, unsigned long val1, long val2, long val3, long val4)
{
    struct ScriptValue *value;
    if ((script_current_condition < 0) && (next_command_reusable == 0))
    {
        script_process_value(var_index, val1, val2, val3, val4);
        return;
    }
    value = allocate_script_value();
    if (value == NULL)
    {
        SCRPTERRLOG("Too many VALUEs in script (limit is %d)", SCRIPT_VALUES_COUNT);
        return;
    }
    set_flag_byte(&value->flags, TrgF_REUSABLE, next_command_reusable);
    set_flag_byte(&value->flags, TrgF_DISABLED, false);
    value->valtype = var_index;
    value->field_3 = val1;
    value->field_4 = val2;
    value->field_8 = val3;
    value->field_C = val4;
    value->condit_idx = script_current_condition;
}

void command_display_information(long msg_num, const char *where, long x, long y)
{
    TbMapLocation location;
    if ((msg_num < 0) || (msg_num >= STRINGS_MAX))
    {
      SCRPTERRLOG("Invalid TEXT number");
      return;
    }
    if (!get_map_location_id(where, &location))
      return;
    command_add_value(Cmd_DISPLAY_INFORMATION, ALL_PLAYERS, msg_num, location, get_subtile_number(x,y));
}

void command_set_generate_speed(long game_turns)
{
    if (game_turns <= 0)
    {
      SCRPTERRLOG("Generation speed must be positive number");
      return;
    }
    command_add_value(Cmd_SET_GENERATE_SPEED, 0, game_turns, 0, 0);
}

void command_dead_creatures_return_to_pool(long val)
{
    command_add_value(Cmd_DEAD_CREATURES_RETURN_TO_POOL, 0, val, 0, 0);
}

void command_bonus_level_time(long game_turns)
{
    if (game_turns < 0)
    {
        SCRPTERRLOG("Bonus time must be nonnegative");
        return;
    }
    command_add_value(Cmd_BONUS_LEVEL_TIME, 0, game_turns, 0, 0);
}

void player_command_add_start_money(int plridx, long gold_val)
{
  struct Dungeon *dungeon;
  // note that we can't get_players_num_dungeon() because players
  // may be uninitialized yet when this is called.
  dungeon = get_dungeon(plridx);
  if (dungeon_invalid(dungeon))
      return;
  dungeon->offmap_money_owned += gold_val;
  dungeon->total_money_owned += gold_val;
}

void player_reveal_map_area(int plyr_idx, long x, long y, long w, long h)
{
  SYNCDBG(0,"Revealing around (%d,%d)",x,y);
  reveal_map_area(plyr_idx, x-(w>>1), x+(w>>1)+(w%1), y-(h>>1), y+(h>>1)+(h%1));
}

void player_reveal_map_location(int plyr_idx, TbMapLocation target, long r)
{
  long x,y;
  SYNCDBG(0,"Revealing location type %d",target);
  x = 0;
  y = 0;
  find_map_location_coords(target, &x, &y, __func__);
  if ((x == 0) && (y == 0))
  {
    WARNLOG("Can't decode location %d",target);
    return;
  }
  reveal_map_area(plyr_idx, x-(r>>1), x+(r>>1)+(r%1), y-(r>>1), y+(r>>1)+(r%1));
}

void command_set_start_money(char *plrname, long gold_val)
{
  int plr_start, plr_end;
  int i;
  if (get_players_range(plrname, &plr_start, &plr_end) < 0)
    return;
  if (script_current_condition != -1)
  {
    SCRPTWRNLOG("Start money set inside conditional block");
  }
  for (i=plr_start; i < plr_end; i++)
    player_command_add_start_money(i, gold_val);
}

void command_room_available(char *plrname, char *roomname, unsigned long can_resrch, unsigned long can_build)
{
    long plr_id,room_id;
    if (!get_player_id(plrname, &plr_id))
      return;
    room_id = get_rid(room_desc, roomname);
    if (room_id == -1)
    {
      SCRPTERRLOG("Unknown room name, '%s'", roomname);
      return;
    }
    command_add_value(Cmd_ROOM_AVAILABLE, plr_id, room_id, can_resrch, can_build);
}

void command_creature_available(char *plrname, char *crtr_name, unsigned long can_be_avail, unsigned long force_avail)
{
    long plr_id,crtr_id;
    if (!get_player_id(plrname, &plr_id))
      return;
    crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
      SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
      return;
    }
    command_add_value(Cmd_CREATURE_AVAILABLE, plr_id, crtr_id, can_be_avail, force_avail);
}

void command_magic_available(char *plrname, char *magname, unsigned long can_resrch, unsigned long can_use)
{
    long plr_id,mag_id;
    if (!get_player_id(plrname, &plr_id))
      return;
    mag_id = get_rid(power_desc, magname);
    if (mag_id == -1)
    {
      SCRPTERRLOG("Unknown magic, '%s'", magname);
      return;
    }
    command_add_value(Cmd_MAGIC_AVAILABLE, plr_id, mag_id, can_resrch, can_use);
}

void command_trap_available(char *plrname, char *trapname, unsigned long can_build, unsigned long amount)
{
    long plr_id,trap_id;
    if (!get_player_id(plrname, &plr_id))
      return;
    trap_id = get_rid(trap_desc, trapname);
    if (trap_id == -1)
    {
      SCRPTERRLOG("Unknown trap, '%s'", trapname);
      return;
    }
    command_add_value(Cmd_TRAP_AVAILABLE, plr_id, trap_id, can_build, amount);
}

/**
 * Updates amount of RESEARCH points needed for the item to be researched.
 * Will not reorder the RESEARCH items.
 */
void command_research(char *plrname, char *trg_type, char *trg_name, unsigned long val)
{
    long plr_id;
    int plr_start, plr_end;
    long item_type,item_id;
    plr_id = get_players_range(plrname, &plr_start, &plr_end);
    if (plr_id < 0)
      return;
    item_type = get_rid(research_desc, trg_type);
    item_id = get_research_id(item_type, trg_name, __func__);
    if (item_id < 0)
      return;
    command_add_value(Cmd_RESEARCH, plr_id, item_type, item_id, val);
}

/**
 * Updates amount of RESEARCH points needed for the item to be researched.
 * Reorders the RESEARCH items - needs all items to be re-added.
 */
void command_research_order(char *plrname, char *trg_type, char *trg_name, unsigned long val)
{
    struct Dungeon *dungeon;
    long plr_id;
    int plr_start, plr_end;
    long item_type,item_id;
    long i;
    plr_id = get_players_range(plrname, &plr_start, &plr_end);
    if (plr_id < 0)
      return;
    for (i=plr_start; i < plr_end; i++)
    {
        dungeon = get_dungeon(i);
        if (dungeon_invalid(dungeon))
            continue;
        if (dungeon->research_num >= 34)
        {
          SCRPTERRLOG("Too many RESEARCH ITEMS, for player %d", i);
          return;
        }
    }
    item_type = get_rid(research_desc, trg_type);
    item_id = get_research_id(item_type, trg_name, __func__);
    if (item_id < 0)
      return;
    command_add_value(Cmd_RESEARCH_ORDER, plr_id, item_type, item_id, val);
}

void command_if_action_point(long apt_num, char *plrname)
{
    long plyr_idx;
    long apt_idx;
    if (game.script.conditions_num >= CONDITIONS_COUNT)
    {
        SCRPTERRLOG("Too many (over %d) conditions in script", CONDITIONS_COUNT);
        return;
    }
    // Check the Action Point
    apt_idx = action_point_number_to_index(apt_num);
    if (!action_point_exists_idx(apt_idx))
    {
        SCRPTERRLOG("Non-existing Action Point, no %d", apt_num);
        return;
    }
    // Recognize player
    if (!get_player_id(plrname, &plyr_idx))
        return;
    command_add_condition(plyr_idx, 0, 19, apt_idx, 0);
}

void command_computer_player(char *plrname, long comp_model)
{
    long plr_id;
    if (!get_player_id(plrname, &plr_id))
      return;
    if (script_current_condition != -1)
    {
      SCRPTWRNLOG("Computer player setup inside conditional block");
    }
    script_support_setup_player_as_computer_keeper(plr_id, comp_model);
}

void command_set_timer(char *plrname, char *timrname)
{
    long plr_id,timr_id;
    if (!get_player_id(plrname, &plr_id))
        return;
    timr_id = get_rid(timer_desc, timrname);
    if (timr_id == -1)
    {
        SCRPTERRLOG("Unknown timer, '%s'", timrname);
        return;
    }
    command_add_value(Cmd_SET_TIMER, plr_id, timr_id, 0, 0);
}

void command_win_game(void)
{
    if (script_current_condition == -1)
    {
        SCRPTERRLOG("Command WIN GAME found with no condition");
        return;
    }
    if (game.script.win_conditions_num >= WIN_CONDITIONS_COUNT)
    {
        SCRPTERRLOG("Too many WIN GAME conditions in script");
        return;
    }
    game.script.win_conditions[game.script.win_conditions_num] = script_current_condition;
    game.script.win_conditions_num++;
}

void command_lose_game(void)
{
  if (script_current_condition == -1)
  {
    SCRPTERRLOG("Command LOSE GAME found with no condition");
    return;
  }
  if (game.script.lose_conditions_num >= WIN_CONDITIONS_COUNT)
  {
    SCRPTERRLOG("Too many LOSE GAME conditions in script");
    return;
  }
  game.script.lose_conditions[game.script.lose_conditions_num] = script_current_condition;
  game.script.lose_conditions_num++;
}

void command_set_flag(char *plrname, char *flgname, long val)
{
  long plr_id,flg_id;
  if (!get_player_id(plrname, &plr_id))
    return;
  flg_id = get_rid(flag_desc, flgname);
  if (flg_id == -1)
  {
    SCRPTERRLOG("Unknown flag, '%s'", flgname);
    return;
  }
  command_add_value(Cmd_SET_FLAG, plr_id, flg_id, val, 0);
}

void command_max_creatures(char *plrname, long val)
{
  long plr_id;
  if (!get_player_id(plrname, &plr_id))
    return;
  command_add_value(Cmd_MAX_CREATURES, plr_id, val, 0, 0);
}

void command_door_available(char *plrname, char *doorname, unsigned long a3, unsigned long a4)
{
  long plr_id,door_id;
  if (!get_player_id(plrname, &plr_id))
    return;
  door_id = get_rid(door_desc, doorname);
  if (door_id == -1)
  {
    SCRPTERRLOG("Unknown door, '%s'", doorname);
    return;
  }
  command_add_value(Cmd_DOOR_AVAILABLE, plr_id, door_id, a3, a4);
}

void command_display_objective(long msg_num, char *where, long x, long y)
{
  TbMapLocation location;
  if ((msg_num < 0) || (msg_num >= STRINGS_MAX))
  {
    SCRPTERRLOG("Invalid TEXT number");
    return;
  }
  if (!get_map_location_id(where, &location))
    return;
  command_add_value(Cmd_DISPLAY_OBJECTIVE, ALL_PLAYERS, msg_num, location, get_subtile_number(x,y));
}

void command_add_tunneller_to_level(char *plrname, char *locname, char *objectv, long target, unsigned char crtr_level, unsigned long carried_gold)
{
    struct TunnellerTrigger *tn_trig;
    TbMapLocation location, heading;
    long plr_id;
    if ((crtr_level < 1) || (crtr_level > CREATURE_MAX_LEVEL))
    {
        SCRPTERRLOG("Invalid CREATURE LEVEL parameter");
        return;
    }
    if (game.script.tunneller_triggers_num >= TUNNELLER_TRIGGERS_COUNT)
    {
        SCRPTERRLOG("Too many ADD_TUNNELLER commands in script");
        return;
    }
    // Recognize player
    if (!get_player_id(plrname, &plr_id))
        return;
    // Recognize place where party is created
    if (!get_map_location_id(locname, &location))
        return;
    // Recognize place where party is going
    if (!get_map_heading_id(objectv, target, &heading))
        return;
    if (script_current_condition < 0)
    {
        script_process_new_tunneler(plr_id, location, heading, crtr_level-1, carried_gold);
    } else
    {
        tn_trig = &game.script.tunneller_triggers[game.script.tunneller_triggers_num%TUNNELLER_TRIGGERS_COUNT];
        set_flag_byte(&(tn_trig->flags), TrgF_REUSABLE, next_command_reusable);
        set_flag_byte(&(tn_trig->flags), TrgF_DISABLED, false);
        tn_trig->plyr_idx = plr_id;
        tn_trig->location = location;
        tn_trig->heading = heading;
        tn_trig->heading_OLD = 0; //target is now contained in heading and this is unused
        tn_trig->carried_gold = carried_gold;
        tn_trig->crtr_level = crtr_level-1;
        tn_trig->carried_gold = carried_gold;
        tn_trig->party_id = 0;
        tn_trig->condit_idx = script_current_condition;
        game.script.tunneller_triggers_num++;
    }
}

void command_add_tunneller_party_to_level(char *plrname, char *prtname, char *locname, char *objectv, long target, char crtr_level, unsigned long carried_gold)
{
    struct TunnellerTrigger *tn_trig;
    struct Party *party;
    TbMapLocation location, heading;
    long plr_id,prty_id;
    if ((crtr_level < 1) || (crtr_level > CREATURE_MAX_LEVEL))
    {
        SCRPTERRLOG("Invalid CREATURE LEVEL parameter");
        return;
    }
    if (game.script.tunneller_triggers_num >= TUNNELLER_TRIGGERS_COUNT)
    {
        SCRPTERRLOG("Too many ADD_TUNNELLER commands in script");
        return;
    }
    // Recognize player
    if (!get_player_id(plrname, &plr_id))
        return;
    // Recognize place where party is created
    if (!get_map_location_id(locname, &location))
        return;
    // Recognize place where party is going
    if (!get_map_heading_id(objectv, target, &heading))
        return;
    // Recognize party name
    prty_id = get_party_index_of_name(prtname);
    if (prty_id < 0)
    {
        SCRPTERRLOG("Party of requested name, '%s', is not defined", prtname);
        return;
    }
    party = &game.script.creature_partys[prty_id];
    if (party->members_num >= PARTY_MEMBERS_COUNT-1)
    {
        SCRPTERRLOG("Party too big for ADD_TUNNELLER (Max %d members)", PARTY_MEMBERS_COUNT-1);
        return;
    }
    // Either add the party or add item to conditional triggers list
    if (script_current_condition < 0)
    {
        script_process_new_tunneller_party(plr_id, prty_id, location, heading, crtr_level-1, carried_gold);
    } else
    {
        tn_trig = &game.script.tunneller_triggers[game.script.tunneller_triggers_num%TUNNELLER_TRIGGERS_COUNT];
        set_flag_byte(&(tn_trig->flags), TrgF_REUSABLE, next_command_reusable);
        set_flag_byte(&(tn_trig->flags), TrgF_DISABLED, false);
        tn_trig->plyr_idx = plr_id;
        tn_trig->location = location;
        tn_trig->heading = heading;
        tn_trig->heading_OLD = 0; //target is now contained in heading and this is unused
        tn_trig->carried_gold = carried_gold;
        tn_trig->crtr_level = crtr_level-1;
        tn_trig->carried_gold = carried_gold;
        tn_trig->party_id = prty_id+1;
        tn_trig->condit_idx = script_current_condition;
        game.script.tunneller_triggers_num++;
    }
}

void command_add_creature_to_pool(char *crtr_name, long amount)
{
  long crtr_id;
  crtr_id = get_rid(creature_desc, crtr_name);
  if (crtr_id == -1)
  {
    SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
    return;
  }
  if ((amount < 0) || (amount >= CREATURES_COUNT))
  {
    SCRPTERRLOG("Invalid number of '%s' creatures for pool, %d", crtr_name, amount);
    return;
  }
  command_add_value(Cmd_ADD_CREATURE_TO_POOL, 0, crtr_id, amount, 0);
}

void command_reset_action_point(long apt_num)
{
  long apt_idx;
  apt_idx = action_point_number_to_index(apt_num);
  if (!action_point_exists_idx(apt_idx))
  {
    SCRPTERRLOG("Non-existing Action Point, no %d", apt_num);
    return;
  }
  command_add_value(Cmd_RESET_ACTION_POINT, 0, apt_idx, 0, 0);
}

void command_set_creature_max_level(char *plrname, char *crtr_name, long crtr_level)
{
  long plr_id,crtr_id;
  if (!get_player_id(plrname, &plr_id))
    return;
  crtr_id = get_rid(creature_desc, crtr_name);
  if (crtr_id == -1)
  {
    SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
    return;
  }
  if ((crtr_level < 1) || (crtr_level > CREATURE_MAX_LEVEL))
  {
    SCRPTERRLOG("Invalid '%s' experience level, %d", crtr_name, crtr_level);
  }
  command_add_value(Cmd_SET_CREATURE_MAX_LEVEL, plr_id, crtr_id, crtr_level-1, 0);
}

void command_set_music(long val)
{
  if (script_current_condition != -1)
  {
    SCRPTWRNLOG("Music set inside conditional block");
  }
  game.music_track_index = val;
}

void command_set_hate(long a1, long a2, long a3)
{
  command_add_value(Cmd_SET_HATE, a1, a2, a3, 0);
}

void command_if_available(char *plrname, char *varib_name, char *operatr, long value)
{
  long plr_id,opertr_id;
  long varib_type,varib_id;
  if (game.script.conditions_num >= CONDITIONS_COUNT)
  {
    SCRPTERRLOG("Too many (over %d) conditions in script", CONDITIONS_COUNT);
    return;
  }
  // Recognize player
  if (!get_player_id(plrname, &plr_id))
    return;
  // Recognize variable
  varib_id = -1;
  if (varib_id == -1)
  {
    varib_id = get_id(door_desc, varib_name);
    varib_type = SVar_AVAILABLE_DOOR;
  }
  if (varib_id == -1)
  {
    varib_id = get_id(trap_desc, varib_name);
    varib_type = SVar_AVAILABLE_TRAP;
  }
  if (varib_id == -1)
  {
    varib_id = get_id(room_desc, varib_name);
    varib_type = SVar_AVAILABLE_ROOM;
  }
  if (varib_id == -1)
  {
    varib_id = get_id(power_desc, varib_name);
    varib_type = SVar_AVAILABLE_MAGIC;
  }
  if (varib_id == -1)
  {
    SCRPTERRLOG("Unrecognized VARIABLE, '%s'", varib_name);
    return;
  }
  // Recognize comparison
  opertr_id = get_id(comparison_desc, operatr);
  if (opertr_id == -1)
  {
    SCRPTERRLOG("Unknown comparison name, '%s'", operatr);
    return;
  }
  // Add the condition to script structure
  command_add_condition(plr_id, opertr_id, varib_type, varib_id, value);
}

void command_set_computer_globals(char *plrname, long val1, long val2, long val3, long val4, long val5, long val6)
{
  struct Computer2 *comp;
  int plr_start, plr_end;
  long i;
  if (get_players_range(plrname, &plr_start, &plr_end) < 0)
    return;
  if (script_current_condition != -1)
  {
    SCRPTWRNLOG("Computer globals altered inside conditional block");
  }
  for (i=plr_start; i < plr_end; i++)
  {
    comp = &game.computer[i];
    comp->field_1C = val1;
    comp->field_14 = val2;
    comp->field_18 = val3;
    comp->max_room_build_tasks = val4;
    comp->field_2C = val5;
    comp->field_20 = val6;
  }
}

void command_set_computer_checks(char *plrname, char *chkname, long val1, long val2, long val3, long val4, long val5)
{
  struct ComputerCheck *check;
  int plr_start, plr_end;
  long i,k,n;
  if (get_players_range(plrname, &plr_start, &plr_end) < 0)
    return;
  if (script_current_condition != -1)
  {
    SCRPTWRNLOG("Computer check altered inside conditional block");
  }
  n = 0;
  for (i=plr_start; i < plr_end; i++)
  {
    for (k=0; k < COMPUTER_CHECKS_COUNT; k++)
    {
      check = &game.computer[i].checks[k];
      if ((check->flags & 0x02) != 0)
        break;
      if (check->name == NULL)
        break;
      if (strcasecmp(chkname, check->name) == 0)
      {
        check->turns_interval = val1;
        check->param1 = val2;
        check->param2 = val3;
        check->param3 = val4;
        check->param4 = val5;
        n++;
      }
    }
  }
  if (n == 0)
  {
    SCRPTERRLOG("No computer check found called '%s'", chkname);
    return;
  }
  SCRIPTDBG(6,"Altered %d checks",n);
}

void command_set_computer_events(char *plrname, char *evntname, long val1, long val2)
{
  struct ComputerEvent *event;
  int plr_start, plr_end;
  long i,k,n;
  if (get_players_range(plrname, &plr_start, &plr_end) < 0)
    return;
  if (script_current_condition != -1)
  {
    SCRPTWRNLOG("Computer event altered inside conditional block");
  }
  n = 0;
  for (i=plr_start; i < plr_end; i++)
  {
    for (k=0; k < COMPUTER_EVENTS_COUNT; k++)
    {
      event = &game.computer[i].events[k];
      if (event->name == NULL)
        break;
      if (stricmp(evntname, event->name) == 0)
      {
        event->param1 = val1;
        event->param2 = val2;
        n++;
      }
    }
  }
  if (n == 0)
  {
    SCRPTERRLOG("no computer event found called '%s'", evntname);
    return;
  }
  SCRIPTDBG(6,"Altered %d events",n);
}

void command_set_computer_process(char *plrname, char *procname, long val1, long val2, long val3, long val4, long val5)
{
  struct ComputerProcess *process;
  int plr_start, plr_end;
  long i,k,n;
  if (get_players_range(plrname, &plr_start, &plr_end) < 0)
    return;
  if (script_current_condition != -1)
  {
    SCRPTWRNLOG("Computer process altered inside conditional block");
  }
  n = 0;
  for (i=plr_start; i < plr_end; i++)
  {
    for (k=0; k < COMPUTER_PROCESSES_COUNT; k++)
    {
      process = &game.computer[i].processes[k];
      if ((process->flags & 0x02) != 0)
        break;
      if (process->name == NULL)
        break;
      if (stricmp(procname, process->name) == 0)
      {
        process->field_4 = val1;
        process->field_8 = val2;
        process->field_C = val3;
        process->field_10 = val4;
        process->field_14 = val5;
        n++;
      }
    }
  }
  if (n == 0)
  {
    SCRPTERRLOG("No computer process found called '%s'", procname);
    return;
  }
  SCRIPTDBG(6,"Altered %d processes",n);
}

void command_set_creature_health(char *crtr_name, long val)
{
  long crtr_id;
  crtr_id = get_rid(creature_desc, crtr_name);
  if (crtr_id == -1)
  {
    SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
    return;
  }
  if ((val < 0) || (val > 65535))
  {
    SCRPTERRLOG("Invalid '%s' health value, %d", crtr_name, val);
    return;
  }
  command_add_value(Cmd_SET_CREATURE_HEALTH, 0, crtr_id, val, 0);
}

void command_set_creature_strength(char *crtr_name, long val)
{
  long crtr_id;
  crtr_id = get_rid(creature_desc, crtr_name);
  if (crtr_id == -1)
  {
    SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
    return;
  }
  if ((val < 0) || (val > 255))
  {
    SCRPTERRLOG("Invalid '%s' strength value, %d", crtr_name, val);
    return;
  }
  command_add_value(Cmd_SET_CREATURE_STRENGTH, 0, crtr_id, val, 0);
}

void command_set_creature_armour(char *crtr_name, long val)
{
  long crtr_id;
  crtr_id = get_rid(creature_desc, crtr_name);
  if (crtr_id == -1)
  {
    SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
    return;
  }
  if ((val < 0) || (val > 255))
  {
    SCRPTERRLOG("Invalid '%s' armour value, %d", crtr_name, val);
    return;
  }
  command_add_value(Cmd_SET_CREATURE_ARMOUR, 0, crtr_id, val, 0);
}

void command_set_creature_fear(char *crtr_name, long val)
{
  long crtr_id;
  crtr_id = get_rid(creature_desc, crtr_name);
  if (crtr_id == -1)
  {
    SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
    return;
  }
  if ((val < 0) || (val > 255))
  {
    SCRPTERRLOG("Invalid '%s' fear value, %d", crtr_name, val);
    return;
  }
  command_add_value(Cmd_SET_CREATURE_FEAR_WOUNDED, 0, crtr_id, val, 0);
}

/**
 * Enables or disables an alliance between two players.
 *
 * @param plr1name First player text name.
 * @param plr2name Second player text name.
 * @param ally Controls whether the alliance is being created or being broken.
 */
void command_ally_players(char *plr1name, char *plr2name, TbBool ally)
{
  long plr1_id,plr2_id;
  if (!get_player_id(plr1name, &plr1_id))
    return;
  if (!get_player_id(plr2name, &plr2_id))
    return;
  command_add_value(Cmd_ALLY_PLAYERS, 0, plr1_id, plr2_id, ally);
}

void command_quick_objective(int idx, char *msgtext, char *where, long x, long y)
{
  TbMapLocation location;
  if ((idx < 0) || (idx >= QUICK_MESSAGES_COUNT))
  {
    SCRPTERRLOG("Invalid QUICK OBJECTIVE number (%d)", idx);
    return;
  }
  if (strlen(msgtext) > MESSAGE_TEXT_LEN)
  {
    SCRPTWRNLOG("Objective TEXT too long; truncating to %d characters", MESSAGE_TEXT_LEN-1);
    msgtext[MESSAGE_TEXT_LEN-1] = '\0';
  }
  if ((gameadd.quick_messages[idx][0] != '\0') && (strcmp(gameadd.quick_messages[idx],msgtext) != 0))
  {
    SCRPTWRNLOG("Quick Objective no %d overwritten by different text.", idx);
  }
  strcpy(gameadd.quick_messages[idx], msgtext);
  if (!get_map_location_id(where, &location))
    return;
  command_add_value(Cmd_QUICK_OBJECTIVE, ALL_PLAYERS, idx, location, get_subtile_number(x,y));
}

void command_quick_information(int idx, char *msgtext, const char *where, long x, long y)
{
  TbMapLocation location;
  if ((idx < 0) || (idx >= QUICK_MESSAGES_COUNT))
  {
    SCRPTERRLOG("Invalid information ID number (%d)", idx);
    return;
  }
  if (strlen(msgtext) > MESSAGE_TEXT_LEN)
  {
    SCRPTWRNLOG("Information TEXT too long; truncating to %d characters", MESSAGE_TEXT_LEN-1);
    msgtext[MESSAGE_TEXT_LEN-1] = '\0';
  }
  if ((gameadd.quick_messages[idx][0] != '\0') && (strcmp(gameadd.quick_messages[idx],msgtext) != 0))
  {
    SCRPTWRNLOG("Quick Message no %d overwritten by different text.", idx);
  }
  strcpy(gameadd.quick_messages[idx], msgtext);
  if (!get_map_location_id(where, &location))
    return;
  command_add_value(Cmd_QUICK_INFORMATION, ALL_PLAYERS, idx, location, get_subtile_number(x,y));
}

void command_play_message(char *plrname, char *msgtype, int msg_num)
{
  long plr_id,msgtype_id;
  if (!get_player_id(plrname, &plr_id))
    return;
  msgtype_id = get_id(msgtype_desc, msgtype);
  if (msgtype_id == -1)
  {
    SCRPTERRLOG("Unrecognized message type, '%s'", msgtype);
    return;
  }
  command_add_value(Cmd_PLAY_MESSAGE, plr_id, msgtype_id, msg_num, 0);
}

void command_add_gold_to_player(char *plrname, long amount)
{
  long plr_id;
  if (!get_player_id(plrname, &plr_id)) {
      return;
  }
  command_add_value(Cmd_ADD_GOLD_TO_PLAYER, plr_id, amount, 0, 0);
}

void command_set_creature_tendencies(char *plrname, char *tendency, long value)
{
  long plr_id,tend_id;
  if (!get_player_id(plrname, &plr_id))
    return;
  tend_id = get_rid(tendency_desc, tendency);
  if (tend_id == -1)
  {
    SCRPTERRLOG("Unrecognized tendency type, '%s'", tendency);
    return;
  }
  command_add_value(Cmd_SET_CREATURE_TENDENCIES, plr_id, tend_id, value, 0);
}

void command_reveal_map_rect(char *plrname, long x, long y, long w, long h)
{
  long plr_id;
  if (!get_player_id(plrname, &plr_id)) {
      return;
  }
  command_add_value(Cmd_REVEAL_MAP_RECT, plr_id, x, y, (h<<16)+w);
}

void command_reveal_map_location(char *plrname, char *locname, long range)
{
    long plr_id;
    TbMapLocation location;
    if (!get_player_id(plrname, &plr_id)) {
        return;
    }
    if (!get_map_location_id(locname, &location)) {
        return;
    }
    command_add_value(Cmd_REVEAL_MAP_LOCATION, plr_id, location, range, 0);
}

void command_message(char *msgtext, unsigned char kind)
{
  const char *cmd;
  if (kind == 80)
    cmd = script_get_command_name(Cmd_PRINT);
  else
    cmd = script_get_command_name(Cmd_MESSAGE);
  SCRPTWRNLOG("Command '%s' is only supported in Dungeon Keeper Beta", cmd);
}

void command_swap_creature(char *ncrt_name, char *crtr_name)
{
  long ncrt_id,crtr_id;
  ncrt_id = get_rid(newcrtr_desc, ncrt_name);
  if (ncrt_id == -1)
  {
    SCRPTERRLOG("Unknown new creature, '%s'", ncrt_name);
    return;
  }
  crtr_id = get_rid(creature_desc, crtr_name);
  if (crtr_id == -1)
  {
    SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
    return;
  }
  if ((crtr_id == 23) || (crtr_id == 8))
  {
    SCRPTERRLOG("Unable to swap IMPs or TUNNELLERs");
  }
  if (script_current_condition != -1)
  {
    SCRPTWRNLOG("Creature swapping placed inside conditional statement");
  }
  if (!swap_creature(ncrt_id, crtr_id))
  {
    SCRPTERRLOG("Error swapping creatures '%s'<->'%s'", ncrt_name, crtr_name);
  }
}

void command_kill_creature(char *plrname, char *crtr_name, char *criteria, int count)
{
  long plr_id,crtr_id,select_id;
  SCRIPTDBG(11,"Starting");
  if (!get_player_id(plrname, &plr_id)) {
    return;
  }
  if (count <= 0) {
    SCRPTERRLOG("Bad creatures count, %d", count);
    return;
  }
  crtr_id = get_rid(creature_desc, crtr_name);
  if (crtr_id == -1) {
    SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
    return;
  }
  select_id = get_rid(creature_select_criteria_desc, criteria);
  if (select_id == -1) {
    SCRPTERRLOG("Unknown select criteria, '%s'", criteria);
    return;
  }
  command_add_value(Cmd_KILL_CREATURE, plr_id, crtr_id, select_id, count);
}

long script_scan_line(char *line,TbBool preloaded)
{
  const struct CommandDesc *cmd_desc;
  struct ScriptLine *scline;
  unsigned char line_end;
  char *text;
  char chr;
  int i;
  SCRIPTDBG(12,"Starting");
  scline = (struct ScriptLine *)LbMemoryAlloc(sizeof(struct ScriptLine));
  if (scline == NULL)
  {
    SCRPTERRLOG("Can't allocate buffer to recognize line");
    return 0;
  }
  line_end = false;
  LbMemorySet(scline, 0, sizeof(struct ScriptLine));
  if (next_command_reusable > 0)
    next_command_reusable--;
  cmd_desc = get_next_word(&line, scline->tcmnd, &line_end);
  if (cmd_desc == NULL)
  {
    if ( isalnum(scline->tcmnd[0]) )
    {
      SCRPTERRLOG("Invalid command, '%s' (lev ver %d)", scline->tcmnd,level_file_version);
    }
    LbMemoryFree(scline);
    return 0;
  }
  SCRIPTDBG(12,"Executing command %lu",cmd_desc->index);
  // Handling comments
  if (cmd_desc->index == Cmd_REM)
  {
    LbMemoryFree(scline);
    return 0;
  }
  // selecting only preloaded/not preloaded commands
  if (script_is_preloaded_command(cmd_desc->index) != preloaded)
  {
    LbMemoryFree(scline);
    return 0;
  }
  // Recognizing parameters
  for (i=0; i < COMMANDDESC_ARGS_COUNT; i++)
  {
    chr = cmd_desc->args[i];
    if ((chr == ' ') || (chr == '\0'))
      break;
    if (line_end)
      break;
    get_next_word(&line, scline->tp[i], &line_end);
    if (scline->tp[i][0] == '\0')
      break;
    chr = cmd_desc->args[i];
    if ((chr == 'N') || (chr == 'n'))
    {
      scline->np[i] = strtol(scline->tp[i],&text,0);
      if (text != &scline->tp[i][strlen(scline->tp[i])])
        SCRPTWRNLOG("Numerical value '%s' interpreted as %ld", scline->tp[i], scline->np[i]);
    }
  }
  if (i < COMMANDDESC_ARGS_COUNT)
  {
    chr = cmd_desc->args[i];
    if ((chr == 'A') || (chr == 'N'))
    {
      SCRPTERRLOG("Not enough parameters for \"%s\"", cmd_desc->textptr);
      LbMemoryFree(scline);
      return -1;
    }
  }
  switch (cmd_desc->index)
  {
  case Cmd_CREATE_PARTY:
      command_create_party(scline->tp[0]);
      break;
  case Cmd_ADD_TO_PARTY:
      command_add_to_party(scline->tp[0], scline->tp[1], scline->np[2], scline->np[3], scline->tp[4], scline->np[5]);
      break;
  case Cmd_ADD_PARTY_TO_LEVEL:
      command_add_party_to_level(scline->tp[0], scline->tp[1], scline->tp[2], scline->np[3]);
      break;
  case Cmd_ADD_CREATURE_TO_LEVEL:
      command_add_creature_to_level(scline->tp[0], scline->tp[1], scline->tp[2], scline->np[3], scline->np[4], scline->np[5]);
      break;
  case Cmd_IF:
      command_if(scline->tp[0], scline->tp[1], scline->tp[2], scline->np[3]);
      break;
  case Cmd_ENDIF:
      pop_condition();
      break;
  case Cmd_SET_HATE:
      command_set_hate(scline->np[0], scline->np[1], scline->np[2]);
      break;
  case Cmd_SET_GENERATE_SPEED:
      command_set_generate_speed(scline->np[0]);
      break;
  case Cmd_START_MONEY:
      command_set_start_money(scline->tp[0], scline->np[1]);
      break;
  case Cmd_ROOM_AVAILABLE:
      command_room_available(scline->tp[0], scline->tp[1], scline->np[2], scline->np[3]);
      break;
  case Cmd_CREATURE_AVAILABLE:
      command_creature_available(scline->tp[0], scline->tp[1], scline->np[2], scline->np[3]);
      break;
  case Cmd_MAGIC_AVAILABLE:
      command_magic_available(scline->tp[0], scline->tp[1], scline->np[2], scline->np[3]);
      break;
  case Cmd_TRAP_AVAILABLE:
      command_trap_available(scline->tp[0], scline->tp[1], scline->np[2], scline->np[3]);
      break;
  case Cmd_RESEARCH:
      command_research(scline->tp[0], scline->tp[1], scline->tp[2], scline->np[3]);
      break;
  case Cmd_RESEARCH_ORDER:
      command_research_order(scline->tp[0], scline->tp[1], scline->tp[2], scline->np[3]);
      break;
  case Cmd_COMPUTER_PLAYER:
      command_computer_player(scline->tp[0], scline->np[1]);
      break;
  case Cmd_SET_TIMER:
      command_set_timer(scline->tp[0], scline->tp[1]);
      break;
  case Cmd_IF_ACTION_POINT:
      command_if_action_point(scline->np[0], scline->tp[1]);
      break;
  case Cmd_ADD_TUNNELLER_TO_LEVEL:
      command_add_tunneller_to_level(scline->tp[0], scline->tp[1], scline->tp[2], scline->np[3], scline->np[4], scline->np[5]);
      break;
  case Cmd_WIN_GAME:
      command_win_game();
      break;
  case Cmd_LOSE_GAME:
      command_lose_game();
      break;
  case Cmd_SET_FLAG:
      command_set_flag(scline->tp[0], scline->tp[1], scline->np[2]);
      break;
  case Cmd_MAX_CREATURES:
      command_max_creatures(scline->tp[0], scline->np[1]);
      break;
  case Cmd_NEXT_COMMAND_REUSABLE:
      next_command_reusable = 2;
      break;
  case Cmd_DOOR_AVAILABLE:
      command_door_available(scline->tp[0], scline->tp[1], scline->np[2], scline->np[3]);
      break;
  case Cmd_DISPLAY_OBJECTIVE:
      command_display_objective(scline->np[0], scline->tp[1], 0, 0);
      break;
  case Cmd_DISPLAY_INFORMATION:
      if (level_file_version > 0)
        command_display_information(scline->np[0], scline->tp[1], 0, 0);
      else
        command_display_information(scline->np[0], "ALL_PLAYERS", 0, 0);
      break;
  case Cmd_ADD_TUNNELLER_PARTY_TO_LEVEL:
      command_add_tunneller_party_to_level(scline->tp[0], scline->tp[1], scline->tp[2], scline->tp[3], scline->np[4], scline->np[5], scline->np[6]);
      break;
  case Cmd_ADD_CREATURE_TO_POOL:
      command_add_creature_to_pool(scline->tp[0], scline->np[1]);
      break;
  case Cmd_RESET_ACTION_POINT:
      command_reset_action_point(scline->np[0]);
      break;
  case Cmd_TUTORIAL_FLASH_BUTTON:
      command_tutorial_flash_button(scline->np[0], scline->np[1]);
      break;
  case Cmd_SET_CREATURE_MAX_LEVEL:
      command_set_creature_max_level(scline->tp[0], scline->tp[1], scline->np[2]);
      break;
  case Cmd_SET_MUSIC:
      command_set_music(scline->np[0]);
      break;
  case Cmd_SET_CREATURE_HEALTH:
      command_set_creature_health(scline->tp[0], scline->np[1]);
      break;
  case Cmd_SET_CREATURE_STRENGTH:
      command_set_creature_strength(scline->tp[0], scline->np[1]);
      break;
  case Cmd_SET_CREATURE_ARMOUR:
      command_set_creature_armour(scline->tp[0], scline->np[1]);
      break;
  case Cmd_SET_CREATURE_FEAR_WOUNDED:
      command_set_creature_fear(scline->tp[0], scline->np[1]);
      break;
  case Cmd_DISPLAY_OBJECTIVE_WITH_POS:
      command_display_objective(scline->np[0], NULL, scline->np[1], scline->np[2]);
      break;
  case Cmd_IF_AVAILABLE:
      command_if_available(scline->tp[0], scline->tp[1], scline->tp[2], scline->np[3]);
      break;
  case Cmd_SET_COMPUTER_GLOBALS:
      command_set_computer_globals(scline->tp[0], scline->np[1], scline->np[2], scline->np[3], scline->np[4], scline->np[5], scline->np[6]);
      break;
  case Cmd_SET_COMPUTER_CHECKS:
      command_set_computer_checks(scline->tp[0], scline->tp[1], scline->np[2], scline->np[3], scline->np[4], scline->np[5], scline->np[6]);
      break;
  case Cmd_SET_COMPUTER_EVENT:
      command_set_computer_events(scline->tp[0], scline->tp[1], scline->np[2], scline->np[3]);
      break;
  case Cmd_SET_COMPUTER_PROCESS:
      command_set_computer_process(scline->tp[0], scline->tp[1], scline->np[2], scline->np[3], scline->np[4], scline->np[5], scline->np[6]);
      break;
  case Cmd_ALLY_PLAYERS:
      if (level_file_version > 0)
          command_ally_players(scline->tp[0], scline->tp[1], scline->np[2]);
      else
          command_ally_players(scline->tp[0], scline->tp[1], true);
      break;
  case Cmd_DEAD_CREATURES_RETURN_TO_POOL:
      command_dead_creatures_return_to_pool(scline->np[0]);
      break;
  case Cmd_DISPLAY_INFORMATION_WITH_POS:
      command_display_information(scline->np[0], NULL, scline->np[1], scline->np[2]);
      break;
  case Cmd_BONUS_LEVEL_TIME:
      command_bonus_level_time(scline->np[0]);
      break;
  case Cmd_QUICK_OBJECTIVE:
      command_quick_objective(scline->np[0], scline->tp[1], scline->tp[2], 0, 0);
      break;
  case Cmd_QUICK_INFORMATION:
      if (level_file_version > 0)
        command_quick_information(scline->np[0], scline->tp[1], scline->tp[2], 0, 0);
      else
        command_quick_information(scline->np[0], scline->tp[1], "ALL_PLAYERS", 0, 0);
      break;
  case Cmd_QUICK_OBJECTIVE_WITH_POS:
      command_quick_objective(scline->np[0], scline->tp[1], NULL, scline->np[2], scline->np[3]);
      break;
  case Cmd_QUICK_INFORMATION_WITH_POS:
      command_quick_information(scline->np[0], scline->tp[1], NULL, scline->np[2], scline->np[3]);
      break;
  case Cmd_SWAP_CREATURE:
      command_swap_creature(scline->tp[0], scline->tp[1]);
      break;
  case Cmd_PRINT:
      command_message(scline->tp[0],80);
      break;
  case Cmd_MESSAGE:
      command_message(scline->tp[0],68);
      break;
  case Cmd_PLAY_MESSAGE:
      command_play_message(scline->tp[0], scline->tp[1], scline->np[2]);
      break;
  case Cmd_ADD_GOLD_TO_PLAYER:
      command_add_gold_to_player(scline->tp[0], scline->np[1]);
      break;
  case Cmd_SET_CREATURE_TENDENCIES:
      command_set_creature_tendencies(scline->tp[0], scline->tp[1], scline->np[2]);
      break;
  case Cmd_REVEAL_MAP_RECT:
      command_reveal_map_rect(scline->tp[0], scline->np[1], scline->np[2], scline->np[3], scline->np[4]);
      break;
  case Cmd_REVEAL_MAP_LOCATION:
      command_reveal_map_location(scline->tp[0], scline->tp[1], scline->np[2]);
      break;
  case Cmd_KILL_CREATURE:
      command_kill_creature(scline->tp[0], scline->tp[1], scline->tp[2], scline->np[3]);
      break;
  case Cmd_LEVEL_VERSION:
      level_file_version = scline->np[0];
      SCRPTLOG("Level files version %d.",level_file_version);
      break;
  default:
      SCRPTERRLOG("Unhandled SCRIPT command '%s'", scline->tcmnd);
      break;
  }
  LbMemoryFree(scline);
  SCRIPTDBG(13,"Finished");
  return 0;
}

short clear_script(void)
{
    LbMemorySet(&game.script, 0, sizeof(struct LevelScript));
    script_current_condition = -1;
    text_line_number = 1;
    return true;
}

short clear_quick_messages(void)
{
  long i;
  for (i=0; i < QUICK_MESSAGES_COUNT; i++)
      LbMemorySet(gameadd.quick_messages[i],0,MESSAGE_TEXT_LEN);
  return true;
}

short preload_script(long lvnum)
{
  char *buf;
  char *buf_end;
  int lnlen;
  char *script_data;
  long script_len;
  SYNCDBG(7,"Starting");
  script_current_condition = -1;
  next_command_reusable = 0;
  text_line_number = 1;
  level_file_version = DEFAULT_LEVEL_VERSION;
  clear_quick_messages();
  // Load the file
  script_len = 1;
  script_data = (char *)load_single_map_file_to_buffer(lvnum,"txt",&script_len,LMFF_None);
  if (script_data == NULL)
    return false;
  // Process the file lines
  buf = script_data;
  buf_end = script_data+script_len;
  while (buf < buf_end)
  {
    // Find end of the line
    lnlen = 0;
    while (&buf[lnlen] < buf_end)
    {
      if ((buf[lnlen] == '\r') || (buf[lnlen] == '\n'))
        break;
      lnlen++;
    }
    // Get rid of the next line characters
    buf[lnlen] = 0;
    lnlen++;
    if (&buf[lnlen] < buf_end)
    {
      if ((buf[lnlen] == '\r') || (buf[lnlen] == '\n'))
        lnlen++;
    }
    //SCRPTLOG("Analyse");
    // Analyze the line
    script_scan_line(buf, true);
    // Set new line start
    text_line_number++;
    buf += lnlen;
  }
  LbMemoryFree(script_data);
  SYNCDBG(8,"Finished");
  return true;
}

short load_script(long lvnum)
{
    char *buf;
    char *buf_end;
    int lnlen;
    char *script_data;
    long script_len;
    SYNCDBG(7,"Starting");
    //return _DK_load_script(lvnum);

    // Clear script data
    gui_set_button_flashing(0, 0);
    clear_script();
    script_current_condition = -1;
    next_command_reusable = 0;
    text_line_number = 1;
    game.bonus_time = 0;
    game.flags_gui &= ~GGUI_CountdownTimer;
    game.flags_cd |= MFlg_DeadBackToPool;
    reset_creature_max_levels();
    reset_script_timers_and_flags();
    if (game.numfield_C & 0x08)
    {
      convert_old_column_file(lvnum);
      set_flag_byte(&game.numfield_C,0x08,false);
    }
    // Load the file
    script_len = 1;
    script_data = (char *)load_single_map_file_to_buffer(lvnum,"txt",&script_len,LMFF_None);
    if (script_data == NULL)
      return false;
    // Process the file lines
    buf = script_data;
    buf_end = script_data+script_len;
    while (buf < buf_end)
    {
      // Find end of the line
      lnlen = 0;
      while (&buf[lnlen] < buf_end)
      {
        if ((buf[lnlen] == '\r') || (buf[lnlen] == '\n'))
          break;
        lnlen++;
      }
      // Get rid of the next line characters
      buf[lnlen] = 0;
      lnlen++;
      if (&buf[lnlen] < buf_end)
      {
        if ((buf[lnlen] == '\r') || (buf[lnlen] == '\n'))
          lnlen++;
      }
      // Analyze the line
      script_scan_line(buf, false);
      // Set new line start
      text_line_number++;
      buf += lnlen;
    }
    LbMemoryFree(script_data);
    if (game.script.win_conditions_num == 0)
      WARNMSG("No WIN GAME conditions in script file.");
    if (script_current_condition != -1)
      WARNMSG("Missing ENDIF's in script file.");
    JUSTLOG("Used script resources: %d/%d tunneller triggers, %d/%d party triggers, %d/%d script values, %d/%d IF conditions, %d/%d party definitions",
        (int)game.script.tunneller_triggers_num,TUNNELLER_TRIGGERS_COUNT,
        (int)game.script.party_triggers_num,PARTY_TRIGGERS_COUNT,
        (int)game.script.values_num,SCRIPT_VALUES_COUNT,
        (int)game.script.conditions_num,CONDITIONS_COUNT,
        (int)game.script.creature_partys_num,CREATURE_PARTYS_COUNT);
    return true;
}

void script_process_win_game(unsigned short plyr_idx)
{
    struct PlayerInfo *player;
    player = get_player(plyr_idx);
    set_player_as_won_level(player);
}

void script_process_lose_game(unsigned short plyr_idx)
{
    struct PlayerInfo *player;
    player = get_player(plyr_idx);
    set_player_as_lost_level(player);
}

struct Thing *create_thing_at_position_then_move_to_valid_and_add_light(struct Coord3d *pos, unsigned char tngclass, unsigned char tngmodel, unsigned char tngowner)
{
    struct Thing *thing;
    struct InitLight ilght;
    long light_rand;
    thing = create_thing(pos, tngclass, tngmodel, tngowner, -1);
    if (thing_is_invalid(thing))
    {
        return INVALID_THING;
    }
    thing->mappos.z.val = get_thing_height_at(thing, &thing->mappos);
    // Try to move thing out of the solid wall if it's inside one
    if (thing_in_wall_at(thing, &thing->mappos))
    {
        if (!move_creature_to_nearest_valid_position(thing)) {
            ERRORLOG("The %s was created in wall, removing",thing_model_name(thing));
            delete_thing_structure(thing, 0);
            return INVALID_THING;
        }
    }

    if (thing_is_creature(thing))
    {
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(thing);
        cctrl->flee_pos.x.val = thing->mappos.x.val;
        cctrl->flee_pos.y.val = thing->mappos.y.val;
        cctrl->flee_pos.z.val = thing->mappos.z.val;
        cctrl->flee_pos.z.val = get_thing_height_at(thing, &thing->mappos);
        cctrl->party.target_plyr_idx = -1;
    }

    light_rand = ACTION_RANDOM(8);
    if (light_rand < 2)
    {
        LbMemorySet(&ilght, 0, sizeof(struct InitLight));
        ilght.mappos.x.val = thing->mappos.x.val;
        ilght.mappos.y.val = thing->mappos.y.val;
        ilght.mappos.z.val = thing->mappos.z.val;
        if (light_rand == 1)
        {
            ilght.field_2 = 48;
            ilght.field_3 = 5;
        } else
        {
            ilght.field_2 = 36;
            ilght.field_3 = 1;
        }
        ilght.is_dynamic = 1;
        ilght.field_0 = 2560;
        thing->light_id = light_create_light(&ilght);
        if (thing->light_id != 0) {
            light_set_light_never_cache(thing->light_id);
        } else {
            ERRORLOG("Cannot allocate light to new hero");
        }
    }
    return thing;
}

long script_support_create_thing_at_hero_door(long gate_num, ThingClass tngclass, ThingModel tngmodel, unsigned char tngowner, unsigned char random_factor)
{
    struct Thing *thing;
    struct CreatureControl *cctrl;
    struct Thing *gatetng;
    struct Coord3d pos;
    SYNCDBG(7,"Starting creation of %s at HG%d",thing_class_and_model_name(tngclass,tngmodel),(int)gate_num);
    //return _DK_script_support_create_thing_at_hero_door(gate_num, tngclass, tngmodel, tngowner, random_factor);
    if (gate_num <= 0)
    {
      ERRORLOG("Script error - invalid hero gate index %d",(int)gate_num);
      return 0;
    }
    gatetng = find_hero_gate_of_number(gate_num);
    if (thing_is_invalid(gatetng))
    {
      ERRORLOG("Script error - could not find hero gate index %d",(int)gate_num);
      return 0;
    }
    pos.x.val = gatetng->mappos.x.val;
    pos.y.val = gatetng->mappos.y.val;
    pos.z.val = gatetng->mappos.z.val + 384;
    thing = create_thing_at_position_then_move_to_valid_and_add_light(&pos, tngclass, tngmodel, tngowner);
    if (thing_is_invalid(thing))
    {
        // Error is already logged
        return 0;
    }
    cctrl = creature_control_get_from_thing(thing);
    cctrl->field_AE |= 0x02;
    cctrl->spell_flags |= CSAfF_Unkn2000;
    thing->acceleration.x.val += ACTION_RANDOM(193) - 96;
    thing->acceleration.y.val += ACTION_RANDOM(193) - 96;
    if ((thing->movement_flags & TMvF_Flying) != 0) {
        thing->acceleration.z.val -= ACTION_RANDOM(32);
    } else {
        thing->acceleration.z.val += ACTION_RANDOM(96) + 80;
    }
    thing->field_1 |= TF1_PushdByAccel;

    if ((get_creature_model_flags(thing) & MF_IsLordOTLand) != 0)
    {
        output_message(SMsg_LordOfLandComming, 0, 1);
        output_message(SMsg_EnemyLordQuote + ACTION_RANDOM(8), 0, 1);
    }
    return thing->index;
}

long script_support_create_thing_at_action_point(long apt_idx, ThingClass tngclass, ThingModel tngmodel, PlayerNumber tngowner, unsigned char random_factor)
{
    SYNCDBG(7,"Starting creation of %s at action point %d",thing_class_and_model_name(tngclass,tngmodel),(int)apt_idx);
    //return _DK_script_support_create_thing_at_action_point(apt_idx, tngclass, tngmodel, tngowner, random_factor);
    struct Thing *thing;
    struct CreatureControl *cctrl;
    struct ActionPoint *apt;
    long direction,delta_x,delta_y;
    struct Coord3d pos;
    struct Thing *heartng;

    apt = action_point_get(apt_idx);
    if (!action_point_exists(apt))
    {
        ERRORLOG("Attempt to create thing at non-existing action point %d",(int)apt_idx);
        return 0;
    }

    if ( (random_factor == 0) || (apt->range == 0) )
    {
        pos.x.val = apt->mappos.x.val;
        pos.y.val = apt->mappos.y.val;
    } else
    {
        direction = ACTION_RANDOM(2*LbFPMath_PI);
        delta_x = (apt->range * LbSinL(direction) >> 8);
        delta_y = (apt->range * LbCosL(direction) >> 8);
        pos.x.val = apt->mappos.x.val + (delta_x >> 8);
        pos.y.val = apt->mappos.y.val - (delta_y >> 8);
    }

    thing = create_thing_at_position_then_move_to_valid_and_add_light(&pos, tngclass, tngmodel, tngowner);
    if (thing_is_invalid(thing))
    {
        // Error is already logged
        return 0;
    }

    cctrl = creature_control_get_from_thing(thing);
    heartng = get_player_soul_container(thing->owner);
    if ( thing_exists(heartng) && creature_can_navigate_to(thing, &heartng->mappos, 1) )
    {
        cctrl->field_AE |= 0x01;
    }

    if ((get_creature_model_flags(thing) & MF_IsLordOTLand) != 0)
    {
        output_message(SMsg_LordOfLandComming, 0, 1);
        output_message(SMsg_EnemyLordQuote + ACTION_RANDOM(8), 0, 1);
    }
    return thing->index;
}

/**
 * Creates a thing on given players dungeon heart.
 * Originally was script_support_create_creature_at_dungeon_heart().
 * @param tngclass
 * @param tngmodel
 * @param tngowner
 * @param plyr_idx
 */
long script_support_create_thing_at_dungeon_heart(ThingClass tngclass, ThingModel tngmodel, PlayerNumber tngowner, PlayerNumber plyr_idx)
{
    SYNCDBG(7,"Starting creation of %s at player %d",thing_class_and_model_name(tngclass,tngmodel),(int)plyr_idx);
    //return _DK_script_support_create_creature_at_dungeon_heart(tngmodel, tngowner, plyr_idx);
    struct Thing *heartng;
    heartng = get_player_soul_container(plyr_idx);
    TRACE_THING(heartng);
    if (thing_is_invalid(heartng))
    {
        ERRORLOG("Attempt to create thing in player %d dungeon with no heart",(int)plyr_idx);
        return 0;
    }
    struct Coord3d pos;
    pos.x.val = heartng->mappos.x.val + ACTION_RANDOM(65) - 32;
    pos.y.val = heartng->mappos.y.val + ACTION_RANDOM(65) - 32;
    pos.z.val = heartng->mappos.z.val;
    struct Thing *thing;
    thing = create_thing_at_position_then_move_to_valid_and_add_light(&pos, tngclass, tngmodel, tngowner);
    if (thing_is_invalid(thing))
    {
        // Error is already logged
        return 0;
    }
    if (thing_is_creature(thing))
    {
        if ((get_creature_model_flags(thing) & MF_IsLordOTLand) != 0)
        {
            output_message(SMsg_LordOfLandComming, 0, 1);
            output_message(SMsg_EnemyLordQuote + ACTION_RANDOM(8), 0, 1);
        }
    }
    return thing->index;
}

long send_tunneller_to_point(struct Thing *thing, struct Coord3d *pos)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->party.target_plyr_idx = -1;
    setup_person_tunnel_to_position(thing, pos->x.stl.num, pos->y.stl.num, 0);
    thing->continue_state = CrSt_TunnellerDoingNothing;
    return 1;
}

TbBool script_support_send_tunneller_to_action_point(struct Thing *thing, long apt_idx)
{
    struct ActionPoint *apt;
    SYNCDBG(7,"Starting");
    //return _DK_script_support_send_tunneller_to_action_point(thing, apt_idx);
    apt = action_point_get(apt_idx);
    struct Coord3d pos;
    if (action_point_exists(apt)) {
        pos.x.val = apt->mappos.x.val;
        pos.y.val = apt->mappos.y.val;
    } else {
        ERRORLOG("Attempt to send to non-existing action point %d",(int)apt_idx);
        pos.x.val = subtile_coord_center(map_subtiles_x/2);
        pos.y.val = subtile_coord_center(map_subtiles_y/2);
    }
    pos.z.val = subtile_coord(1,0);
    send_tunneller_to_point(thing, &pos);
    return true;
}

TbBool script_support_send_tunneller_to_dungeon(struct Thing *creatng, PlayerNumber plyr_idx)
{
    SYNCDBG(7,"Starting");
    //return _DK_script_support_send_tunneller_to_dungeon(creatng, a2);
    struct Thing *heartng;
    heartng = get_player_soul_container(plyr_idx);
    TRACE_THING(heartng);
    if (thing_is_invalid(heartng))
    {
        WARNLOG("Tried to send %s to player %d which has no heart", thing_model_name(creatng), (int)plyr_idx);
        return false;
    }
    struct Coord3d pos;
    if (!get_random_position_in_dungeon_for_creature(plyr_idx, 1, creatng, &pos)) {
        WARNLOG("Tried to send %s to player %d but can't find position", thing_model_name(creatng), (int)plyr_idx);
        return false;
    }
    if (!send_tunneller_to_point_in_dungeon(creatng, plyr_idx, &pos)) {
        WARNLOG("Tried to send %s to player %d but can't start the task", thing_model_name(creatng), (int)plyr_idx);
        return false;
    }
    return true;
}

TbBool script_support_send_tunneller_to_dungeon_heart(struct Thing *creatng, PlayerNumber plyr_idx)
{
    SYNCDBG(7,"Starting");
    //return _DK_script_support_send_tunneller_to_dungeon_heart(creatng, a2);
    struct Thing *heartng;
    heartng = get_player_soul_container(plyr_idx);
    TRACE_THING(heartng);
    if (thing_is_invalid(heartng)) {
        WARNLOG("Tried to send %s to player %d which has no heart", thing_model_name(creatng), (int)plyr_idx);
        return false;
    }
    if (send_tunneller_to_point_in_dungeon(creatng, plyr_idx, &heartng->mappos)) {
        WARNLOG("Tried to send %s to player %d but can't start the task", thing_model_name(creatng), (int)plyr_idx);
        return false;
    }
    return true;
}

long script_support_send_tunneller_to_appropriate_dungeon(struct Thing *thing)
{
    SYNCDBG(7,"Starting");
    return _DK_script_support_send_tunneller_to_appropriate_dungeon(thing);
}

struct Thing *script_create_creature_at_location(PlayerNumber plyr_idx, ThingModel crmodel, TbMapLocation location)
{
    struct CreatureControl *cctrl;
    struct Thing *thing;
    long tng_idx;
    long effect;
    long i;
    switch (get_map_location_type(location))
    {
    case MLoc_ACTIONPOINT:
        i = get_map_location_longval(location);
        tng_idx = script_support_create_thing_at_action_point(i, TCls_Creature, crmodel, plyr_idx, 1);
        effect = 1;
        break;
    case MLoc_HEROGATE:
        i = get_map_location_longval(location);
        tng_idx = script_support_create_thing_at_hero_door(i, TCls_Creature, crmodel, plyr_idx, 1);
        effect = 0;
        break;
    case MLoc_PLAYERSHEART:
        i = get_map_location_longval(location);
        tng_idx = script_support_create_thing_at_dungeon_heart(TCls_Creature, crmodel, plyr_idx, i);
        effect = 0;
        break;
    case MLoc_CREATUREKIND:
    case MLoc_OBJECTKIND:
    case MLoc_ROOMKIND:
    case MLoc_THING:
    case MLoc_PLAYERSDUNGEON:
    case MLoc_APPROPRTDUNGEON:
    case MLoc_DOORKIND:
    case MLoc_TRAPKIND:
    case MLoc_NONE:
    default:
        tng_idx = 0;
        effect = 0;
        break;
    }
    thing = thing_get(tng_idx);
    if (thing_is_invalid(thing))
    {
        ERRORLOG("Couldn't create %s at location %d",thing_class_and_model_name(TCls_Creature,crmodel),(int)location);
        return INVALID_THING;
    }
    cctrl = creature_control_get_from_thing(thing);
    switch (effect)
    {
    case 1:
        if (plyr_idx == game.hero_player_num)
        {
            thing->mappos.z.val = get_ceiling_height(&thing->mappos);
            create_effect(&thing->mappos, TngEff_Unknown36, thing->owner);
            initialise_thing_state(thing, CrSt_CreatureHeroEntering);
            thing->field_4F |= 0x01;
            cctrl->field_282 = 24;
        }
        break;
    default:
        break;
    }
    return thing;
}

struct Thing *script_process_new_tunneler(unsigned char plyr_idx, TbMapLocation location, TbMapLocation heading, unsigned char crtr_level, unsigned long carried_gold)
{
    struct Thing *creatng;
    //return _DK_script_process_new_tunneller(plyr_idx, location, a3, a4, a5, a6);
    ThingModel diggerkind = get_players_special_digger_breed(game.hero_player_num);
    creatng = script_create_creature_at_location(plyr_idx, diggerkind, location);
    if (thing_is_invalid(creatng))
        return INVALID_THING;
    creatng->creature.gold_carried = carried_gold;
    init_creature_level(creatng, crtr_level);
    switch (get_map_location_type(heading))
    {
    case MLoc_ACTIONPOINT:
        script_support_send_tunneller_to_action_point(creatng, get_map_location_longval(heading));
        break;
    case MLoc_PLAYERSDUNGEON:
        script_support_send_tunneller_to_dungeon(creatng, get_map_location_longval(heading));
        break;
    case MLoc_PLAYERSHEART:
        script_support_send_tunneller_to_dungeon_heart(creatng, get_map_location_longval(heading));
        break;
    case MLoc_APPROPRTDUNGEON:
        script_support_send_tunneller_to_appropriate_dungeon(creatng);
        break;
    default:
        ERRORLOG("Invalid Heading objective %d",(int)get_map_location_type(heading));
        break;
    }
    return creatng;
}

struct Thing *script_process_new_party(struct Party *party, unsigned char plyr_idx, TbMapLocation location, long copies_num)
{
    struct CreatureControl *cctrl;
    struct PartyMember *member;
    struct Thing *prthing;
    struct Thing *ldthing;
    struct Thing *thing;
    long i,k;
    //return _DK_script_process_new_party(party, plyr_idx, location, copies_num);
    ldthing = NULL;
    for (i=0; i < copies_num; i++)
    {
      prthing = NULL;
      for (k=0; k < party->members_num; k++)
      {
        if (k >= PARTY_MEMBERS_COUNT)
        {
            ERRORLOG("Party too big");
            break;
        }
        member = &(party->members[k]);
        thing = script_create_new_creature(plyr_idx, member->crtr_kind, location, member->carried_gold, member->crtr_level);
        if (!thing_is_invalid(thing))
        {
            cctrl = creature_control_get_from_thing(thing);
            cctrl->party_objective = member->objectv;
            cctrl->field_5 = game.play_gameturn + member->countdown;
            if (!thing_is_invalid(prthing))
            {
                if (cctrl->explevel <= get_highest_experience_level_in_group(prthing))
                {
                    add_creature_to_group(thing, prthing);
                } else
                {
                    add_creature_to_group_as_leader(thing, prthing);
                    ldthing = thing;
                }
            }
            prthing = thing;
        }
      }
    }
    return ldthing;
}

struct Thing *script_create_new_creature(PlayerNumber plyr_idx, ThingModel crmodel, TbMapLocation location, long carried_gold, long crtr_level)
{
    struct Thing *creatng;
    //return _DK_script_create_new_creature(plyr_idx, breed, location, carried_gold, crtr_level);
    creatng = script_create_creature_at_location(plyr_idx, crmodel, location);
    if (thing_is_invalid(creatng))
        return INVALID_THING;
    creatng->creature.gold_carried = carried_gold;
    init_creature_level(creatng, crtr_level);
    return creatng;
}

void script_process_new_tunneller_party(unsigned char plyr_idx, long prty_id, TbMapLocation location, TbMapLocation heading, unsigned char crtr_level, unsigned long carried_gold)
{
    struct Thing *gpthing;
    struct Thing *ldthing;
    //_DK_script_process_new_tunneller_party(a1, a2, a3, a4, a5, a6, a7);
    ldthing = script_process_new_tunneler(plyr_idx, location, heading, crtr_level, carried_gold);
    if (thing_is_invalid(ldthing))
    {
        ERRORLOG("Couldn't create tunneling group leader");
        return;
    }
    gpthing = script_process_new_party(&game.script.creature_partys[prty_id], plyr_idx, location, 1);
    if (thing_is_invalid(gpthing))
    {
        ERRORLOG("Couldn't create creature group");
        return;
    }
    add_creature_to_group_as_leader(ldthing, gpthing);
}

void script_process_new_creatures(unsigned char plyr_idx, long crtr_breed, long location, long copies_num, long carried_gold, long crtr_level)
{
    long i;
    for (i=0; i < copies_num; i++) {
        script_create_new_creature(plyr_idx, crtr_breed, location, carried_gold, crtr_level);
    }
}

TbBool script_kill_creature_with_criteria(PlayerNumber plyr_idx, long crtr_breed, long criteria)
{
    //TODO SCRIPT finish killing code!
    return false;
}

void script_kill_creatures(PlayerNumber plyr_idx, long crtr_breed, long criteria, long copies_num)
{
    long i;
    for (i=0; i < copies_num; i++) {
        script_kill_creature_with_criteria(plyr_idx, crtr_breed, criteria);
    }
}

/**
 * Returns if the action point condition was activated.
 * Action point index and player to be activated should be stored inside condition.
 */
TbBool process_activation_status(struct Condition *condt)
{
  return action_point_activated_by_player(condt->variabl_idx,condt->plyr_idx);
}

/**
 * Returns if the action point of given index was triggered by given player.
 */
TbBool action_point_activated_by_player(long apt_idx,long plyr_idx)
{
  unsigned long i;
  i = get_action_point_activated_by_players_mask(apt_idx);
  if (plyr_idx == 8)
    return (i != 0);
  else
    return ((i & (1 << plyr_idx)) != 0);
}

long get_condition_value(char plyr_idx, unsigned char valtype, unsigned char validx)
{
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  SYNCDBG(10,"Checking condition %d for player %d",(int)valtype,(int)plyr_idx);
  switch (valtype)
  {
  case SVar_MONEY:
      dungeon = get_dungeon(plyr_idx);
      return dungeon->total_money_owned;
  case SVar_GAME_TURN:
      return game.play_gameturn;
  case SVar_BREAK_IN:
      dungeon = get_dungeon(plyr_idx);
      return dungeon->field_AF5;
  case SVar_CREATURE_NUM:
      if (validx == get_players_special_digger_breed(plyr_idx))
      {
          dungeon = get_dungeon(plyr_idx);
          return dungeon->num_active_diggers;
      } else
      {
        return count_player_creatures_of_model(plyr_idx, validx);
      }
  case SVar_TOTAL_IMPS:
      dungeon = get_dungeon(plyr_idx);
      return dungeon->num_active_diggers;
  case SVar_TOTAL_CREATURES:
      dungeon = get_dungeon(plyr_idx);
      return dungeon->num_active_creatrs - count_player_creatures_not_counting_to_total(plyr_idx);
  case SVar_TOTAL_RESEARCH:
      dungeon = get_dungeon(plyr_idx);
      return dungeon->total_research_points / 256;
  case SVar_TOTAL_DOORS:
      dungeon = get_dungeon(plyr_idx);
      return dungeon->total_doors;
  case SVar_TOTAL_AREA:
      dungeon = get_dungeon(plyr_idx);
      return dungeon->total_area;
  case SVar_TOTAL_CREATURES_LEFT:
      dungeon = get_dungeon(plyr_idx);
      return dungeon->total_creatures_left;
  case SVar_CREATURES_ANNOYED:
      dungeon = get_dungeon(plyr_idx);
      return dungeon->creatures_annoyed;
  case SVar_BATTLES_LOST:
      dungeon = get_dungeon(plyr_idx);
      return dungeon->battles_lost;
  case SVar_BATTLES_WON:
      dungeon = get_dungeon(plyr_idx);
      return dungeon->battles_won;
  case SVar_ROOMS_DESTROYED:
      dungeon = get_dungeon(plyr_idx);
      return dungeon->rooms_destroyed;
  case SVar_SPELLS_STOLEN:
      dungeon = get_dungeon(plyr_idx);
      return dungeon->spells_stolen;
  case SVar_TIMES_BROKEN_INTO:
      dungeon = get_dungeon(plyr_idx);
      return dungeon->times_broken_into;
  case SVar_GOLD_POTS_STOLEN:
      dungeon = get_dungeon(plyr_idx);
      return dungeon->gold_pots_stolen;
  case SVar_TIMER:
      dungeon = get_dungeon(plyr_idx);
      if (dungeon->turn_timers[validx].state)
        return game.play_gameturn - dungeon->turn_timers[validx].count;
      else
        return 0;
  case SVar_DUNGEON_DESTROYED:
      return !player_has_heart(plyr_idx);
  case SVar_TOTAL_GOLD_MINED:
      dungeon = get_dungeon(plyr_idx);
      return dungeon->lvstats.gold_mined;
  case SVar_FLAG:
      dungeon = get_dungeon(plyr_idx);
      return dungeon->script_flags[validx];
  case SVar_ROOM_SLABS:
      return get_room_slabs_count(plyr_idx, validx);
  case SVar_DOORS_DESTROYED:
      dungeon = get_dungeon(plyr_idx);
      return dungeon->doors_destroyed;
  case SVar_CREATURES_SCAVENGED_LOST:
      dungeon = get_dungeon(plyr_idx);
      return dungeon->creatures_scavenge_lost;
  case SVar_CREATURES_SCAVENGED_GAINED:
      dungeon = get_dungeon(plyr_idx);
      return dungeon->creatures_scavenge_gain;
  case SVar_AVAILABLE_MAGIC: // IF_AVAILABLE(MAGIC)
      return is_power_available(plyr_idx, validx);
  case SVar_AVAILABLE_TRAP: // IF_AVAILABLE(TRAP)
      dungeon = get_dungeon(plyr_idx);
      return dungeon->trap_amount[validx%TRAP_TYPES_COUNT];
  case SVar_AVAILABLE_DOOR: // IF_AVAILABLE(DOOR)
      dungeon = get_dungeon(plyr_idx);
      return dungeon->door_amount[validx%DOOR_TYPES_COUNT];
  case SVar_AVAILABLE_ROOM: // IF_AVAILABLE(ROOM)
      dungeon = get_dungeon(plyr_idx);
      return dungeon->room_buildable[validx%ROOM_TYPES_COUNT];
  case SVar_ALL_DUNGEONS_DESTROYED:
      player = get_player(plyr_idx);
      return all_dungeons_destroyed(player);
  case SVar_DOOR_NUM:
      return find_door_of_type(validx, plyr_idx);
  default:
      return 0;
  };
}

TbBool get_condition_status(unsigned char opkind, long val1, long val2)
{
  return LbMathOperation(opkind, val1, val2) != 0;
}

TbBool is_condition_met(long cond_idx)
{
    unsigned long i;
    if ((cond_idx < 0) || (cond_idx >= CONDITIONS_COUNT))
    {
      if (cond_idx == -1)
          return true;
      else
          return false;
    }
    i = game.script.conditions[cond_idx].status;
    return ((i & 0x01) != 0);
}

TbBool condition_inactive(long cond_idx)
{
  unsigned long i;
  if ((cond_idx < 0) || (cond_idx >= CONDITIONS_COUNT))
  {
    if (cond_idx == -1)
      return false;
    else
      return false;
  }
  i = game.script.conditions[cond_idx].status;
  if (((i & 0x01) == 0) || ((i & 0x04) != 0))
    return true;
  return false;
}

void process_condition(struct Condition *condt)
{
    TbBool new_status;
    int plr_start, plr_end;
    long i,k;
    SYNCDBG(18,"Starting for type %d, player %d",(int)condt->variabl_type,(int)condt->plyr_idx);
    if (condition_inactive(condt->condit_idx))
    {
      set_flag_byte(&condt->status, 0x01, false);
      return;
    }
    if (condt->plyr_idx == ALL_PLAYERS)
    {
      plr_start = 0;
      plr_end = game.hero_player_num;
    } else
    {
      plr_start = condt->plyr_idx;
      plr_end = plr_start+1;
    }
    if (plr_start > PLAYERS_COUNT)
        plr_start = PLAYERS_COUNT;
    if (plr_start < 0)
        plr_start = 0;
    if (plr_end > PLAYERS_COUNT)
        plr_end = PLAYERS_COUNT;
    if (condt->variabl_type == 19)
    {
      new_status = false;
      for (i=plr_start; i < plr_end; i++)
      {
        new_status = action_point_activated_by_player(condt->variabl_idx,i);
        if (new_status) break;
      }
    } else
    {
      new_status = false;
      for (i=plr_start; i < plr_end; i++)
      {
        k = get_condition_value(i, condt->variabl_type, condt->variabl_idx);
        new_status = get_condition_status(condt->operation, k, condt->rvalue);
        if (new_status != false) break;
      }
    }
    SYNCDBG(19,"Condition type %d status %d",(int)condt->variabl_type,(int)new_status);
    set_flag_byte(&condt->status, 0x01,  new_status);
    if (((condt->status & 0x01) == 0) || ((condt->status & 0x02) != 0))
    {
      set_flag_byte(&condt->status, 0x04,  false);
    } else
    {
      set_flag_byte(&condt->status, 0x02,  true);
      set_flag_byte(&condt->status, 0x04,  true);
    }
    SCRIPTDBG(19,"Finished");
}

void process_conditions(void)
{
    long i;
    if (game.script.conditions_num > CONDITIONS_COUNT)
      game.script.conditions_num = CONDITIONS_COUNT;
    for (i=0; i < game.script.conditions_num; i++)
    {
      process_condition(&game.script.conditions[i]);
    }
}

void process_check_new_creature_partys(void)
{
    struct PartyTrigger *pr_trig;
    long i,n;
    for (i=0; i < game.script.party_triggers_num; i++)
    {
      pr_trig = &game.script.party_triggers[i];
      if ((pr_trig->flags & TrgF_DISABLED) == 0)
      {
        if (is_condition_met(pr_trig->condit_idx))
        {
          n = pr_trig->creatr_id;
          if (n <= 0)
          {
            SYNCDBG(6,"Adding player %d party %d in location %d",(int)pr_trig->plyr_idx,(int)-n,(int)pr_trig->location);
            script_process_new_party(&game.script.creature_partys[-n],
                pr_trig->plyr_idx, pr_trig->location, pr_trig->ncopies);
          } else
          {
            SCRIPTDBG(6,"Adding creature %d",n);
            script_process_new_creatures(pr_trig->plyr_idx, n, pr_trig->location,
                pr_trig->ncopies, pr_trig->carried_gold, pr_trig->crtr_level);
          }
          if ((pr_trig->flags & TrgF_REUSABLE) == 0)
            set_flag_byte(&pr_trig->flags, TrgF_DISABLED, true);
        }
      }
    }
}

void process_check_new_tunneller_partys(void)
{
    struct TunnellerTrigger *tn_trig;
    struct Thing *grptng;
    struct Thing *thing;
    long i,k,n;
    for (i=0; i < game.script.tunneller_triggers_num; i++)
    {
      tn_trig = &game.script.tunneller_triggers[i];
      if ((tn_trig->flags & TrgF_DISABLED) == 0)
      {
        if (is_condition_met(tn_trig->condit_idx))
        {
          k = tn_trig->party_id;
          if (k > 0)
          {
            n = tn_trig->plyr_idx;
            SCRIPTDBG(6,"Adding tunneler party %d",k);
            thing = script_process_new_tunneler(n, tn_trig->location, tn_trig->heading,
                        tn_trig->crtr_level, tn_trig->carried_gold);
             if (!thing_is_invalid(thing))
             {
                grptng = script_process_new_party(&game.script.creature_partys[k-1], n, tn_trig->location, 1);
                if (!thing_is_invalid(grptng))
                  add_creature_to_group_as_leader(thing, grptng);
             }
          } else
          {
            SCRIPTDBG(6,"Adding tunneler, heading %d",tn_trig->heading);
            script_process_new_tunneler(tn_trig->plyr_idx, tn_trig->location, tn_trig->heading,
                  tn_trig->crtr_level, tn_trig->carried_gold);
          }
          if ((tn_trig->flags & TrgF_REUSABLE) == 0)
            tn_trig->flags |= TrgF_DISABLED;
        }
      }
    }
}

void process_win_and_lose_conditions(long plyr_idx)
{
    struct PlayerInfo *player;
    long i,k;
    player = get_player(plyr_idx);
    if ((game.system_flags & GSF_NetworkActive) != 0)
      return;
    for (i=0; i < game.script.win_conditions_num; i++)
    {
      k = game.script.win_conditions[i];
      if (is_condition_met(k)) {
          SYNCDBG(8,"Win condition %d (cond. %d) met for player %d.",(int)i,(int)k,(int)plyr_idx);
          set_player_as_won_level(player);
      }
    }
    for (i=0; i < game.script.lose_conditions_num; i++)
    {
      k = game.script.lose_conditions[i];
      if (is_condition_met(k)) {
          SYNCDBG(8,"Lose condition %d (cond. %d) met for player %d.",(int)i,(int)k,(int)plyr_idx);
          set_player_as_lost_level(player);
      }
    }
}

void process_values(void)
{
    struct ScriptValue *value;
    long i;
    for (i=0; i < game.script.values_num; i++)
    {
        value = &game.script.values[i];
        if ((value->flags & TrgF_DISABLED) == 0)
        {
            if (is_condition_met(value->condit_idx))
            {
                script_process_value(value->valtype, value->field_3, value->field_4, value->field_8, value->field_C);
                if ((value->flags & TrgF_REUSABLE) == 0)
                  set_flag_byte(&value->flags, TrgF_DISABLED, true);
            }
        }
    }
}

/**
 * Processes given VALUE immediately.
 */
void script_process_value(unsigned long var_index, unsigned long plr_id, long val2, long val3, long val4)
{
  struct CreatureStats *crstat;
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  int plr_start, plr_end;
  long i;
//  _DK_script_process_value(var_index, plr_id, val2, val3, val4);
  if (plr_id > ALL_PLAYERS)
  {
      WARNLOG("Invalid player index %ld in VALUE command %ld.",plr_id,var_index);
      return;
  }
  if (plr_id == ALL_PLAYERS)
  {
      plr_start = 0;
      plr_end = PLAYERS_COUNT;
  } else
  {
      plr_start = plr_id;
      plr_end = plr_id+1;
  }
  switch (var_index)
  {
  case Cmd_SET_HATE:
      for (i=plr_start; i < plr_end; i++)
      {
        dungeon = get_dungeon(i);
        if (dungeon_invalid(dungeon))
            continue;
        dungeon->hates_player[val2%DUNGEONS_COUNT] = val3;
      }
      break;
  case Cmd_SET_GENERATE_SPEED:
      game.generate_speed = saturate_set_unsigned(val2, 16);
      break;
  case Cmd_ROOM_AVAILABLE:
      for (i=plr_start; i < plr_end; i++)
      {
        set_room_available(i, val2, val3, val4);
      }
      break;
  case Cmd_CREATURE_AVAILABLE:
      for (i=plr_start; i < plr_end; i++)
      {
        if (!set_creature_available(i,val2,val3,val4))
          WARNLOG("Setting creature %ld availability for player %d failed.",val2,(int)i);
      }
      break;
  case Cmd_MAGIC_AVAILABLE:
      for (i=plr_start; i < plr_end; i++)
      {
        if (!set_power_available(i,val2,val3,val4))
          WARNLOG("Setting magic %ld availability for player %d failed.",val2,(int)i);
      }
      break;
  case Cmd_TRAP_AVAILABLE:
      for (i=plr_start; i < plr_end; i++)
      {
          set_trap_buildable_and_add_to_amount(i, val2, val3, val4);
      }
      break;
  case Cmd_RESEARCH:
      for (i=plr_start; i < plr_end; i++)
      {
        update_or_add_players_research_amount(i, val2, val3, val4);
      }
      break;
  case Cmd_RESEARCH_ORDER:
      for (i=plr_start; i < plr_end; i++)
      {
        if (!research_overriden_for_player(i))
          remove_all_research_from_player(i);
        add_research_to_player(i, val2, val3, val4);
      }
      break;
  case Cmd_SET_TIMER:
      for (i=plr_start; i < plr_end; i++)
      {
          restart_script_timer(i,val2);
      }
      break;
  case Cmd_SET_FLAG:
      for (i=plr_start; i < plr_end; i++)
      {
          set_script_flag(i,val2,val3);
      }
      break;
  case Cmd_MAX_CREATURES:
      for (i=plr_start; i < plr_end; i++)
      {
          SYNCDBG(4,"Setting player %d max attracted creatures to %ld.",i,val2);
          dungeon = get_dungeon(i);
          if (dungeon_invalid(dungeon))
              continue;
          dungeon->max_creatures_attracted = val2;
      }
      break;
  case Cmd_DOOR_AVAILABLE:
      for (i=plr_start; i < plr_end; i++) {
          set_door_buildable_and_add_to_amount(i, val2, val3, val4);
      }
      break;
  case Cmd_DISPLAY_OBJECTIVE:
      if ( (my_player_number >= plr_start) && (my_player_number < plr_end) ) {
          set_general_objective(val2, val3, stl_num_decode_x(val4), stl_num_decode_y(val4));
      }
      break;
  case Cmd_DISPLAY_INFORMATION:
      if ((my_player_number >= plr_start) && (my_player_number < plr_end)) {
          set_general_information(val2, val3, stl_num_decode_x(val4), stl_num_decode_y(val4));
      }
      break;
  case Cmd_ADD_CREATURE_TO_POOL:
      add_creature_to_pool(val2, val3, 0);
      break;
  case Cmd_RESET_ACTION_POINT:
      action_point_reset_idx(val2);
      break;
  case Cmd_TUTORIAL_FLASH_BUTTON:
      gui_set_button_flashing(val2, val3);
      break;
  case Cmd_SET_CREATURE_MAX_LEVEL:
      for (i=plr_start; i < plr_end; i++)
      {
          dungeon = get_dungeon(i);
          if (dungeon_invalid(dungeon))
              continue;
          dungeon->creature_max_level[val2%CREATURE_TYPES_COUNT] = val3;
      }
      break;
  case Cmd_SET_CREATURE_HEALTH:
      change_max_health_of_creature_kind(val2, val3);
      break;
  case Cmd_SET_CREATURE_STRENGTH:
      crstat = creature_stats_get(val2);
      if (creature_stats_invalid(crstat))
          break;
      crstat->strength = saturate_set_unsigned(val3, 8);
      creature_stats_updated(val2);
      break;
  case Cmd_SET_CREATURE_ARMOUR:
      crstat = creature_stats_get(val2);
      if (creature_stats_invalid(crstat))
          break;
      crstat->armour = saturate_set_unsigned(val3, 8);
      creature_stats_updated(val2);
      break;
  case Cmd_SET_CREATURE_FEAR_WOUNDED:
      crstat = creature_stats_get(val2);
      if (creature_stats_invalid(crstat))
          break;
      if (level_file_version > 0)
          crstat->fear_wounded = saturate_set_unsigned(val3, 8);
      else
          crstat->fear_wounded = saturate_set_unsigned(101*val3/255, 8); // old fear was scaled 0..255
      creature_stats_updated(val2);
      break;
  case Cmd_SET_CREATURE_FEAR_STRONGER:
      crstat = creature_stats_get(val2);
      if (creature_stats_invalid(crstat))
          break;
      crstat->fear_stronger = saturate_set_unsigned(val3, 16);
      creature_stats_updated(val2);
      break;
  case Cmd_ALLY_PLAYERS:
      set_ally_with_player(val2, val3, val4);
      set_ally_with_player(val3, val2, val4);
      break;
  case Cmd_DEAD_CREATURES_RETURN_TO_POOL:
      set_flag_byte(&game.flags_cd, MFlg_DeadBackToPool, val2);
      break;
  case Cmd_BONUS_LEVEL_TIME:
      game.bonus_time = val2;
      set_flag_byte(&game.flags_gui,GGUI_CountdownTimer,(val2 > 0));
      break;
  case Cmd_QUICK_OBJECTIVE:
      if ((my_player_number >= plr_start) && (my_player_number < plr_end))
          process_objective(gameadd.quick_messages[val2%QUICK_MESSAGES_COUNT], val3, stl_num_decode_x(val4), stl_num_decode_y(val4));
      break;
  case Cmd_QUICK_INFORMATION:
      if ((my_player_number >= plr_start) && (my_player_number < plr_end))
          set_quick_information(val2, val3, stl_num_decode_x(val4), stl_num_decode_y(val4));
      break;
  case Cmd_PLAY_MESSAGE:
      if ((my_player_number >= plr_start) && (my_player_number < plr_end))
      {
          switch (val2)
          {
          case 1:
              output_message(val3, 0, true);
              break;
          case 2:
              play_non_3d_sample(val3);
              break;
          }
      }
      break;
  case Cmd_ADD_GOLD_TO_PLAYER:
      for (i=plr_start; i < plr_end; i++)
      {
          player_command_add_start_money(i, val2);
      }
      break;
  case Cmd_SET_CREATURE_TENDENCIES:
      for (i=plr_start; i < plr_end; i++)
      {
          player = get_player(i);
          set_creature_tendencies(player, val2, val3);
      }
      break;
  case Cmd_REVEAL_MAP_RECT:
      for (i=plr_start; i < plr_end; i++)
      {
          player_reveal_map_area(i, val2, val3, (val4)&0xffff, (val4>>16)&0xffff);
      }
      break;
  case Cmd_REVEAL_MAP_LOCATION:
      for (i=plr_start; i < plr_end; i++)
      {
          player_reveal_map_location(i, val2, val3);
      }
      break;
  case Cmd_KILL_CREATURE:
      for (i=plr_start; i < plr_end; i++)
      {
          script_kill_creatures(i, val2, val3, val3);
      }
      break;
  default:
      WARNMSG("Unsupported Game VALUE, command %d.",var_index);
      break;
  }
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
