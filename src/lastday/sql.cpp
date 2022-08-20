
#ifdef CONF_SQL
/* SQL class 0.5 by Sushi */
/* SQL class 0.6 by FFS   */
#include <game/server/gamecontext.h>
#include <engine/shared/config.h>
#include "sql.h"

static LOCK SQLLock = 0;
CGameContext *m_pGameServer;
CGameContext *GameServer() { return m_pGameServer; }
CSQL::CSQL(class CGameContext *pGameServer)
{
	if(SQLLock == 0)
		SQLLock = lock_create();

	m_pGameServer = pGameServer;
		
	// set Database info
	m_Database = g_Config.m_SvSqlDatabase;
	m_Prefix = g_Config.m_SvSqlPrefix;
	m_User = g_Config.m_SvSqlUser;
	m_Pass = g_Config.m_SvSqlPassword;
	m_IP = g_Config.m_SvSqlIp;
	m_Port = g_Config.m_SvSqlPort;
}

bool CSQL::connect()
{
	try 
	{
		// Create connection
		driver = get_driver_instance();
		char buf[256];
		str_format(buf, sizeof(buf), "tcp://%s:%d", m_IP, m_Port);
		connection = driver->connect(buf, m_User, m_Pass);
		
		// Create Statement
		statement = connection->createStatement();
		
		// Create Database if not exists
		str_format(buf, sizeof(buf), "CREATE Database IF NOT EXISTS %s", m_Database);
		statement->execute(buf);
		
		// Connect to specific Database
		connection->setSchema(m_Database);
		return true;
	} 
	catch (sql::SQLException &e)
	{
		dbg_msg("SQL", "ERROR: SQL connection failed (%s)", e.what());
		return false;
	}
}

void CSQL::disconnect()
{
	try
	{
		delete connection;
		dbg_msg("SQL", "SQL connection disconnected");
	}
	catch (sql::SQLException &e)
	{
		dbg_msg("SQL", "ERROR: No SQL connection (%s)", e.what());
	}
}

// create tables... should be done only once
void CSQL::create_tables()
{
	// create connection
	if(connect())
	{
		try
		{
			// create tables
			char buf[2048];
			str_format(buf, sizeof(buf), 
			"CREATE TABLE IF NOT EXISTS %s_Account "
			"(UserID INT AUTO_INCREMENT PRIMARY KEY, "
			"Username VARCHAR(31) NOT NULL, "
			"Password VARCHAR(32) NOT NULL);", m_Prefix);
			statement->execute(buf);

			str_format(buf, sizeof(buf), 
			"CREATE TABLE IF NOT EXISTS %s_Item "
			"(UserID INT DEFAULT 0, "
			"Metal BIGINT DEFAULT 0,"
			"Shotgun BOOLEAN DEFAULT 0, "
			"Grenade BOOLEAN DEFAULT 0, "
			"Rifle BOOLEAN DEFAULT 0,"
			"ShotgunAmmo BOOLEAN DEFAULT 0, "
			"GrenadeAmmo BOOLEAN DEFAULT 0, "
			"RifleAmmo BOOLEAN DEFAULT 0);", m_Prefix);
			statement->execute(buf);
			dbg_msg("SQL", "Tables were created successfully");

			// delete statement
			delete statement;
		}
		catch (sql::SQLException &e)
		{
			dbg_msg("SQL", "ERROR: Tables were NOT created (%s)", e.what());
		}
		
		// disconnect from Database
		disconnect();
	}	
}

// create Account
static void create_account_thread(void *user)
{
	lock_wait(SQLLock);
	
	CSqlData *Data = (CSqlData *)user;
	
	if(GameServer()->m_apPlayers[Data->m_ClientID])
	{
		// Connect to Database
		if(Data->m_SqlData->connect())
		{
			try
			{
				// check if allready exists
				char buf[512];
				str_format(buf, sizeof(buf), "SELECT * FROM %s_Account WHERE Username='%s';", Data->m_SqlData->m_Prefix, Data->m_Name);
				Data->m_SqlData->results = Data->m_SqlData->statement->executeQuery(buf);
				if(Data->m_SqlData->results->next())
				{
					// Account found
					dbg_msg("SQL", "Account '%s' already exists", Data->m_Name);
					
					GameServer()->SendChatTarget(Data->m_ClientID, _("This acoount already exists!"));
				}
				else
				{
					// create Account \o/
					str_format(buf, sizeof(buf), "INSERT INTO %s_Account(Username, Password) VALUES ('%s', '%s');", 
					Data->m_SqlData->m_Prefix, 
					Data->m_Name, Data->m_Pass);
					Data->m_SqlData->statement->execute(buf);

					str_format(buf, sizeof(buf), "SELECT * FROM %s_Account WHERE Username='%s';", Data->m_SqlData->m_Prefix, Data->m_Name);
					Data->m_SqlData->results = Data->m_SqlData->statement->executeQuery(buf);
					
					if(Data->m_SqlData->results->next())
					{
						str_format(buf, sizeof(buf), "INSERT INTO %s_Item(UserID) VALUES (%d);", 
						Data->m_SqlData->m_Prefix, 
						Data->m_SqlData->results->getInt("UserID"));					
					}

					Data->m_SqlData->statement->execute(buf);
					dbg_msg("SQL", "Account '%s' was successfully created", Data->m_Name);
					
					GameServer()->SendChatTarget(Data->m_ClientID, _("Acoount was created successfully."));
					Data->m_SqlData->login(Data->m_Name, Data->m_Pass, Data->m_ClientID);
				}
				
				// delete statement
				delete Data->m_SqlData->statement;
				delete Data->m_SqlData->results;
			}
			catch (sql::SQLException &e)
			{
				dbg_msg("SQL", "ERROR: Could not create Account (%s)", e.what());
			}
			
			// disconnect from Database
			Data->m_SqlData->disconnect();
		}
	}
	
	delete Data;
	
	lock_unlock(SQLLock);
}

void CSQL::create_account(const char* name, const char* pass, int m_ClientID)
{
	CSqlData *tmp = new CSqlData();
	str_copy(tmp->m_Name, name, sizeof(tmp->m_Name));
	str_copy(tmp->m_Pass, pass, sizeof(tmp->m_Pass));
	tmp->m_ClientID = m_ClientID;
	tmp->m_SqlData = this;
	
	void *register_thread = thread_init(create_account_thread, tmp);
#if defined(CONF_FAMILY_UNIX)
	pthread_detach((pthread_t)register_thread);
#endif
}

// change password
static void change_password_thread(void *user)
{
	lock_wait(SQLLock);
	
	CSqlData *Data = (CSqlData *)user;
	
	// Connect to Database
	if(Data->m_SqlData->connect())
	{
		try
		{
			// Connect to Database
			Data->m_SqlData->connect();
			
			// check if Account exists
			char buf[512];
			str_format(buf, sizeof(buf), "SELECT * FROM %s_Account WHERE Username='%s';", Data->m_SqlData->m_Prefix, Data->m_Name[Data->m_ClientID]);
			Data->m_SqlData->results = Data->m_SqlData->statement->executeQuery(buf);
			if(Data->m_SqlData->results->next())
			{
				// update Account data
				str_format(buf, sizeof(buf), "UPDATE %s_Account SET Password='%s' WHERE UserID=%d", Data->m_SqlData->m_Prefix, Data->m_Pass, Data->UserID[Data->m_ClientID]);
				Data->m_SqlData->statement->execute(buf);
				
				// get Account name from Database
				str_format(buf, sizeof(buf), "SELECT Username FROM %s_Account WHERE UserID=%d;", Data->m_SqlData->m_Prefix, Data->UserID[Data->m_ClientID]);
				
				// create results
				Data->m_SqlData->results = Data->m_SqlData->statement->executeQuery(buf);

				// jump to result
				Data->m_SqlData->results->next();
				
				// finally the name is there \o/
				char acc_name[32];
				str_copy(acc_name, Data->m_SqlData->results->getString("Username").c_str(), sizeof(acc_name));	
				dbg_msg("SQL", "Account '%s' changed password.", acc_name);
				
				// Success
				str_format(buf, sizeof(buf), "Successfully changed your password to '%s'.", Data->m_Pass);
				GameServer()->SendBroadcast(buf, Data->m_ClientID);
				GameServer()->SendChatTarget(Data->m_ClientID, buf);
			}
			else
				dbg_msg("SQL", "Account seems to be deleted");
			
			// delete statement and results
			delete Data->m_SqlData->statement;
			delete Data->m_SqlData->results;
		}
		catch (sql::SQLException &e)
		{
			dbg_msg("SQL", "ERROR: Could not update Account (Why: %s) (ClientID: %d, UserID: %d)", e.what(), Data->m_ClientID, Data->UserID);
		}
		
		// disconnect from Database
		Data->m_SqlData->disconnect();
	}
	
	delete Data;
	
	lock_unlock(SQLLock);
}

void CSQL::change_password(int m_ClientID, const char* new_pass)
{
	CSqlData *tmp = new CSqlData();
	tmp->m_ClientID = m_ClientID;
	tmp->UserID[m_ClientID] = GameServer()->m_apPlayers[m_ClientID]->m_AccData.m_UserID;
	str_copy(tmp->m_Pass, new_pass, sizeof(tmp->m_Pass));
	tmp->m_SqlData = this;
	
	void *change_pw_thread = thread_init(change_password_thread, tmp);
#if defined(CONF_FAMILY_UNIX)
	pthread_detach((pthread_t)change_pw_thread);
#endif
}

// login stuff
static void login_thread(void *user)
{
	lock_wait(SQLLock);
	
	CSqlData *Data = (CSqlData *)user;

	if(GameServer()->m_apPlayers[Data->m_ClientID] && !GameServer()->m_apPlayers[Data->m_ClientID]->LoggedIn)
	{
		// Connect to Database
		if(Data->m_SqlData->connect())
		{
			try
			{		
				// check if Account exists
				char buf[1024];
				str_format(buf, sizeof(buf), "SELECT * FROM %s_Account WHERE Username='%s';", Data->m_SqlData->m_Prefix, Data->m_Name);
				Data->m_SqlData->results = Data->m_SqlData->statement->executeQuery(buf);
				if(Data->m_SqlData->results->next())
				{
					// check for right pw and get data
					str_format(buf, sizeof(buf), "SELECT * "
					"FROM %s_Account WHERE Username='%s' AND Password='%s';", Data->m_SqlData->m_Prefix, Data->m_Name, Data->m_Pass);
					
					// create results
					Data->m_SqlData->results = Data->m_SqlData->statement->executeQuery(buf);
					
					// if match jump to it
					if(Data->m_SqlData->results->next())
					{
						// never use player directly!
						// finally save the result to AccountData() \o/

						// check if Account allready is logged in
						for(int i = 0; i < MAX_CLIENTS; i++)
						{
							if(!GameServer()->m_apPlayers[i])
								continue;
							
							if(GameServer()->m_apPlayers[i]->IsZomb())
								continue;

							if(GameServer()->m_apPlayers[i]->m_AccData.m_UserID == Data->m_SqlData->results->getInt("UserID"))
							{								
								GameServer()->SendChatTarget(Data->m_ClientID, _("This Account is already logged in."));
								
								// delete statement and results
								delete Data->m_SqlData->statement;
								delete Data->m_SqlData->results;
								
								// disconnect from Database
								Data->m_SqlData->disconnect();
								
								// delete Data
								delete Data;
	
								// release lock
								lock_unlock(SQLLock);
								
								return;
							}
						}

						GameServer()->m_apPlayers[Data->m_ClientID]->m_AccData.m_UserID = Data->m_SqlData->results->getInt("UserID");

						str_format(buf, sizeof(buf), "SELECT * "
						"FROM %s_Item WHERE UserID=%d;", Data->m_SqlData->m_Prefix, Data->m_SqlData->results->getInt("UserID"));

						Data->m_SqlData->results = Data->m_SqlData->statement->executeQuery(buf);

						if(Data->m_SqlData->results->next())
						{
							for(int i = RESOURCE_METAL; i < NUM_RESOURCE; i++)
							{
								GameServer()->m_apPlayers[Data->m_ClientID]->m_aResource[i].m_Num = Data->m_SqlData->results->getInt(GameServer()->GetResourceName(i));
							}
							GameServer()->m_apPlayers[Data->m_ClientID]->m_aWeapons[WEAPON_SHOTGUN].m_Got = Data->m_SqlData->results->getInt("Shotgun");
							GameServer()->m_apPlayers[Data->m_ClientID]->m_aWeapons[WEAPON_GRENADE].m_Got = Data->m_SqlData->results->getInt("Grenade");
							GameServer()->m_apPlayers[Data->m_ClientID]->m_aWeapons[WEAPON_RIFLE].m_Got = Data->m_SqlData->results->getInt("Rifle");
							GameServer()->m_apPlayers[Data->m_ClientID]->m_aWeapons[WEAPON_SHOTGUN].m_Ammo = Data->m_SqlData->results->getInt("ShotgunAmmo");
							GameServer()->m_apPlayers[Data->m_ClientID]->m_aWeapons[WEAPON_GRENADE].m_Ammo = Data->m_SqlData->results->getInt("GrenadeAmmo");
							GameServer()->m_apPlayers[Data->m_ClientID]->m_aWeapons[WEAPON_RIFLE].m_Ammo = Data->m_SqlData->results->getInt("RifleAmmo");
						
						}
						// login should be the last thing
						GameServer()->m_apPlayers[Data->m_ClientID]->LoggedIn = true;
						dbg_msg("SQL", "Account '%s' logged in sucessfully", Data->m_Name);
						
						GameServer()->SendChatTarget(Data->m_ClientID, _("You are now logged in."));
						GameServer()->SendBroadcast_VL(_("Welcome {str:Name}!"), Data->m_ClientID, "Name", Data->m_Name);
					}
					else
					{
						// wrong password
						dbg_msg("SQL", "Account '%s' is not logged in due to wrong password", Data->m_Name);
						
						GameServer()->SendChatTarget(Data->m_ClientID, _("The password you entered is wrong."));
					}
				}
				else
				{
					// no Account
					dbg_msg("SQL", "Account '%s' does not exists", Data->m_Name);
					
					GameServer()->SendChatTarget(Data->m_ClientID, _("This Account does not exists."));
					GameServer()->SendChatTarget(Data->m_ClientID, _("Please register first. (/register <user> <pass>)"));
				}
				
				// delete statement and results
				delete Data->m_SqlData->statement;
				delete Data->m_SqlData->results;
			}
			catch (sql::SQLException &e)
			{
				dbg_msg("SQL", "ERROR: Could not login Account (%s)", e.what());
			}
			
			// disconnect from Database
			Data->m_SqlData->disconnect();
		}
	}
	
	delete Data;
	
	lock_unlock(SQLLock);
}

void CSQL::login(const char* name, const char* pass, int m_ClientID)
{
	CSqlData *tmp = new CSqlData();
	str_copy(tmp->m_Name, name, sizeof(tmp->m_Name));
	str_copy(tmp->m_Pass, pass, sizeof(tmp->m_Pass));
	tmp->m_ClientID = m_ClientID;
	tmp->m_SqlData = this;
	
	void *login_account_thread = thread_init(login_thread, tmp);
#if defined(CONF_FAMILY_UNIX)
	pthread_detach((pthread_t)login_account_thread);
#endif
}

static void SaveItemThread(void *user)
{
	lock_wait(SQLLock);
	
	CSqlData *Data = (CSqlData *)user;

	// Connect to Database
	if(Data->m_SqlData->connect())
	{
		try
		{
			// check if Account exists
			char buf[1024];
			str_format(buf, sizeof(buf), "SELECT * FROM %s_Account WHERE UserID=%d;", Data->m_SqlData->m_Prefix, Data->UserID[Data->m_ClientID]);
			Data->m_SqlData->results = Data->m_SqlData->statement->executeQuery(buf);
			if(Data->m_SqlData->results->next())
			{
				// update Account data
				CPlayer *p = GameServer()->m_apPlayers[Data->m_ClientID];
				if(!p)
				{
					lock_unlock(SQLLock);
					return;
				}
				str_format(buf, sizeof(buf), "UPDATE %s_Item SET %s=%d WHERE UserID=%d;", Data->m_SqlData->m_Prefix, Data->m_Resource, Data->m_Num, Data->UserID[Data->m_ClientID]);
				Data->m_SqlData->statement->execute(buf);
			}
			else
				dbg_msg("SQL", "Account seems to be deleted");
			
			// delete statement and results
			delete Data->m_SqlData->statement;
			delete Data->m_SqlData->results;
		}
		catch (sql::SQLException &e)
		{
			dbg_msg("SQL", "ERROR: Could not update Account (Why: %s) (ClientID: %d, UserID: %d)", e.what(), Data->m_ClientID, Data->UserID[Data->m_ClientID]);
		}

		Data->m_SqlData->disconnect();
	}
	lock_unlock(SQLLock);
}
// update all
void CSQL::update_all()
{
	lock_wait(SQLLock);
	
	// Connect to Database
	if(connect())
	{
		try
		{
			char buf[512];
			char acc_name[32];
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(!GameServer()->m_apPlayers[i])
					continue;
				
				if(!GameServer()->m_apPlayers[i]->LoggedIn)
					continue;
				
				// check if Account exists
				str_format(buf, sizeof(buf), "SELECT * FROM %s_Account WHERE UserID=%d;", m_Prefix, GameServer()->m_apPlayers[i]->m_AccData.m_UserID);
				results = statement->executeQuery(buf);
				char aBuf[16];
				str_format(aBuf, sizeof(aBuf), "UPDATE %s_Item SET ", m_Prefix);
				if(results->next())
				{
					CPlayer *p = GameServer()->m_apPlayers[i];
					str_format(buf, sizeof(buf), "UPDATE %s_Item SET "
					"Shotgun=%d,Grenade=%d,Rifle=%d, "
					"ShotgunAmmo=%d,GrenadeAmmo=%d,RifleAmmo=%d "
					"WHERE UserID=%d;",
					m_Prefix,p->m_aWeapons[WEAPON_SHOTGUN].m_Got,p->m_aWeapons[WEAPON_GRENADE].m_Got,p->m_aWeapons[WEAPON_RIFLE].m_Got,
					p->m_aWeapons[WEAPON_SHOTGUN].m_Ammo,p->m_aWeapons[WEAPON_GRENADE].m_Ammo,p->m_aWeapons[WEAPON_RIFLE].m_Ammo,p->m_AccData.m_UserID);
					statement->executeQuery(buf);

					std::string buf(aBuf);
					for(int j = 0; j < NUM_RESOURCE; j++)
					{
						char buffer[256];
						str_format(buffer, sizeof(buffer), " %s=%d", GameServer()->GetResourceName(j), GameServer()->m_apPlayers[i]->m_aResource[j].m_Num);
						buf.append(buffer);
						if(j != NUM_RESOURCE-1)
						{
							buf.append(",");
						}
					}
				}
				else
					dbg_msg("SQL", "Account seems to be deleted");
				
				// delete results
				delete results;
			}
			
			// delete statement
			delete statement;
		}
		catch (sql::SQLException &e)
		{
			dbg_msg("SQL", "ERROR: Could not update Account (Why: %s)");
		}
		
		// disconnect from Database
		disconnect();
	}

	lock_unlock(SQLLock);
}

void CSQL::SaveItem(int ClientID, const char* Resource, int Num)
{
	CSqlData *tmp = new CSqlData();
	tmp->m_ClientID = ClientID;
	tmp->UserID[ClientID] = GameServer()->m_apPlayers[ClientID]->m_AccData.m_UserID;
	tmp->m_Resource = Resource;
	tmp->m_Num = Num;

	tmp->m_SqlData = this;
	
	void *save_item_thread = thread_init(SaveItemThread, tmp);
#if defined(CONF_FAMILY_UNIX)
	pthread_detach((pthread_t)save_item_thread);
#endif
}
#endif
