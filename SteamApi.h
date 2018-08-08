#pragma once

typedef int HSteamPipe;
typedef int HSteamUser;
typedef unsigned long long SteamAPICall_t;

class CCallbackBase
{
public:

	CCallbackBase() { m_nCallbackFlags = 0; m_iCallback = 0; }

	// don't add a virtual destructor because we export this binary interface across dll's

	virtual void Run(void *pvParam) = 0;

	virtual void Run(void *pvParam, bool bIOFailure, SteamAPICall_t hSteamAPICall) = 0;

	int GetICallback() { return m_iCallback; }

	virtual int GetCallbackSizeBytes() = 0;

protected:

	enum { k_ECallbackFlagsRegistered = 0x01, k_ECallbackFlagsGameServer = 0x02 };

	unsigned __int8 m_nCallbackFlags;
	int m_iCallback;
	friend class CCallbackMgr;
};

template< class T, class P, bool bGameServer >
class CCallback : private CCallbackBase
{
public:
	typedef void (T::*func_t)(P*);

	// If you can't support constructing a callback with the correct parameters
	// then uncomment the empty constructor below and manually call ::Register() for your object
	// Or, just call the regular constructor with (NULL, NULL)
	
	// CCallback() {}

	// constructor for initializing this object in owner's constructor

	CCallback(T *pObj, func_t func) : m_pObj(pObj), m_Func(func)
	{
		if (pObj && func)
			Register(pObj, func);
	}

	~CCallback()
	{
		if (m_nCallbackFlags & k_ECallbackFlagsRegistered)
			Unregister();
	}

	// manual registration of the callback

	void Register(T *pObj, func_t func)
	{
		if (!pObj || !func)
			return;

		if (m_nCallbackFlags & k_ECallbackFlagsRegistered)
			Unregister();

		if (bGameServer)
		{
			m_nCallbackFlags |= k_ECallbackFlagsGameServer;
		}

		m_pObj = pObj;
		m_Func = func;

		// SteamAPI_RegisterCallback sets k_ECallbackFlagsRegistered

		SteamAPI_RegisterCallback(this, P::k_iCallback);
	}

	void Unregister()
	{
		// SteamAPI_UnregisterCallback removes k_ECallbackFlagsRegistered
		SteamAPI_UnregisterCallback(this);
	}

	void SetGameserverFlag() { m_nCallbackFlags |= k_ECallbackFlagsGameServer; }

private:

	virtual void Run(void *pvParam)
	{
		(m_pObj->*m_Func)((P *)pvParam);
	}

	virtual void Run(void *pvParam, bool, SteamAPICall_t)
	{
		(m_pObj->*m_Func)((P *)pvParam);
	}

	int GetCallbackSizeBytes()
	{
		return sizeof(P);
	}

	T *m_pObj;
	func_t m_Func;
};

void SteamAPI_RegisterCallback(CCallbackBase* pCallback, int iCallback);
void SteamAPI_UnregisterCallback(CCallbackBase* pCallback);

struct GameOverlayActivated_t { bool m_bActive; };

class ISteamUser019;
class ISteamUserStats011;