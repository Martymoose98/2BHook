#pragma once

#include "Common.h"

#include "Entity/Pl0000.h"

#define CAMERA_FLAGS_DONT_UPDATE_VIEWANGLES 0x01000000
#define CAMERA_FLAGS_LOCK					0x80000000 // Free cam?

// NOTE: camera types 0x390
//0 default
//1 offset right
//2 far
//5 side far
//6 bottom up view
//7 distanced normal
//8, 9  (slightly farther that default ? )
//10 normal up far
//11 free cam(restrict up)
//12 fixed top down
//13 - 22 (inc)fixed point normal
//23 normal far view
//24 rotating far view
//25 side very far
//26 waterfall ? ?
//27, 29 isometric view far ? (same handler)
//28, 30 isometric view fixed far
//31 rotating skybox cam
//32 fixed circular buffer cam
//33 static cam
//34 spaz top down view
//35 close angled chase view
//36 close fixed cam
//37 close normal cam
//38 arcball very close view(head view)
//39 arcball far view
//40 restricted arcball close view
//41 angled top down far view
//42 floating up top down view
//43 fixed origin view ? ? (-81, 0, 0)
//44 fixed view low poly terrain (menu)
//45 normal slight rotation arcball(default) ?
//46 normal slight rotation arcball(default) ?
// TODO: finish
enum CameraType
{
	CAM_DEFAULT = 0,
	CAM_OFFSET_RIGHT = 1,
	CAM_FAR = 2,
	CAM_PATH_SIDE = 3,
	CAM_SIDE_FAR = 5,
	CAM_BOTTOM_UP = 6,
	CAM_DISTANCED_NORMAL = 7,
	CAM_UP_FAR_NORMAL = 10,
	CAM_FREE_RESTRICT_UP = 11,
	CAM_STATIC_TOP_DOWN = 12,
	CAM_STATIC_POINT = 13,
	CAM_FAR_NORMAL = 23,
	CAM_ROTATING_FAR_NORMAL = 24,
	CAM_SIDE_VERY_FAR = 25,
};

struct CCameraInstance
{
	Matrix4x4 m_View;					//0x0000 | probably inverted or smth / m_ViewMatrix
	Matrix4x4 m_Projection;				//0x0040 | correct
	Matrix4x4 m_ViewProjection;			//0x0080 | correct
	Matrix4x4 m_World;					//0x00C0 | world matrix (camera)
	char pad0xC0[256];					//0x0100
	Matrix4x4 m_OldViewProjection;		//0x0200 | save
	void* m_ptr240;						//0x0240
	char pad0x248[24];					//0x0248
	int m_iWidth;
	int m_iHeight;
	float m_flZNear;
	float m_flZFar;
	float m_flAspectRatio;
	float m_flFov;
	float pad[4];
	BOOL m_bSetViewProjection;			//0x0284
	BOOL m_bSetViewProjection2;			//0x0288
	BOOL m_bSetViewProjection3;			//0x028C
	char pad0x290[44];					//0x0290
	float m_flFovY;						//0x02BC
};


struct __declspec(align(16)) CCameraTransform
{
	virtual ~CCameraTransform();
	//virtual QWORD* CCameraTransform::sub_140184180(char a2);

	//void* m_vtable;				//0x0000
	//char alignment8[8];			//0x0008 | align 16 padding tbh
	Vector3Aligned m_vSource;		//0x0010
	Vector3Aligned m_vTarget;		//0x0020
	Vector3Aligned m_vUp;			//0x0030
	Vector3Aligned m_vRotation;		//0x0040 | cam rotation
	float m_flDistance;				//0x0050 | cam distance__m128 most likely
	//char alignment54[12];			//0x0054
};
VALIDATE_OFFSET(CCameraTransform, m_vSource, 0x10);

struct __declspec(align(16)) CCameraBase : public CCameraTransform
{
	virtual ~CCameraBase();
	//void* m_vtable;				//0x0000
	//char alignment8[8];			//0x0008 | align 16 padding tbh
	//CCameraTransform m_Transform; //0x0010
	CCameraInstance* m_pInstance;	//0x0060
};
VALIDATE_OFFSET(CCameraBase, m_pInstance, 0x60);

/*

141020870

https://www.braynzarsoft.net/viewtutorial/q16390-34-aabb-cpu-side-frustum-culling

This struct is 16 byte aligned
*/
class __declspec(align(16)) CCameraGame : public CCameraBase
{
public:
	typedef float NormalCameraFloatOffset;

	virtual void* function0(char a1) PURE;
	virtual BOOL SetViewAngles(void) PURE;
	virtual void* Move(void) PURE;

	//void* m_vtable;									//0x0000
	//char align8[8];									//0x0008 | align 16 padding tbh
	//CCameraBase m_Base;								//0x0010
	//char align68[8];									//0x0068 | probably part of CCameraBase alignment
	Vector3Aligned m_v0x70;								//0x0070
	Vector3Aligned m_v0x80;								//0x0080
	Vector3Aligned m_v0x90;								//0x0090
	float m_fl0xA0;										//0x00A0
	float m_fl0xA4;										//0x00A4
	Pl0000* m_pLocalPlayer;								//0x00A8 | probs wrong
	DWORD dwUnk;										//0x00B0
	char _0x00B4[28];									//0x00B4
	EntityHandle m_hEntity;								//0x00D0
	DWORD alignment;									//0x00D4
	Pl0000* m_pEntity;									//0x00D8
	Pl0000* m_pEntity2;									//0x00E0
	QWORD align;										//0x00E8
	Vector3Aligned m_vPosition;							//0x00F0 | CamPos: Debug Build: target0 (gets copied to transform vSource)
	Vector3Aligned m_vTarget2;							//0x0100 | CamView: Debug Build: pos0 (gets copied to transform vTarget)
	Vector3Aligned m_vUnk2;								//0x0110
	Vector3Aligned m_vUp2;								//0x0120
	float m_flRotationZ;								//0x0130
	float m_flDistance;									//0x0134
	float m_flFov;										//0x0138
	char _0x13C[4];										//0x013C
	Vector3Aligned m_vPositionOffset;					//0x0140 | Debug Build: target1
	Vector3Aligned m_vTargetOffset;						//0x0150 | Debug Build: pos1
	Vector3Aligned m_vUnk3;								//0x0160
	Vector3Aligned m_vGlobalUp;							//0x0170
	float m_flRotationZ2;								//0x0180
	float m_flDistance2;								//0x0184
	float m_flFov2;										//0x0188
	float m_flUnk;										//0x018C
	BOOL m_bCameraShake;								//0x0190
	char _0x0194[12];									//0x0194
	Vector3Aligned m_vShake;							//0x01A0
	Vector3Aligned m_vShake2;							//0x01B0
	char _0x01C0[128];									//0x01C0
	//0x0200 | world to screen matrix??
	void* m_pUnk;										//0x0240
	char _0x0248[56];									//0x0248
	Matrix4x4 m_Frustrum;								//0x0280
	char _0x02C0[176];									//0x02C0
	//float m_flFovy;									//0x02BC
	CBehaviorAppBase* m_pCamEntity;						//0x0370
	char _0x0378[24];									//0x0378
	CameraType m_CamType;								//0x0390
	char _0x0394[92];									//0x0394
	Vector3 m_vViewangles;								//0x03F0 | radians (p, y, r) / m_target_ang
	char pad3FC[44];									//0x03FC
	StaticArray<NormalCameraFloatOffset, 2> m_Offsets;	//0x0428 | lib::StaticArray<NormalCameraFloatOffset,2,4>::`vftable'
	char pad450[33788];									//0x0450
	//0x04AC | affects viewangle pitch
	//0x04B0 | affects viewangle yaw
	float m_flUnknown2;									//0x884C
	char pad8850[160];									//0x8850
	float m_flUnknown;									//0x88F0
	char pad88F4[92];									//0x88F4
	Vector3Aligned m_vUnknown[2];						//0x8950
};
VALIDATE_OFFSET(CCameraGame, m_v0x70, 0x70);
VALIDATE_OFFSET(CCameraGame, m_pLocalPlayer, 0xA8);
VALIDATE_OFFSET(CCameraGame, m_hEntity, 0xD0);
VALIDATE_OFFSET(CCameraGame, m_vPosition, 0xF0);
VALIDATE_OFFSET(CCameraGame, m_bCameraShake, 0x190);
VALIDATE_OFFSET(CCameraGame, m_vShake, 0x1A0);
VALIDATE_OFFSET(CCameraGame, m_Frustrum, 0x0280);
VALIDATE_OFFSET(CCameraGame, m_pCamEntity, 0x0370);
VALIDATE_OFFSET(CCameraGame, m_CamType, 0x390);
VALIDATE_OFFSET(CCameraGame, m_vViewangles, 0x3F0);
VALIDATE_OFFSET(CCameraGame, m_flUnknown2, 0x884C);
VALIDATE_OFFSET(CCameraGame, m_flUnknown, 0x88F0);
