#pragma once
#include <d3d11.h>
#include <d3dx9math.h>
#include "StaticAssert.h"
#include "SteamApi.h"
#include "Math.h"

#define PURE = 0

#define SAVE_FLAGS_READ_SLOTS	0x00000001
#define SAVE_FLAGS_READ			0x00000002
#define SAVE_FLAGS_WRITE		0x00000003
#define SAVE_FLAGS_DELETE		0x00000004

#define SAVE_SYSTEMDATA -2
#define SAVE_GAMEDATA -1
#define SAVE_SLOT0DATA 0
#define SAVE_SLOT1DATA 1
#define SAVE_SLOT2DATA 2

#define ARRAY_FLAGS_FREE 1

#define _MM_RSHUFFLE(mask, x, y, z, w) \
	 (x) = ((mask & 0xC) >> 2);		   \
	 (y) = ((mask & 0x30) >> 4);	   \
	 (z) = ((mask & 0xC0) >> 6);	   \
	 (w) = (mask & 0x3);			   \

#define IS_SAVE_SLOTDATA(s) ((s) > -1 && (s) < 3)

// visual studio says correct offsets are wrong before compalation on interfaces rifk
#define IS_OFFSET_CORRECT(s, m, offset) CASSERT(offsetof(s, m) == (offset), ReversedStructs)
#define IS_SIZE_CORRECT(s, size) CASSERT(sizeof(s) == (size), ReversedStructs)

#define ENTITY_REAL_VTABLE

typedef ULONGLONG QWORD;

typedef DWORD EntityHandle;

enum TypeId {};

class Entity_t;

/*
Nier Automata's lib::Array<T>
*/
template<typename T>
class Array
{
public:
	virtual Array<T>* Clear(BYTE flags);
	virtual QWORD GetSize();
	virtual bool GetItem(OUT T* pItem);
	virtual T* Find(IN T* pSearchItem, OUT T* pFound);

	T* m_pItems;	//0x0008 
	QWORD m_count;	//0x0010
	QWORD m_size;	//0x0018
};

/*
Nier Automata's lib::StaticArray<T, size, element_size>
*/
template<typename T, size_t size, size_t element_size = sizeof(T)>
class StaticArray : public Array<T>
{
public:
	T m_items[size];	//0x0020
};

struct Sound
{
	const char* m_szName;
	DWORD m_dwFNV1Hash;
	DWORD m_Flags;		 //probably a bitflag - (values I've seen, 0x05000000, 0x00000000, 0x800000 [mute probably])
};

struct SoundEmitter
{
	Entity_t* m_pEntity; // this is some lower interface like behaviour
	int m_index;		 // have no fucking clue what this is checks if its -1 so it's an int
};

class CUnknown_t
{
public:
	DWORD id;			//0x0000 idk?
	char aligment[4];	//0x0004
	void* punk;			//0x0008
	void* punk2;		//0x0010
	void* pFunction;	//0x0018
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


/*
Size [8 byte aligned] = 2808 (0xAF8) bytes
Size [4 byte aligned] = 2804 (0xAF4) bytes
*/
struct ExExpInfo
{
	void* m_vtable;			//0x0000
	void** unk;				//0x0008
	QWORD unk0x10;			//0x0010
	QWORD unk0x18;			//0x0018
	Level_t m_levels[99];	//0x0020 ordered 1 - 99 (99 is max level)
};

struct Unknown_t;

struct EntityInfo
{
	Unknown_t* m_pUnknown;						//0x0000 | i don't really know what this is (confirmed a struct pointer) maybe a void*
	char m_szEntityType[32];					//0x0008
	unsigned int m_ObjectId;					//0x0028
	BYTE m_Flags;								//0x002C  | An Entity cannot have this flag or'd with 3 game crashes (possibly a destroyed flag)
	char alignment[3];							//0x002D
	EntityHandle m_hParent;						//0x0030
	char _0x0038[4];							//0x0034
	char* m_pszData[2];							//0x0038
	Entity_t* m_pEntity;						//0x0048
	const char** m_pszDat;						//0x0050  | struct pointer
	DWORD* m_pUnk;								//0x0058  | dword array 2 members (0x1415F6B50)
	Entity_t* m_pParent;						//0x0060
	BOOL bSetInfo;								//0x0068
	DWORD _0x006C;								//0x006C
	DWORD _0x0070;								//0x0070
};
IS_OFFSET_CORRECT(EntityInfo, m_pParent, 0x60)

struct ModelInfo
{
	Vector4 m_vTint;			//0x0000
	char _0x0040[3];			//0x0010
	Vector3Aligned m_vPosition; //0x0070
};

/*
Model Part
size = 0x70 (112) bytes
*/
struct ModelPart
{
	Vector4 m_vUnknown0;		//0x0008
	Vector4 m_vColor;			//0x0010
	Vector4 m_vUnknown20;		//0x0020
	Vector4 m_vUnknown30;		//0x0030
	const char* m_szModelPart;	//0x0040
	void* m_pUnknown;			//0x0048
	DWORD m_dwUnknown50;		//0x0050
	QWORD m_qwUnknown58;		//0x0058
	DWORD m_dwUnknown60;		//0x0060
	BOOL m_bShow;				//0x0064
	BOOL m_bUpdate;				//0x0068
	DWORD m_qwUnknown6C;		//0x006C
};

struct MappedModel
{
	void* m_vtbl;		//0x0000
	char _0x0008[184];	//0x0008
	void* m_pModel;		//0x00B8
	DWORD dw0xC0;		//0x00C0
};

class BehaviorAppBase
{
public:
	typedef DWORD TimeLimitedFlag;
};

class BehaviorExtension
{

};

class CModelExtendWorkBase
{
	virtual void function0();
	virtual void function1();
};

class CModelExtendWork : public CModelExtendWorkBase
{
	virtual void function2();
	virtual void function3();

	float flUnk;		//0x08
	DWORD _0x0010;		//0x10
	DWORD _0x0014;		//0x14
	char _0x0018[12];	//0x18
	Entity_t* m_pParent;//0x2C	
};

class ExWaypoint : public BehaviorExtension
{

};

class ExBaseInfo : public BehaviorExtension
{
public:
	virtual void function0();
	virtual void function1();
	virtual void function2();
	virtual void function3();
	virtual void function4();
	virtual void function5();


};

class CXmlBinary
{
	void* m_vTable;
};

/*
NieR:Automata's Entity class idk if it's the right name (actual Pl0000). Just started to
reverse this and this structure is fucking huge!
*/
class Entity_t
{
public:
#ifdef ENTITY_REAL_VTABLE
	virtual void function0() PURE;
	virtual void function1() PURE;
	virtual void function2() PURE;
	virtual void function3() PURE;
	virtual void function4() PURE;
	virtual void function5() PURE;
	virtual void function6() PURE;
	virtual void function7() PURE;
	virtual void function8() PURE;
	virtual void function9() PURE;
	virtual void function10() PURE;
	virtual void function11() PURE;
	virtual void function12() PURE;
	virtual void function13() PURE;
	virtual void function14() PURE;
	virtual void function15() PURE;
	virtual void function16() PURE;
	virtual void function17() PURE;
	virtual void Animate(__int64 id, QWORD mode) PURE;
	virtual void function19() PURE;
	virtual void function20() PURE;
	virtual void function21() PURE;
	virtual void function22() PURE;
	virtual void function23() PURE;
	virtual void function24() PURE;
	virtual void function25() PURE;
	virtual void function26() PURE;
	virtual void function27() PURE;
	virtual EntityInfo* function28(__int64 a2, unsigned int model_type) PURE;
	virtual bool IsAliveAndEntityInfoFlag() PURE;
	virtual int GetMaxHealth() PURE; //index: 30
	virtual int GetHealth() PURE;
	virtual void function32() PURE;
	virtual bool IsDead() PURE;
	virtual void function34() PURE;
	virtual void function35() PURE;
	virtual void function36() PURE;
	virtual void function37_off() PURE;
	virtual void function38_on() PURE;
	virtual void function39() PURE;
	virtual __int64 __fastcall EmitSound(const char *szName, Entity_t* pSourceEntity, __int64 a4, __int64 a5, unsigned int a6) PURE;
	virtual void function41() PURE;
	virtual void function42() PURE;
	virtual void function43() PURE;
	virtual void function44() PURE;
	virtual void function45() PURE;
	virtual void function46() PURE;
	virtual void function47() PURE;
	virtual void function48() PURE;
	virtual void function49() PURE;
	virtual void function50() PURE;
	virtual void function51() PURE;
	virtual void function52() PURE;
	virtual void function53() PURE;
	virtual void function54() PURE;
	virtual void function55() PURE;
	virtual void function56() PURE;
	virtual void function57() PURE;
	virtual void function58() PURE;
	virtual void function59() PURE;
	virtual void function60() PURE;
	virtual void function61() PURE;
	virtual void function62() PURE;
	virtual void function63() PURE;
	virtual void function64() PURE;
	virtual void function65() PURE;
	virtual void function66() PURE;
	virtual void function67() PURE;
	virtual void function68() PURE;
	virtual void function69() PURE;
	virtual void function70() PURE;
	virtual void function71() PURE;
	virtual void function72() PURE;
	virtual void function73() PURE;
	virtual void function74() PURE;
	virtual void function75() PURE;
	virtual void function76() PURE;
	virtual void function77() PURE;
	virtual void function78() PURE;
	virtual void function79() PURE;
	virtual void function80() PURE;
	virtual void function81() PURE;
	virtual void function82() PURE;
	virtual void function83() PURE;
	virtual void Hurt(int iDamage, BOOL bKeepAlive) PURE;
	virtual void Heal(int iHealAmount) PURE;
	virtual void function86() PURE;
	virtual void function87() PURE;
	virtual void function88() PURE;
	virtual void function89() PURE;
	virtual void function90() PURE;
	virtual void function91() PURE;
	virtual void function92() PURE;
	virtual void function93() PURE;
	virtual void function94() PURE;
	virtual void function95() PURE;
	virtual void function96() PURE;
	virtual void function97() PURE;
	virtual void function98() PURE;
	virtual void function99() PURE;
	virtual void function100() PURE;
	virtual void function101() PURE;
	virtual void function102() PURE;
	virtual void function103() PURE;
	virtual void function104() PURE;
	virtual void function105() PURE;
	virtual void function106() PURE;
#else
	void* m_pVtable;					//0x00000	 
#endif // ENTITY_REAL_VTABLE
	//char _0x0008[8];						//0x00008  CMODEL STARTS AT 0x0000
	Matrix4x4 m_matUnk;						//0x00010
	Vector3Aligned m_vPosition;				//0x00050
	Matrix4x4 m_matModelToWorld;			//0x00060 | 1st row: ? 2nd row: scale(x,y,z) 3rd: none: 4th rotation(p,y,r)
	char _0x005C[160];						//0x000A0
	CModelExtendWork m_pModelExtendWork;	//0x00140
	char _0x0168[560];						//0x00168
											//0x00180	lib::StaticArray<Pl0000Mouse::MouseKeyMap,3,4>
											//0x001B8	lib::StaticArray<MouseInput,3,4>
											//0x00310	lib::StaticArray<Pl0000KeyBoard::KeyMap,22,4>
											//0x003E0	lib::StaticArray<KeyInput,22,4>
	MappedModel* m_pMappedModel;			//0x00398
	ModelPart* m_pModelParts;				//0x003A0 
	DWORD m_nModelParts;					//0x003A8
	char _0x03AC[364];						//0x003AC
	CModelExtendWork* m_pModelExtend;		//0x00518
	char _0x0520[16];						//0x00520
	void* m_pScene;							//0x00530 | when null scene is transparent
	QWORD qw0x00538;						//0x00538
	ModelInfo* m_pModelInfo;				//0x00540
	Unknown_t* m_pUnknown0x548;				//0x00548
	void* m_pWMB_Buffer;					//0x00550
	void* m_p0x00558;						//0x00558
	Unknown_t* qw0x00560;					//0x00560
	int m_iVerticeUpdateCount;				//0x00568 | changes the amount of vertices to get updated each frame (can't be more than 198 for 2B)
	char _0x056C[12];						//0x0056C
	Vector3 m_vUnknown;						//0x00578
	BOOL m_bExtraDetail;					//0x00584
	char _0x0588[8];						//0x00588
	DWORD dw0x590;							//0x00590
	char _0x594[4];							//0x00594
	DWORD m_Flags;							//0x00598
	DWORD dw0x0059C;						//0x0059C
	char _0x5A0[24];						//0x005A0
	unsigned int m_ObjectId;				//0x005B8 | 10000 = 2B | 10200 = 9S | 10203 = A2? (2B only accepts regualar, mech suit, and static -1)
	unsigned int unk0x005BC;				//0x005BC
	unsigned int flag0x005C0;				//0x005C0 | one-way invisible 2B
	BYTE unk0x05C4;							//0x005C4 | disable dynamic skirt?
	char _0x05C5[3];						//0x005C5
	DWORD dw0x5C8;							//0x005C8
	float fl0x5CC[7];						//0x005CC
	Vector3 vInnerBrightness;				//0x005E8 (x = brightness [paired with z, not modifyable], y = unknown, z = brightness [paired with z, modifyable])
	float fl0x5F4[5];						//0x005F4
	float fl0x60C;							//0x0060C
	EntityInfo* m_pInfo;					//0x00610 
	CXmlBinary m_xmlBinary;					//0x00618
	char _0x0620[144];						//0x00620
	StaticArray<std::pair<TypeId, BehaviorExtension*>, 16, 8> m_BehaviourExtensions;//0x006B0 | lib::StaticArray<std::pair<enum  lib::TypeId,BehaviorExtension *>,16,8>  BehaviorExtension is an interface
	char _0x07D0[136];						//0x007D0
	//ExActionState							//0x00830  ExActionState
	int m_iHealth;							//0x00858
	int m_iMaxHealth;						//0x0085C
	char _0x0860[124];						//0x00860
	void* m_waypointVtbl;					//0x008E0
	char _0x08E8[192];						//0x008E8
	StaticArray<BehaviorAppBase::TimeLimitedFlag, 4> m_LimitedTimeFlags;//0x009A8 class lib::StaticArray<BehaviorAppBase::TimeLimitedFlag,4,4>
	char _0x09D8[2788];						//0x009D8
	int m_iLevel;							//0x014BC
	BOOL m_bLevelUp;						//0x014C0 | pretty sure
	char _0x14C4[44];						//0x014C4
	void* m_pCObjHitVtable;					//0x014F0 | start cObjHit
	char _0x014EC[2936];					//0x014F8
	float fl0x02070;						//0x02070
	char _0x02074[4];						//0x02074
	ExExpInfo m_LevelsContainer;			//0x02078
	char _0x02B70[56056];					//0x02B70
	int m_iHealth2;							//0x10668
	char _0x1066C[21736];					//0x1066C
	EntityHandle m_hUnknown;				//0x15B54 | one of these unknown handles are probably your tamed animal lel
	char _0x15B58[880];						//0x15B58
	StaticArray<EntityHandle, 64> m_Handles;//0x15EC8
	char _0x15FE8[1156];					//0x15FE8
	EntityHandle m_hBuddy;					//0x1646C | Localplayer = 0x1000C00, Buddy = 0x1020400
	char _0x16470[272];						//0x16470
	StaticArray<EntityHandle, 4> m_Handles2;//0x16580
	char _0x165B0[1852];					//0x165B0
	EntityHandle m_hUnknown2;				//0x16CEC
	char _0x16CF0[924];						//0x16CF0
	DWORD m_dwAccessory;					//0x1708C
	char _0x17090[988];						//0x17090
	EntityHandle m_hUnknown3;				//0x1746C
};
typedef Entity_t Pl0000;
IS_OFFSET_CORRECT(Entity_t, m_vPosition, 0x50)
IS_OFFSET_CORRECT(Entity_t, m_pModelExtendWork, 0x140)
IS_OFFSET_CORRECT(Entity_t, m_pMappedModel, 0x398)
IS_OFFSET_CORRECT(Entity_t, m_pModelExtend, 0x518)
IS_OFFSET_CORRECT(Entity_t, m_pModelInfo, 0x540)
IS_OFFSET_CORRECT(Entity_t, m_Flags, 0x598)
IS_OFFSET_CORRECT(Entity_t, m_ObjectId, 0x5B8)
IS_OFFSET_CORRECT(Entity_t, m_pInfo, 0x610)
IS_OFFSET_CORRECT(Entity_t, m_BehaviourExtensions, 0x6B0)
IS_OFFSET_CORRECT(Entity_t, m_iHealth, 0x858)
IS_OFFSET_CORRECT(Entity_t, m_pCObjHitVtable, 0x14F0)
IS_OFFSET_CORRECT(Entity_t, m_LevelsContainer, 0x2078)
IS_OFFSET_CORRECT(Entity_t, m_hUnknown, 0x15B54)
IS_OFFSET_CORRECT(Entity_t, m_hBuddy, 0x1646C)
IS_OFFSET_CORRECT(Entity_t, m_hUnknown2, 0x16CEC)
IS_OFFSET_CORRECT(Entity_t, m_dwAccessory, 0x1708C)
IS_OFFSET_CORRECT(Entity_t, m_hUnknown3, 0x1746C)

class CCamera
{
public:
	void* m_vtable;				//0x0000
	QWORD unk;					//0x0008
	Matrix4x4 m_mat1;			//0x0010
	float m_flZoom;				//0x0050
	char _0x0054[84];			//0x0054
	Entity_t* m_pEntity;		//0x00A8
	DWORD dwUnk;				//0x00B0
	char _0x00B4[28];			//0x00B4
	EntityHandle m_hEntity;		//0x00D0
	DWORD alignment;			//0x00D4
	Entity_t* m_pEntites[2];	//0x00E0
	QWORD unknown;				//0x00F0
	Matrix4x4 m_mat2;			//0x00F8
};

// Found by Dennis
// Address = 0x14160EB40
struct Sun
{
	D3DXVECTOR2 m_vPosition; //0x0000
	char _0x0008[56];		 //0x0008
	Vector4 m_vColor;		 //0x0040
	float m_flBloom;		 //0x0044
};

/*
SceneStateHead
size = 0x20 (32) bytes
*/
struct SceneStateHead
{

};

// this has a doubly linked list for sure prbably a different struct tho
// that is 32 bytes that is the head
struct SceneState
{
	SceneStateHead* m_pHead;	//0x0000
	SceneState* m_pNext;		//0x0008
	SceneState* m_pLast;		//0x0010
	char _0x0018;				//0x0018
	bool m_bLast;				//0x0019  not 100%
	char _0x0020[6];			//0x0020
	DWORD m_DescriptionHash;	//crc32
	char alignment[4];
	char* m_szDescription;
	size_t m_ReferenceCount;	//not sure
};

class CSceneStateSystem
{
public:
	void* m_vtable;					//0x0000
	char _0x0008[88];				//0x0008
	Array<SceneState> m_oldstates;	//0x0060 //const lib::DynamicArray<hap::scene_state::SceneState,hap::configure::Allocator>
	char _0x00[16];					//0x0080
	Array<SceneState> m_states;		//0x0090 //const lib::DynamicArray<hap::scene_state::SceneState,hap::configure::Allocator>
};
IS_OFFSET_CORRECT(CSceneStateSystem, m_states, 0x90)

//implement these when possible
class CDepthSurface;
class CRenderTarget;
class CVertexBuffer;
class CVertexLayout;
class CConstantBuffer;

struct Keyboard_t
{
	IDirectInputDevice8A* pKeyboard;
	BOOL bAcquired;
	HKL Layout;
};

struct Mouse_t
{
	HWND hWnd;
	IDirectInputDevice8A* pMouse;
	BOOL bAcquired;
	BOOL bShowCursor;
	BOOL bShowCursorOld;
	int field_1C;
	int field_20;
	BOOL bClipCursor;
	int field_28;
	float fl_field_2C;
	float fl_field_30;
};

enum eControllerButtons
{
	DPAD_LEFT = 0x1,
	DPAD_RIGHT = 0x2,
	DPAD_DOWN = 0x4,
	DPAD_UP = 0x8,
	A = 0x10,
	B = 0x20,
	X = 0x40,
	Y = 0x80,
	START = 0x100,
	BACK = 0x200,
	LEFT_SHOULDER = 0x400,
	LEFT_THUMB = 0x1000,
	RIGHT_SHOULDER = 0x2000,
	RIGHT_THUMB = 0x8000
};

// apperantly 128 bytes according to stack memset, but asm says 32 bytes
struct controller_input_state
{
	BOOL bUpdated;				//0x0000
	float m_flThumbLX;			//0x0004
	float m_flThumbLY;			//0x0008
	float m_flThumbRX;			//0x000C
	float m_flThumbRY;			//0x0010
	float m_flLeftTrigger;		//0x0014
	float m_flRightTrigger;		//0x0018
	DWORD m_dwButtons;			//0x001C | eConrollerButtons
};

/*
this structure is used to create d3d and is used in many of NieR: Automata's
direct3d functions. It might have more to it that I haven't explored so, this is just
the basic layout of the stucture. Strangley enough, it has device pointers and factory
pointers but not a swapchain pointer nor an adapter pointer. Perhaps, I just have not
disovered them. In addition I haven't found a global pointer that directly points to
this structure. This is a virtual class after further investigation. Correctly
renamed the class to it's proper name. It seems there might be a nested struct too.

 Size of struct 0x110 (272) bytes
 */
class CGraphicDeviceDx11
{
	//virtual CGraphicDeviceDx11* function0();
	//virtual void function1();
	//virtual unsigned int function2(__int64 a1, CDisplay* pDisplay, HWND hWnd, __int64 a4);
	//virtual bool function3();

public:
	void* pVtable;							//0x0000
	__int32 iScreenWidth;					//0x0008 
	__int32 iScreenHeight;					//0x000C 
	float flScreenHz;						//0x0010 / fps / 60hz capped
	BOOL isFullscreen;						//0x0014
	DWORD dwUnk0x018;						//0x0018
	char _0x0014[12];						//0x001C
	ID3D11Device* pID3D11Device;			//0x0028
	IDXGIFactory* pFactory;					//0x0030
	char _0x0038[8];						//0x0038
	CVertexBuffer* m_pVertexBuffer;			//0x0040 | Hw::cVertexBufferImpl
	void* unk0x048[2];						//0x0048
	CVertexLayout* m_pVertexLayout;			//0x0058 | Hw::cVertexLayoutImpl
	CConstantBuffer* m_pConstantBuffer;		//0x0060 | Hw::cConstantBufferImpl
	char _0x0048[8];						//0x0068
	ID3D11Device* pD3D11Device2;			//0x0070 | same pointer as first
	IDXGIFactory* pFactory2;				//0x0078 | same pointer as first
	char _0x0080[96];						//0x0080 | pointers in here
	BOOL isWindowed;						//0x00E0
	float flFrameRate;						//0x00E4
	char _0x00E8[24];						//0x00E8
	wchar_t* pszGraphicsAdapterName;		//0x0100 (array?) maybe DXGI_OUTPUT_DESC*
	QWORD qwCurrentAdapterNameIndex;		//0x0108
};

class CGraphicContextDx11
{
public:
	void* m_pVtbl;							//0x0000
	ID3D11DeviceContext* m_pContext;		//0x0008
	char unk0x0010[16];						//0x0010
	CRenderTarget* m_pRenderTarget;			//0x0020
	char _0x0030[32];						//0x0028
	CDepthSurface* m_pDepthSurface;			//0x0048
	CVertexBuffer* m_pVertexBuffer;			//0x0050
	ID3D11VertexShader* m_pVertexShader;	//0x0058
	ID3D11PixelShader* m_pPixelShader;		//0x0060
	char _0x0068[16];						//0x0068
	ID3D11Buffer* m_pBuffers[2];			//0x0070
};

class CDisplay
{
public:
	void* m_pVtble;							//0x0000
	DWORD dwUnk0x0008;						//0x0008
	int m_iWidth;							//0x000C
	int m_iHeight;							//0x0010
	DWORD dwUnk0x0014;						//0x0014
	void* m_pConstantBuffer[2];				//0x0018
	char _0x0030[40];						//0x0030
	IDXGISwapChain** m_ppSwapChain;			//0x0058
	char _0x0060[16];						//0x0060
	CGraphicDeviceDx11* m_pGraphicDevice;   //0x0068
};

// Size = 0xF0 (240) bytes
class CGraphics
{
public:
	void* unk;							//0x0000
	char unk0x08[8];					//0x0008
	void* m_pSamplerState;				//0x0010
	char unk0x18[8];					//0x0018
	CGraphicContextDx11* m_pContext;	//0x0020
	CDisplay m_Display;					//0x0028
	char _0x98[8];						//0x0098
	CRITICAL_SECTION m_CriticalSection;	//0x00A0
};
IS_OFFSET_CORRECT(CGraphics, m_Display, 0x28)
IS_OFFSET_CORRECT(CGraphics, m_CriticalSection, 0xA0)

struct CSaveSlot
{
	DWORD dwAccountId;
	BOOL InUse;
};

/*
Size of the struct is 0xD0 (208) bytes
*/
struct CSaveDataDevice
{
	void* m_pVtable;				//0x0000
	DWORD dwUnk0x08;				//0x0008
	char _0x000C[4];				//0x000C
	CSaveSlot* pSaveSlots;			//0x0010
	QWORD nMaxSlot;					//0x0018
	QWORD qw0x0020;					//0x0020
	int i0x0028;					//0x0028
	char _0x002C[4];				//0x002C
	union
	{
		QWORD qwFlags;				//0x0030
		struct
		{
			DWORD dwFlags;			//0x0030
			DWORD dwStatus;			//0x0034
		};
	};
	int nSlot;						//0x0038
	char unk0x003C[4];				//0x003C | Aligment maybe
	HANDLE hFile;					//0x0040
	OVERLAPPED overlapped;			//0x0048
	DWORD nBytesToIO;				//0x0068 | maybe nBytesToIO and nBytesIO are a struct
	DWORD nBytesIO;					//0x006C
	void* pSlotDataBuffer;			//0x0070
	QWORD qwSlotDataBufferSize;		//0x0078
	void* pGameDataBuffer;			//0x0080
	QWORD qwGameDataBufferSize;		//0x0088
	void* pSystemDataBuffer;		//0x0090 | includes screen dimensions
	QWORD qwSystemDataBufferSize;	//0x0098
	void* pBuffer;					//0x00A0
	QWORD nBytesIO_SystemData;		//0x00A8
	QWORD nBytesIO_GameData;		//0x00B0
	QWORD nBytesIO_SaveData;		//0x00B8
	DWORD dwUnk0xC0;				//0x00C0
	DWORD dwUnk0xC4;				//0x00C4
	QWORD qwUnk0xC8;				//0x00C8 | this is an inline struct
	DWORD dwUnk0xD0;				//0x00D0 | this is a member of unk0xc8
	DWORD dwUnk0xD4;				//0x00D4 | this is a member of unk0xc8
};

class CUserInfoBase
{
	virtual ~CUserInfoBase();
};

class CUserInfo
{
	virtual ~CUserInfo();

	BOOL bActive;	//0x08
	char _0x0C[4];	//0x0C
	int idk;		//0x10
	int index[2];	//0x14
	char _0x1C[4];  //0x1C	
};

class HeapAlloc_t
{
	LPVOID Pointer;
	BOOL Succeeded;
};

class CHeapInstance
{
	void* m_pVtbl;
	CRITICAL_SECTION m_CriticalSection;		//0x0008
	HeapAlloc_t* m_pAllocation;				//0x0028
	//const char* m_szId;						 //0x0028 | doesn't line up  
};


template<typename T>
struct ConstructionInfo
{
	DWORD modeltype;
	char alignment[4];
	T* (*Construtor)(CHeapInstance**);
	QWORD id;
	const char* szName;
	void* pUnk;
};

struct Create_t
{
	const char* m_szName;			//0x0000
	unsigned int m_ModelTypes[2];	//0x0008
	void* m_p0x0010;				//0x0010
	void* m_p0x0018;				//0x0018
	void* m_p0x0020;				//0x0020
	int m_iUnk;						//0x0028
	char alignment[4];				//0x002C
	void* m_p0x0030;				//0x0030
	Vector4 m_vec[2];				//0x0038
	void* m_p0x0058;				//0x0058
};

class HandlerBase;
typedef int DlcInstalled_t;

/*
This struct is completely guessed. The only thing correct is the size

Size of is 0xC (12) bytes
*/
class CallbackInstalled
{
	DlcInstalled_t m_installed;
	BOOL m_bInstalled;
	DWORD dwUnk;
};

/*
Size of is 0x38 (56) bytes
*/
class CGameContentDeviceSteam
{
public:
	void* m_pVtbl;															//0x0000
	CCallback<CGameContentDeviceSteam, DlcInstalled_t, false> m_Callback;	//0x0008  | id is 1005
	CallbackInstalled* m_pItems;											//0x0028
	QWORD m_nItems;															//0x0030
};
IS_SIZE_CORRECT(CGameContentDeviceSteam, 56)

/*
Size of is 0x48 (72) bytes
*/
class CGameContentDevice
{
public:
	void* m_pVtbl;														//0x0000
	HandlerBase* m_pHandler;											//0x0008
	QWORD unk0x10;														//0x0010
	DWORD unk0x18;														//0x0018
	void(__fastcall* sub_1407EBE30)(__int64 a1, int *a2);				//0x0020
	void(__fastcall* sub_1407EBEE0)(int a1);							//0x0028
	CHeapInstance** m_ppHeap;											//0x0030 | probably a parent pointer
	CGameContentDeviceSteam* m_pSteamContent;							//0x0038
	DWORD unk0x40;														//0x0040
};
IS_SIZE_CORRECT(CGameContentDevice, 72)

/*
Size of is 0x20 (32) bytes
*/
class CAchievementDeviceSteam
{
	void* m_pVtable;					//0x00
	ISteamUser019* m_pUser;				//0x08
	ISteamUserStats011* m_pUserStats;	//0x10
	void* m_pUnknown;					//0x18
};
IS_SIZE_CORRECT(CAchievementDeviceSteam, 32)

/*
Size of is 0x68 (104) bytes
*/
class CAchievementDevice
{
	QWORD qw0x00;
	QWORD qw0x08;
	DWORD dw0x10;
	DWORD dw0x14;
	QWORD qw0x18;
	QWORD qw0x20;
	CRITICAL_SECTION m_CriticalSection;
	DWORD bCriticalSectionValid;
	CAchievementDeviceSteam* m_pAchievementDeviceSteam;
	DWORD dw0x60;
};
IS_SIZE_CORRECT(CAchievementDevice, 104)

/*
Size of is 0x20 (32) bytes
*/
class CUserSignInProcess
{
public:
	void* m_pVtbl;			//0x00
	void* pUnk0x08;			//0x08
	char unknown[16];		//0x10
};
IS_SIZE_CORRECT(CUserSignInProcess, 32)

/* 
This struct is an oddball and is quite fucked

Size of is 0x18 (24) bytes
*/
class CGameBootProcess
{
public:
	void* m_pVtbl;			//0x00
	char unknown[16];		//0x08
};
IS_SIZE_CORRECT(CGameBootProcess, 24)

class YorhaManager
{
public:
	void* m_pvtable;				//0x0000
	Array<EntityHandle> m_handles;	//0x0008
};

class FlightSuitManager
{
public:
	virtual void function0(); //maybe constructor
	virtual EntityHandle GetHandle(Entity_t* pEntity);
	virtual Array<EntityHandle>* GetHandles();
	virtual Entity_t* GetClosestEntity(Vector3Aligned* pvPosition, Entity_t* pEntity);
	virtual unsigned int CountEntitiesWithinDistance(Vector3Aligned* pvPosition, float max_distance);
	virtual void function4();

	//void* m_pvtable;				//0x0000
	Array<EntityHandle> m_handles;	//0x0008
};

/* incomplete getters say this exceeds 0x434 */
class NPCManager
{
public:
	virtual void function0(); //maybe constructor
	virtual EntityInfo* sub_1439C5CA0(Entity_t* pEntity);
	virtual Array<EntityHandle>* GetHandles();
	virtual __int64 GetField428();
	virtual DWORD* GetField42C(OUT DWORD* a1);
	virtual BOOL GetField430(); // *(_DWORD *)(pNPCManager + 0x430) = 1;
	virtual EntityHandle* GetEntityHandle(OUT EntityHandle* pHandle); //field - 0x434
	virtual void function7();
	virtual void function8();

	//void* m_pvtable;				//0x0000
	Array<EntityHandle> m_handles;	//0x0008
};

class EmBaseManager
{
public:
	virtual void function0(); //maybe constructor
	virtual void function1();
	virtual void function2();
	virtual void function3();
	virtual void function4();
	virtual void function5();
	virtual void InvalidateHandle(); //sets 0x14bc handle to 0
	virtual void function7();
	virtual void function8();
	virtual Array<EntityHandle>* GetHandles();
	virtual unsigned int GetHandleIndex();
	virtual Array<EntityHandle>* GetHandles2();
	virtual Entity_t* GetEntity();  // from the 0x14bc handle
	virtual Entity_t* GetEntity2(); // from the 0x14bc handle
	virtual void* GetAddressOfField858(); //probably another Array<EntityHandle>
	virtual void SetField14B8(float a1);
	virtual Array<EntityHandle>* GetHandles3();
	virtual Entity_t* function17(Vector3Aligned* pvPosition, int a2);
	virtual Array<EntityHandle>* GetHandles4();
	virtual Entity_t* function19(Vector3Aligned* pvPosition, int a2);
	virtual Entity_t* function20(Vector3Aligned* pvPosition, int a2);
	virtual bool function21(Entity_t* pEntity);
	virtual bool function22(EntityInfo* pInfo); //calls/returns vfunc 21
	virtual bool function23(EntityHandle* pHandle); //calls/returns vfunc 21
	virtual bool function24(Entity_t* pEntity);
	virtual bool function25(EntityInfo* pInfo);//calls/returns vfunc 24
	virtual bool function26(EntityHandle* pHandle); //calls/returns vfunc 24
	virtual bool IsEntityValid(); //return GetEntity() != 0;
	virtual EntityHandle function28(EntityInfo* pInfo); //calls/returns IsEntityValid (vfunc 27)
	virtual void function29(EntityHandle* pHandle);
	virtual bool function30(Entity_t* pEntity);
	virtual bool function31(EntityInfo* pInfo); //calls/returns vfunc 30
	virtual bool function32(EntityHandle* pHandle); //calls/returns vfunc 30
	virtual void function33();
	virtual void function34();
	virtual bool IsFloatField14C8LessThanOrEqualToZero(); //return *(float *)(a1 + 0x14C8) <= 0.0;
	virtual void function36();
	virtual unsigned int GetField14CC();
	virtual unsigned int GetField14D0();
	virtual BOOL function39(); //could be bool tbh but uses eax not al
	virtual bool IsField1508GreaterThanZero(); //return *(_DWORD *)(a1 + 0x1508) > 0;
	virtual unsigned int CountEntitiesFromHandle2ModelType(); //sub_1434B1740
	virtual unsigned int CountEntitiesFromHandle2();		  //sub_1434B5B10
	virtual void function43(int a2);
	virtual void function44();

	//void* m_pvtable;				//0x0000
	Array<EntityHandle> m_handles;	//0x0008
	char _0x0028[1024];				//0x0028
	Array<EntityHandle> m_handles2;	//0x0428
	char _0x0x0448[2096];			//0x0448
	Array<EntityHandle> m_handles3; //0x0C78
	char _0x0x0C98[1024];			//0x0C98
	Array<EntityHandle> m_handles4; //0x1098
	char _0x0x10B8[1028];			//0x10B8
	EntityHandle m_hEntity;			//0x14BC
};
IS_OFFSET_CORRECT(EmBaseManager, m_handles, 0x8)
IS_OFFSET_CORRECT(EmBaseManager, m_handles2, 0x428)
IS_OFFSET_CORRECT(EmBaseManager, m_handles3, 0xC78)
IS_OFFSET_CORRECT(EmBaseManager, m_hEntity, 0x14BC)

/* Size of struct 0x108 (264) bytes	*/
class CUserManager
{
public:
	void* m_pVtable;							//0x0000
	CRITICAL_SECTION m_CriticalSection;			//0x0008
	BOOL m_bCriticalSectionValid;				//0x0030
	char alignment[4];							//0x0034
	CUserInfo m_UsersInfo[4];					//0x0038
	int m_iActiveUser;							//0x00B8
	char unk0x00BC[12];							//0x00BC
	DWORD dwUnknown;							//0x00C8
	char unk0x00CC[12];							//0x00CC
	CGameBootProcess* m_pBootProcess;			//0x00D8
	CUserSignInProcess* m_pUserSignInProcess;	//0x00E0
	CGameContentDevice* m_pGameContentDevice;	//0x00E8
	CSaveDataDevice* m_pSaveDataDevice;			//0x00F0
	CAchievementDevice* m_pAchivementDevice;	//0x00F8
	DWORD _0x100;								//0x0100
};
IS_SIZE_CORRECT(CUserManager, 264)
IS_OFFSET_CORRECT(CUserManager, m_UsersInfo, 0x38)
IS_OFFSET_CORRECT(CUserManager, m_iActiveUser, 0xB8)
IS_OFFSET_CORRECT(CUserManager, m_pBootProcess, 0xD8)
IS_OFFSET_CORRECT(CUserManager, m_pSaveDataDevice, 0xF0)