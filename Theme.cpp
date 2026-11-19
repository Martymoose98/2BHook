#include "Theme.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include "ImGui/imgui_internal.h"
#include "ImGui/backends/imgui_impl_win32.h"

const ImColor Theme::s_DefaultAccent = ImColor(7, 74, 25, 255);
const ImColor Theme::s_DefaultAccentBg = ImColor(7, 17, 74, 255);

ThemeSettings::ThemeSettings(void)
	: m_Preset(THEME_DEFAULT), m_Accent(Theme::s_DefaultAccent), m_AccentBg(Theme::s_DefaultAccentBg),
	m_bOverride{}, m_Overrides{}, m_bAutoScale(true), m_flScale(1.0f)
{
}

bool ThemeSettings::operator==(const ThemeSettings& Other) const
{
	if (m_Preset != Other.m_Preset || m_bAutoScale != Other.m_bAutoScale || m_flScale != Other.m_flScale)
		return false;

	if (memcmp(&m_Accent.Value, &Other.m_Accent.Value, sizeof(ImVec4)) ||
		memcmp(&m_AccentBg.Value, &Other.m_AccentBg.Value, sizeof(ImVec4)))
		return false;

	for (int i = 0; i < ImGuiCol_COUNT; ++i)
	{
		if (m_bOverride[i] != Other.m_bOverride[i])
			return false;

		if (m_bOverride[i] && memcmp(&m_Overrides[i], &Other.m_Overrides[i], sizeof(ImVec4)))
			return false;
	}

	return true;
}

void ThemeSettings::Reset(void)
{
	memset(m_bOverride, 0, sizeof(m_bOverride));

	if (m_Preset == THEME_CUSTOM)
	{
		m_Accent = Theme::s_DefaultAccent;
		m_AccentBg = Theme::s_DefaultAccentBg;
	}
}

namespace
{
	ImVec4 WithAlpha(const ImVec4& Color, float flAlpha)
	{
		return ImVec4(Color.x, Color.y, Color.z, flAlpha);
	}

	ImVec4 Opaque(const ImVec4& Color)
	{
		return WithAlpha(Color, 1.0f);
	}

	// Rough perceived brightness of an sRGB colour; only used to pick light or dark text.
	float Luminance(const ImVec4& Color)
	{
		return 0.2126f * Color.x + 0.7152f * Color.y + 0.0722f * Color.z;
	}

	// Rotates the hue (in turns, so 1/12 is 30 degrees) and scales saturation and value,
	// keeping the alpha. This replaces the old HLS-based "analogous" colours, which pushed
	// Windows' 0-240 hue towards 255 (out of range for most colours) and ended up blue or
	// purple whatever the input was.
	ImVec4 Shift(const ImVec4& Color, float flHueTurns, float flSaturation = 1.0f, float flValue = 1.0f)
	{
		float h, s, v;

		ImGui::ColorConvertRGBtoHSV(Color.x, Color.y, Color.z, h, s, v);

		h = fmodf(h + flHueTurns + 1.0f, 1.0f);
		s = ImSaturate(s * flSaturation);
		v = ImSaturate(v * flValue);

		ImVec4 Out(0.0f, 0.0f, 0.0f, Color.w);
		ImGui::ColorConvertHSVtoRGB(h, s, v, Out.x, Out.y, Out.z);

		return Out;
	}

	ImVec4 FromRGB(uint32_t uRGB, float flAlpha = 1.0f)
	{
		return ImVec4(((uRGB >> 16) & 0xFF) / 255.0f, ((uRGB >> 8) & 0xFF) / 255.0f, (uRGB & 0xFF) / 255.0f, flAlpha);
	}

	// WCAG 2's relative luminance and contrast ratio (1:1 to 21:1) of sRGB colours
	float LinearChannel(float flChannel)
	{
		return (flChannel <= 0.04045f) ? flChannel / 12.92f : powf((flChannel + 0.055f) / 1.055f, 2.4f);
	}

	float RelativeLuminance(const ImVec4& Color)
	{
		return 0.2126f * LinearChannel(Color.x) + 0.7152f * LinearChannel(Color.y) + 0.0722f * LinearChannel(Color.z);
	}

	float ContrastRatio(const ImVec4& A, const ImVec4& B)
	{
		const float flA = RelativeLuminance(A);
		const float flB = RelativeLuminance(B);

		return (ImMax(flA, flB) + 0.05f) / (ImMin(flA, flB) + 0.05f);
	}

	// Top drawn over an opaque Bottom: what ends up on screen
	ImVec4 Over(const ImVec4& Top, const ImVec4& Bottom)
	{
		return ImVec4(ImLerp(Bottom.x, Top.x, Top.w), ImLerp(Bottom.y, Top.y, Top.w), ImLerp(Bottom.z, Top.z, Top.w), 1.0f);
	}

	// Color (opaque) as it is if it has at least flMinRatio contrast with Against, otherwise
	// brightened or darkened until it does, whichever needs the smaller change in luminance.
	// Brightening raises the HSV value first and only then mixes in white, so an accent keeps
	// its hue as long as it can. If neither way gets there, the better of white and black.
	ImVec4 EnsureContrast(const ImVec4& Color, const ImVec4& Against, float flMinRatio)
	{
		constexpr int STEPS = 64;
		const ImVec4 White(1.0f, 1.0f, 1.0f, 1.0f);
		const ImVec4 Black(0.0f, 0.0f, 0.0f, 1.0f);

		if (ContrastRatio(Color, Against) >= flMinRatio)
			return Color;

		float h, s, v;
		ImGui::ColorConvertRGBtoHSV(Color.x, Color.y, Color.z, h, s, v);

		const float flStart = RelativeLuminance(Color);
		ImVec4 Best = (ContrastRatio(White, Against) >= ContrastRatio(Black, Against)) ? White : Black;
		float flBestChange = FLT_MAX;

		// True once Candidate is far enough; keeps it if it is the smallest change so far
		auto Consider = [&](const ImVec4& Candidate)
		{
			if (ContrastRatio(Candidate, Against) < flMinRatio)
				return false;

			const float flChange = fabsf(RelativeLuminance(Candidate) - flStart);

			if (flChange < flBestChange)
			{
				Best = Candidate;
				flBestChange = flChange;
			}

			return true;
		};

		ImVec4 FullValue = Black;
		ImGui::ColorConvertHSVtoRGB(h, s, 1.0f, FullValue.x, FullValue.y, FullValue.z);

		for (int i = 1; i <= 2 * STEPS; ++i)
		{
			ImVec4 Candidate = Black;

			if (i <= STEPS)
				ImGui::ColorConvertHSVtoRGB(h, s, ImLerp(v, 1.0f, i / (float)STEPS), Candidate.x, Candidate.y, Candidate.z);
			else
				Candidate = ImLerp(FullValue, White, (i - STEPS) / (float)STEPS);

			if (Consider(Candidate))
				break;
		}

		for (int i = 1; i <= STEPS; ++i)
		{
			ImVec4 Candidate = Black;
			ImGui::ColorConvertHSVtoRGB(h, s, ImLerp(v, 0.0f, i / (float)STEPS), Candidate.x, Candidate.y, Candidate.z);

			if (Consider(Candidate))
				break;
		}

		return Best;
	}

	// The classic 2B Hook palette, built from two colours: the accent for interactive items
	// and the window background. Text and the neutral greys follow the background's
	// brightness, so a light background gets dark text instead of light grey on beige.
	void BuildClassic(const ImVec4& Accent, const ImVec4& AccentBg, ImGuiStyle& Style)
	{
		ImVec4* Colors = Style.Colors;

		const bool bLight = Luminance(AccentBg) > 0.5f;
		const ImVec4 Text = bLight ? ImVec4(0.10f, 0.10f, 0.10f, 1.00f) : ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		const float flDarker = bLight ? 0.88f : 0.60f;

		// Neighbouring hues, 30 degrees either side
		const ImVec4 Secondary = Shift(Accent, -1.0f / 12.0f);
		const ImVec4 Tertiary = Shift(Accent, 1.0f / 12.0f);
		const ImVec4 SecondaryBg = Shift(AccentBg, -1.0f / 12.0f);
		const ImVec4 AccentBright = Shift(Accent, 0.0f, 1.0f, 1.35f);

		Style.WindowRounding = 5.0f;
		Style.ChildRounding = 3.0f;

		Colors[ImGuiCol_Text] = Text;
		Colors[ImGuiCol_TextDisabled] = Opaque(ImLerp(Text, AccentBg, 0.40f));
		Colors[ImGuiCol_WindowBg] = AccentBg;
		Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		Colors[ImGuiCol_PopupBg] = WithAlpha(AccentBg, ImMax(AccentBg.w, 0.94f));
		Colors[ImGuiCol_Border] = WithAlpha(Text, 0.25f);
		Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		Colors[ImGuiCol_FrameBg] = WithAlpha(SecondaryBg, 0.54f);
		Colors[ImGuiCol_FrameBgHovered] = WithAlpha(Accent, 0.40f);
		Colors[ImGuiCol_FrameBgActive] = WithAlpha(Accent, 0.67f);
		Colors[ImGuiCol_TitleBg] = Opaque(Shift(AccentBg, 0.0f, 1.0f, flDarker));
		Colors[ImGuiCol_TitleBgActive] = Opaque(SecondaryBg);
		Colors[ImGuiCol_TitleBgCollapsed] = WithAlpha(Colors[ImGuiCol_TitleBg], 0.51f);
		Colors[ImGuiCol_MenuBarBg] = Opaque(Shift(AccentBg, 0.0f, 1.0f, (1.0f + flDarker) * 0.5f));
		Colors[ImGuiCol_ScrollbarBg] = WithAlpha(Colors[ImGuiCol_TitleBg], 0.53f);
		Colors[ImGuiCol_ScrollbarGrab] = WithAlpha(Text, 0.30f);
		Colors[ImGuiCol_ScrollbarGrabHovered] = WithAlpha(Text, 0.40f);
		Colors[ImGuiCol_ScrollbarGrabActive] = WithAlpha(Text, 0.50f);
		Colors[ImGuiCol_SliderGrab] = Opaque(Tertiary);
		Colors[ImGuiCol_SliderGrabActive] = Opaque(Accent);
		Colors[ImGuiCol_Button] = WithAlpha(Accent, 0.40f);
		Colors[ImGuiCol_ButtonHovered] = Opaque(Accent);
		Colors[ImGuiCol_ButtonActive] = Opaque(AccentBright);
		Colors[ImGuiCol_Header] = WithAlpha(Accent, 0.31f);
		Colors[ImGuiCol_HeaderHovered] = WithAlpha(Accent, 0.80f);
		Colors[ImGuiCol_HeaderActive] = Opaque(Accent);
		Colors[ImGuiCol_Separator] = Colors[ImGuiCol_Border];
		Colors[ImGuiCol_SeparatorHovered] = WithAlpha(Accent, 0.78f);
		Colors[ImGuiCol_SeparatorActive] = Opaque(Accent);
		Colors[ImGuiCol_ResizeGrip] = WithAlpha(Accent, 0.20f);
		Colors[ImGuiCol_ResizeGripHovered] = WithAlpha(Accent, 0.67f);
		Colors[ImGuiCol_ResizeGripActive] = WithAlpha(Accent, 0.95f);
		Colors[ImGuiCol_InputTextCursor] = Text;

		// Checkboxes. ImGui draws an unchecked box in FrameBg and a checked one in
		// CheckboxSelectedBg, straight on the window with no border in this palette, the tick
		// in CheckMark, and a hovered or held box in FrameBgHovered/Active. Built from the
		// accents alone these were dark on dark (the default tick was dark green on dark
		// blue), so each is pushed to a minimum contrast with what it sits on: the checked box
		// 3:1 with the window (WCAG's minimum for UI parts), the unchecked one 2:1 (FrameBg is
		// every input's and slider's background too, so it stays quieter), the tick 4.5:1
		// with the checked box and 3:1 with the hover colours.
		const ImVec4 WindowBg = Opaque(AccentBg);

		if (ContrastRatio(Over(Colors[ImGuiCol_FrameBg], WindowBg), WindowBg) < 2.0f)
			Colors[ImGuiCol_FrameBg] = EnsureContrast(Over(Colors[ImGuiCol_FrameBg], WindowBg), WindowBg, 2.0f);

		Colors[ImGuiCol_CheckboxSelectedBg] = EnsureContrast(Opaque(Accent), WindowBg, 3.0f);
		Colors[ImGuiCol_CheckMark] = EnsureContrast(Text, Colors[ImGuiCol_CheckboxSelectedBg], 4.5f);

		const ImGuiCol Hovers[] = { ImGuiCol_FrameBgHovered, ImGuiCol_FrameBgActive };

		for (ImGuiCol Hover : Hovers)
		{
			if (ContrastRatio(Over(Colors[Hover], WindowBg), Colors[ImGuiCol_CheckMark]) < 3.0f)
				Colors[Hover] = EnsureContrast(Over(Colors[Hover], WindowBg), Colors[ImGuiCol_CheckMark], 3.0f);
		}

		Colors[ImGuiCol_Tab] = ImLerp(Colors[ImGuiCol_Header], Colors[ImGuiCol_TitleBgActive], 0.80f);
		Colors[ImGuiCol_TabHovered] = Colors[ImGuiCol_HeaderHovered];
		Colors[ImGuiCol_TabSelected] = ImLerp(Colors[ImGuiCol_HeaderActive], Colors[ImGuiCol_TitleBgActive], 0.60f);
		Colors[ImGuiCol_TabSelectedOverline] = Colors[ImGuiCol_HeaderActive];
		Colors[ImGuiCol_TabDimmed] = ImLerp(Colors[ImGuiCol_Tab], Colors[ImGuiCol_TitleBg], 0.80f);
		Colors[ImGuiCol_TabDimmedSelected] = ImLerp(Colors[ImGuiCol_TabSelected], Colors[ImGuiCol_TitleBg], 0.40f);
		Colors[ImGuiCol_TabDimmedSelectedOverline] = WithAlpha(Colors[ImGuiCol_HeaderActive], 0.00f);

		Colors[ImGuiCol_PlotLines] = WithAlpha(Text, 0.70f);
		Colors[ImGuiCol_PlotLinesHovered] = Opaque(AccentBright);
		Colors[ImGuiCol_PlotHistogram] = Opaque(Secondary);
		Colors[ImGuiCol_PlotHistogramHovered] = Opaque(AccentBright);

		Colors[ImGuiCol_TableHeaderBg] = Colors[ImGuiCol_TitleBg];
		Colors[ImGuiCol_TableBorderStrong] = Opaque(ImLerp(AccentBg, Text, 0.30f));
		Colors[ImGuiCol_TableBorderLight] = Opaque(ImLerp(AccentBg, Text, 0.18f));
		Colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		Colors[ImGuiCol_TableRowBgAlt] = WithAlpha(Text, 0.06f);

		Colors[ImGuiCol_TextLink] = Opaque(AccentBright);
		Colors[ImGuiCol_TextSelectedBg] = WithAlpha(Accent, 0.35f);
		Colors[ImGuiCol_TreeLines] = Colors[ImGuiCol_Border];
		Colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		Colors[ImGuiCol_DragDropTargetBg] = WithAlpha(Accent, 0.15f);
		Colors[ImGuiCol_UnsavedMarker] = Text;
		Colors[ImGuiCol_NavCursor] = Opaque(AccentBright);
		Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	}

	// NieR:Automata's menu look: the palette is read from the game's own UI layout files
	// (ui_core.dat). Ink #4E4B42 is the most used colour there (247 times), then paper
	// #DAD4BB (87) and shade #B4AF9A (52). ImGui has one text colour, so the game's dark
	// selection bar with light text can't be reproduced; hover and selection use the shade.
	void BuildYoRHa(ImGuiStyle& Style)
	{
		const ImVec4 Ink = FromRGB(0x4E4B42);
		const ImVec4 Paper = FromRGB(0xDAD4BB);
		const ImVec4 PaperLight = FromRGB(0xD6D0B8);
		const ImVec4 PaperDim = FromRGB(0xCDC8B0);
		const ImVec4 Shade = FromRGB(0xB4AF9A);
		const ImVec4 Highlight = FromRGB(0xF5EFE6);

		ImVec4* Colors = Style.Colors;

		Colors[ImGuiCol_Text] = Ink;
		Colors[ImGuiCol_TextDisabled] = Opaque(ImLerp(Ink, Paper, 0.50f));
		Colors[ImGuiCol_WindowBg] = WithAlpha(Paper, 0.97f);
		Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		Colors[ImGuiCol_PopupBg] = WithAlpha(Paper, 0.98f);
		Colors[ImGuiCol_Border] = WithAlpha(Ink, 0.60f);
		Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		Colors[ImGuiCol_FrameBg] = PaperDim;
		Colors[ImGuiCol_FrameBgHovered] = Shade;
		Colors[ImGuiCol_FrameBgActive] = Shade;
		Colors[ImGuiCol_TitleBg] = PaperDim;
		Colors[ImGuiCol_TitleBgActive] = Shade;
		Colors[ImGuiCol_TitleBgCollapsed] = WithAlpha(PaperDim, 0.75f);
		Colors[ImGuiCol_MenuBarBg] = PaperLight;
		Colors[ImGuiCol_ScrollbarBg] = WithAlpha(PaperDim, 0.60f);
		Colors[ImGuiCol_ScrollbarGrab] = Shade;
		Colors[ImGuiCol_ScrollbarGrabHovered] = WithAlpha(Ink, 0.70f);
		Colors[ImGuiCol_ScrollbarGrabActive] = Ink;
		// Checkboxes: the box always has the 1 px ink border (FrameBorderSize below), which is
		// what sets it off from the paper, so the border is kept at 3:1 with the window; a
		// checked box is filled with the shade and an unchecked one with the dim paper, and
		// the tick is the ink, darkened just enough for 4.5:1 on the shade (plain ink is 4:1).
		Colors[ImGuiCol_Border] = EnsureContrast(Over(Colors[ImGuiCol_Border], Paper), Paper, 3.0f);
		Colors[ImGuiCol_CheckMark] = EnsureContrast(Ink, Shade, 4.5f);
		Colors[ImGuiCol_CheckboxSelectedBg] = Shade;
		Colors[ImGuiCol_SliderGrab] = WithAlpha(Ink, 0.80f);
		Colors[ImGuiCol_SliderGrabActive] = Ink;
		Colors[ImGuiCol_Button] = PaperDim;
		Colors[ImGuiCol_ButtonHovered] = Shade;
		Colors[ImGuiCol_ButtonActive] = Shade;
		Colors[ImGuiCol_Header] = PaperDim;
		Colors[ImGuiCol_HeaderHovered] = Shade;
		Colors[ImGuiCol_HeaderActive] = Shade;
		Colors[ImGuiCol_Separator] = WithAlpha(Ink, 0.50f);
		Colors[ImGuiCol_SeparatorHovered] = WithAlpha(Ink, 0.80f);
		Colors[ImGuiCol_SeparatorActive] = Ink;
		Colors[ImGuiCol_ResizeGrip] = WithAlpha(Ink, 0.25f);
		Colors[ImGuiCol_ResizeGripHovered] = WithAlpha(Ink, 0.60f);
		Colors[ImGuiCol_ResizeGripActive] = WithAlpha(Ink, 0.90f);
		Colors[ImGuiCol_InputTextCursor] = Ink;

		Colors[ImGuiCol_Tab] = PaperDim;
		Colors[ImGuiCol_TabHovered] = Shade;
		Colors[ImGuiCol_TabSelected] = Paper;
		Colors[ImGuiCol_TabSelectedOverline] = Ink;
		Colors[ImGuiCol_TabDimmed] = PaperDim;
		Colors[ImGuiCol_TabDimmedSelected] = PaperLight;
		Colors[ImGuiCol_TabDimmedSelectedOverline] = WithAlpha(Ink, 0.50f);

		Colors[ImGuiCol_PlotLines] = Ink;
		Colors[ImGuiCol_PlotLinesHovered] = WithAlpha(Ink, 0.70f);
		Colors[ImGuiCol_PlotHistogram] = Ink;
		Colors[ImGuiCol_PlotHistogramHovered] = WithAlpha(Ink, 0.70f);

		Colors[ImGuiCol_TableHeaderBg] = PaperDim;
		Colors[ImGuiCol_TableBorderStrong] = WithAlpha(Ink, 0.60f);
		Colors[ImGuiCol_TableBorderLight] = Shade;
		Colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		Colors[ImGuiCol_TableRowBgAlt] = WithAlpha(Ink, 0.06f);

		Colors[ImGuiCol_TextLink] = Ink;
		Colors[ImGuiCol_TextSelectedBg] = WithAlpha(Ink, 0.30f);
		Colors[ImGuiCol_TreeLines] = WithAlpha(Ink, 0.50f);
		Colors[ImGuiCol_DragDropTarget] = Ink;
		Colors[ImGuiCol_DragDropTargetBg] = WithAlpha(Ink, 0.10f);
		Colors[ImGuiCol_UnsavedMarker] = Ink;
		Colors[ImGuiCol_NavCursor] = Ink;
		Colors[ImGuiCol_NavWindowingHighlight] = WithAlpha(Highlight, 0.70f);
		Colors[ImGuiCol_NavWindowingDimBg] = WithAlpha(Ink, 0.20f);
		Colors[ImGuiCol_ModalWindowDimBg] = WithAlpha(Ink, 0.35f);

		// Square and thin, like the game's menus
		Style.WindowRounding = 0.0f;
		Style.ChildRounding = 0.0f;
		Style.FrameRounding = 0.0f;
		Style.PopupRounding = 0.0f;
		Style.ScrollbarRounding = 0.0f;
		Style.GrabRounding = 0.0f;
		Style.TabRounding = 0.0f;
		Style.WindowBorderSize = 1.0f;
		Style.ChildBorderSize = 1.0f;
		Style.PopupBorderSize = 1.0f;
		Style.FrameBorderSize = 1.0f;
		Style.TabBorderSize = 1.0f;
		Style.TabBarBorderSize = 1.0f;
		Style.TabBarOverlineSize = 2.0f;
	}
}

const char* Theme::GetPresetName(ThemePreset Preset)
{
	switch (Preset)
	{
	case THEME_YORHA:
		return "YoRHa";
	case THEME_CUSTOM:
		return "Custom";
	case THEME_DEFAULT:
	default:
		return "Default";
	}
}

const wchar_t* Theme::GetPresetKey(ThemePreset Preset)
{
	switch (Preset)
	{
	case THEME_YORHA:
		return L"yorha";
	case THEME_CUSTOM:
		return L"custom";
	case THEME_DEFAULT:
	default:
		return L"default";
	}
}

ThemePreset Theme::PresetFromKey(const wchar_t* szKey)
{
	if (szKey && !_wcsicmp(szKey, L"yorha"))
		return THEME_YORHA;

	if (szKey && !_wcsicmp(szKey, L"custom"))
		return THEME_CUSTOM;

	return THEME_DEFAULT;
}

void Theme::Build(const ThemeSettings& Settings, ImGuiStyle& Style)
{
	// A fresh style: sizes back to ImGui's defaults and every colour, newer ones included,
	// set by ImGui's dark style before the preset paints over it.
	Style = ImGuiStyle();

	switch (Settings.m_Preset)
	{
	case THEME_YORHA:
		BuildYoRHa(Style);
		break;
	case THEME_CUSTOM:
		BuildClassic(Settings.m_Accent.Value, Settings.m_AccentBg.Value, Style);
		break;
	case THEME_DEFAULT:
	default:
		BuildClassic(s_DefaultAccent.Value, s_DefaultAccentBg.Value, Style);
		break;
	}

	for (int i = 0; i < ImGuiCol_COUNT; ++i)
	{
		if (Settings.m_bOverride[i])
			Style.Colors[i] = Settings.m_Overrides[i];
	}

	// Never build an invisible menu while a colour is being edited. Sanitize repairs the
	// stored values on load; this only keeps the live style readable in the meantime.
	Style.Colors[ImGuiCol_Text].w = ImMax(Style.Colors[ImGuiCol_Text].w, MIN_VISIBLE_ALPHA);
	Style.Colors[ImGuiCol_WindowBg].w = ImMax(Style.Colors[ImGuiCol_WindowBg].w, MIN_VISIBLE_ALPHA);
}

const char* Theme::Sanitize(ThemeSettings& Settings)
{
	const bool bAccent = Settings.m_Accent.Value.w < MIN_VISIBLE_ALPHA;
	const bool bAccentBg = Settings.m_AccentBg.Value.w < MIN_VISIBLE_ALPHA;
	const bool bText = Settings.m_bOverride[ImGuiCol_Text] && Settings.m_Overrides[ImGuiCol_Text].w < MIN_VISIBLE_ALPHA;
	const bool bWindowBg = Settings.m_bOverride[ImGuiCol_WindowBg] && Settings.m_Overrides[ImGuiCol_WindowBg].w < MIN_VISIBLE_ALPHA;

	if (bAccent)
		Settings.m_Accent = s_DefaultAccent;

	if (bAccentBg)
		Settings.m_AccentBg = s_DefaultAccentBg;

	// Dropping the override falls back to the preset's own colour
	if (bText)
		Settings.m_bOverride[ImGuiCol_Text] = false;

	if (bWindowBg)
		Settings.m_bOverride[ImGuiCol_WindowBg] = false;

	if (!(bAccent || bAccentBg || bText || bWindowBg))
		return NULL;

	static char s_szWhat[160];

	s_szWhat[0] = '\0';

	if (bAccent)
		strcat_s(s_szWhat, "i_theme_fg ");

	if (bAccentBg)
		strcat_s(s_szWhat, "i_theme_bg ");

	if (bText)
		strcat_s(s_szWhat, "Text ");

	if (bWindowBg)
		strcat_s(s_szWhat, "WindowBg ");

	s_szWhat[strlen(s_szWhat) - 1] = '\0';

	return s_szWhat;
}

float Theme::GetTargetScale(const ThemeSettings& Settings, HWND hWnd)
{
	if (!Settings.m_bAutoScale)
		return ImClamp(Settings.m_flScale, MIN_SCALE, MAX_SCALE);

	float flScale = 1.0f;

	if (hWnd)
	{
		RECT ClientRect;

		flScale = ImGui_ImplWin32_GetDpiScaleForHwnd(hWnd);

		if (GetClientRect(hWnd, &ClientRect))
			flScale = ImMax(flScale, (ClientRect.bottom - ClientRect.top) / 1080.0f);
	}

	// Quarter steps, so dragging the game window doesn't rebuild the style every pixel
	flScale = floorf(flScale * 4.0f + 0.5f) / 4.0f;

	return ImClamp(flScale, 1.0f, MAX_SCALE);
}

void Theme::FormatColor(const ImVec4& Color, wchar_t (&szBuffer)[16])
{
	const ImU32 uColor = ImGui::ColorConvertFloat4ToU32(Color);

	swprintf_s(szBuffer, L"#%02X%02X%02X%02X",
		(uColor >> IM_COL32_R_SHIFT) & 0xFF, (uColor >> IM_COL32_G_SHIFT) & 0xFF,
		(uColor >> IM_COL32_B_SHIFT) & 0xFF, (uColor >> IM_COL32_A_SHIFT) & 0xFF);
}

bool Theme::ParseColor(const wchar_t* szValue, ImVec4& Color)
{
	if (!szValue)
		return false;

	while (iswspace(*szValue))
		++szValue;

	if (*szValue == L'#')
		++szValue;

	wchar_t* szEnd = NULL;
	const unsigned long uRGBA = wcstoul(szValue, &szEnd, 16);

	// Exactly eight hex digits, RRGGBBAA
	if (szEnd - szValue != 8)
		return false;

	Color = ImVec4(((uRGBA >> 24) & 0xFF) / 255.0f, ((uRGBA >> 16) & 0xFF) / 255.0f,
		((uRGBA >> 8) & 0xFF) / 255.0f, (uRGBA & 0xFF) / 255.0f);

	return true;
}
