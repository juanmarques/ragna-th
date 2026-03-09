-- Ragna-TH PostgreSQL Schema
-- Auto-generated from RODatabaseSchema.h

CREATE TABLE IF NOT EXISTS accounts (
  account_id SERIAL PRIMARY KEY,
  username VARCHAR(24) NOT NULL UNIQUE,
  password_hash VARCHAR(128) NOT NULL,
  password_salt VARCHAR(64) NOT NULL,
  email VARCHAR(128),
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  last_login_at TIMESTAMP,
  is_banned BOOLEAN DEFAULT FALSE,
  ban_reason TEXT,
  login_attempts INTEGER DEFAULT 0,
  gm_level INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS characters (
  character_id SERIAL PRIMARY KEY,
  account_id INTEGER NOT NULL,
  character_name VARCHAR(24) NOT NULL UNIQUE,
  job_class INTEGER NOT NULL DEFAULT 0,
  base_level INTEGER NOT NULL DEFAULT 1,
  job_level INTEGER NOT NULL DEFAULT 1,
  base_exp BIGINT NOT NULL DEFAULT 0,
  job_exp BIGINT NOT NULL DEFAULT 0,
  hp INTEGER NOT NULL DEFAULT 40,
  sp INTEGER NOT NULL DEFAULT 10,
  str INTEGER NOT NULL DEFAULT 1,
  agi INTEGER NOT NULL DEFAULT 1,
  vit INTEGER NOT NULL DEFAULT 1,
  int_stat INTEGER NOT NULL DEFAULT 1,
  dex INTEGER NOT NULL DEFAULT 1,
  luk INTEGER NOT NULL DEFAULT 1,
  status_points INTEGER NOT NULL DEFAULT 48,
  skill_points INTEGER NOT NULL DEFAULT 0,
  zeny BIGINT NOT NULL DEFAULT 0,
  saved_map VARCHAR(32) NOT NULL DEFAULT 'prontera',
  saved_x FLOAT NOT NULL DEFAULT 0,
  saved_y FLOAT NOT NULL DEFAULT 0,
  saved_z FLOAT NOT NULL DEFAULT 0,
  spawn_map VARCHAR(32) NOT NULL DEFAULT 'prontera',
  guild_id INTEGER DEFAULT 0,
  party_id INTEGER DEFAULT 0,
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  last_save_at TIMESTAMP,
  FOREIGN KEY (account_id) REFERENCES accounts(account_id)
);

CREATE TABLE IF NOT EXISTS inventory (
  inventory_id SERIAL PRIMARY KEY,
  character_id INTEGER NOT NULL,
  item_id INTEGER NOT NULL,
  amount INTEGER NOT NULL DEFAULT 1,
  refine_level INTEGER NOT NULL DEFAULT 0,
  card_slot_0 INTEGER DEFAULT 0,
  card_slot_1 INTEGER DEFAULT 0,
  card_slot_2 INTEGER DEFAULT 0,
  card_slot_3 INTEGER DEFAULT 0,
  unique_id VARCHAR(64) NOT NULL,
  equip_slot INTEGER DEFAULT -1,
  FOREIGN KEY (character_id) REFERENCES characters(character_id)
);

CREATE TABLE IF NOT EXISTS guilds (
  guild_id SERIAL PRIMARY KEY,
  guild_name VARCHAR(24) NOT NULL UNIQUE,
  leader_character_id INTEGER NOT NULL,
  guild_level INTEGER NOT NULL DEFAULT 1,
  guild_exp BIGINT NOT NULL DEFAULT 0,
  guild_message TEXT,
  guild_emblem BYTEA,
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  FOREIGN KEY (leader_character_id) REFERENCES characters(character_id)
);

CREATE TABLE IF NOT EXISTS guild_members (
  guild_id INTEGER NOT NULL,
  character_id INTEGER NOT NULL,
  position INTEGER NOT NULL DEFAULT 0,
  exp_contribution BIGINT NOT NULL DEFAULT 0,
  joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (guild_id, character_id),
  FOREIGN KEY (guild_id) REFERENCES guilds(guild_id),
  FOREIGN KEY (character_id) REFERENCES characters(character_id)
);

CREATE TABLE IF NOT EXISTS storage (
  storage_id SERIAL PRIMARY KEY,
  account_id INTEGER NOT NULL,
  item_id INTEGER NOT NULL,
  amount INTEGER NOT NULL DEFAULT 1,
  refine_level INTEGER NOT NULL DEFAULT 0,
  card_slot_0 INTEGER DEFAULT 0,
  card_slot_1 INTEGER DEFAULT 0,
  card_slot_2 INTEGER DEFAULT 0,
  card_slot_3 INTEGER DEFAULT 0,
  unique_id VARCHAR(64) NOT NULL,
  FOREIGN KEY (account_id) REFERENCES accounts(account_id)
);

CREATE TABLE IF NOT EXISTS guild_storage (
  storage_id SERIAL PRIMARY KEY,
  guild_id INTEGER NOT NULL,
  item_id INTEGER NOT NULL,
  amount INTEGER NOT NULL DEFAULT 1,
  refine_level INTEGER NOT NULL DEFAULT 0,
  card_slot_0 INTEGER DEFAULT 0,
  card_slot_1 INTEGER DEFAULT 0,
  card_slot_2 INTEGER DEFAULT 0,
  card_slot_3 INTEGER DEFAULT 0,
  unique_id VARCHAR(64) NOT NULL,
  FOREIGN KEY (guild_id) REFERENCES guilds(guild_id)
);

CREATE TABLE IF NOT EXISTS quests (
  quest_id INTEGER NOT NULL,
  character_id INTEGER NOT NULL,
  status INTEGER NOT NULL DEFAULT 0,
  progress INTEGER NOT NULL DEFAULT 0,
  started_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  completed_at TIMESTAMP,
  PRIMARY KEY (quest_id, character_id),
  FOREIGN KEY (character_id) REFERENCES characters(character_id)
);

CREATE TABLE IF NOT EXISTS friends (
  character_id INTEGER NOT NULL,
  friend_character_id INTEGER NOT NULL,
  added_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (character_id, friend_character_id),
  FOREIGN KEY (character_id) REFERENCES characters(character_id),
  FOREIGN KEY (friend_character_id) REFERENCES characters(character_id)
);

CREATE TABLE IF NOT EXISTS skills (
  character_id INTEGER NOT NULL,
  skill_id INTEGER NOT NULL,
  skill_level INTEGER NOT NULL DEFAULT 0,
  PRIMARY KEY (character_id, skill_id),
  FOREIGN KEY (character_id) REFERENCES characters(character_id)
);

CREATE TABLE IF NOT EXISTS castles (
  castle_id INTEGER PRIMARY KEY,
  guild_id INTEGER DEFAULT 0,
  economy INTEGER DEFAULT 0,
  defense INTEGER DEFAULT 0,
  last_captured_at TIMESTAMP,
  FOREIGN KEY (guild_id) REFERENCES guilds(guild_id)
);

CREATE TABLE IF NOT EXISTS anticheat_logs (
  log_id SERIAL PRIMARY KEY,
  player_net_id VARCHAR(64),
  account_id INTEGER,
  activity_type VARCHAR(32) NOT NULL,
  details TEXT,
  severity INTEGER NOT NULL DEFAULT 0,
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Create indexes for common queries
CREATE INDEX IF NOT EXISTS idx_characters_account ON characters(account_id);
CREATE INDEX IF NOT EXISTS idx_inventory_character ON inventory(character_id);
CREATE INDEX IF NOT EXISTS idx_storage_account ON storage(account_id);
CREATE INDEX IF NOT EXISTS idx_guild_storage_guild ON guild_storage(guild_id);
CREATE INDEX IF NOT EXISTS idx_guild_members_guild ON guild_members(guild_id);
CREATE INDEX IF NOT EXISTS idx_quests_character ON quests(character_id);
CREATE INDEX IF NOT EXISTS idx_skills_character ON skills(character_id);
CREATE INDEX IF NOT EXISTS idx_anticheat_player ON anticheat_logs(player_net_id);
CREATE INDEX IF NOT EXISTS idx_anticheat_account ON anticheat_logs(account_id);
