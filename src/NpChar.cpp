#include <stdint.h>

#include <SDL_rwops.h>
#include "WindowsWrapper.h"

#include "CommonDefines.h"
#include "Tags.h"
#include "NpChar.h"
#include "Game.h"
#include "Flags.h"
#include "NpcTbl.h"

#define NPC_MAX 0x200

NPCHAR gNPC[NPC_MAX];

const char *gPassPixEve = "PXE\0";

void InitNpChar()
{
	memset(gNPC, 0, sizeof(gNPC));
}

void SetUniqueParameter(NPCHAR *npc)
{
	int code = npc->code_char;
	npc->surf = gNpcTable[code].surf;
	npc->hit_voice = gNpcTable[code].hit_voice;
	npc->destroy_voice = gNpcTable[code].destroy_voice;
	npc->damage = gNpcTable[code].damage;
	npc->size = gNpcTable[code].size;
	npc->life = gNpcTable[code].life;
	npc->hit.front = gNpcTable[code].hit.front << 9;
	npc->hit.back = gNpcTable[code].hit.back << 9;
	npc->hit.top = gNpcTable[code].hit.top << 9;
	npc->hit.bottom = gNpcTable[code].hit.bottom << 9;
	npc->view.front = gNpcTable[code].view.front << 9;
	npc->view.back = gNpcTable[code].view.back << 9;
	npc->view.top = gNpcTable[code].view.top << 9;
	npc->view.bottom = gNpcTable[code].view.bottom << 9;
}

bool LoadEvent(char *path_event)
{
	char path[PATH_LENGTH];
	sprintf(path, "%s/%s", gDataPath, path_event);
	
	SDL_RWops *fp = SDL_RWFromFile(path, "rb");
	if (!fp)
		return false;
	
	//Read "PXE" check
	char code[4];
	fp->read(fp, code, 1, 4);
	if (memcmp(code, gPassPixEve, 3))
		return false;
	
	//Get amount of npcs
	int count = SDL_ReadLE32(fp);
	
	//Load npcs
	memset(gNPC, 0, sizeof(gNPC));
	
	int n = 170;
	for (int i = 0; i < count; i++)
	{
		//Get data from file
		EVENT eve;
		eve.x = SDL_ReadLE16(fp);
		eve.y = SDL_ReadLE16(fp);
		eve.code_flag = SDL_ReadLE16(fp);
		eve.code_event = SDL_ReadLE16(fp);
		eve.code_char = SDL_ReadLE16(fp);
		eve.bits = SDL_ReadLE16(fp);
		
		//Set NPC parameters
		if (eve.bits & npc_altDir)
			gNPC[n].direct = 2;
		else
			gNPC[n].direct = 0;
		gNPC[n].code_char = eve.code_char;
		gNPC[n].code_event = eve.code_event;
		gNPC[n].code_flag = eve.code_flag;
		gNPC[n].x = eve.x << 13;
		gNPC[n].y = eve.y << 13;
		gNPC[n].bits = eve.bits;
		gNPC[n].bits |= gNpcTable[gNPC[n].code_char].bits;
		gNPC[n].exp = gNpcTable[gNPC[n].code_char].exp;
		SetUniqueParameter(&gNPC[n]);
		
		//Check flags
		if (gNPC[n].bits & npc_appearSet)
		{
			if (GetNPCFlag(gNPC[n].code_flag))
				gNPC[n].cond |= 0x80u;
		}
		else if (gNPC[n].bits & npc_hideSet)
		{
			if (!GetNPCFlag(gNPC[n].code_flag))
				gNPC[n].cond |= 0x80u;
		}
		else
		{
			gNPC[n].cond = 0x80;
		}
		
		//Increase index
		n++;
	}
	
	return true;
}

void SetNpChar(int code_char, int x, int y, int xm, int ym, int dir, NPCHAR *npc, int start_index)
{
	for (int n = start_index; n < NPC_MAX; n++)
	{
		if (!gNPC[n].cond)
		{
			//Set NPC parameters
			memset(&gNPC[n], 0, sizeof(NPCHAR));
			gNPC[n].cond |= 0x80u;
			gNPC[n].direct = dir;
			gNPC[n].code_char = code_char;
			gNPC[n].x = x;
			gNPC[n].y = y;
			gNPC[n].xm = xm;
			gNPC[n].ym = ym;
			gNPC[n].pNpc = npc;
			gNPC[n].bits = gNpcTable[gNPC[n].code_char].bits;
			gNPC[n].exp = gNpcTable[gNPC[n].code_char].exp;
			SetUniqueParameter(&gNPC[n]);
			break;
		}
	}
}

void SetDestroyNpChar(int x, int y, int w, int num)
{
	//Create smoke
	int wa = w / 0x200;
	for (int i = 0; i < num; i++)
	{
		int offset_x = Random(-wa, wa) << 9;
		int offset_y = Random(-wa, wa) << 9;
		SetNpChar(4, x + offset_x, offset_y + y, 0, 0, 0, NULL, 0x100);
	}
	
	//Flash effect
	//SetCaret(x, y, 12, 0);
}

void SetDestroyNpCharUp(int x, int y, int w, int num)
{
	//Create smoke
	int wa = w / 0x200;
	for (int i = 0; i < num; i++)
	{
		int offset_x = Random(-wa, wa) << 9;
		int offset_y = Random(-wa, wa) << 9;
		SetNpChar(4, x + offset_x, offset_y + y, 0, 0, 1, NULL, 0x100);
	}
	
	//Flash effect
	//SetCaret(x, y, 12, 0);
}

void SetExpObjects(int x, int y, int exp)
{
	for (int n = 0x100; exp; SetUniqueParameter(&gNPC[n]))
	{
		if (!gNPC[n].cond)
		{
			memset(&gNPC[n], 0, sizeof(NPCHAR));
			
			int sub_exp = 0;
			if (exp < 20)
			{
				if (exp < 5)
				{
					if (exp > 0)
					{
						--exp;
						sub_exp = 1;
					}
				}
				else
				{
					exp -= 5;
					sub_exp = 5;
				}
			}
			else
			{
				exp -= 20;
				sub_exp = 20;
			}
			
			gNPC[n].cond |= 0x80u;
			gNPC[n].direct = 0;
			gNPC[n].code_char = 1;
			gNPC[n].x = x;
			gNPC[n].y = y;
			gNPC[n].bits = gNpcTable[gNPC[n].code_char].bits;
			gNPC[n].exp = sub_exp;
		}
	}
}
