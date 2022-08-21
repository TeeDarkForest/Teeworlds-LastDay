/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef LASTDAY_SQL_H
#define LASTDAY_SQL_H

#ifdef CONF_SQL
/* SQL Class by Sushi */

#include <engine/shared/protocol.h>

#include <mysql_connection.h>
	
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

class CSQL
{
public:
	CSQL(class CGameContext *pGameServer);


	sql::Driver *driver;
	sql::Connection *connection;
	sql::Statement *statement;
	sql::ResultSet *results;
	
	// copy of config vars
	const char* m_Database;
	const char* m_Prefix;
	const char* m_User;
	const char* m_Pass;
	const char* m_IP;
	int m_Port;
	
	bool connect();
	void disconnect();
	
	void create_tables();
	void create_account(const char* name, const char* pass, int client_id);
	void delete_account(const char* name);
	void delete_account(int client_id);
	void change_password(int client_id, const char* new_pass);

	void login(const char* name, const char* pass, int client_id);
	void update_all();
	void SaveItem(int ClientID, const char* Resource, int Num);

/*	static void change_password_thread(void *user);
	static void login_thread(void *user);
	static void update_thread(void *user);
	static void create_account_thread(void *user);*/
};

struct CSqlData
{
	CSQL *m_SqlData;
	int UserID[MAX_CLIENTS];
	char m_Name[32];
	char m_Pass[32];
	const char* m_Resource;
	int m_Num;
	struct WeaponStat
	{
		int m_AmmoRegenStart;
		int m_Ammo;
		int m_Ammocost;
		bool m_Got;

	} m_aWeapons[NUM_WEAPONS];
	int m_ClientID;
};

struct CAccountData
{
	int UserID[MAX_CLIENTS];
	
	bool m_LoggedIn[MAX_CLIENTS];
};
#endif

#endif