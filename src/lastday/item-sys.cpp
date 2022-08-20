/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <new>
#include <base/math.h>

#include "item-sys.h"

CItemSys::CItemSys()
{
    m_pGameServer = 0;
}

CItemSys::CItemSys(CGameContext *pGameServer)
{
    m_pGameServer = pGameServer;
}