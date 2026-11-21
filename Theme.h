#pragma once

#include "WinInclude.h"

#include <stdint.h>

#include "imgui/imgui.h"

// Menu theme: a preset (the base palette and style) plus per-colour overrides on top.
//
// The style is always rebuilt from a fresh ImGuiStyle, so every ImGuiCol - including ones
// a newer ImGui adds - starts from ImGui's own default instead of whatever an older build of
// this code happened to leave in the live style.

enum ThemePreset : int32_t
{
	THEME_DEFAULT,	// the classic 2B Hook green/blue, built from s_DefaultAccent(Bg)
	THEME_YORHA,	// NieR:Automata's menu palette
	THEME_CUSTOM,	// the classic palette built from the user's two accent colours
	THEME_PRESET_COUNT
};

struct ThemeSettings
{
	ThemeSettings(void);

	bool operator==(const ThemeSettings& Other) const;
	bool operator!=(const ThemeSettings& Other) const { return !(*this == Other); }

	// Clears the per-colour overrides, and for the Custom preset its accent colours too.
	void Reset(void);

	ThemePreset m_Preset;

	// Custom preset only. Stored in <Menu> as i_theme_fg / i_theme_bg, where they lived
	// before the presets existed.
	ImColor m_Accent;
	ImColor m_AccentBg;

	bool m_bOverride[ImGuiCol_COUNT];
	ImVec4 m_Overrides[ImGuiCol_COUNT];

	// UI scale. Auto follows the window's DPI and the game's resolution (1080p = 1x).
	bool m_bAutoScale;
	float m_flScale;
};

namespace Theme
{
	// Alpha below this counts as invisible. It used to take exactly zero, so an alpha of
	// 1/255 from a hand-edited or damaged config still hid the menu.
	constexpr float MIN_VISIBLE_ALPHA = 0.1f;

	constexpr float MIN_SCALE = 0.5f;
	constexpr float MAX_SCALE = 4.0f;

	extern const ImColor s_DefaultAccent;
	extern const ImColor s_DefaultAccentBg;

	const char* GetPresetName(ThemePreset Preset);

	// "default" / "yorha" / "custom", as stored in the config
	const wchar_t* GetPresetKey(ThemePreset Preset);
	ThemePreset PresetFromKey(const wchar_t* szKey);

	// Builds the complete style for these settings, unscaled.
	void Build(const ThemeSettings& Settings, ImGuiStyle& Style);

	// Repairs near-invisible colours (the Custom accents, and Text/WindowBg overrides) that
	// would hide the menu. Returns a description of what was repaired, or NULL.
	const char* Sanitize(ThemeSettings& Settings);

	// The UI scale to use: the manual one, or for auto the larger of the window's DPI scale
	// and the client height relative to 1080p, in 0.25 steps.
	float GetTargetScale(const ThemeSettings& Settings, HWND hWnd);

	// "#RRGGBBAA", the format colour overrides are stored in.
	void FormatColor(const ImVec4& Color, wchar_t (&szBuffer)[16]);
	bool ParseColor(const wchar_t* szValue, ImVec4& Color);
}
