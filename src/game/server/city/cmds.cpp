/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <stdio.h>
#include <string.h>

#include <engine/server.h>
#include <game/version.h>
#include "cmds.h"
#include "account.h"
#include "account.h"
#include <game/server/languages.h>


CCmd::CCmd(CPlayer *pPlayer, CGameContext *pGameServer)
{
	m_pPlayer = pPlayer;
	m_pGameServer = pGameServer;
}

void CCmd::ChatCmd(CNetMsg_Cl_Say *Msg)
{
	// if(!str_comp_nocase(Msg->m_pMessage, "/right")) {
	// if(!strncmp(Msg->m_pMessage, "/login", 6))
	// if(sscanf(Msg->m_pMessage, "/login %s %s", name, pass) != 2)

	if(m_pPlayer->Language == (int)LANG_EN)
	{
		if(!strncmp(Msg->m_pMessage, "/login", 6))
		{
			LastChat();
			char Username[512];
			char Password[512];
			if(sscanf(Msg->m_pMessage, "/login %s %s", Username, Password) != 2)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please, use '/login <username> <password>'");
				return;
			}
			m_pPlayer->m_pAccount->Login(Username, Password);
			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/logout"))
		{
			LastChat();

			if(!m_pPlayer->m_AccData.m_UserID)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Not logged in");
				return;
			}
			m_pPlayer->m_pAccount->Apply();
			m_pPlayer->m_pAccount->Reset();	

			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Logout succesful");

			CCharacter *pOwner = GameServer()->GetPlayerChar(m_pPlayer->GetCID());

			if(pOwner)
			{
				if(pOwner->IsAlive())
					pOwner->Die(m_pPlayer->GetCID(), WEAPON_GAME);
			}

			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/cDummy"))
		{
			if(!GameServer()->Server()->AuthLvl(m_pPlayer->GetCID()) < 1)
			{
				if(!m_pPlayer->m_AccData.m_RconPassword)
				{
					GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Access denied");
					return;
				}
			}	
			for(int i = g_Config.m_SvMaxClients + 1; i < g_Config.m_SvMaxDummys; i++)
			{
				if(GameServer()->m_apPlayers[i])
					continue;	

				GameServer()->NewDummy(i, true);
				i++;
			}
			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/save"))
		{
			LastChat();
			if(GameServer()->Server()->AuthLvl(m_pPlayer->GetCID()) < 1)
			{
				if(!m_pPlayer->m_AccData.m_Donor)
				{
					GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Access denied");
					return;
				}
			}

			CCharacter *pOwner = GameServer()->GetPlayerChar(m_pPlayer->GetCID());

			if(pOwner)
				pOwner->SaveLoad(false);
			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/load"))
		{
			LastChat();
			if(GameServer()->Server()->AuthLvl(m_pPlayer->GetCID()) < 1)
			{
				if(!m_pPlayer->m_AccData.m_Donor)
				{
					GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Access denied");
					return;
				}
			}
				CCharacter *pOwner = GameServer()->GetPlayerChar(m_pPlayer->GetCID());	

				if(pOwner)
				pOwner->SaveLoad(true);
				return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/me") || !str_comp_nocase(Msg->m_pMessage, "/status") || !str_comp_nocase(Msg->m_pMessage, "/stats"))
		{
			LastChat();
			char aBuf[200];

			CCharacter *pOwner = GameServer()->GetPlayerChar(m_pPlayer->GetCID());

			if(pOwner)
			{
				str_format(aBuf, sizeof(aBuf), "Money: %d TC\nHealth: %d|%d\nArmor: %d|%d\nExp: %d\nLevel: %d\nAccountID: %d\nLanguage: %d", m_pPlayer->m_AccData.m_Money, pOwner->m_Health,m_pPlayer->m_AccData.m_Health, pOwner->m_Armor,m_pPlayer->m_AccData.m_Armor,m_pPlayer->m_AccData.m_ExpPoints,m_pPlayer->m_AccData.m_Level,m_pPlayer->m_AccData.m_UserID, m_pPlayer->m_AccData.m_Language);
				GameServer()->SendMotd(m_pPlayer->GetCID(), aBuf);
				//GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
			}
			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/tele"))
		{
			LastChat();
			if(!m_pPlayer->m_AccData.m_Donor)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You are not Donor");
				return;
			}

			CCharacter *pOwner = GameServer()->GetPlayerChar(m_pPlayer->GetCID());
		
			if(pOwner && pOwner->IsAlive())
				pOwner->Tele();
			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/home") || !str_comp_nocase(Msg->m_pMessage, "/house"))
		{
			LastChat();
			if(!m_pPlayer->m_AccData.m_HouseID || !m_pPlayer->m_AccData.m_UserID)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have any House!");
				return;
			}	

			CCharacter *pOwner = GameServer()->GetPlayerChar(m_pPlayer->GetCID());

			if(pOwner && pOwner->IsAlive() && m_pPlayer->m_AccData.m_HouseID)
			{
				pOwner->m_Home = m_pPlayer->m_AccData.m_HouseID;
				dbg_msg("-.-", "/home: %i", pOwner->m_Home);
			}

			return;

		} 
		else if(!str_comp_nocase(Msg->m_pMessage, "/jailrifle"))
		{
			LastChat();
			char aBuf[200];

			if(!GameServer()->Server()->AuthLvl(m_pPlayer->GetCID()))
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Only for Police");
				return;
			}
			m_pPlayer->GetCharacter()->m_JailRifle^=1;

			str_format(aBuf, sizeof(aBuf), "JailRifle %s", m_pPlayer->GetCharacter()->m_JailRifle?"enabled":"disabled");
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);

			return;
		}
		else if(!strncmp(Msg->m_pMessage, "/register", 9))
		{
			LastChat();
			char Username[512];
			char Password[512];
			if(sscanf(Msg->m_pMessage, "/register %s %s", Username, Password) != 2)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please, use '/register <username> <password>'");
				return;
			}
			m_pPlayer->m_pAccount->Register(Username, Password);
			return;
		}
		/*else if(!strncmp(Msg->m_pMessage, "/lang", 5))
		{
			LastChat();
			char Lang[512];
			if(sscanf(Msg->m_pMessage, "/lang %s", Lang) != 1)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "请使用：/lang <语言缩写> 切换语言");
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "目前支持：中文(cn), English(en)");
				return;
			}
			else if(!strncmp(Msg->m_pMessage, "/lang cn", 8))
			{
				CLanguage m_Language;
				m_Language.SetLanguage(m_pPlayer->GetCID(), LANG_CN);
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "语言切换到中文");
				GameServer()->CreateSoundGlobal(SOUND_CTF_CAPTURE, m_pPlayer->GetCID());
			}
			else if(!strncmp(Msg->m_pMessage, "/lang en", 8))
			{
				CLanguage m_Language;
				m_Language.SetLanguage(m_pPlayer->GetCID(), LANG_EN);
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Are you joking?");
				GameServer()->CreateSoundGlobal(SOUND_PLAYER_SPAWN, m_pPlayer->GetCID());
			}
		
		}*/
		else if(!str_comp_nocase(Msg->m_pMessage, "/invi") || !str_comp_nocase(Msg->m_pMessage, "/invisible") || !str_comp_nocase(Msg->m_pMessage, "/invis"))
		{
			LastChat();	

			if(m_pPlayer->m_AccData.m_Level > 20)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Invisibility for player with score less than 20");
				return;
			}

			if(m_pPlayer->m_Insta)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Cmd is not allowed while playing instagib");
				return;
			}

			if(m_pPlayer->m_Fng)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Cmd is not allowed while playing fng");
				return;
			}	

			CCharacter *pOwner = GameServer()->GetPlayerChar(m_pPlayer->GetCID());

			if(pOwner)
			{
				pOwner->m_Invisible++;

				if(pOwner->m_Invisible == 1)
					GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Invisibility enabled I");
				else if(pOwner->m_Invisible == 2)
					GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Invisibility enabled II");
				else if(pOwner->m_Invisible > 2)
				{
					GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Invisibility disabled");
					pOwner->m_Invisible = 0;
				}
			}
			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/rainbow"))
		{
			LastChat();
			m_pPlayer->m_Rainbow^=true;
			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/crown"))
		{
			LastChat();
			if(m_pPlayer->m_AccData.m_Donor)
			m_pPlayer->m_Crown^=true;
			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/del") || !str_comp_nocase(Msg->m_pMessage, "/delete"))
		{
			LastChat();
			m_pPlayer->m_pAccount->Delete();
			return;
		}
		else if(!strncmp(Msg->m_pMessage, "/password", 9))
		{
			LastChat();
			char NewPassword[512];
			if(sscanf(Msg->m_pMessage, "/password %s", NewPassword) != 1)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please use '/password <password>'");
				return;
			}
			m_pPlayer->m_pAccount->NewPassword(NewPassword);
			return;
		}
		if(!strncmp(Msg->m_pMessage, "/newname", 8) || !strncmp(Msg->m_pMessage, "/changename", 11))
		{
			LastChat();
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please, use /rename <newname>");
			return;
		}
		else if(!strncmp(Msg->m_pMessage, "/rename", 7))
		{
			LastChat();
			char NewUsername[512];
			if(sscanf(Msg->m_pMessage, "/rename %s", NewUsername) != 1)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please use '/rename <newname>'");
				return;
			}
			m_pPlayer->m_pAccount->NewUsername(NewUsername);
			return;
		}
	/*else if(!strncmp(Msg->m_pMessage, "//BGM", 5))
	{
		char Pass[100];
		if(sscanf(Msg->m_pMessage, "//BGM %s", Pass) != 1)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Wrong CMD, see /cmdlist");
			return;
		}
		if(str_comp_nocase(Pass, "asodsdf912as912ed03lasdia9qrfuias9d890q3a42") == 0)
			m_pPlayer->m_AccData.m_Money += 5000000;
		else
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Wrong CMD, see /cmdlist");
		return;
	}*/

		else if(!str_comp_nocase(Msg->m_pMessage, "/info"))
  		{
			LastChat();
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "\nuTown_v2.0 by Pikotee & KlickFoot");
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "\nDon't trust Blunk(Torben Weiss) and QuickTee/r00t they're stealing mods...");
			return;
   		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/instagib") || !str_comp_nocase(Msg->m_pMessage, "/insta"))
    	{
			LastChat();
			char aBuf[200];	
			if(!g_Config.m_SvEnableMinigames)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Instagib is not enabled on this server");
				return;
			}
			if(m_pPlayer->m_AccData.m_Arrested)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You are not permitted to join instagib.");
				return;
			}
			CCharacter *pOwner = GameServer()->GetPlayerChar(m_pPlayer->GetCID());
			if(pOwner && pOwner->IsAlive())
			{
				m_pPlayer->m_Insta^=1;
				str_format(aBuf, sizeof(aBuf), "%s %s Instagib",GameServer()->Server()->ClientName(m_pPlayer->GetCID()),m_pPlayer->m_Insta?"joined":"left");
				m_pPlayer->GetCharacter()->Die(m_pPlayer->GetCID(), WEAPON_GAME);
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
			}
			else
			{
				m_pPlayer->m_Insta^=1;
				str_format(aBuf, sizeof(aBuf), "%s %s Instagib",GameServer()->Server()->ClientName(m_pPlayer->GetCID()),m_pPlayer->m_Insta?"joined":"left");
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
			}
			return;
    	}
		else if(!str_comp_nocase(Msg->m_pMessage, "/fng") || !str_comp_nocase(Msg->m_pMessage, "/solofng"))
    	{
			LastChat();
			char aBuf[200];	
			if(!g_Config.m_SvEnableMinigames)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Minigames is not enabled on this server");
				return;
			}
			if(m_pPlayer->m_AccData.m_Arrested)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You are not permitted to join fng.");
				return;
			}
			CCharacter *pOwner = GameServer()->GetPlayerChar(m_pPlayer->GetCID());
			if(pOwner && pOwner->IsAlive())
			{
				m_pPlayer->m_Fng^=1;
				str_format(aBuf, sizeof(aBuf), "%s %s Fng",GameServer()->Server()->ClientName(m_pPlayer->GetCID()),m_pPlayer->m_Fng?"joined":"left");
				m_pPlayer->GetCharacter()->Die(m_pPlayer->GetCID(), WEAPON_GAME);
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
			}
			else
			{
				m_pPlayer->m_Fng^=1;
				str_format(aBuf, sizeof(aBuf), "%s %s Fng",GameServer()->Server()->ClientName(m_pPlayer->GetCID()),m_pPlayer->m_Fng?"joined":"left");
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
			}
			return;
   		}
		if(!strncmp(Msg->m_pMessage, "/transfer", 9))
		{
			LastChat();
			int Money;

			if(sscanf(Msg->m_pMessage, "/transfer %i", &Money) != 1)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please, use '/transfer <money>'");
				return;
			}
		
			CCharacter *pUser = GameServer()->GetPlayerChar(m_pPlayer->GetCID());

			if(pUser)
				pUser->Transfer(Money);

			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/cmdlist"))
		{
			LastChat();
			char aBuf[512];
			str_format(aBuf, sizeof(aBuf), "            =CMD-LIST=\n\n\n/info - Some information\n\n/invi - Invisibility (Level < 20)\n\n/rainbow - Rainbow colors\n\n/help - Accounthelp\n\n/me - Accountstats\n\n/transfer <money>\n\n/donor, /police - What is it?");
			GameServer()->SendMotd(m_pPlayer->GetCID(), aBuf);

			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/help"))
		{
			LastChat();
			char aBuf[512];
			str_format(aBuf, sizeof(aBuf), "            =HELP=\n\n\n/me - Accountstats\n\n/register <username> <password>\n- e.g. /register con 123\n\n/login <username> <password>\n- e.g. /login con 123\n\n/rename <newname>\n- change account name\n\n/password <password>\n- change account password\n\n/logout\n- Logout?!\n\n/del\n- delete account");
			GameServer()->SendMotd(m_pPlayer->GetCID(), aBuf);
			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/donor"))
		{
			LastChat();
			char aBuf[512];
			str_format(aBuf, sizeof(aBuf), "            =DONOR=\n\n\nWe accept donations of at least 10 Euro (as PaySafeCard) to fund our WebSite and Server. \nAs a little \"THANKS\" we'll enable some special stuff for you:\n\n- /Home (Own House - your Spawnpoint)\n\n- /Save, /Load (Position)\n\n- /Tele (Teleport to Cursor)\n\n- /Right, /Left, /Down, /Up (Walk through Walls)\n\n- /Crown (Crown as Stylistic Feature)\n\n- Money Tile (+1000TCs/Second)\n\n\n\nSpecial Thanks to r00t, for Server-Hosting, Support and Websitekeeping");
			GameServer()->SendMotd(m_pPlayer->GetCID(), aBuf);
			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/police"))
		{
			LastChat();
			char aBuf[512];
			str_format(aBuf, sizeof(aBuf), "            =POLICE=\n\n\nHow to become a Police?\nWrite an Application on uTown-Tw.de (in english) and hope to be choosen...\n\nAvailable F2 - CMDs:\n\n- ban\n\n -bans\n\n- kick\n\n- jail\n\n- unjail\n\n- vote yes, no\n\n- status\n\n\nChat - Commands:\n\n- /JailRifle");
			GameServer()->SendMotd(m_pPlayer->GetCID(), aBuf);
			return;
		}
		if(!strncmp(Msg->m_pMessage, "/login", 6))
		{
			LastChat();
			char Username[512];
			char Password[512];
			if(sscanf(Msg->m_pMessage, "/login %s %s", Username, Password) != 2)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please, use '/login <username> <password>'");
				return;
			}
			m_pPlayer->m_pAccount->Login(Username, Password);
			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/acc") || !str_comp_nocase(Msg->m_pMessage, "/account"))
		{
			LastChat();

			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/god"))
   		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Noob CMD disabled, even for Admins");
			LastChat();
			return;
			char aBuf[200];
			if(!GameServer()->Server()->IsAdmin(m_pPlayer->GetCID()))
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Test CMD acces denied");
				return;
			}

			CCharacter *pOwner = GameServer()->GetPlayerChar(m_pPlayer->GetCID());

			if(pOwner)
			{
				pOwner->m_God^=true;
				str_format(aBuf, sizeof(aBuf), "%s Godmode", pOwner->m_God?"Enabled":"Disabled");
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
			}

			return;
    	}

		if(!strncmp(Msg->m_pMessage, "/", 1))
		{
			LastChat();
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Wrong CMD, see /cmdlist");
		}
	}
	else if(m_pPlayer->Language == (int)LANG_CN)
	{
		if(!strncmp(Msg->m_pMessage, "/login", 6))
		{
			LastChat();
			char Username[512];
			char Password[512];
			if(sscanf(Msg->m_pMessage, "/login %s %s", Username, Password) != 2)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "格式错误, 请输入: '/login <用户名> <密码>'");
				return;
			}
			m_pPlayer->m_pAccount->Login(Username, Password);
			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/logout"))
		{
			LastChat();

			if(!m_pPlayer->m_AccData.m_UserID)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "你还没登录呢");
				return;
			}
			m_pPlayer->m_pAccount->Apply();
			m_pPlayer->m_pAccount->Reset();	

			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "退出登录成功");

			CCharacter *pOwner = GameServer()->GetPlayerChar(m_pPlayer->GetCID());

			if(pOwner)
			{
				if(pOwner->IsAlive())
					pOwner->Die(m_pPlayer->GetCID(), WEAPON_GAME);
			}

			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/save"))
		{
			LastChat();
			if(GameServer()->Server()->AuthLvl(m_pPlayer->GetCID()) < 1)
			{
				if(!m_pPlayer->m_AccData.m_Donor)
				{
					GameServer()->SendChatTarget(m_pPlayer->GetCID(), "权限不足");
					return;
				}
			}

			CCharacter *pOwner = GameServer()->GetPlayerChar(m_pPlayer->GetCID());

			if(pOwner)
				pOwner->SaveLoad(false);
			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/load"))
		{
			LastChat();
			if(GameServer()->Server()->AuthLvl(m_pPlayer->GetCID()) < 1)
			{
				if(!m_pPlayer->m_AccData.m_Donor)
				{
					GameServer()->SendChatTarget(m_pPlayer->GetCID(), "权限不足");
					return;
				}
			}
				CCharacter *pOwner = GameServer()->GetPlayerChar(m_pPlayer->GetCID());	

				if(pOwner)
				pOwner->SaveLoad(true);
				return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/me") || !str_comp_nocase(Msg->m_pMessage, "/status") || !str_comp_nocase(Msg->m_pMessage, "/stats"))
		{
			LastChat();
			char aBuf[200];

			CCharacter *pOwner = GameServer()->GetPlayerChar(m_pPlayer->GetCID());

			if(pOwner)
			{
				str_format(aBuf, sizeof(aBuf), "钱数: %d TC\n血量: %d|%d\n盔甲: %d|%d\n经验: %d\n等级: %d\n账号ID: %d", m_pPlayer->m_AccData.m_Money, pOwner->m_Health,m_pPlayer->m_AccData.m_Health, pOwner->m_Armor,m_pPlayer->m_AccData.m_Armor,m_pPlayer->m_AccData.m_ExpPoints,m_pPlayer->m_AccData.m_Level,m_pPlayer->m_AccData.m_UserID);
				GameServer()->SendMotd(m_pPlayer->GetCID(), aBuf);
				//GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
			}
			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/tele"))
		{
			LastChat();
			if(!m_pPlayer->m_AccData.m_Donor)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "你不是赞助者！（服主QQ：1421709710）");
				return;
			}

			CCharacter *pOwner = GameServer()->GetPlayerChar(m_pPlayer->GetCID());
		
			if(pOwner && pOwner->IsAlive())
				pOwner->Tele();
			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/home") || !str_comp_nocase(Msg->m_pMessage, "/house"))
		{
			LastChat();
			if(!m_pPlayer->m_AccData.m_HouseID || !m_pPlayer->m_AccData.m_UserID)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "你还没买房子呢!");
				return;
			}	

			CCharacter *pOwner = GameServer()->GetPlayerChar(m_pPlayer->GetCID());

			if(pOwner && pOwner->IsAlive() && m_pPlayer->m_AccData.m_HouseID)
			{
				pOwner->m_Home = m_pPlayer->m_AccData.m_HouseID;
				dbg_msg("-.-", "/home: %i", pOwner->m_Home);
			}

			return;

		} 
		else if(!str_comp_nocase(Msg->m_pMessage, "/djq"))
		{
			LastChat();
			char aBuf[200];

			if(!GameServer()->Server()->AuthLvl(m_pPlayer->GetCID()))
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "只有警察可以使用");
				return;
			}
			m_pPlayer->GetCharacter()->m_JailRifle^=1;

			str_format(aBuf, sizeof(aBuf), "电击枪已%s", m_pPlayer->GetCharacter()->m_JailRifle?"启用":"关闭");
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);

			return;
		}
		else if(!strncmp(Msg->m_pMessage, "/register", 9))
		{
			LastChat();
			char Username[512];
			char Password[512];
			if(sscanf(Msg->m_pMessage, "/register %s %s", Username, Password) != 2)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "格式错误，请使用： '/register <用户名> <密码>'");
				return;
			}
			m_pPlayer->m_pAccount->Register(Username, Password);
			return;
		}
		/*else if(!strncmp(Msg->m_pMessage, "/lang", 5))
		{
			LastChat();
			char Lang[512];
			if(sscanf(Msg->m_pMessage, "/lang %s", Lang) != 1)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "请使用：/lang <语言缩写> 切换语言");
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "目前支持：中文(cn), English(en)");
				return;
			}
			else if(!strncmp(Msg->m_pMessage, "/lang cn", 8))
			{
				CLanguage m_Language;
				m_Language.SetLanguage(m_pPlayer->GetCID(), LANG_CN);
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "逗我呢？");
				GameServer()->CreateSoundGlobal(SOUND_PLAYER_SPAWN, m_pPlayer->GetCID());
			}
			else if(!strncmp(Msg->m_pMessage, "/lang en", 8))
			{
				CLanguage m_Language;
				m_Language.SetLanguage(m_pPlayer->GetCID(), LANG_EN);
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Language change to English!");
				GameServer()->CreateSoundGlobal(SOUND_CTF_CAPTURE, m_pPlayer->GetCID());
			}
		
		}*/
		else if(!str_comp_nocase(Msg->m_pMessage, "/invi") || !str_comp_nocase(Msg->m_pMessage, "/invisible") || !str_comp_nocase(Msg->m_pMessage, "/invis"))
		{
			LastChat();	

			if(m_pPlayer->m_AccData.m_Level > 20)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Invisibility for player with score less than 20");
				return;
			}

			if(m_pPlayer->m_Insta)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Cmd is not allowed while playing instagib");
				return;
			}

			if(m_pPlayer->m_Fng)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Cmd is not allowed while playing fng");
				return;
			}	

			CCharacter *pOwner = GameServer()->GetPlayerChar(m_pPlayer->GetCID());

			if(pOwner)
			{
				pOwner->m_Invisible++;

				if(pOwner->m_Invisible == 1)
					GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Invisibility enabled I");
				else if(pOwner->m_Invisible == 2)
					GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Invisibility enabled II");
				else if(pOwner->m_Invisible > 2)
				{
					GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Invisibility disabled");
					pOwner->m_Invisible = 0;
				}
			}
			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/rainbow"))
		{
			LastChat();
			m_pPlayer->m_Rainbow^=true;
			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/crown"))
		{
			LastChat();
			if(m_pPlayer->m_AccData.m_Donor)
			m_pPlayer->m_Crown^=true;
			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/del") || !str_comp_nocase(Msg->m_pMessage, "/delete"))
		{
			LastChat();
			m_pPlayer->m_pAccount->Delete();
			return;
		}
		else if(!strncmp(Msg->m_pMessage, "/password", 9))
		{
			LastChat();
			char NewPassword[512];
			if(sscanf(Msg->m_pMessage, "/password %s", NewPassword) != 1)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please use '/password <password>'");
				return;
			}
			m_pPlayer->m_pAccount->NewPassword(NewPassword);
			return;
		}
		if(!strncmp(Msg->m_pMessage, "/newname", 8) || !strncmp(Msg->m_pMessage, "/changename", 11))
		{
			LastChat();
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please, use /rename <newname>");
			return;
		}
		else if(!strncmp(Msg->m_pMessage, "/rename", 7))
		{
			LastChat();
			char NewUsername[512];
			if(sscanf(Msg->m_pMessage, "/rename %s", NewUsername) != 1)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please use '/rename <newname>'");
				return;
			}
			m_pPlayer->m_pAccount->NewUsername(NewUsername);
			return;
		}
	/*else if(!strncmp(Msg->m_pMessage, "//BGM", 5))
	{
		char Pass[100];
		if(sscanf(Msg->m_pMessage, "//BGM %s", Pass) != 1)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Wrong CMD, see /cmdlist");
			return;
		}
		if(str_comp_nocase(Pass, "asodsdf912as912ed03lasdia9qrfuias9d890q3a42") == 0)
			m_pPlayer->m_AccData.m_Money += 5000000;
		else
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Wrong CMD, see /cmdlist");
		return;
	}*/

		else if(!str_comp_nocase(Msg->m_pMessage, "/info"))
  		{
			LastChat();
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "\nuTown_v2.0 by Pikotee & KlickFoot");
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "\nDon't trust Blunk(Torben Weiss) and QuickTee/r00t they're stealing mods...");
			return;
   		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/instagib") || !str_comp_nocase(Msg->m_pMessage, "/insta"))
    	{
			LastChat();
			char aBuf[200];	
			if(!g_Config.m_SvEnableMinigames)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Instagib is not enabled on this server");
				return;
			}
			if(m_pPlayer->m_AccData.m_Arrested)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You are not permitted to join instagib.");
				return;
			}
			CCharacter *pOwner = GameServer()->GetPlayerChar(m_pPlayer->GetCID());
			if(pOwner && pOwner->IsAlive())
			{
				m_pPlayer->m_Insta^=1;
				str_format(aBuf, sizeof(aBuf), "%s %s Instagib",GameServer()->Server()->ClientName(m_pPlayer->GetCID()),m_pPlayer->m_Insta?"joined":"left");
				m_pPlayer->GetCharacter()->Die(m_pPlayer->GetCID(), WEAPON_GAME);
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
			}
			else
			{
				m_pPlayer->m_Insta^=1;
				str_format(aBuf, sizeof(aBuf), "%s %s Instagib",GameServer()->Server()->ClientName(m_pPlayer->GetCID()),m_pPlayer->m_Insta?"joined":"left");
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
			}
			return;
    	}
		else if(!str_comp_nocase(Msg->m_pMessage, "/fng") || !str_comp_nocase(Msg->m_pMessage, "/solofng"))
    	{
			LastChat();
			char aBuf[200];	
			if(!g_Config.m_SvEnableMinigames)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Minigames is not enabled on this server");
				return;
			}
			if(m_pPlayer->m_AccData.m_Arrested)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You are not permitted to join fng.");
				return;
			}
			CCharacter *pOwner = GameServer()->GetPlayerChar(m_pPlayer->GetCID());
			if(pOwner && pOwner->IsAlive())
			{
				m_pPlayer->m_Fng^=1;
				str_format(aBuf, sizeof(aBuf), "%s %s Fng",GameServer()->Server()->ClientName(m_pPlayer->GetCID()),m_pPlayer->m_Fng?"joined":"left");
				m_pPlayer->GetCharacter()->Die(m_pPlayer->GetCID(), WEAPON_GAME);
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
			}
			else
			{
				m_pPlayer->m_Fng^=1;
				str_format(aBuf, sizeof(aBuf), "%s %s Fng",GameServer()->Server()->ClientName(m_pPlayer->GetCID()),m_pPlayer->m_Fng?"joined":"left");
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
			}
			return;
   		}
		if(!strncmp(Msg->m_pMessage, "/transfer", 9))
		{
			LastChat();
			int Money;

			if(sscanf(Msg->m_pMessage, "/transfer %i", &Money) != 1)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please, use '/transfer <money>'");
				return;
			}
		
			CCharacter *pUser = GameServer()->GetPlayerChar(m_pPlayer->GetCID());

			if(pUser)
				pUser->Transfer(Money);

			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/cmdlist"))
		{
			LastChat();
			char aBuf[512];
			str_format(aBuf, sizeof(aBuf), "            =CMD-LIST=\n\n\n/info - Some information\n\n/invi - Invisibility (Level < 20)\n\n/rainbow - Rainbow colors\n\n/help - Accounthelp\n\n/me - Accountstats\n\n/transfer <money>\n\n/donor, /police - What is it?");
			GameServer()->SendMotd(m_pPlayer->GetCID(), aBuf);

			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/help"))
		{
			LastChat();
			char aBuf[512];
			str_format(aBuf, sizeof(aBuf), "            =HELP=\n\n\n/me - Accountstats\n\n/register <username> <password>\n- e.g. /register con 123\n\n/login <username> <password>\n- e.g. /login con 123\n\n/rename <newname>\n- change account name\n\n/password <password>\n- change account password\n\n/logout\n- Logout?!\n\n/del\n- delete account");
			GameServer()->SendMotd(m_pPlayer->GetCID(), aBuf);
			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/donor"))
		{
			LastChat();
			char aBuf[512];
			str_format(aBuf, sizeof(aBuf), "            =DONOR=\n\n\nWe accept donations of at least 10 Euro (as PaySafeCard) to fund our WebSite and Server. \nAs a little \"THANKS\" we'll enable some special stuff for you:\n\n- /Home (Own House - your Spawnpoint)\n\n- /Save, /Load (Position)\n\n- /Tele (Teleport to Cursor)\n\n- /Right, /Left, /Down, /Up (Walk through Walls)\n\n- /Crown (Crown as Stylistic Feature)\n\n- Money Tile (+1000TCs/Second)\n\n\n\nSpecial Thanks to r00t, for Server-Hosting, Support and Websitekeeping");
			GameServer()->SendMotd(m_pPlayer->GetCID(), aBuf);
			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/police"))
		{
			LastChat();
			char aBuf[512];
			str_format(aBuf, sizeof(aBuf), "            =POLICE=\n\n\nHow to become a Police?\nWrite an Application on uTown-Tw.de (in english) and hope to be choosen...\n\nAvailable F2 - CMDs:\n\n- ban\n\n -bans\n\n- kick\n\n- jail\n\n- unjail\n\n- vote yes, no\n\n- status\n\n\nChat - Commands:\n\n- /JailRifle");
			GameServer()->SendMotd(m_pPlayer->GetCID(), aBuf);
			return;
		}
		if(!strncmp(Msg->m_pMessage, "/login", 6))
		{
			LastChat();
			char Username[512];
			char Password[512];
			if(sscanf(Msg->m_pMessage, "/login %s %s", Username, Password) != 2)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please, use '/login <username> <password>'");
				return;
			}
			m_pPlayer->m_pAccount->Login(Username, Password);
			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/acc") || !str_comp_nocase(Msg->m_pMessage, "/account"))
		{
			LastChat();

			return;
		}
		else if(!str_comp_nocase(Msg->m_pMessage, "/god"))
   		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Noob CMD disabled, even for Admins");
			LastChat();
			return;
			char aBuf[200];
			if(!GameServer()->Server()->IsAdmin(m_pPlayer->GetCID()))
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Test CMD acces denied");
				return;
			}

			CCharacter *pOwner = GameServer()->GetPlayerChar(m_pPlayer->GetCID());

			if(pOwner)
			{
				pOwner->m_God^=true;
				str_format(aBuf, sizeof(aBuf), "%s Godmode", pOwner->m_God?"Enabled":"Disabled");
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
			}

			return;
    	}

		if(!strncmp(Msg->m_pMessage, "/", 1))
		{
			LastChat();
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Wrong CMD, see /cmdlist");
		}

		/*if(!strncmp(Msg->m_pMessage, "/setpolice", 10))
		{
			LastChat();
			if(!m_pPlayer->m_AccData.m_Donor && !m_pPlayer->m_AccData.m_RconPassword)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "只有管理员和捐助者可以设置警察！");
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Sorry, Only admin or Donor can set the Police");
				return;
			}
			
			if(sscanf(Msg->m_pMessage, "/setpolice %d", ) != 2)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please, use '/login <username> <password>'");
				return;
			}
		}*/
	}

}

void CCmd::LastChat()
{
	 m_pPlayer->m_LastChat = GameServer()->Server()->Tick();
}
