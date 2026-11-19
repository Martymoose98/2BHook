#pragma once

#include <Windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <vector>

#include "Configuration.h"

// The Spawn tab's entity list. It is built from the game's own construction info table
// (g_ConstructionInfoList), so it has every object the running build can construct: one row per
// object id that GetConstructionInfo matches whole, grouped by the id ranges the game names its
// objects by. An id that has no row of its own (GetConstructionInfo builds it with the pattern
// row for its range) can be typed into the search box as 0x....
//
// Header-only like Features.h, and only the menu includes it.
namespace SpawnList
{
	enum Assets_t : uint8_t
	{
		ASSETS_UNCHECKED,	// not looked at yet
		ASSETS_UNCHECKABLE,	// the game's checks weren't found on this build
		ASSETS_NOT_LOADED,	// its files, or a dependency's, aren't in memory on this map
		ASSETS_RESIDENT,
	};

	struct Entry_t
	{
		uint32_t m_ObjectId;
		const char* m_szClass;		// its construction row's class (CConstructionInfo::m_szName)
		const char* m_szName;		// its name in the old hand-made list (Variables_t::SpawnEntities), or NULL
		const char* m_szBlocked;	// why it is never spawned, or NULL
		char m_szObjectName[16];	// the game's name for the id: "em1000" for 0x21000, whose files are em1000.dat/.dtt
		char m_szSpawnName[32];		// what the entity's CEntityInfo::m_szEntityType becomes
		char m_szSearch[112];		// names, class and hex id in lower case, for the search box
		int m_iGroup;
		bool m_bOwnRow;				// false for a typed id that only a pattern row builds
		Assets_t m_Assets;			// as of the last check, for the list's marks and its Loaded Only box
	};

	namespace Detail
	{
		struct Group_t
		{
			uint32_t m_uFirst;
			uint32_t m_uLast;
			const char* m_szName;
		};

		// The list's groups, first match wins. The game names an object by the high word of its id:
		// g_ObjectIdConvert (0x140F64980) has twelve prefixes, {0x10000, "pl"}, {0x20000, "em"},
		// {0x30000, "wp"}, {0x40000, "et"}, {0x50000, "ef"}, {0x60000, "es"}, {0x70000, "it"},
		// {0x90000, "sc"}, {0xA0000, "um"}, {0xC0000, "bg"}, {0xE0000, "bh"}, {0xF0000, "ba"}, and
		// ObjectIdToObjectName (0x14084AD70) appends the low word, so 0x21000 is em1000. pl and wp are
		// split by the classes the construction table gives their rows: pl0xxx are Pl0000 (2B 0x10000,
		// A2 0x10100, 9S 0x10200, and 0x10300 to 0x10600; the port's BehaviorFunnel.h:25 calls 0x10300
		// and 0x10400 YoRHa NPCs), PlParts, BehaviorFlightUnit and Pl0013; pl1xxx and pl2xxx are NPC and
		// Pl1010 (Emil, 0x11010); plfxxx are BehaviorBa and a Pl0000. wp3xxx and wp4xxx are
		// BehaviorFunnel, the pod (Pl0000_RespawnSupportUnits, 0x140484550, adds 0x33000 at
		// 0x1404846E3 and keeps the handle in m_hPods; the port's BehaviorFunnel.h:10-14), PodMask and
		// the pod skills' models. The other descriptions name the classes the table gives each range.
		inline const Group_t s_Groups[] =
		{
			{ 0x10000, 0x10FFF, "Players and other Pl0000 characters (pl0xxx)" },
			{ 0x11000, 0x1EFFF, "NPCs (pl1xxx, pl2xxx)" },
			{ 0x1F000, 0x1FFFF, "Other player objects (plfxxx)" },
			{ 0x20000, 0x2FFFF, "Enemies (em)" },
			{ 0x33000, 0x34FFF, "Pods and pod skills (wp3xxx, wp4xxx)" },
			{ 0x30000, 0x3FFFF, "Weapons and bullets (wp)" },
			{ 0x40000, 0x4FFFF, "Drops, fish, hacking enemies, masks (et)" },
			{ 0x50000, 0x5FFFF, "Effects (ef)" },
			{ 0x60000, 0x6FFFF, "EsBase objects (es)" },
			{ 0x70000, 0x7FFFF, "Items (it)" },
			{ 0x90000, 0x9FFFF, "Map pieces (sc)" },
			{ 0xA0000, 0xAFFFF, "Enemies and Commun characters (um)" },
			{ 0xC0000, 0xCFFFF, "Backgrounds, hacking stages, props (bg)" },
			{ 0xE0000, 0xEFFFF, "BehaviorBh (bh)" },
			{ 0xF0000, 0xFFFFF, "Doors, gates, lifts, props (ba)" },
			{ 0x00000, 0xFFFFFFFF, "Other ids" },	// outside every prefix, like 0x700001
		};

		constexpr int GROUP_COUNT = (int)ARRAYSIZE(s_Groups);

		// Entries whose assets are checked per Update: the whole list in about eighteen frames, at a
		// few lookups under the read system's locks each
		constexpr int CHECKS_PER_UPDATE = 64;

		inline std::vector<Entry_t> s_Entries;
		inline const CConstructionInfo<void>* s_pBuiltTable;
		inline uint32_t s_uBuiltRows;
		inline DWORD s_dwCheckedSetType;
		inline size_t s_uNextCheck;

		inline std::vector<int> s_GroupRows[GROUP_COUNT];
		inline int s_nGroupSize[GROUP_COUNT];
		inline int s_nGroupLoaded[GROUP_COUNT];

		inline char s_szFilter[64];
		inline bool s_bFilterLoadedOnly;
		inline bool s_bFilterDirty = true;

		inline Entry_t s_Typed;
		inline bool s_bTyped;

		inline int GetGroup(uint32_t ObjectId)
		{
			for (int i = 0; i < GROUP_COUNT; ++i)
			{
				if (ObjectId >= s_Groups[i].m_uFirst && ObjectId <= s_Groups[i].m_uLast)
					return i;
			}

			return GROUP_COUNT - 1;
		}

		// The ids that have no read data of their own, as CObjReadSystem_HasNoReadData (0x14084E650)
		// defines them: the no-preload list g_ObjNoPreloadIds (0x140F64910: 0x50000, 0x50001, 0x400FD,
		// 0x700000, -1) and every ef (0x5xxxx) and sc (0x9xxxx) id. The game's checks answer 1 for them
		// without looking, because whoever creates them brings their data. MapElement_GoLive
		// (0x1408363E0) makes the map piece 0x90001 ("MapInstance", BehaviorScr) from the streamed
		// piece's own wmb, dat buffers and ObjectParam.bxm (Create_t flag 0x40000000), and
		// EffectModelEntity_Create (0x140750840) makes ef ids in the effect system's own spawn system
		// and gives each its owner effect. A free spawn has none of that - for a map piece
		// CObj_LoadTexturesAndModel (0x14084D310) falls back to the dummy WMB - and its assets can't
		// be checked, so these are never spawned.
		inline const char* GetBlockReason(uint32_t ObjectId)
		{
			static const uint32_t NoPreloadIds[] = { 0x50000, 0x50001, 0x400FD, 0x700000 };

			const uint32_t uPrefix = ObjectId & 0xFFFF0000;

			if (uPrefix == 0x90000)
				return "A map piece (sc). The game creates these from a streamed map piece, with that piece's own model and data; a free spawn has neither, and its assets can't be checked.";

			if (uPrefix == 0x50000)
				return "An effect (ef). The game creates these as part of an effect, which gives them their model and owner; a free spawn has neither, and its assets can't be checked.";

			for (uint32_t uId : NoPreloadIds)
			{
				if (ObjectId == uId)
					return "On the game's no-preload list: it has no files of its own, so whatever creates it supplies its data, and its assets can't be checked.";
			}

			return NULL;
		}

		inline const Variables_t::Spawn* FindSeed(uint32_t ObjectId)
		{
			for (const Variables_t::Spawn& Seed : Variables_t::SpawnEntities)
			{
				if (Seed.m_ObjectId == ObjectId)
					return &Seed;
			}

			return NULL;
		}

		inline void FillEntry(Entry_t& Entry, uint32_t ObjectId, const CConstructionInfo<void>* pRow)
		{
			const Variables_t::Spawn* pSeed = FindSeed(ObjectId);
			ObjectIdConvert* pConvert = NULL;

			Entry = Entry_t();
			Entry.m_ObjectId = ObjectId;
			Entry.m_szClass = (pRow && pRow->m_szName) ? pRow->m_szName : "";
			Entry.m_szName = (pSeed) ? pSeed->m_szName : NULL;
			Entry.m_szBlocked = GetBlockReason(ObjectId);
			Entry.m_iGroup = GetGroup(ObjectId);
			Entry.m_bOwnRow = pRow && pRow->m_uObjectId == ObjectId;

			// The old list's entity names where it had the id ("partner" for 2B, A2 and 9S), the class
			// otherwise. Features::CreateEntity cuts it to what CEntityInfo::m_szEntityType holds.
			strncpy_s(Entry.m_szSpawnName, (pSeed) ? pSeed->m_szClass : Entry.m_szClass, _TRUNCATE);

			// Its table is the game's g_ObjectIdConvert, entry for entry
			if (!ObjectIdToObjectName(Entry.m_szObjectName, sizeof(Entry.m_szObjectName), ObjectId, &pConvert))
				Entry.m_szObjectName[0] = '\0';

			_snprintf_s(Entry.m_szSearch, _TRUNCATE, "%s %s %s 0x%x",
				(Entry.m_szName) ? Entry.m_szName : "", Entry.m_szObjectName, Entry.m_szClass, ObjectId);
			_strlwr_s(Entry.m_szSearch);
		}

		inline void CountGroups(void)
		{
			memset(s_nGroupSize, 0, sizeof(s_nGroupSize));
			memset(s_nGroupLoaded, 0, sizeof(s_nGroupLoaded));

			for (const Entry_t& Entry : s_Entries)
			{
				++s_nGroupSize[Entry.m_iGroup];

				if (!Entry.m_szBlocked && Entry.m_Assets == ASSETS_RESIDENT)
					++s_nGroupLoaded[Entry.m_iGroup];
			}
		}

		// The rows GetConstructionInfo matches whole ids over: a row is listed if GetConstructionInfo
		// returns it for its own id, which also drops a repeated id (0xA200D is in the table twice
		// on build 26534214; the lookup stops at the first). The table is never written by the game
		// (the only xref to it is GetConstructionInfo's lea), so this runs once.
		inline bool Build(void)
		{
			if (!g_pConstructionInfoList || !g_uConstructionInfoRows || !GetConstructionInfo)
				return false;

			if (s_pBuiltTable == g_pConstructionInfoList && s_uBuiltRows == g_uConstructionInfoRows)
				return true;

			s_Entries.clear();
			s_Entries.reserve(g_uConstructionInfoRows);

			for (uint32_t i = 0; i < g_uConstructionInfoRows; ++i)
			{
				const CConstructionInfo<void>* pRow = &g_pConstructionInfoList[i];

				if (GetConstructionInfo(pRow->m_uObjectId) != pRow)
					continue;

				s_Entries.emplace_back();
				FillEntry(s_Entries.back(), pRow->m_uObjectId, pRow);
			}

			// The table's own order is close to sorted, but not within a range (0x1000F follows 0x10600)
			std::sort(s_Entries.begin(), s_Entries.end(), [](const Entry_t& a, const Entry_t& b)
			{
				return (a.m_iGroup != b.m_iGroup) ? a.m_iGroup < b.m_iGroup : a.m_ObjectId < b.m_ObjectId;
			});

			s_pBuiltTable = g_pConstructionInfoList;
			s_uBuiltRows = g_uConstructionInfoRows;
			s_uNextCheck = 0;
			s_bTyped = false;
			s_bFilterDirty = true;

			CountGroups();

			return true;
		}
	}

	// The entry's assets now, the way Create Entity checks them (Features::GetSpawnAssets)
	inline Assets_t CheckAssets(const Entry_t& Entry, DWORD dwSetType)
	{
		switch (Features::GetSpawnAssets(Entry.m_ObjectId, dwSetType))
		{
		case Features::SPAWN_ASSETS_RESIDENT:
			return ASSETS_RESIDENT;
		case Features::SPAWN_ASSETS_NOT_LOADED:
			return ASSETS_NOT_LOADED;
		default:
			return ASSETS_UNCHECKABLE;
		}
	}

	// Builds the list the first time, and again only if the table moves or changes size, then checks
	// the assets of the next few entries. False if the table wasn't found.
	inline bool Update(DWORD dwSetType)
	{
		using namespace Detail;

		if (!Build())
			return false;

		if (s_Entries.empty())
			return true;

		// The checks depend on the set type (it picks the dependency record), so a new one starts over
		if (dwSetType != s_dwCheckedSetType)
		{
			for (Entry_t& Entry : s_Entries)
				Entry.m_Assets = ASSETS_UNCHECKED;

			s_dwCheckedSetType = dwSetType;
			s_uNextCheck = 0;
		}

		bool bChanged = false;

		for (int n = 0; n < CHECKS_PER_UPDATE && n < (int)s_Entries.size(); ++n)
		{
			Entry_t& Entry = s_Entries[s_uNextCheck];

			s_uNextCheck = (s_uNextCheck + 1) % s_Entries.size();

			// Never spawned, so not asked about
			if (Entry.m_szBlocked)
				continue;

			const Assets_t Assets = CheckAssets(Entry, dwSetType);

			if (Assets != Entry.m_Assets)
			{
				Entry.m_Assets = Assets;
				bChanged = true;
			}
		}

		if (bChanged)
		{
			CountGroups();

			if (s_bFilterLoadedOnly)
				s_bFilterDirty = true;
		}

		return true;
	}

	// The search box (space-separated terms, all of which must match) and the Loaded Only box.
	// Cheap when neither changed.
	inline void SetFilter(const char* szSearch, bool bLoadedOnly)
	{
		using namespace Detail;

		if (!s_bFilterDirty && bLoadedOnly == s_bFilterLoadedOnly && !strcmp(szSearch, s_szFilter))
			return;

		strncpy_s(s_szFilter, szSearch, _TRUNCATE);
		s_bFilterLoadedOnly = bLoadedOnly;
		s_bFilterDirty = false;

		char szTerms[sizeof(s_szFilter)];
		const char* Terms[16];
		int nTerms = 0;
		char* pContext = NULL;

		strcpy_s(szTerms, s_szFilter);
		_strlwr_s(szTerms);

		for (char* pTerm = strtok_s(szTerms, " \t", &pContext); pTerm && nTerms < (int)ARRAYSIZE(Terms); pTerm = strtok_s(NULL, " \t", &pContext))
			Terms[nTerms++] = pTerm;

		for (std::vector<int>& Rows : s_GroupRows)
			Rows.clear();

		for (int i = 0; i < (int)s_Entries.size(); ++i)
		{
			const Entry_t& Entry = s_Entries[i];

			if (bLoadedOnly && (Entry.m_szBlocked || Entry.m_Assets != ASSETS_RESIDENT))
				continue;

			bool bMatch = true;

			for (int t = 0; t < nTerms && bMatch; ++t)
				bMatch = strstr(Entry.m_szSearch, Terms[t]) != NULL;

			if (bMatch)
				s_GroupRows[Entry.m_iGroup].push_back(i);
		}
	}

	inline int GetGroupCount(void)
	{
		return Detail::GROUP_COUNT;
	}

	inline const char* GetGroupName(int iGroup)
	{
		return Detail::s_Groups[iGroup].m_szName;
	}

	inline int GetGroupSize(int iGroup)
	{
		return Detail::s_nGroupSize[iGroup];
	}

	inline int GetGroupLoaded(int iGroup)
	{
		return Detail::s_nGroupLoaded[iGroup];
	}

	// The group's entries that pass the filter, as indices for GetEntry
	inline const std::vector<int>& GetGroupRows(int iGroup)
	{
		return Detail::s_GroupRows[iGroup];
	}

	inline const Entry_t& GetEntry(int i)
	{
		return Detail::s_Entries[i];
	}

	// The list's entry for an id, or NULL
	inline const Entry_t* Find(uint32_t ObjectId)
	{
		for (const Entry_t& Entry : Detail::s_Entries)
		{
			if (Entry.m_ObjectId == ObjectId)
				return &Entry;
		}

		return NULL;
	}

	// The list's entry for an id or, for an id with no row of its own, one made the way
	// GetConstructionInfo resolves it. NULL without GetConstructionInfo.
	inline const Entry_t* Lookup(uint32_t ObjectId)
	{
		using namespace Detail;

		if (const Entry_t* pEntry = Find(ObjectId))
			return pEntry;

		if (!GetConstructionInfo)
			return NULL;

		if (!s_bTyped || s_Typed.m_ObjectId != ObjectId)
		{
			FillEntry(s_Typed, ObjectId, GetConstructionInfo(ObjectId));
			s_bTyped = true;
		}

		return &s_Typed;
	}
}
