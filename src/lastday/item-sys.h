/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef LASTDAY_ITEM_SYS_H
#define LASTDAY_ITEM_SYS_H

#include <game/server/gamecontext.h>
#include "item.h"

class CItemSys
{
    CGameContext *m_pGameServer;
public:
    CGameContext *GameServer() { return m_pGameServer; }

    CItemSys();
    CItemSys(CGameContext *GameServer);

	bool InitItem(const char *pName, ...);

};

#endif