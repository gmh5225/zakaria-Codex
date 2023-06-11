#pragma once
#include "kRpm.h"
#include "offsets.h"
#include "Utils.h"
#pragma warning(disable:4996)
MoaRpm rpm("Apex Legends", MoaRpm::MOA_MODE::KERNEL);
typedef struct
{
	char szCN[0x100];
	char szEN[0x100];
	char szTexture[0x100];
}ITEM_NAME, * PITEM_NAME;

std::map<int, ITEM_NAME> g_ItemHash;

static int S_width = 1920, S_height = 1080;
static DWORD_PTR base = 0;
D3DMATRIX  vMatrix = D3DMATRIX();
DWORD_PTR MyLocalplayer = 0x0;
std::vector<DWORD_PTR> EntityList = std::vector<DWORD_PTR>();
std::vector<DWORD_PTR> EntityLoots = std::vector<DWORD_PTR>();

class Player {
private:

public:
	Player() {}
	Player(DWORD_PTR _ptr) {
		this->ptr = _ptr;
		unsigned long long pBoneArray = rpm.read<DWORD_PTR>(ptr + m_entity_bones);
		//DWORD_PTR latestPrimaryWeapons = rpm.read<DWORD_PTR>(ptr + 0x19ec) & 0xFFFF;
		//DWORD_PTR listPrimaryWeapons = rpm.read<DWORD_PTR>(base + m_cl_entitylist + (latestPrimaryWeapons * 0x20));
		//this->weaponNameIndex = rpm.read<int>(listPrimaryWeapons + 0x16EC);
		this->Team = rpm.read<INT>(ptr + m_entity_team_num);
		this->Health = rpm.read<INT>(ptr + m_player_health);
		this->ShieldHealth = rpm.read<INT>(ptr + m_shieldHealth);
		this->ShieldHealthMax = rpm.read<INT>(ptr + m_shieldHealthMax);
		this->HealthMax = rpm.read<INT>(ptr + m_player_max_health);
		this->origin = rpm.read<D3DXVECTOR3>(ptr + m_entity_origin); 
		this->IsDowned = rpm.read<INT>(ptr + m_player_bleedout_state);

		Head.x = rpm.read<float>(pBoneArray + 0xCC + (BoneId::head * 0x30)) + origin.x;
		Head.y = rpm.read<float>(pBoneArray + 0xDC + (BoneId::head * 0x30)) + origin.y;
		Head.z = rpm.read<float>(pBoneArray + 0xEC + (BoneId::head * 0x30)) + origin.z;

	}
	DWORD_PTR ptr = 0x0;
	INT Health = 0;
	INT HealthMax = 0;
	INT ShieldHealth = 0;
	INT ShieldHealthMax = 0;
	INT Team = 0;
	D3DXVECTOR3 origin = D3DXVECTOR3();
	D3DXVECTOR3 Head = D3DXVECTOR3();
	FLOAT Distance = 0;
	bool IsDowned = 0;
	int weaponNameIndex = 0;
	bool GetEntityBonePosition(uint32_t BoneId, D3DXVECTOR3& Out)
	{
		unsigned long long pBoneArray = rpm.read<DWORD_PTR>(ptr + m_entity_bones);

		Out.x = rpm.read<float>(pBoneArray + 0xCC + (BoneId * 0x30)) + origin.x;
		Out.y = rpm.read<float>(pBoneArray + 0xDC + (BoneId * 0x30)) + origin.y;
		Out.z = rpm.read<float>(pBoneArray + 0xEC + (BoneId * 0x30)) + origin.z;
		return true;
	}

	BOOL IsValid() {
		if ((this->origin.x != NULL) &&
			(this->Team != 0) &&
			(this->Health > NULL) &&
			(this->ptr != NULL)) return true;
		return false;
	}
};
class Loot {
private:

public:
	Loot() {}
	Loot(DWORD_PTR _ptr) {
		this->ptr = _ptr;
		this->origin = rpm.read<D3DXVECTOR3>(ptr + m_entity_origin);
		this->nameid = rpm.read<DWORD>(ptr + OFFSET_ITEM_ID);
		this->itemtpye = GetItemTpye(this->nameid);
	}
	DWORD_PTR ptr = 0x0;
	D3DXVECTOR3 origin = D3DXVECTOR3();
	DWORD nameid;
	FLOAT Distance = 0;
	int itemtpye = 0;
	int GetItemTpye(int id)
	{
		if(id == 0)
			return ITEM_deathbox;
		if (id >= 1 && id <= 121)
			return ITEM_WEAPONS;
		if (id >= 126 && id <= 130)
			return ITEM_AMMO;
		if (id >= 132 && id <= 134)
			return ITEM_HEALING;
		if (id >= 135 && id <= 136)
			return ITEM_SHIELD;
		if (id >= 137 && id <= 140)
			return ITEM_HEMMET;
		if (id >= 141 && id <= 144)
			return ITEM_BODY_SHILD;
		if (id >= 146 && id <= 149)
			return ITEM_EVO_SHILD;
		if (id >= 154 && id <= 157)
			return ITEM_BACKPACK;
		if (id >= 158 && id <= 160)
			return ITEM_GRENADES;
		if (id >= 161 && id <= 170)
			return ITEM_SCOPES;
		if (id >= 171 && id <= 205)
			return ITEM_ATTACHMENTS;

		return 0;
	}
	BOOL IsValid() {
		if ((this->origin.x != NULL) &&
			(this->ptr != NULL)) return true;
		return false;
	}
};
class Manager {
private:
public:
	Player LocalPlayer = NULL;
	std::vector<Player> Players;
	std::vector<Loot> Loots;
	Player ClosestVisibleEnemy = NULL;
	Player ClosestCrosshairEnemy = NULL;
	INT Total = 0;

	Manager(int width, int height) {
		S_width = width;
		S_height = height;

		this->LocalPlayer = Player(MyLocalplayer);
		for (size_t i = 0; i < EntityList.size(); i++) {
			Player player(EntityList[i]);
			if (player.ptr != 0);// break;
			if (player.IsValid() && player.ptr != this->LocalPlayer.ptr) {
				player.Distance = Distance(this->LocalPlayer.origin,player.origin);
				this->Players.push_back(player);
			}
		}

		for (size_t i = 0; i < EntityLoots.size(); i++) {
			Loot loot(EntityLoots[i]);
			if (loot.ptr != 0);// break;
			if (loot.IsValid() && loot.ptr != this->LocalPlayer.ptr) {
				loot.Distance = Distance(this->LocalPlayer.origin, loot.origin);
				this->Loots.push_back(loot);
			}
		}

		float closestcrosshair = FLT_MAX;
		for (size_t i = 0; i < this->Players.size(); i++) {
			if (!this->LocalPlayer.IsDowned) {
				D3DXVECTOR3 out;
				if (WorldToScreen(this->Players[i].origin, out)) {
					float crossdis = GetCrossDistance(out.x, out.y, S_width / 2, S_height / 2);
					if ((crossdis < closestcrosshair)) {
						if (this->Players[i].Team != this->LocalPlayer.Team && this->Players[i].Distance < 125) {
							if (this->Players[i].IsDowned) {
								if (settings::aimknocked)
									ClosestCrosshairEnemy = this->Players[i];
							}
							else {
								ClosestCrosshairEnemy = this->Players[i];
							}
							closestcrosshair = crossdis;
						}
					}
				}
			}
		}
		float closestdis = FLT_MAX;
		for (int i = 0; i < this->Players.size(); i++) {
				if ((this->Players[i].Distance < closestdis)) {
					if (this->Players[i].Team != this->LocalPlayer.Team) {
						ClosestVisibleEnemy = this->Players[i];
						closestdis = this->Players[i].Distance;
					}
				}
		}

		for (int i = 0; i < this->Players.size(); i++) {
			Total = this->Players.size();

		}
	}

	bool WorldToScreen(D3DXVECTOR3& vIn, D3DXVECTOR3& vOut)
	{

		vOut.x = vMatrix.m[0][0] * vIn.x + vMatrix.m[0][1] * vIn.y + vMatrix.m[0][2] * vIn.z + vMatrix.m[0][3];
		vOut.y = vMatrix.m[1][0] * vIn.x + vMatrix.m[1][1] * vIn.y + vMatrix.m[1][2] * vIn.z + vMatrix.m[1][3];

		float w = vMatrix.m[3][0] * vIn.x + vMatrix.m[3][1] * vIn.y + vMatrix.m[3][2] * vIn.z + vMatrix.m[3][3];

		if (w < 0.65f)
			return false;

		float invw = 1.0f / w;

		vOut.x *= invw;
		vOut.y *= invw;

		float x = S_width / 2;
		float y = S_height / 2;

		x += 0.5 * vOut.x * S_width + 0.5;
		y -= 0.5 * vOut.y * S_height + 0.5;

		vOut.x = x;
		vOut.y = y;

		return true;
	}

	D3DXVECTOR3 CalcSoldierFuturePos(D3DXVECTOR3 InVec)
	{
		D3DXVECTOR3 NewPos, OutPos;
		if (WorldToScreen(InVec, NewPos)) {
			OutPos.x = NewPos.x;
			OutPos.y = NewPos.y;
			OutPos.z = NewPos.z;
		}
		else
		{
			OutPos.x = 0;
			OutPos.y = 0;
			OutPos.z = 0;
		}
		return OutPos;
	}

};

char* GetHelmetType(Player pPlayer)
{
	int HelmetId = 0;
	switch (HelmetId)
	{
		case (int)(HelmetID::HELMET_LV0) :
			return "Helmet LV0";
			break;
			case (int)(HelmetID::HELMET_LV1) :
				return "Helmet LV1";
				break;
				case (int)(HelmetID::HELMET_LV2) :
					return "Helmet LV2";
					break;
					case (int)(HelmetID::HELMET_LV3) :
						return "Helmet LV3";
						break;
						case (int)(HelmetID::HELMET_LV4) :
							return "Helmet LV4";
							break;
	}
	return "NULL";
}
char* GetArmorType(Player pPlayer)
{
	int ArmorId = 0;
	switch (ArmorId)
	{
		case (int)(ArmorID::BODY_ARMOR_LV0) :
			return "Armor LV0";
			break;
			case (int)(ArmorID::BODY_ARMOR_LV1) :
				return "Armor LV1";
				break;
				case (int)(ArmorID::BODY_ARMOR_LV2) :
					return "Armor LV2";
					break;
					case (int)(ArmorID::BODY_ARMOR_LV3) :
						return "Armor LV3";
						break;
						case (int)(ArmorID::BODY_ARMOR_LV4) :
							return "Armor LV4";
							break;
	}
	return "NULL";
}

char* GetGunName(Player pPlayer)
{
	int WeaponId = pPlayer.weaponNameIndex;
	switch (WeaponId)
	{
		case (int)(WeaponID::BARE_HANDS) :
			return "HANDS";
			break;
		case (int)WeaponID::HAVOC:
			return "HAVOC";
			break;
		case (int)WeaponID::LSTAR:
			return "LSTAR";
			break;
		case (int)WeaponID::KRABER:
			return "KRABER";
			break;
		case (int)WeaponID::MASTIFF:
			return "MASTIFF";
			break;
		case (int)WeaponID::DEVOTION:
			return "DEVOTION";
			break;
		case (int)WeaponID::SENTINEL:
			return "SENTINEL";
			break;
		case (int)WeaponID::CHARGE_RIFLE:
			return "CHARGE_RIFLE";
			break;
		case (int)WeaponID::LONGBOW:
			return "LONGBOW";
			break;
		case (int)WeaponID::TRIPLE_TAKE:
			return "TRIPLE_TAKE";
			break;
		case (int)WeaponID::WINGMAN:
			return "WINGMAN";
			break;
		case (int)WeaponID::SPITFIRE:
			return "SPITFIRE";
			break;
		case (int)WeaponID::PROWLER:
			return "PROWLER";
			break;
		case (int)WeaponID::HEMLOK:
			return "HEMLOK";
			break;
		case (int)WeaponID::FLATLINE:
			return "FLATLINE";
			break;
		case (int)WeaponID::RE45:
			return "RE45";
			break;
		case (int)WeaponID::P2020:
			return "P2020";
			break;
		case (int)WeaponID::R301:
			return "R301";
			break;
		case (int)WeaponID::R99:
			return "R99";
			break;
		case (int)WeaponID::ALTERNATOR:
			return "ALTERNATOR";
			break;
		case (int)WeaponID::G7_SCOUT:
			return "G7_SCOUT";
			break;
		case (int)WeaponID::MOZAMBIQUE:
			return "MOZAMBIQUE";
			break;
		case (int)WeaponID::PEACEKEEPER:
			return "PEACEKEEPER";
			break;
		case (int)WeaponID::EVA8_AUTO:
			return "EVA8_AUTO";
			break;

	}
	return "NULL";
}
char* GetGunType(Player pPlayer)
{
	int WeaponId = pPlayer.weaponNameIndex;
	switch (WeaponId)
	{
	case (int)WeaponID::BARE_HANDS:
		return "HANDS";
		break;
	case (int)WeaponID::KRABER:
	case (int)WeaponID::SENTINEL:
	case (int)WeaponID::LONGBOW:
	case (int)WeaponID::TRIPLE_TAKE:
	case (int)WeaponID::CHARGE_RIFLE:
		return "SNIPER";
		break;
	case (int)WeaponID::HEMLOK:
	case (int)WeaponID::FLATLINE:
	case (int)WeaponID::G7_SCOUT:
	case (int)WeaponID::R301:
	case (int)WeaponID::HAVOC:
		return "AR";
		break;
	case (int)WeaponID::DEVOTION:
	case (int)WeaponID::SPITFIRE:
	case (int)WeaponID::LSTAR:
		return "LMG";
		break;
	case (int)WeaponID::PROWLER:
	case (int)WeaponID::R99:
	case (int)WeaponID::ALTERNATOR:
		return "SMG";
		break;
	case (int)WeaponID::MASTIFF:
	case (int)WeaponID::EVA8_AUTO:
	case (int)WeaponID::PEACEKEEPER:
	case (int)WeaponID::MOZAMBIQUE:
		return "SHOTGUN";
		break;
	case (int)WeaponID::WINGMAN:
	case (int)WeaponID::RE45:
	case (int)WeaponID::P2020:
		return "PISTOL";
		break;

	}
	return "NULL";
}
bool IsPistol(Player pPlayer)
{
	int WeaponId = pPlayer.weaponNameIndex;
	if (WeaponId == NULL) return false;

	if (WeaponId == (int)WeaponID::RE45
		|| WeaponId == (int)WeaponID::P2020
		|| WeaponId == (int)WeaponID::WINGMAN)
		return true;
	else
		return false;
}
bool IsSniper(Player pPlayer)
{
	int WeaponId = pPlayer.weaponNameIndex;
	if (WeaponId == NULL) return false;

	if (WeaponId == (int)WeaponID::KRABER
		|| WeaponId == (int)WeaponID::SENTINEL
		|| WeaponId == (int)WeaponID::LONGBOW
		|| WeaponId == (int)WeaponID::TRIPLE_TAKE
		|| WeaponId == (int)WeaponID::CHARGE_RIFLE)
		return true;
	else
		return false;
}

bool IsLMG(Player pPlayer)
{
	int WeaponId = pPlayer.weaponNameIndex;
	if (WeaponId == NULL) return false;

	if (WeaponId == (int)WeaponID::DEVOTION || WeaponId == (int)WeaponID::SPITFIRE || WeaponId == (int)WeaponID::LSTAR)
		return true;
	else
		return false;
}
bool IsSmg(Player pPlayer)
{
	int WeaponId = pPlayer.weaponNameIndex;
	if (WeaponId == NULL) return false;

	if (WeaponId == (int)WeaponID::PROWLER
		|| WeaponId == (int)WeaponID::R99
		|| WeaponId == (int)WeaponID::ALTERNATOR)
		return true;
	else
		return false;
}
bool IsShotgun(Player pPlayer)
{
	int WeaponId = pPlayer.weaponNameIndex;
	if (WeaponId == NULL) return false;

	if (WeaponId == (int)WeaponID::MASTIFF
		|| WeaponId == (int)WeaponID::PEACEKEEPER
		|| WeaponId == (int)WeaponID::EVA8_AUTO
		|| WeaponId == (int)WeaponID::MOZAMBIQUE)
		return true;
	else
		return false;
}
bool IsAR(Player pPlayer)
{
	int WeaponId = pPlayer.weaponNameIndex;
	if (WeaponId == NULL) return false;

	if (WeaponId == (int)WeaponID::FLATLINE
		|| WeaponId == (int)WeaponID::G7_SCOUT
		|| WeaponId == (int)WeaponID::R301
		|| WeaponId == (int)WeaponID::HAVOC
		|| WeaponId == (int)WeaponID::HEMLOK)
		return true;
	else
		return false;
}
void AddItemName(int id, char* pCN, char* pEN, char* pTexture)
{
	ITEM_NAME in;
	lstrcpyA(in.szCN, pCN);
	lstrcpyA(in.szEN, pEN);
	lstrcpyA(in.szTexture, pTexture);
	g_ItemHash[id] = in;
}

void InitItemName()
{
	//SEProtectStartMutation();
	g_ItemHash.clear();
	AddItemName((int)ItemID::DeathBox, "���ױ����ѻ�ǹ", "DeathBox", "��");		//1
	//Weapons
	AddItemName((int)ItemID::KRABER_HEIRLOOM, "���ױ����ѻ�ǹ", "Kraber", "��");		//1
	AddItemName((int)ItemID::MASTIFF, "��Ȯ����ǹ", "Mastiff", "��");			//1
	AddItemName((int)ItemID::MASTIFF_GOLD, "��Ȯ����ǹ", "Mastiff [GOLD]", "��");			//1
	AddItemName((int)ItemID::LSTAR, "L-Star EMG", "L-Star", "A");
	AddItemName((int)ItemID::LSTAR_GOLD, "L-Star EMG", "L-Star [GOLD]", "A");
	AddItemName((int)ItemID::HAVOC, "���ֿ�", "Havoc", "");
	AddItemName((int)ItemID::HAVOC_GOLD, "���ֿ�", "Havoc Rifle [GOLD]", "");
	AddItemName((int)ItemID::DEVOTION, "רע���ǹ", "Devotion", "��");
	AddItemName((int)ItemID::DEVOTION_GOLD, "רע���ǹ", "Devotion [GOLD]", "��");
	AddItemName((int)ItemID::TRIPLE_TAKE, "����ʽ�ѻ�ǹ", "Triple Take", "��");
	AddItemName((int)ItemID::TRIPLE_TAKE_GOLD, "�ƽ�����ʽ�ѻ�ǹ", "Triple Take [GOLD]", "��");
	AddItemName((int)ItemID::FLATLINE, "VK-47ƽ�в�ǹ", "Flatline", "");
	AddItemName((int)ItemID::FLATLINE_GOLD, "�ƽ�VK-47ƽ�в�ǹ", "Flatline [GOLD]", "");
	AddItemName((int)ItemID::HEMLOK, "��ķ���ͻ����ǹ", "Hemlok", "��");		//1
	AddItemName((int)ItemID::HEMLOK_GOLD, "��ķ���ͻ����ǹ", "Hemlok [GOLD]", "��");		//1
	AddItemName((int)ItemID::G7_SCOUT, "G7���ǹ", "G7 Scout", "��");
	AddItemName((int)ItemID::G7_SCOUT_GOLD, "�ƽ�G7���ǹ", "G7 Scout [GOLD]", "��");
	AddItemName((int)ItemID::ALTERNATOR, "ת���߳��ǹ", "Alternator", "��");
	AddItemName((int)ItemID::ALTERNATOR_GOLD, "�ƽ�ת���߳��ǹ", "Alternator [GOLD]", "��");
	AddItemName((int)ItemID::R99, "R-99���ǹ", "R-99", "��");
	AddItemName((int)ItemID::R99_GOLD, "R-99���ǹ", "R-99 [GOLD]", "��");
	AddItemName((int)ItemID::PROWLER_HEIRLOOM, "ǱϮ���ǹ", "Prowler", "˪");
	AddItemName((int)ItemID::LONGBOW, "������ȷ��ǹ", "Longbow", "��");
	AddItemName((int)ItemID::LONGBOW_GOLD, "�ƽ𳤹���ȷ��ǹ", "Longbow [GOLD]", "��");
	AddItemName((int)ItemID::CHARGE_RIFLE, "���ܲ�ǹ", "Charge", "");
	AddItemName((int)ItemID::CHARGE_RIFLE_GOLD, "�ƽ���ܲ�ǹ", "Charge [GOLD]", "");
	AddItemName((int)ItemID::SPITFIRE, "M600������ǹ", "Spitfire", "��");
	AddItemName((int)ItemID::SPITFIRE_GOLD, "M600������ǹ", "Spitfire [GOLD]", "��");
	AddItemName((int)ItemID::R301, "R-301����ǹ", "R-301", "��");
	AddItemName((int)ItemID::R301_GOLD, "R-301����ǹ", "R-301 [GOLD]", "��");
	AddItemName((int)ItemID::EVA8_AUTO, "EVA-8�Զ�����ǹ", "EVA-8 Auto", "��");	//1
	AddItemName((int)ItemID::EVA8_AUTO_GOLD, "EVA-8�Զ�����ǹ", "EVA-8 Auto [GOLD]", "��");	//1
	AddItemName((int)ItemID::PEACEKEEPER_HEIRLOOM, "��ƽ����������ǹ", "Peacekeeper", "��");
	AddItemName((int)ItemID::MOZAMBIQUE, "Īɣ�ȿ�����ǹ", "Mozambique", "Ϊ");
	AddItemName((int)ItemID::MOZAMBIQUE_GOLD, "�ƽ�Īɣ�ȿ�����ǹ", "Mozambique [GOLD]", "Ϊ");
	AddItemName((int)ItemID::WINGMAN, "������ǹ", "Wingman", "¶");
	AddItemName((int)ItemID::WINGMAN_GOLD, "�ƽ�����ǹ", "Wingman [GOLD]", "¶");
	AddItemName((int)ItemID::P2020, "P2020��ǹ", "P2020", "��");
	AddItemName((int)ItemID::P2020_GOLD, "�ƽ�P2020��ǹ", "P2020 [GOLD]", "��");
	AddItemName((int)ItemID::RE45, "RE-45�Զ���ǹ", "RE-45", "��");
	AddItemName((int)ItemID::RE45_GOLD, "�ƽ�RE-45�Զ���ǹ", "RE-45 [GOLD]", "��");
	AddItemName((int)ItemID::Repeater, "RE-45�Զ���ǹ", "Repeater", "��");
	AddItemName((int)ItemID::Repeater_GOLD, "�ƽ�RE-45�Զ���ǹ", "Repeater [GOLD]", "��");
	AddItemName((int)ItemID::VOLT, "�ƽ�RE-45�Զ���ǹ", "Volt", "��");
	AddItemName((int)ItemID::VOLT_GOLD, "�ƽ�RE-45�Զ���ǹ", "Volt [GOLD]", "��");
	AddItemName((int)ItemID::SENTINEL, "�ƽ�RE-45�Զ���ǹ", "SENTINEL", "��");
	AddItemName((int)ItemID::SENTINEL_GOLD, "�ƽ�RE-45�Զ���ǹ", "SENTINEL [GOLD]", "��");

	//Ammo
	AddItemName((int)ItemID::LIGHT_ROUNDS, "���͵�ҩ", "Light Ammo", "");
	AddItemName((int)ItemID::ENERGY_AMMO, "������ҩ", "Energy Ammo", "");
	AddItemName((int)ItemID::SHOTGUN_SHELLS, "������ҩ", "Shotgun Ammo", "");
	AddItemName((int)ItemID::HEAVY_ROUNDS, "���͵�ҩ", "Heavy Ammo", "");
	AddItemName((int)ItemID::SNIPER_AMMO, "���͵�ҩ", "Sniper Ammo", "");

	//Healing Items
	AddItemName((int)ItemID::ULTIMATE_ACCELERANT, "���м��ټ�", "Ultimate Accelerant", "");
	AddItemName((int)ItemID::PHOENIX_KIT, "������ư�", "Phoenix Kit", "��");
	AddItemName((int)ItemID::MED_KIT, "ҽ����", "Med Kit", "ˮ");
	AddItemName((int)ItemID::SYRINGE, "ע����", "Syringe", "��");
	AddItemName((int)ItemID::SHIELD_BATTERY, "���ͻ��ܵ��", "Shield Battery", "��");
	AddItemName((int)ItemID::SHIELD_CELL, "С�ͻ��ܵ��", "Shield Cell", "��");

	//Shield Items

	AddItemName((int)ItemID::HELMET_LV1, "ͷ��(lv1)", "Helmet Level 1", "�� lv1");
	AddItemName((int)ItemID::HELMET_LV2, "ͷ��(lv2)", "Helmet Level 2", "�� lv2");
	AddItemName((int)ItemID::HELMET_LV3, "ͷ��(lv3)", "Helmet Level 3", "�� lv3");
	AddItemName((int)ItemID::HELMET_LV4, "ͷ��(lv4)", "Helmet Level 4", "�� lv4");
	AddItemName((int)ItemID::BODY_ARMOR_LV1, "������(lv1)", "Body Armor Level 1", "�� lv1");
	AddItemName((int)ItemID::BODY_ARMOR_LV2, "������(lv2)", "Body Armor Level 2", "�� lv2");
	AddItemName((int)ItemID::BODY_ARMOR_LV3, "������(lv3)", "Body Armor Level 3", "�� lv3");
	AddItemName((int)ItemID::BODY_ARMOR_LV4, "������(lv4)", "Body Armor Level 4", "�� lv4");
	AddItemName((int)ItemID::EVO_SHIELD_LV1, "������(lv1)", "Evo Shield Level 1", "�� lv1");
	AddItemName((int)ItemID::EVO_SHIELD_LV2, "������(lv2)", "Evo Shield Level 2", "�� lv2");
	AddItemName((int)ItemID::EVO_SHIELD_LV3, "������(lv3)", "Evo Shield Level 3", "�� lv3");
	AddItemName((int)ItemID::EVO_SHIELD_LV4, "������(lv4)", "Evo Shield Level 4", "�� lv4");

	AddItemName((int)ItemID::KNOCKDOWN_SHIELD_LV1, "��������(lv1)", "Knockdown Shield Level 1", "�� lv1");
	AddItemName((int)ItemID::KNOCKDOWN_SHIELD_LV2, "��������(lv2)", "Knockdown Shield Level 2", "�� lv2");
	AddItemName((int)ItemID::KNOCKDOWN_SHIELD_LV3, "��������(lv3)", "Knockdown Shield Level 3", "�� lv3");
	AddItemName((int)ItemID::KNOCKDOWN_SHIELD_LV4, "��������(lv4)", "Knockdown Shield Level 4", "�� lv4");
	AddItemName((int)ItemID::BACKPACK_LV1, "����(lv1)", "Backpack Level 1", "�� lv1");
	AddItemName((int)ItemID::BACKPACK_LV2, "����(lv2)", "Backpack Level 2", "�� lv2");
	AddItemName((int)ItemID::BACKPACK_LV3, "����(lv3)", "Backpack Level 3", "�� lv3");
	AddItemName((int)ItemID::BACKPACK_LV4, "����(lv4)", "Backpack Level 4", "�� lv4");

	//Grenades
	AddItemName((int)ItemID::THERMITE_GRENADE, "���ȼ�����", "Thermite Grenade", "");
	AddItemName((int)ItemID::FRAG_GRENADE, "��Ƭ����", "Frag Grenade", "��");
	AddItemName((int)ItemID::ARC_STAR, "����", "Arc Star", "��");


	//Attachment Scopes
	AddItemName((int)ItemID::HCOG_CLASSIC, "����ȫϢ", "1x HCOG (Classic)", "ӯ");
	AddItemName((int)ItemID::HCOG_BRUISER, "2��ȫϢ", "2x HCOG (Bruiser)", "��");
	AddItemName((int)ItemID::HOLO, "������Ӱ", "1x Holo", "�");
	AddItemName((int)ItemID::VARIABLE_HOLO, "1-2����Ӱ", "1x-2x Variable Holo", "��");
	AddItemName((int)ItemID::DIGITAL_THREAT, "1�����ֻ���", "1x Digital Threat", "��");
	AddItemName((int)ItemID::HCOG_RANGER, "3��ȫϢ", "3x HCOG (Ranger)", "��");
	AddItemName((int)ItemID::VARIABLE_AOG, "2-4����׼��", "2x-4x Variable AOG", "��");
	AddItemName((int)ItemID::SNIPER, "6���ѻ���׼��", "6x Sniper", "��");
	AddItemName((int)ItemID::VARIABLE_SNIPER, "4-8����׼��", "4x-8x Variable Sniper", "��");
	AddItemName((int)ItemID::DIGITAL_SNIPER_THREAT, "4-10����׼��", "4x-10x Digital Sniper Threat", "��");

	//Attachments
	AddItemName((int)ItemID::BARREL_STABILIZER_LV1, "ǹ���ȶ���(lv1)", "Barrel Stabilizer Level 1", "�� lv1");
	AddItemName((int)ItemID::BARREL_STABILIZER_LV2, "ǹ���ȶ���(lv2)", "Barrel Stabilizer Level 2", "�� lv2");
	AddItemName((int)ItemID::BARREL_STABILIZER_LV3, "ǹ���ȶ���(lv3)", "Barrel Stabilizer Level 3", "�� lv3");
	AddItemName((int)ItemID::BARREL_STABILIZER_LV4, "ǹ���ȶ���(lv4)", "Barrel Stabilizer Level 4", "�� lv4");
	AddItemName((int)ItemID::LIGHT_MAGAZINE_LV1, "�ӳ�ʽ���͵�ϻlv1)", "Light Magazine Level 1", "�� lv1");
	AddItemName((int)ItemID::LIGHT_MAGAZINE_LV2, "�ӳ�ʽ���͵�ϻ(lv2)", "Light Magazine Level 2", "�� lv2");
	AddItemName((int)ItemID::LIGHT_MAGAZINE_LV3, "�ӳ�ʽ���͵�ϻ(lv3)", "Light Magazine Level 3", "�� lv3");
	AddItemName((int)ItemID::HEAVY_MAGAZINE_LV1, "�ӳ�ʽ���͵�ϻ(lv1)", "Heavy Magazine Level 1", "�� lv1");
	AddItemName((int)ItemID::HEAVY_MAGAZINE_LV2, "�ӳ�ʽ���͵�ϻ(lv2)", "Heavy Magazine Level 2", "�� lv2");
	AddItemName((int)ItemID::HEAVY_MAGAZINE_LV3, "�ӳ�ʽ���͵�ϻ(lv3)", "Heavy Magazine Level 3", "�� lv3");

	AddItemName((int)ItemID::ENERGY_MAGAZINE_LV1, "�ӳ�ʽ������ϻ(lv1)", "ENERGY Magazine Level 1", "�� lv1");
	AddItemName((int)ItemID::ENERGY_MAGAZINE_LV2, "�ӳ�ʽ������ϻ(lv2)", "ENERGY Magazine Level 2", "�� lv2");
	AddItemName((int)ItemID::ENERGY_MAGAZINE_LV3, "�ӳ�ʽ������ϻ(lv3)", "ENERGY Magazine Level 3", "�� lv3");
	AddItemName((int)ItemID::SNIPER_MAGAZINE_LV1, "�ӳ�ʽ������ϻ(lv1)", "ENERGY Magazine Level 1", "�� lv1");
	AddItemName((int)ItemID::SNIPER_MAGAZINE_LV2, "�ӳ�ʽ������ϻ(lv2)", "ENERGY Magazine Level 2", "�� lv2");
	AddItemName((int)ItemID::SNIPER_MAGAZINE_LV3, "�ӳ�ʽ������ϻ(lv3)", "ENERGY Magazine Level 3", "�� lv3");

	AddItemName((int)ItemID::SHOTGUN_BOLT_LV1, "����ǹǹ˨(lv1)", "Shotgun Bolt Level 1", "�� lv1");
	AddItemName((int)ItemID::SHOTGUN_BOLT_LV2, "����ǹǹ˨(lv2)", "Shotgun Bolt Level 2", "�� lv2");
	AddItemName((int)ItemID::SHOTGUN_BOLT_LV3, "����ǹǹ˨(lv3)", "Shotgun Bolt Level 3", "�� lv3");
	AddItemName((int)ItemID::STANDARD_STOCK_LV1, "��׼ǹ��(lv1)", "Standard Stock Level 1", "�� lv1");
	AddItemName((int)ItemID::STANDARD_STOCK_LV2, "��׼ǹ��(lv2)", "Standard Stock Level 2", "�� lv2");
	AddItemName((int)ItemID::STANDARD_STOCK_LV3, "��׼ǹ��(lv3)", "Standard Stock Level 3", "�� lv3");
	AddItemName((int)ItemID::SNIPER_STOCK_LV1, "�ѻ�ǹǹ��(lv1)", "Sniper Stock Level 1", "�� lv1");
	AddItemName((int)ItemID::SNIPER_STOCK_LV2, "�ѻ�ǹǹ��(lv2)", "Sniper Stock Level 2", "�� lv2");
	AddItemName((int)ItemID::SNIPER_STOCK_LV3, "�ѻ�ǹǹ��(lv3)", "Sniper Stock Level 3", "�� lv3");
	AddItemName((int)ItemID::TURBOCHARGER, "������ѹ��", "Turbocharger", "��");
	AddItemName((int)ItemID::SKULLPIERCER_RIFLING, "ѡ�����ģʽ��", "Selectfire Receiver", "��");
	AddItemName((int)ItemID::HAMMERPOINT_ROUNDS, "��׼����", "Precision Choke", "��");
	AddItemName((int)ItemID::DOUBLE_TAP_TRIGGER, "Disruptor Rounds", "Precision Choke", "��");
	AddItemName((int)ItemID::HOPUP_187, "˫����", "doubletaptrigger", "��");
	AddItemName((int)ItemID::QUICKDRAW_HOLSTER, "Hammerpoint Rounds", "Hammerpoint Rounds", "��");
	AddItemName((int)ItemID::VAULT_KEY, "Hammerpoint Rounds", "Hammerpoint Rounds", "��");
	AddItemName((int)ItemID::MOBILE_RESPAWN_BEACON, "Hammerpoint Rounds", "Hammerpoint Rounds", "��");
	AddItemName((int)ItemID::ITEM_191, "Hammerpoint Rounds", "Hammerpoint Rounds", "��");
	AddItemName((int)ItemID::TREASURE_PACK, "Hammerpoint Rounds", "Hammerpoint Rounds", "��");

	//AddItemName(89, "��������", "Skullpiercer Rifling", "��");
	//�� ������
	//SEProtectEnd();
}
