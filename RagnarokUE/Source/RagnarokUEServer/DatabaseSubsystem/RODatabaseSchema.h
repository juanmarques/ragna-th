// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * RODatabaseSchema
 * SQL schema string constants for all game database tables.
 * These can be executed against a PostgreSQL backend to create the schema.
 * Compatible with SQLite for local development.
 */
namespace RODatabaseSchema
{
	/** Accounts table - stores login credentials and account status. */
	static const TCHAR* CreateAccountsTable = TEXT(
		"CREATE TABLE IF NOT EXISTS accounts ("
		"  account_id SERIAL PRIMARY KEY,"
		"  username VARCHAR(24) NOT NULL UNIQUE,"
		"  password_hash VARCHAR(128) NOT NULL,"
		"  password_salt VARCHAR(64) NOT NULL,"
		"  email VARCHAR(128),"
		"  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
		"  last_login_at TIMESTAMP,"
		"  is_banned BOOLEAN DEFAULT FALSE,"
		"  ban_reason TEXT,"
		"  login_attempts INTEGER DEFAULT 0,"
		"  gm_level INTEGER DEFAULT 0"
		");"
	);

	/** Characters table - stores character data and progression. */
	static const TCHAR* CreateCharactersTable = TEXT(
		"CREATE TABLE IF NOT EXISTS characters ("
		"  character_id SERIAL PRIMARY KEY,"
		"  account_id INTEGER NOT NULL,"
		"  character_name VARCHAR(24) NOT NULL UNIQUE,"
		"  job_class INTEGER NOT NULL DEFAULT 0,"
		"  base_level INTEGER NOT NULL DEFAULT 1,"
		"  job_level INTEGER NOT NULL DEFAULT 1,"
		"  base_exp BIGINT NOT NULL DEFAULT 0,"
		"  job_exp BIGINT NOT NULL DEFAULT 0,"
		"  hp INTEGER NOT NULL DEFAULT 40,"
		"  sp INTEGER NOT NULL DEFAULT 10,"
		"  str INTEGER NOT NULL DEFAULT 1,"
		"  agi INTEGER NOT NULL DEFAULT 1,"
		"  vit INTEGER NOT NULL DEFAULT 1,"
		"  int_stat INTEGER NOT NULL DEFAULT 1,"
		"  dex INTEGER NOT NULL DEFAULT 1,"
		"  luk INTEGER NOT NULL DEFAULT 1,"
		"  status_points INTEGER NOT NULL DEFAULT 48,"
		"  skill_points INTEGER NOT NULL DEFAULT 0,"
		"  zeny BIGINT NOT NULL DEFAULT 0,"
		"  saved_map VARCHAR(32) NOT NULL DEFAULT 'prontera',"
		"  saved_x FLOAT NOT NULL DEFAULT 0,"
		"  saved_y FLOAT NOT NULL DEFAULT 0,"
		"  saved_z FLOAT NOT NULL DEFAULT 0,"
		"  spawn_map VARCHAR(32) NOT NULL DEFAULT 'prontera',"
		"  guild_id INTEGER DEFAULT 0,"
		"  party_id INTEGER DEFAULT 0,"
		"  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
		"  last_save_at TIMESTAMP,"
		"  FOREIGN KEY (account_id) REFERENCES accounts(account_id)"
		");"
	);

	/** Inventory table - stores items per character. */
	static const TCHAR* CreateInventoryTable = TEXT(
		"CREATE TABLE IF NOT EXISTS inventory ("
		"  inventory_id SERIAL PRIMARY KEY,"
		"  character_id INTEGER NOT NULL,"
		"  item_id INTEGER NOT NULL,"
		"  amount INTEGER NOT NULL DEFAULT 1,"
		"  refine_level INTEGER NOT NULL DEFAULT 0,"
		"  card_slot_0 INTEGER DEFAULT 0,"
		"  card_slot_1 INTEGER DEFAULT 0,"
		"  card_slot_2 INTEGER DEFAULT 0,"
		"  card_slot_3 INTEGER DEFAULT 0,"
		"  unique_id VARCHAR(64) NOT NULL,"
		"  equip_slot INTEGER DEFAULT -1,"
		"  FOREIGN KEY (character_id) REFERENCES characters(character_id)"
		");"
	);

	/** Guild table - stores guild information. */
	static const TCHAR* CreateGuildsTable = TEXT(
		"CREATE TABLE IF NOT EXISTS guilds ("
		"  guild_id SERIAL PRIMARY KEY,"
		"  guild_name VARCHAR(24) NOT NULL UNIQUE,"
		"  leader_character_id INTEGER NOT NULL,"
		"  guild_level INTEGER NOT NULL DEFAULT 1,"
		"  guild_exp BIGINT NOT NULL DEFAULT 0,"
		"  guild_message TEXT,"
		"  guild_emblem BLOB,"
		"  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
		"  FOREIGN KEY (leader_character_id) REFERENCES characters(character_id)"
		");"
	);

	/** Guild members table - stores guild membership data. */
	static const TCHAR* CreateGuildMembersTable = TEXT(
		"CREATE TABLE IF NOT EXISTS guild_members ("
		"  guild_id INTEGER NOT NULL,"
		"  character_id INTEGER NOT NULL,"
		"  position INTEGER NOT NULL DEFAULT 0,"
		"  exp_contribution BIGINT NOT NULL DEFAULT 0,"
		"  joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
		"  PRIMARY KEY (guild_id, character_id),"
		"  FOREIGN KEY (guild_id) REFERENCES guilds(guild_id),"
		"  FOREIGN KEY (character_id) REFERENCES characters(character_id)"
		");"
	);

	/** Storage table - Kafra storage per account. */
	static const TCHAR* CreateStorageTable = TEXT(
		"CREATE TABLE IF NOT EXISTS storage ("
		"  storage_id SERIAL PRIMARY KEY,"
		"  account_id INTEGER NOT NULL,"
		"  item_id INTEGER NOT NULL,"
		"  amount INTEGER NOT NULL DEFAULT 1,"
		"  refine_level INTEGER NOT NULL DEFAULT 0,"
		"  card_slot_0 INTEGER DEFAULT 0,"
		"  card_slot_1 INTEGER DEFAULT 0,"
		"  card_slot_2 INTEGER DEFAULT 0,"
		"  card_slot_3 INTEGER DEFAULT 0,"
		"  unique_id VARCHAR(64) NOT NULL,"
		"  FOREIGN KEY (account_id) REFERENCES accounts(account_id)"
		");"
	);

	/** Guild storage table - shared storage per guild. */
	static const TCHAR* CreateGuildStorageTable = TEXT(
		"CREATE TABLE IF NOT EXISTS guild_storage ("
		"  storage_id SERIAL PRIMARY KEY,"
		"  guild_id INTEGER NOT NULL,"
		"  item_id INTEGER NOT NULL,"
		"  amount INTEGER NOT NULL DEFAULT 1,"
		"  refine_level INTEGER NOT NULL DEFAULT 0,"
		"  card_slot_0 INTEGER DEFAULT 0,"
		"  card_slot_1 INTEGER DEFAULT 0,"
		"  card_slot_2 INTEGER DEFAULT 0,"
		"  card_slot_3 INTEGER DEFAULT 0,"
		"  unique_id VARCHAR(64) NOT NULL,"
		"  FOREIGN KEY (guild_id) REFERENCES guilds(guild_id)"
		");"
	);

	/** Quests table - tracks quest progress per character. */
	static const TCHAR* CreateQuestsTable = TEXT(
		"CREATE TABLE IF NOT EXISTS quests ("
		"  quest_id INTEGER NOT NULL,"
		"  character_id INTEGER NOT NULL,"
		"  status INTEGER NOT NULL DEFAULT 0,"
		"  progress INTEGER NOT NULL DEFAULT 0,"
		"  started_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
		"  completed_at TIMESTAMP,"
		"  PRIMARY KEY (quest_id, character_id),"
		"  FOREIGN KEY (character_id) REFERENCES characters(character_id)"
		");"
	);

	/** Friends table - friend list per character. */
	static const TCHAR* CreateFriendsTable = TEXT(
		"CREATE TABLE IF NOT EXISTS friends ("
		"  character_id INTEGER NOT NULL,"
		"  friend_character_id INTEGER NOT NULL,"
		"  added_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
		"  PRIMARY KEY (character_id, friend_character_id),"
		"  FOREIGN KEY (character_id) REFERENCES characters(character_id),"
		"  FOREIGN KEY (friend_character_id) REFERENCES characters(character_id)"
		");"
	);

	/** Skills table - learned skills per character. */
	static const TCHAR* CreateSkillsTable = TEXT(
		"CREATE TABLE IF NOT EXISTS skills ("
		"  character_id INTEGER NOT NULL,"
		"  skill_id INTEGER NOT NULL,"
		"  skill_level INTEGER NOT NULL DEFAULT 0,"
		"  PRIMARY KEY (character_id, skill_id),"
		"  FOREIGN KEY (character_id) REFERENCES characters(character_id)"
		");"
	);

	/** Castle ownership table - WoE castle data. */
	static const TCHAR* CreateCastlesTable = TEXT(
		"CREATE TABLE IF NOT EXISTS castles ("
		"  castle_id INTEGER PRIMARY KEY,"
		"  guild_id INTEGER DEFAULT 0,"
		"  economy INTEGER DEFAULT 0,"
		"  defense INTEGER DEFAULT 0,"
		"  last_captured_at TIMESTAMP,"
		"  FOREIGN KEY (guild_id) REFERENCES guilds(guild_id)"
		");"
	);

	/** Anticheat logs table - suspicious activity tracking. */
	static const TCHAR* CreateAnticheatLogsTable = TEXT(
		"CREATE TABLE IF NOT EXISTS anticheat_logs ("
		"  log_id SERIAL PRIMARY KEY,"
		"  player_net_id VARCHAR(64),"
		"  account_id INTEGER,"
		"  activity_type VARCHAR(32) NOT NULL,"
		"  details TEXT,"
		"  severity INTEGER NOT NULL DEFAULT 0,"
		"  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
		");"
	);

	/** Helper to get all CREATE TABLE statements in order. */
	inline TArray<const TCHAR*> GetAllCreateStatements()
	{
		return {
			CreateAccountsTable,
			CreateCharactersTable,
			CreateInventoryTable,
			CreateGuildsTable,
			CreateGuildMembersTable,
			CreateStorageTable,
			CreateGuildStorageTable,
			CreateQuestsTable,
			CreateFriendsTable,
			CreateSkillsTable,
			CreateCastlesTable,
			CreateAnticheatLogsTable
		};
	}
}
