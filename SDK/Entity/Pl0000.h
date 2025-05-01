#pragma once

#include "..\Common.h"

#include "CBehavior.h"

/*
NieR:Automata's Entity class idk if it's the right name (actual Pl0000). Just started to
reverse this and this structure is fucking huge!

Offsets are -0x50 from the debug build

Size of struct 0x178F0 (96496) bytes old version (denuvo)
Size of struct 0x17920 (96544) bytes new version

CBehaviorAppBase, CBehavior, CObj, CModel, CParts

48 89 5C 24 08 57 48 83 EC 60 48 8B D9 E8 - Pl0000::SetBuddyFromNpc
*/
class Pl0000 : public CBehaviorAppBase
{
public:
	typedef void(* SetBuddyFromNpcFn)(Pl0000* pThis);

	typedef int32_t PassiveSkill;

	virtual void function150() = 0;
	virtual void CalculateDamage(int32_t a2, __int64 a3, float a4, int32_t a5) = 0;
	virtual void function152() = 0;
	virtual void function153() = 0;
	virtual void function154() = 0;
	virtual void function155() = 0;
	virtual void function156() = 0;

	EntityHandle m_hUnk;					//0x00C50
	char _0x0C58[8];						//0x00C58
	CReadWriteLock m_Lock;					//0x00C60
	//EntityHandle m_hUnk;					//0x00C80
	char _0x0C90[0x82C];					//0x00C90
	//EntityHandle m_hUnk;					//0x00DA0
	//float m_flTickBase;					//0x01470
	int32_t m_iLevel;						//0x014BC
	BOOL m_bLevelUp;						//0x014C0 | pretty sure
	char _0x14C4[44];						//0x014C4
	void* m_pCObjHitVtable;					//0x014F0 | start cObjHit
	char _0x014F8[664];						//0x014F8
	ExCollision m_VerticalCollision;		//0x01790
	ExCollision m_HorizontalCollision;		//0x018C0
	char _0x019F0[160];						//0x018F0
	float m_flSprintSpeed;					//0x01A90
	char _0x01A94[1500];					//0x01A94
	float fl0x02070;						//0x02070
	char _0x02074[4];						//0x02074
	ExExpInfo m_LevelsContainer;			//0x02078
	//	EntityHandle m_hCaughtFish;			//0x03ED8 | on the pod
	char _0x02B70[56056];					//0x02B70
	int32_t m_iHealth2;						//0x10668									
	char _0x1066C[420];						//0x1066C
	EntityHandle_t m_hPod;					//0x10810
	EntityHandle_t m_hUnk2;					//0x10814
	char _0x10818[844];						//0x10818
	BOOL m_bUnkCamera;						//0x10B64 | Camera boolean
	char _0x10B68[9048];					//0x10B68
	StaticArray<CObj*, 64> m_pObjects;		//0x12EC0
	char _0x130E0[10880];					//0x130E0
	BOOL m_bFlightSuit;						//0x15B50
	EntityHandle_t m_hFlightSuit;			//0x15B54
	char _0x15B58[880];						//0x15B58
	StaticArray<EntityHandle, 64> m_Handles;//0x15EC8
	char _0x15FE8[1156];					//0x15FE8
	EntityHandle_t m_hBuddy;				//0x1646C now 0x1647C | Localplayer = 0x1000C00, Buddy = 0x1020400
	EntityHandle_t m_hPascal;				//0x16470
	char _0x16474[268];						//0x16474
	StaticArray<EntityHandle, 4> m_Handles2;//0x16580
	char _0x165B0[72];						//0x165B0 | const hap::hap_action::PuidMixin
	StaticArray<PassiveSkill, 100> m_PassiveSkills;	//0x165F8
	char _0x167A8[1320];					//0x167A8
	BOOL b0x16CD0;							//0x16CD0
	BOOL m_bRenderSkirt;					//0x16CD4
	BOOL b0x16CD8;							//0x16CD8
	BOOL m_bRenderEyemask;					//0x16CDC
	BOOL b0x16CE0;							//0x16CE0
	BOOL m_bNotRenderFeather;				//0x16CE4
	BOOL b0x16CE8;							//0x16CE8
	EntityHandle m_hUnknown2;				//0x16CEC
	char _0x16CF0[868];						//0x16CF0
	EntityHandle m_hWig;					//0x17054
	char _0x17058[52];						//0x17058
	DWORD m_dwAccessory;					//0x1708C
	int32_t unk0x17090;						//0x17090
	int32_t m_iBodyType;					//0x17094
	char _0x17090[980];						//0x17098
	EntityHandle m_hUnknown3;				//0x1746C
	char _0x17470[896];						//0x17470
	Vector3Aligned m_vDestinationTarget;	//0x177F0
	BOOL m_bWalkToTarget;					//0x17800
	BOOL m_bRunToTarget;					//0x17804
	char _0x17808[256];						//0x17808
};
typedef Pl0000 Entity2B;
VALIDATE_OFFSET(Pl0000, m_vPosition, 0x50);
VALIDATE_OFFSET(Pl0000, m_ExtendWork, 0x140);
VALIDATE_OFFSET(Pl0000, m_Work, 0x390);
VALIDATE_OFFSET(Pl0000, m_pModelInfo, 0x540);
VALIDATE_OFFSET(Pl0000, m_ObjectId, 0x5B8);
VALIDATE_OFFSET(Pl0000, m_pInfo, 0x610);
VALIDATE_OFFSET(Pl0000, m_iAnimationId, 0x670);
VALIDATE_OFFSET(Pl0000, m_iAnimationMode, 0x674);
VALIDATE_OFFSET(Pl0000, m_BehaviourExtensions, 0x6B0);
VALIDATE_OFFSET(Pl0000, m_iHealth, 0x858);
VALIDATE_OFFSET(Pl0000, m_Lock, 0xC60);
VALIDATE_OFFSET(Pl0000, m_iLevel, 0x14BC);
VALIDATE_OFFSET(Pl0000, m_pCObjHitVtable, 0x14F0);
VALIDATE_OFFSET(Pl0000, m_LevelsContainer, 0x2078);
VALIDATE_OFFSET(Pl0000, m_VerticalCollision, 0x1790);
VALIDATE_OFFSET(Pl0000, m_HorizontalCollision, 0x18C0);
VALIDATE_OFFSET(Pl0000, m_bUnkCamera, 0x10B64);
VALIDATE_OFFSET(Pl0000, m_pObjects, 0x12EC0);
VALIDATE_OFFSET(Pl0000, m_hFlightSuit, 0x15B64);
VALIDATE_OFFSET(Pl0000, m_hBuddy, 0x1647C);
VALIDATE_OFFSET(Pl0000, m_hUnknown2, 0x16CFC);
VALIDATE_OFFSET(Pl0000, m_dwAccessory, 0x1709C);
VALIDATE_OFFSET(Pl0000, m_hUnknown3, 0x1747C);
VALIDATE_SIZE(Pl0000, 0x17920);