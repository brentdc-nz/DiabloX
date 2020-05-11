#include "all.h"
#include "display.h"
#include "stubs.h"
#include "utf8.h"
#include <string>
#include <algorithm>

#include "controls/menu_controls.h"

#include "DiabloUI/scrollbar.h"
#include "DiabloUI/diabloui.h"

#include "DiabloUI/art_draw.h"
#include "DiabloUI/text_draw.h"
#include "DiabloUI/fonts.h"
#include "DiabloUI/button.h"
#include "DiabloUI/dialogs.h"
#include "controls/controller.h"

#ifdef __SWITCH__
// for virtual keyboard on Switch
#include "platform/switch/keyboard.h"
#endif

namespace dvl {

int SelectedItemMin = 1;
int SelectedItemMax = 1;

std::size_t ListViewportSize = 1;
const std::size_t *ListOffset = NULL;

Art ArtLogos[3];
Art ArtFocus[3];
Art ArtBackground;
Art ArtCursor;
Art ArtHero;
bool gbSpawned;

void (*gfnSoundFunction)(char *file);
void (*gfnListFocus)(int value);
void (*gfnListSelect)(int value);
void (*gfnListEsc)();
bool (*gfnListYesNo)();
std::vector<UiItemBase*> gUiItems;
int gUiItemCnt;
bool UiItemsWraps;
char *UiTextInput;
int UiTextInputLen;

int SelectedItem = 0;

namespace {

DWORD fadeTc;
int fadeValue = 0;

struct SBARS {
	bool upArrowPressed;
	bool downArrowPressed;

	SBARS()
	{
		upArrowPressed = false;
		downArrowPressed = false;
	}

} scrollBarState;

} // namespace

void UiDestroy()
{
	ArtHero.Unload();
	UnloadTtfFont();
	UnloadArtFonts();
}

void UiInitList(int min, int max, void (*fnFocus)(int value), void (*fnSelect)(int value), void (*fnEsc)(), vUiItemBase items, int itemCnt, bool itemsWraps, bool (*fnYesNo)())
{
	SelectedItem = min;
	SelectedItemMin = min;
	SelectedItemMax = max;
	ListViewportSize = SelectedItemMax - SelectedItemMin + 1;
	gfnListFocus = fnFocus;
	gfnListSelect = fnSelect;
	gfnListEsc = fnEsc;
	gfnListYesNo = fnYesNo;
	gUiItems = items;
	gUiItemCnt = itemCnt;
	UiItemsWraps = itemsWraps;
	if (fnFocus)
		fnFocus(min);

	SDL_StopTextInput(); // input is enabled by default
	for (int i = 0; i < itemCnt; i++) {
		if (items[i]->m_type == UI_EDIT) {
#ifdef __SWITCH__
			switch_start_text_input(items[i - 1].art_text.text, items[i].edit.value, /*multiline=*/0);
#endif
			UiEdit* pItemUIEdit = (UiEdit*)items[i];

			SDL_StartTextInput();
			UiTextInput = pItemUIEdit->m_value;
			UiTextInputLen = pItemUIEdit->m_max_length;
		}
	}
}

void UiInitScrollBar(UiScrollBar *ui_sb, std::size_t viewport_size, const std::size_t *current_offset)
{
	ListViewportSize = viewport_size;
	ListOffset = current_offset;
	if (ListViewportSize >= static_cast<std::size_t>(SelectedItemMax - SelectedItemMin + 1)) {
		ui_sb->add_flag(UIS_HIDDEN);
	} else {
		ui_sb->remove_flag(UIS_HIDDEN);
	}
}

void UiInitList_clear()
{
	SelectedItem = 0;
	SelectedItemMin = 0;
	SelectedItemMax = 0;
	ListViewportSize = SelectedItemMax - SelectedItemMin + 1;
	gfnListFocus = NULL;
	gfnListSelect = NULL;
	gfnListEsc = NULL;
	gfnListYesNo = NULL;
//	gUiItems = NULL;
	gUiItemCnt = 0;
	UiItemsWraps = false;
}

void UiPlayMoveSound()
{
	if (gfnSoundFunction)
		gfnSoundFunction("sfx\\items\\titlemov.wav");
}

void UiPlaySelectSound()
{
	if (gfnSoundFunction)
		gfnSoundFunction("sfx\\items\\titlslct.wav");
}

void UiFocus(int itemIndex, bool wrap = false)
{
	if (!wrap) {
		if (itemIndex < SelectedItemMin) {
			itemIndex = SelectedItemMin;
			return;
		} else if (itemIndex > SelectedItemMax) {
			itemIndex = SelectedItemMax ? SelectedItemMax : SelectedItemMin;
			return;
		}
	} else if (itemIndex < SelectedItemMin) {
		itemIndex = SelectedItemMax ? SelectedItemMax : SelectedItemMin;
	} else if (itemIndex > SelectedItemMax) {
		itemIndex = SelectedItemMin;
	}

	if (SelectedItem == itemIndex)
		return;

	SelectedItem = itemIndex;

	UiPlayMoveSound();

	if (gfnListFocus)
		gfnListFocus(itemIndex);
}

// UiFocusPageUp/Down mimics the slightly weird behaviour of actual Diablo.

void UiFocusPageUp()
{
	if (ListOffset == NULL || *ListOffset == 0) {
		UiFocus(SelectedItemMin);
	} else {
		const std::size_t relpos = (SelectedItem - SelectedItemMin) - *ListOffset;
		std::size_t prev_page_start = SelectedItem - relpos;
		if (prev_page_start >= ListViewportSize)
			prev_page_start -= ListViewportSize;
		else
			prev_page_start = 0;
		UiFocus(prev_page_start);
		UiFocus(*ListOffset + relpos);
	}
}

void UiFocusPageDown()
{
	if (ListOffset == NULL || *ListOffset + ListViewportSize > static_cast<std::size_t>(SelectedItemMax)) {
		UiFocus(SelectedItemMax);
	} else {
		const std::size_t relpos = (SelectedItem - SelectedItemMin) - *ListOffset;
		std::size_t next_page_end = SelectedItem + (ListViewportSize - relpos - 1);
		if (next_page_end + ListViewportSize <= static_cast<std::size_t>(SelectedItemMax))
			next_page_end += ListViewportSize;
		else
			next_page_end = SelectedItemMax;
		UiFocus(next_page_end);
		UiFocus(*ListOffset + relpos);
	}
}

void selhero_CatToName(char *in_buf, char *out_buf, int cnt)
{
	std::string output = utf8_to_latin1(in_buf);
	strncat(out_buf, output.c_str(), cnt - strlen(out_buf));
}

void UiFocusNavigation(SDL_Event *event)
{
	switch (event->type) {
	case SDL_KEYUP:
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEMOTION:
#ifndef USE_SDL1
	case SDL_MOUSEWHEEL:
#endif
	case SDL_JOYBUTTONUP:
	case SDL_JOYAXISMOTION:
	case SDL_JOYBALLMOTION:
	case SDL_JOYHATMOTION:
#ifndef USE_SDL1
	case SDL_FINGERUP:
	case SDL_FINGERMOTION:
	case SDL_CONTROLLERBUTTONUP:
	case SDL_CONTROLLERAXISMOTION:
	case SDL_WINDOWEVENT:
#endif
	case SDL_SYSWMEVENT:
		mainmenu_restart_repintro();
		break;
	}

	switch (GetMenuAction(*event)) {
	case MenuActionNS::SELECT:
		UiFocusNavigationSelect();
		return;
	case MenuActionNS::UP:
		UiFocus(SelectedItem - 1, UiItemsWraps);
		return;
	case MenuActionNS::DOWN:
		UiFocus(SelectedItem + 1, UiItemsWraps);
		return;
	case MenuActionNS::PAGE_UP:
		UiFocusPageUp();
		return;
	case MenuActionNS::PAGE_DOWN:
		UiFocusPageDown();
		return;
	case MenuActionNS::MADELETE:
		UiFocusNavigationYesNo();
		return;
	case MenuActionNS::BACK:
		if (!gfnListEsc)
			break;
		UiFocusNavigationEsc();
		return;
	default:
		break;
	}

#ifndef USE_SDL1
	if (event->type == SDL_MOUSEWHEEL) {
		if (event->wheel.y > 0) {
			UiFocus(SelectedItem - 1, UiItemsWraps);
		} else if (event->wheel.y < 0) {
			UiFocus(SelectedItem + 1, UiItemsWraps);
		}
		return;
	}
#endif

	if (SDL_IsTextInputActive()) {
		switch (event->type) {
		case SDL_KEYDOWN: {
			switch (event->key.keysym.sym) {
#ifndef USE_SDL1
			case SDLK_v:
				if (SDL_GetModState() & KMOD_CTRL) {
					char *clipboard = SDL_GetClipboardText();
					if (clipboard == NULL) {
						SDL_Log(SDL_GetError());
					} else {
						selhero_CatToName(clipboard, UiTextInput, UiTextInputLen);
					}
				}
				return;
#endif
			case SDLK_BACKSPACE:
			case SDLK_LEFT: {
				int nameLen = strlen(UiTextInput);
				if (nameLen > 0) {
					UiTextInput[nameLen - 1] = '\0';
				}
				return;
			}
			default:
				break;
			}
#ifdef USE_SDL1
			if ((event->key.keysym.mod & KMOD_CTRL) == 0) {
				Uint16 unicode = event->key.keysym.unicode;
				if (unicode && (unicode & 0xFF80) == 0) {
					char utf8[SDL_TEXTINPUTEVENT_TEXT_SIZE];
					utf8[0] = (char)unicode;
					utf8[1] = '\0';
					selhero_CatToName(utf8, UiTextInput, UiTextInputLen);
				}
			}
#endif
			break;
		}
#ifndef USE_SDL1
		case SDL_TEXTINPUT:
			selhero_CatToName(event->text.text, UiTextInput, UiTextInputLen);
			return;
#endif
		default:
			break;
		}
	}

	if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP) {
		if (UiItemMouseEvents(event, gUiItems, gUiItemCnt))
			return;
	}
}

void UiHandleEvents(SDL_Event *event)
{
	if (event->type == SDL_MOUSEMOTION) {
#ifdef USE_SDL1
		OutputToLogical(&event->motion.x, &event->motion.y);
#endif
		MouseX = event->motion.x;
		MouseY = event->motion.y;
		return;
	}

	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_RETURN) {
		const Uint8 *state = SDLC_GetKeyState();
		if (state[SDLC_KEYSTATE_LALT] || state[SDLC_KEYSTATE_RALT]) {
			dx_reinit();
			return;
		}
	}

	if (event->type == SDL_QUIT)
		diablo_quit(0);

#ifndef USE_SDL1
	if (event->type == SDL_JOYDEVICEADDED || event->type == SDL_JOYDEVICEREMOVED) {
		InitController();
		return;
	}

	if (event->type == SDL_WINDOWEVENT) {
		if (event->window.event == SDL_WINDOWEVENT_SHOWN)
			gbActive = true;
		else if (event->window.event == SDL_WINDOWEVENT_HIDDEN)
			gbActive = false;
	}
#endif
}

void UiFocusNavigationSelect()
{
	UiPlaySelectSound();
	if (SDL_IsTextInputActive()) {
		if (strlen(UiTextInput) == 0) {
			return;
		}
		SDL_StopTextInput();
		UiTextInput = NULL;
		UiTextInputLen = 0;
	}
	if (gfnListSelect)
		gfnListSelect(SelectedItem);
}

void UiFocusNavigationEsc()
{
	UiPlaySelectSound();
	if (SDL_IsTextInputActive()) {
		SDL_StopTextInput();
		UiTextInput = NULL;
		UiTextInputLen = 0;
	}
	if (gfnListEsc)
		gfnListEsc();
}

void UiFocusNavigationYesNo()
{
	if (gfnListYesNo == NULL)
		return;

	if (gfnListYesNo())
		UiPlaySelectSound();
}

bool IsInsideRect(const SDL_Event &event, const SDL_Rect &rect)
{
	const SDL_Point point = { event.button.x, event.button.y };
	return SDL_PointInRect(&point, &rect);
}

void LoadUiGFX()
{
	LoadMaskedArt("ui_art\\smlogo.pcx", &ArtLogos[LOGO_MED], 15);
	LoadMaskedArt("ui_art\\focus16.pcx", &ArtFocus[FOCUS_SMALL], 8);
	LoadMaskedArt("ui_art\\focus.pcx", &ArtFocus[FOCUS_MED], 8);
	LoadMaskedArt("ui_art\\focus42.pcx", &ArtFocus[FOCUS_BIG], 8);
	LoadMaskedArt("ui_art\\cursor.pcx", &ArtCursor, 1, 0);
	LoadArt("ui_art\\heros.pcx", &ArtHero, 4);
}

void UiInitialize()
{
	LoadUiGFX();
	LoadArtFonts();
	if (ArtCursor.surface != NULL) {
		if (SDL_ShowCursor(SDL_DISABLE) <= -1) {
			ErrSdl();
		}
	}
}

const char **UiProfileGetString()
{
	return NULL;
}

char connect_plrinfostr[128];
char connect_categorystr[128];
void UiSetupPlayerInfo(char *infostr, _uiheroinfo *pInfo, DWORD type)
{
	SStrCopy(connect_plrinfostr, infostr, 128);
	char format[32] = "";
	strncpy(format, (char *)&type, 4);
	strcat(format, " %d %d %d %d %d %d %d %d %d");

	snprintf(
	    connect_categorystr,
	    128,
	    format,
	    pInfo->level,
	    pInfo->heroclass,
	    pInfo->herorank,
	    pInfo->strength,
	    pInfo->magic,
	    pInfo->dexterity,
	    pInfo->vitality,
	    pInfo->gold,
	    pInfo->spawned);
}

BOOL UiValidPlayerName(char *name)
{
	if (!strlen(name))
		return false;

	if (strpbrk(name, ",<>%&\\\"?*#/:") || strpbrk(name, " "))
		return false;

	for (BYTE *letter = (BYTE *)name; *letter; letter++)
		if (*letter < 0x20 || (*letter > 0x7E && *letter < 0xC0))
			return false;

	char *reserved[] = {
		"gvdl",
		"dvou",
		"tiju",
		"cjudi",
		"bttipmf",
		"ojhhfs",
		"cmj{{bse",
		"benjo",
	};

	char tmpname[PLR_NAME_LEN];
	strcpy(tmpname, name);
	for (size_t i = 0, n = strlen(tmpname); i < n; i++)
		tmpname[i]++;

	for (uint32_t i = 0; i < sizeof(reserved) / sizeof(*reserved); i++) {
		if (strstr(tmpname, reserved[i]))
			return false;
	}

	return true;
}

void UiProfileCallback()
{
	UNIMPLEMENTED();
}

void UiProfileDraw()
{
	UNIMPLEMENTED();
}

BOOL UiCategoryCallback(int a1, int a2, int a3, int a4, int a5, DWORD *a6, DWORD *a7)
{
	UNIMPLEMENTED();
}

BOOL UiGetDataCallback(int game_type, int data_code, void *a3, int a4, int a5)
{
	UNIMPLEMENTED();
}

BOOL UiAuthCallback(int a1, char *a2, char *a3, char a4, char *a5, char *lpBuffer, int cchBufferMax)
{
	UNIMPLEMENTED();
}

BOOL UiSoundCallback(int a1, int type, int a3)
{
	UNIMPLEMENTED();
}

void UiMessageBoxCallback(HWND hWnd, char *lpText, const char *lpCaption, UINT uType)
{
	UNIMPLEMENTED();
}

BOOL UiDrawDescCallback(int game_type, DWORD color, const char *lpString, char *a4, int a5, UINT align, time_t a7,
    HDC *a8)
{
	UNIMPLEMENTED();
}

BOOL UiCreateGameCallback(int a1, int a2, int a3, int a4, int a5, int a6)
{
	UNIMPLEMENTED();
}

BOOL UiArtCallback(int game_type, unsigned int art_code, SDL_Color *pPalette, BYTE *pBuffer,
    DWORD dwBuffersize, DWORD *pdwWidth, DWORD *pdwHeight, DWORD *pdwBpp)
{
	UNIMPLEMENTED();
}

BOOL UiCreatePlayerDescription(_uiheroinfo *info, DWORD mode, char *desc)
{
	char format[32] = "";
	strncpy(format, (char *)&mode, 4);
	strcat(format, " %d %d %d %d %d %d %d %d %d");

	snprintf(
	    desc,
	    128,
	    format,
	    info->level,
	    info->heroclass,
	    info->herorank,
	    info->strength,
	    info->magic,
	    info->dexterity,
	    info->vitality,
	    info->gold,
	    info->spawned);

	return true;
}

int GetCenterOffset(int w, int bw)
{
	if (bw == 0) {
		bw = PANEL_WIDTH;
	}

	return (bw - w) / 2;
}

void LoadBackgroundArt(const char *pszFile)
{
	SDL_Color pPal[256];
	LoadArt(pszFile, &ArtBackground, 1, pPal);
	if (ArtBackground.surface == NULL)
		return;

	LoadPalInMem(pPal);
	ApplyGamma(logical_palette, orig_palette, 256);

	fadeTc = 0;
	fadeValue = 0;
	BlackPalette();
	SDL_FillRect(GetOutputSurface(), NULL, 0x000000);
	RenderPresent();
}

void UiFadeIn()
{
	if (fadeValue < 256) {
		if (fadeValue == 0 && fadeTc == 0)
			fadeTc = SDL_GetTicks();
		fadeValue = (SDL_GetTicks() - fadeTc) / 2.083; // 32 frames @ 60hz
		if (fadeValue > 256) {
			fadeValue = 256;
			fadeTc = 0;
		}
		SetFadeLevel(fadeValue);
	}

	RenderPresent();
}

void DrawSelector(const SDL_Rect &rect)
{
	int size = FOCUS_SMALL;
	if (rect.h >= 42)
		size = FOCUS_BIG;
	else if (rect.h >= 30)
		size = FOCUS_MED;
	Art *art = &ArtFocus[size];

	int frame = GetAnimationFrame(art->frames);
	int y = rect.y + (rect.h - art->h()) / 2; // TODO FOCUS_MED appares higher then the box

	DrawArt(rect.x, y, art, frame);
	DrawArt(rect.x + rect.w - art->w(), y, art, frame);
}

void UiPollAndRender()
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		UiFocusNavigation(&event);
		UiHandleEvents(&event);
	}
	UiRenderItems(gUiItems, gUiItemCnt);
	DrawMouse();
	UiFadeIn();
}

namespace {

void Render(UiText* ui_text)
{
	DrawTTF(ui_text->m_text,
	    ui_text->m_rect,
	    ui_text->m_iFlags,
	    ui_text->m_color,
	    ui_text->m_shadow_color,
	    &ui_text->m_render_cache);
}

void Render(UiArtText* ui_art_text)
{
	DrawArtStr(ui_art_text->m_text, ui_art_text->m_rect, ui_art_text->m_iFlags);
}

void Render(UiImage* ui_image)
{
	int x = ui_image->m_rect.x;
	if ((ui_image->m_iFlags & UIS_CENTER) && ui_image->m_art != NULL) {
		const int x_offset = GetCenterOffset(ui_image->m_art->w(), ui_image->m_rect.w);
		x += x_offset;
	}
	if (ui_image->m_animated) {
		DrawAnimatedArt(ui_image->m_art, x, ui_image->m_rect.y);
	} else {
		DrawArt(x, ui_image->m_rect.y, ui_image->m_art, ui_image->m_frame, ui_image->m_rect.w);
	}
}

void Render(UiArtTextButton* ui_button)
{
	DrawArtStr(ui_button->m_text, ui_button->m_rect, ui_button->m_iFlags);
}

void Render(UiList* ui_list)
{
	for (std::size_t i = 0; i < ui_list->m_length; ++i) {
		SDL_Rect rect = ui_list->itemRect(i);
		const UiListItem* item = ui_list->GetItem(i);
		if (item->m_value == SelectedItem)
			DrawSelector(rect);
		DrawArtStr(item->m_text, rect, ui_list->m_iFlags);
	}
}

void Render(UiScrollBar* ui_sb)
{
	// Bar background (tiled):
	{
		const std::size_t bg_y_end = DownArrowRect(ui_sb).y;
		std::size_t bg_y = ui_sb->m_rect.y + ui_sb->m_arrow->h();
		while (bg_y < bg_y_end) {
			std::size_t drawH = min(bg_y + ui_sb->m_bg->h(), bg_y_end) - bg_y;
			DrawArt(ui_sb->m_rect.x, bg_y, ui_sb->m_bg, 0, SCROLLBAR_BG_WIDTH, drawH);
			bg_y += drawH;
		}
	}

	// Arrows:
	{
		const SDL_Rect rect = UpArrowRect(ui_sb);
		const int frame = static_cast<int>(scrollBarState.upArrowPressed ? ScrollBarArrowFrameNS::UP_ACTIVE : ScrollBarArrowFrameNS::UP);
		DrawArt(rect.x, rect.y, ui_sb->m_arrow, frame, rect.w);
	}
	{
		const SDL_Rect rect = DownArrowRect(ui_sb);
		const int frame = static_cast<int>(scrollBarState.downArrowPressed ? ScrollBarArrowFrameNS::DOWN_ACTIVE : ScrollBarArrowFrameNS::DOWN);
		DrawArt(rect.x, rect.y, ui_sb->m_arrow, frame, rect.w);
	}

	// Thumb:
	{
		const SDL_Rect rect = ThumbRect(
		    ui_sb, SelectedItem - SelectedItemMin, SelectedItemMax - SelectedItemMin + 1);
		DrawArt(rect.x, rect.y, ui_sb->m_thumb);
	}
}

void Render(const UiEdit* ui_edit)
{
	DrawSelector(ui_edit->m_rect);
	SDL_Rect rect = ui_edit->m_rect;
	rect.x += 43;
	rect.y += 1;
	rect.w -= 86;
	DrawArtStr(ui_edit->m_value, rect, ui_edit->m_iFlags, /*drawTextCursor=*/true);
}

void RenderItem(UiItemBase* item)
{
	if (item->has_flag(UIS_HIDDEN))
		return;
	switch (item->m_type) {
	case UI_TEXT:
		Render((UiText*)item);
		break;
	case UI_ART_TEXT:
		Render((UiArtText*)item);
		break;
	case UI_IMAGE:
		Render((UiImage*)item);
		break;
	case UI_ART_TEXT_BUTTON:
		Render((UiArtTextButton*)item);
		break;
	case UI_BUTTON:
		RenderButton((UiButton*)item);
		break;
	case UI_LIST:
		Render((UiList*)item);
		break;
	case UI_SCROLLBAR:
		Render((UiScrollBar*)item);
		break;
	case UI_EDIT:
		Render((UiEdit*)item);
		break;
	}
}

bool HandleMouseEventArtTextButton(const SDL_Event &event, UiArtTextButton* ui_button)
{
	if (event.type != SDL_MOUSEBUTTONDOWN || event.button.button != SDL_BUTTON_LEFT)
		return false;
	ui_button->m_action();
	return true;
}

bool HandleMouseEventList(const SDL_Event &event, UiList* ui_list)
{
	if (event.type != SDL_MOUSEBUTTONDOWN || event.button.button != SDL_BUTTON_LEFT)
		return false;

	const UiListItem* list_item = ui_list->itemAt(event.button.y);

	if (gfnListFocus != NULL /*&& SelectedItem != list_item.m_value*/) {
		UiFocus(list_item->m_value);
#ifdef USE_SDL1
	} else if (gfnListFocus == NULL) {
#else
	} else if (gfnListFocus == NULL || event.button.clicks >= 2) {
#endif
		SelectedItem = list_item->m_value;
		UiFocusNavigationSelect();
	}

	return true;
}

bool HandleMouseEventScrollBar(const SDL_Event &event, const UiScrollBar *ui_sb)
{
	if (event.button.button != SDL_BUTTON_LEFT)
		return false;
	if (event.type == SDL_MOUSEBUTTONUP) {
		if (scrollBarState.upArrowPressed && IsInsideRect(event, UpArrowRect(ui_sb))) {
			UiFocus(SelectedItem - 1);
			return true;
		} else if (scrollBarState.downArrowPressed && IsInsideRect(event, DownArrowRect(ui_sb))) {
			UiFocus(SelectedItem + 1);
			return true;
		}
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		if (IsInsideRect(event, BarRect(ui_sb))) {
			// Scroll up or down based on thumb position.
			const SDL_Rect thumb_rect = ThumbRect(
			    ui_sb, SelectedItem - SelectedItemMin, SelectedItemMax - SelectedItemMin + 1);
			if (event.button.y < thumb_rect.y) {
				UiFocusPageUp();
			} else if (event.button.y > thumb_rect.y + thumb_rect.h) {
				UiFocusPageDown();
			}
			return true;
		} else if (IsInsideRect(event, UpArrowRect(ui_sb))) {
			scrollBarState.upArrowPressed = true;
			return true;
		} else if (IsInsideRect(event, DownArrowRect(ui_sb))) {
			scrollBarState.downArrowPressed = true;
			return true;
		}
	}
	return false;
}

bool HandleMouseEvent(const SDL_Event &event, UiItemBase *item)
{
	if (item->has_any_flag(UIS_HIDDEN | UIS_DISABLED) || !IsInsideRect(event, item->m_rect))
		return false;
	switch (item->m_type) {
	case UI_ART_TEXT_BUTTON:
		return HandleMouseEventArtTextButton(event, (UiArtTextButton*)item);
	case UI_BUTTON:
		return HandleMouseEventButton(event, (UiButton*)item);
	case UI_LIST:
		return HandleMouseEventList(event, (UiList*)item);
	case UI_SCROLLBAR:
		return HandleMouseEventScrollBar(event, (UiScrollBar*)item);
	default:
		return false;
	}
}

} // namespace

void LoadPalInMem(const SDL_Color *pPal)
{
	for (int i = 0; i < 256; i++) {
		orig_palette[i] = pPal[i];
	}
}

void UiRenderItems(vUiItemBase items, std::size_t size)
{
	for(int i = 0; i < (int)size; i++)
		RenderItem((UiItemBase*)items[i]);
}

bool UiItemMouseEvents(SDL_Event *event, vUiItemBase items, std::size_t size)
{
	if (!items.size() || size == 0)
		return false;

	// In SDL2 mouse events already use logical coordinates.
#ifdef USE_SDL1
	OutputToLogical(&event->button.x, &event->button.y);
#endif

	bool handled = false;
	for (std::size_t i = 0; i < size; i++) {
		if (HandleMouseEvent(*event, items[i])) {
			handled = true;
			break;
		}
	}

	if (event->type == SDL_MOUSEBUTTONUP && event->button.button == SDL_BUTTON_LEFT) {
		scrollBarState.downArrowPressed = scrollBarState.upArrowPressed = false;
		for (std::size_t i = 0; i < size; ++i) {
			UiItemBase* &item = items[i];
			if (item->m_type == UI_BUTTON)
				HandleGlobalMouseUpButton((UiButton*)item);
		}
	}

	return handled;
}

void DrawMouse()
{
	if (sgbControllerActive)
		return;

	DrawArt(MouseX, MouseY, &ArtCursor);
}

/**
 * @brief Get int from ini, if not found the provided value will be added to the ini instead
 */
void DvlIntSetting(const char *valuename, int *value)
{
	if (!SRegLoadValue("devilutionx", valuename, 0, value)) {
		SRegSaveValue("devilutionx", valuename, 0, *value);
	}
}

/**
 * @brief Get string from ini, if not found the provided value will be added to the ini instead
 */
void DvlStringSetting(const char *valuename, char *string, int len)
{
	if (!getIniValue("devilutionx", valuename, string, len)) {
		setIniValue("devilutionx", valuename, string);
	}
}
} // namespace dvl
