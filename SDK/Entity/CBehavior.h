#pragma once

#include "..\Common.h"

#include "CObj.h"

enum TypeId 
{

};

class CBehaviorExtension
{
public:
	virtual void function0(char a1) = 0;
	virtual void function1() = 0; // no return? wtf
	virtual void function2() = 0; // xor al, al	 ret
	virtual void function3() = 0; // mov al, 1 ret
	virtual void function4() = 0; // mov al, 1 ret
	virtual void function5() = 0; // nullsub

	StaticArray<std::pair<TypeId, CBehaviorExtension*>, 16, 8>* m_pBehaviorExtensions;	//0x0008
};

class ExCollision : public CBehaviorExtension // lib::Noncopyable
{
	enum Type : DWORD
	{
		COLLISION_TYPE_INVALID,
		COLLISION_TYPE_SPHERE,
		COLLISION_TYPE_BOX,
		COLLISION_TYPE_CAPSULE
	};

public:
	virtual void function0(char a1);
	virtual void function1();
	virtual void function2();
	virtual void function3();
	virtual void function4();
	virtual void function5();

	EntityHandle m_hOwner;										//0x0010
	BOOL m_bEnabled;											//0x0014
	BOOL m_bCollison;											//0x0018
	BOOL dword1C;												//0x001C
	Vector4 m_vPosition;										//0x0020 | CollisionInfo starts here	
	float m_boundingbox[6];										//0x0030
	BYTE gap48[60];												//0x0048
	DWORD dwCollisionType;										//0x0084
	DWORD dword88;												//0x0088
	int m_iBoneId;
	int m_iBoneId2;
	DWORD dword94;
	DWORD dword98;
	DWORD dword9C;
	Vector4 m_vExtents;
	Vector4 v0xB0;
	Vector4 v0xC0;
	Vector4 m_vSphereRadius;
	Vector3Aligned m_vMin;	// vector pos of collision
	void* unk[2];
	Vector3Aligned m_vMax;
	DWORD dword110;
	DWORD dword114;
	DWORD dword118;
	DWORD dword11C;
	Vector4 vec120;
};
VALIDATE_OFFSET(ExCollision, m_iBoneId, 0x8C);
VALIDATE_OFFSET(ExCollision, m_iBoneId2, 0x90);
VALIDATE_OFFSET(ExCollision, m_vExtents, 0xA0);
VALIDATE_SIZE(ExCollision, 0x130);

class ExLockOn : public CBehaviorExtension
{
	void* m_vtable;			//0x0000
	void* m_pEntity;		//0x0008 
};

class ExCatch : public CBehaviorExtension
{
	void* m_vtable;			//0x0000
};

class ExAttack : public CBehaviorExtension
{
	void* m_vtable;			//0x0000
};

class ExDamage : public CBehaviorExtension
{
	void* m_vtable;			//0x0000
};

class ExAttackCombo : public CBehaviorExtension
{
	void* m_vtable;			//0x0000
};

class ExActionState : public CBehaviorExtension
{
public:
	virtual void function0(char a1);
	virtual void function1();
	virtual void function2();
	virtual void function3();
	virtual void function4();
	virtual void function5();

	char _0x0010[0x18];		//0x0010
};
VALIDATE_SIZE(ExActionState, 0x28);

// Size of struct 0xB0
class ExWaypoint : public CBehaviorExtension
{
public:
	virtual void function0(char a1);
	virtual void function1();
	virtual void function2();
	virtual void function3();
	virtual void function4();
	virtual void function5();

	char _0x0010[0xA0];		//0x0010
};
VALIDATE_SIZE(ExWaypoint, 0xB0);

// Size of struct 0x38
class ExNpc : public CBehaviorExtension
{
public:
	virtual void function0(char a1);
	virtual void function1();
	virtual void function2();
	virtual void function3();
	virtual void function4();
	virtual void function5();

	char _0x0010[0x28];		//0x0010
};

/*
Level structure used for level and when you level up
size = 28 bytes
*/
struct Level_t
{
	int m_iMinimumExperience;		//0x0000 
	int m_iHealth;					//0x0004 new max health 
	int m_iBaseAttack;				//0x0008 idk about this one for sure
	int m_iDefense;					//0x000C
	int m_iLevel;					//0x0010 new level
	int unk0x14;					//0x0014
	int unk0x18;					//0x0018
};

class ExExpInfo : public CBehaviorExtension
{
public:
	virtual void function0(char a1);
	virtual void function1();
	virtual void function2();
	virtual void function3();
	virtual void function4();
	virtual void function5();

	QWORD unk0x10;			//0x0010
	QWORD unk0x18;			//0x0018
	Level_t m_levels[99];	//0x0020 ordered 1 - 99 (99 is max level)
	int m_nLevels;			//0x0AF4
};
VALIDATE_SIZE(ExExpInfo, 0x0AF8);

// for CBehaviorAppBase
class Animation
{
public:
	class Motion
	{
	public:
		class NodeParallel
		{
		};

		class NodePlay
		{
		public:
			void* m_pVtbl;
			Animation::Motion::NodeParallel* m_pParallel;
		};

		class Unit
		{
		public:
			class NodeHandler
			{
			public:
				Animation::Motion::NodePlay* m_pNodes;
			};
		};
	};
};

// sound type name prefix
enum SoundType
{
	SOUND_TYPE_PLAYER = 0, // "_pl"
	SOUND_TYPE_NPC, // "_npc"
	SOUND_TYPE_OT, // "_ot"
};

// TODO:
// 
// lib::tr::Mixin
// lib::message_slot::StaticListenerRequest
// lib::tr::Function
// hap::detail::PuidClassCall
// hap::detail::PuidClassCall2
// hap::script_export::FunctionExporterF

/*
* CBehavior - Fuck the UK bellends
* 
* Size of struct 0x830 (2096) bytes
*
* Alignment: 32 bytes??
*/
class CBehavior : public CObj
{
public:

	//virtual void function0(char a1) = 0;
	virtual void function0(void) = 0;
	virtual void UpdateModelPhysics(Vector4* v1, Vector4* v2, Vector4* v3) = 0; // from CModel
	virtual void function2(void) = 0; // CModel stub function
	virtual void* GetTypeInfo(void) = 0;
	virtual float function4(void) = 0; // return 1.0f;
	virtual void function5() = 0;
	virtual void function6() = 0;
	virtual void PreUpdate(void) = 0;
	virtual void Update(void) = 0;
	virtual void function9() = 0;
	virtual void function10() = 0;
	virtual void function11() = 0;
	virtual void function12() = 0;
	virtual void function13() = 0;
	virtual void function14() = 0;
	virtual void function15(Matrix4x4* pModel) = 0;
	virtual void function16() = 0;
	virtual void function17() = 0;
	virtual void Animate(unsigned int id, int mode, int a4, int a5) = 0;
	virtual void function19() = 0;
	virtual void function20() = 0;
	virtual void function21() = 0;
	virtual void function22() = 0;
	virtual void function23() = 0;
	virtual void function24() = 0;
	virtual void function25() = 0;
	virtual void function26() = 0;
	virtual void function27() = 0;
	virtual CEntityInfo* function28(__int64 a1, unsigned int uObjectId) = 0;
	virtual bool IsValid(void) = 0; //  return m_pInfo && (m_pInfo->m_Flags & 3) == 0 && this->GetMaxHealth() > 0;
	virtual int GetMaxHealth(void) = 0; //index: 30
	virtual int GetHealth(void) = 0;
	virtual void function32() = 0;
	virtual bool IsDead(void) = 0;
	virtual void function34() = 0;
	virtual void function35() = 0;
	virtual void function36() = 0;
	virtual void CollisionOff(void) = 0;
	virtual void CollisionOn(void) = 0;
	virtual void function39() = 0; // nullsub_2
	virtual __int64 EmitSound(const char* szName, CBehavior* pSourceEntity, __int64 boneId, __int64 flags, unsigned int a6) = 0;
	virtual void function41(const char* szName, Matrix4x4* pModelMatrix, void* pUnk, int, int, int) = 0;
	virtual void function42(const char* szName, int a2, SoundType Type) = 0; // EmitSound
	virtual void function43() = 0;
	virtual void function44() = 0;
	virtual void function45() = 0;
	virtual void function46() = 0;
	virtual void function47() = 0;
	virtual void function48() = 0;
	virtual void function49() = 0;
	virtual void function50() = 0;
	virtual void function51() = 0;
	virtual void function52() = 0;
	virtual void function53() = 0;
	virtual void function54() = 0;
	virtual void function55() = 0;
	virtual void function56() = 0;
	virtual void function57() = 0;
	virtual void function58() = 0;
	virtual void function59() = 0;
	virtual void function60() = 0;
	virtual void function61() = 0;
	virtual void function62() = 0;
	virtual void function63() = 0;
	virtual void function64() = 0;

	int m_iAnimationId;						//0x00670
	int m_iAnimationMode;					//0x00674
	int m_iAnimationA4;						//0x00678
	int m_iAnimationA5;						//0x0067C
	char _0x0680[4];						//0x00680
	int m_iGroupId[2];						//0x00684
	char _0x068C[12];						//0x0068C
	int m_iUnknown69C;
	void* m_pUnknown6A0; // set to 0 (on creation)
	void* m_pUnknown6A8; // points to pThis
	StaticArray<std::pair<TypeId, CBehaviorExtension*>, 16> m_BehaviourExtensions; // 0x06B0
	char _0x07D0[48];						//0x07D0
	BOOL m_bUnkCam800;						//0x0800
	char _0x0804[44];						//0x0804
};
VALIDATE_OFFSET(CBehavior, m_iAnimationId, 0x670);
VALIDATE_OFFSET(CBehavior, m_iAnimationMode, 0x674);
VALIDATE_OFFSET(CBehavior, m_BehaviourExtensions, 0x6B0);
VALIDATE_OFFSET(CBehavior, m_bUnkCam800, 0x800);
VALIDATE_SIZE(CBehavior, 0x830);

// Size of struct 0xC50
class CBehaviorAppBase : public CBehavior
{
public:
	typedef DWORD TimeLimitedFlag;

	virtual void function65() = 0;
	virtual void function66() = 0;
	virtual void function67() = 0;
	virtual void function68() = 0;
	virtual void function69() = 0;
	virtual void function70() = 0;
	virtual void function71() = 0;
	virtual void function72() = 0;
	virtual void function73() = 0;
	virtual void function74() = 0;
	virtual void function75() = 0;
	virtual void function76() = 0;
	virtual void function77() = 0;
	virtual void function78() = 0;
	virtual void function79() = 0;
	virtual void function80() = 0;
	virtual void function81() = 0;
	virtual void function82() = 0;
	virtual void function83() = 0;
	virtual void Hurt(int iDamage, BOOL bKeepAlive) = 0;
	virtual void Heal(int iHealAmount) = 0;
	virtual int GetLevel() = 0;
	virtual bool IsValid() = 0;
	virtual void function88() = 0;
	virtual void function89() = 0;
	virtual void function90() = 0;
	virtual void function91() = 0;
	virtual void function92() = 0;
	virtual void function93() = 0;
	virtual void function94() = 0;
	virtual void function95() = 0;
	virtual void function96() = 0;
	virtual void function97() = 0;
	virtual void function98() = 0;
	virtual void function99() = 0;
	virtual void function100() = 0;
	virtual void function101() = 0;
	virtual void function102() = 0;
	virtual void function103() = 0;
	virtual void SetLevel(int iLevel) = 0;
	virtual void function105() = 0;
	virtual void function106() = 0;
	virtual void function107() = 0;
	virtual void function108() = 0;
	virtual void function109() = 0;
	virtual void function110() = 0;
	virtual void function111() = 0;
	virtual void RequestAnim(uint32_t uAnim) = 0;
	virtual void RequestAnimTable(uint32_t uAnim) = 0;
	virtual void virtual_function_114() = 0;
	virtual void RequestAnimOp(void* a1, void* a2) = 0;
	virtual void RequestMove(Vector3Aligned* vPos, int32_t unk) = 0;
	virtual void function117() = 0;
	virtual void function118() = 0;
	virtual void function119() = 0;
	virtual void function120() = 0;
	virtual void function121() = 0;
	virtual void function122() = 0;
	virtual void function123() = 0;
	virtual void function124() = 0;
	virtual void function125() = 0;
	virtual void function126() = 0;
	virtual void function127() = 0;
	virtual void function128() = 0;
	virtual void function129() = 0;
	virtual void function130() = 0;
	virtual void function131() = 0;
	virtual void function132() = 0;
	virtual void function133() = 0;
	virtual void function134() = 0;
	virtual void function135() = 0;
	virtual void function136() = 0;
	virtual void function137() = 0;
	virtual void function138() = 0;
	virtual void function139() = 0;
	virtual void function140() = 0;
	virtual void function141() = 0;
	virtual void function142() = 0;
	virtual void function143() = 0;
	virtual void function144() = 0;
	virtual void function145() = 0;
	virtual void function146() = 0;
	virtual void function147() = 0;
	virtual void function148() = 0;
	virtual void function149() = 0;

	ExActionState m_ActionState;						//0x00830  ExActionState (BehaviorExtension)
	int m_iHealth;										//0x00858
	int m_iMaxHealth;									//0x0085C
	char _0x0860[36];									//0x00860 | Animation functions access a pointer here maybe ( Animation::Motion::Unit::NodeHandler::`vftable')
	BYTE m_flags;										//0x00884 | 0x40 = no collision
	char _0x0885[91];									//0x00885
	ExWaypoint m_Waypoint;								//0x008E0 | ExWaypoint (BehaviorExtension)
	int m_iOldAnimationId;								//0x00990
	int m_iOldAnimationMode;							//0x00994
	int m_iOldAnimationA4;								//0x00998
	int m_iOldAnimationA5;								//0x0099C
	char _0x09A0[8];									//0x009A0
	StaticArray<TimeLimitedFlag, 4> m_LimitedTimeFlags;	//0x009A8 class lib::StaticArray<BehaviorAppBase::TimeLimitedFlag,4,4>
	char _0x09D8[0xC0];									//0x009D8
	ExNpc m_Npc;										//0x00A98
	char _0x0AD0[0xF0];									//0x009D8
	UINT m_uWaitFlags;									//0x00BC0
	char _0x0BC4[0x08];									//0x00BC4
	UINT m_uTalkFlags;									//0x00BCC
	char _0x0BD0[0x70];									//0x00BD0
	float m_flAnimationSpeed;							//0x00C40
	char _0x0C44[0x0C];									//0x00C44
};
VALIDATE_OFFSET(CBehaviorAppBase, m_ActionState, 0x0830);
VALIDATE_OFFSET(CBehaviorAppBase, m_iHealth, 0x0858);
VALIDATE_OFFSET(CBehaviorAppBase, m_iOldAnimationId, 0x0990);
VALIDATE_OFFSET(CBehaviorAppBase, m_LimitedTimeFlags, 0x09A8);
VALIDATE_OFFSET(CBehaviorAppBase, m_Npc, 0x0A98);
VALIDATE_SIZE(CBehaviorAppBase, 0xC50);