
#include "controls/devices/joystick.h"

#include "controls/controller_motion.h"
#include "stubs.h"
#ifdef _XBOX
#include "controls/xbgamepaddefs.h"
#endif

#pragma warning(disable:4244)

namespace dvl {

ControllerButtonNS::ControllerButton JoyButtonToControllerButton(const SDL_Event &event)
{
	switch (event.type) {
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
		switch (event.jbutton.button) {
#ifdef JOY_BUTTON_A
		case JOY_BUTTON_A:
			return ControllerButtonNS::BUTTON_A;
#endif
#ifdef JOY_BUTTON_B
		case JOY_BUTTON_B:
			return ControllerButtonNS::BUTTON_B;
#endif
#ifdef JOY_BUTTON_X
		case JOY_BUTTON_X:
			return ControllerButtonNS::BUTTON_X;
#endif
#ifdef JOY_BUTTON_Y
		case JOY_BUTTON_Y:
			return ControllerButtonNS::BUTTON_Y;
#endif
#ifdef JOY_BUTTON_LEFTSTICK
		case JOY_BUTTON_LEFTSTICK:
			return ControllerButtonNS::BUTTON_LEFTSTICK;
#endif
#ifdef JOY_BUTTON_RIGHTSTICK
		case JOY_BUTTON_RIGHTSTICK:
			return ControllerButtonNS::BUTTON_RIGHTSTICK;
#endif
#ifdef JOY_BUTTON_LEFTSHOULDER
		case JOY_BUTTON_LEFTSHOULDER:
			return ControllerButtonNS::BUTTON_LEFTSHOULDER;
#endif
#ifdef JOY_BUTTON_RIGHTSHOULDER
		case JOY_BUTTON_RIGHTSHOULDER:
			return ControllerButtonNS::BUTTON_RIGHTSHOULDER;
#endif
#ifdef JOY_BUTTON_TRIGGERLEFT
		case JOY_BUTTON_TRIGGERLEFT:
			return ControllerButtonNS::AXIS_TRIGGERLEFT;
#endif
#ifdef JOY_BUTTON_TRIGGERRIGHT
		case JOY_BUTTON_TRIGGERRIGHT:
			return ControllerButtonNS::AXIS_TRIGGERRIGHT;
#endif
#ifdef JOY_BUTTON_START
		case JOY_BUTTON_START:
			return ControllerButtonNS::BUTTON_START;
#endif
#ifdef JOY_BUTTON_BACK
		case JOY_BUTTON_BACK:
			return ControllerButtonNS::BUTTON_BACK;
#endif
#ifdef JOY_BUTTON_DPAD_LEFT
		case JOY_BUTTON_DPAD_LEFT:
			return ControllerButton::BUTTON_DPAD_LEFT;
#endif
#ifdef JOY_BUTTON_DPAD_UP
		case JOY_BUTTON_DPAD_UP:
			return ControllerButton::BUTTON_DPAD_UP;
#endif
#ifdef JOY_BUTTON_DPAD_RIGHT
		case JOY_BUTTON_DPAD_RIGHT:
			return ControllerButton::BUTTON_DPAD_RIGHT;
#endif
#ifdef JOY_BUTTON_DPAD_DOWN
		case JOY_BUTTON_DPAD_DOWN:
			return ControllerButton::BUTTON_DPAD_DOWN;
#endif
		default:
			break;
		}
		break;
	case SDL_JOYHATMOTION:
#if defined(JOY_HAT_DPAD_UP_HAT) && defined(JOY_HAT_DPAD_UP)
		if (event.jhat.hat == JOY_HAT_DPAD_UP_HAT && (event.jhat.value & JOY_HAT_DPAD_UP) != 0)
			return ControllerButtonNS::BUTTON_DPAD_UP;
#endif
#if defined(JOY_HAT_DPAD_DOWN_HAT) && defined(JOY_HAT_DPAD_DOWN)
		if (event.jhat.hat == JOY_HAT_DPAD_DOWN_HAT && (event.jhat.value & JOY_HAT_DPAD_DOWN) != 0)
			return ControllerButtonNS::BUTTON_DPAD_DOWN;
#endif
#if defined(JOY_HAT_DPAD_LEFT_HAT) && defined(JOY_HAT_DPAD_LEFT)
		if (event.jhat.hat == JOY_HAT_DPAD_LEFT_HAT && (event.jhat.value & JOY_HAT_DPAD_LEFT) != 0)
			return ControllerButtonNS::BUTTON_DPAD_LEFT;
#endif
#if defined(JOY_HAT_DPAD_RIGHT_HAT) && defined(JOY_HAT_DPAD_RIGHT)
		if (event.jhat.hat == JOY_HAT_DPAD_RIGHT_HAT && (event.jhat.value & JOY_HAT_DPAD_RIGHT) != 0)
			return ControllerButtonNS::BUTTON_DPAD_RIGHT;
#endif
		return ControllerButtonNS::CBIGNORE;
		break;
	default:
		break;
	}
	return ControllerButtonNS::NONE;
}

namespace {

int JoyButtonToControllerButton(ControllerButtonNS::ControllerButton button)
{
	if (button == ControllerButtonNS::AXIS_TRIGGERLEFT || button == ControllerButtonNS::AXIS_TRIGGERRIGHT)
		UNIMPLEMENTED();
	switch (button) {
#ifdef JOY_BUTTON_A
	case ControllerButtonNS::BUTTON_A:
		return JOY_BUTTON_A;
#endif
#ifdef JOY_BUTTON_B
	case ControllerButtonNS::BUTTON_B:
		return JOY_BUTTON_B;
#endif
#ifdef JOY_BUTTON_X
	case ControllerButtonNS::BUTTON_X:
		return JOY_BUTTON_X;
#endif
#ifdef JOY_BUTTON_Y
	case ControllerButtonNS::BUTTON_Y:
		return JOY_BUTTON_Y;
#endif
#ifdef JOY_BUTTON_BACK
	case ControllerButtonNS::BUTTON_BACK:
		return JOY_BUTTON_BACK;
#endif
#ifdef JOY_BUTTON_START
	case ControllerButtonNS::BUTTON_START:
		return JOY_BUTTON_START;
#endif
#ifdef JOY_BUTTON_LEFTSTICK
	case ControllerButtonNS::BUTTON_LEFTSTICK:
		return JOY_BUTTON_LEFTSTICK;
#endif
#ifdef JOY_BUTTON_RIGHTSTICK
	case ControllerButtonNS::BUTTON_RIGHTSTICK:
		return JOY_BUTTON_RIGHTSTICK;
#endif
#ifdef JOY_BUTTON_LEFTSHOULDER
	case ControllerButtonNS::BUTTON_LEFTSHOULDER:
		return JOY_BUTTON_LEFTSHOULDER;
#endif
#ifdef JOY_BUTTON_RIGHTSHOULDER
	case ControllerButtonNS::BUTTON_RIGHTSHOULDER:
		return JOY_BUTTON_RIGHTSHOULDER;
#endif
#ifdef JOY_BUTTON_DPAD_UP
	case ControllerButtonNS::BUTTON_DPAD_UP:
		return JOY_BUTTON_DPAD_UP;
#endif
#ifdef JOY_BUTTON_DPAD_DOWN
	case ControllerButtonNS::BUTTON_DPAD_DOWN:
		return JOY_BUTTON_DPAD_DOWN;
#endif
#ifdef JOY_BUTTON_DPAD_LEFT
	case ControllerButtonNS::BUTTON_DPAD_LEFT:
		return JOY_BUTTON_DPAD_LEFT;
#endif
#ifdef JOY_BUTTON_DPAD_RIGHT
	case ControllerButtonNS::BUTTON_DPAD_RIGHT:
		return JOY_BUTTON_DPAD_RIGHT;
#endif
	default:
		return -1;
	}
}

bool IsJoystickHatButtonPressed(ControllerButtonNS::ControllerButton button)
{
	switch (button) {
#if defined(JOY_HAT_DPAD_UP_HAT) && defined(JOY_HAT_DPAD_UP)
	case ControllerButtonNS::BUTTON_DPAD_UP:
		return (SDL_JoystickGetHat(CurrentJoystick(), JOY_HAT_DPAD_UP_HAT) & JOY_HAT_DPAD_UP) != 0;
#endif
#if defined(JOY_HAT_DPAD_DOWN_HAT) && defined(JOY_HAT_DPAD_DOWN)
	case ControllerButtonNS::BUTTON_DPAD_DOWN:
		return (SDL_JoystickGetHat(CurrentJoystick(), JOY_HAT_DPAD_DOWN_HAT) & JOY_HAT_DPAD_DOWN) != 0;
#endif
#if defined(JOY_HAT_DPAD_LEFT_HAT) && defined(JOY_HAT_DPAD_LEFT)
	case ControllerButtonNS::BUTTON_DPAD_LEFT:
		return (SDL_JoystickGetHat(CurrentJoystick(), JOY_HAT_DPAD_LEFT_HAT) & JOY_HAT_DPAD_LEFT) != 0;
#endif
#if defined(JOY_HAT_DPAD_RIGHT_HAT) && defined(JOY_HAT_DPAD_RIGHT)
	case ControllerButtonNS::BUTTON_DPAD_RIGHT:
		return (SDL_JoystickGetHat(CurrentJoystick(), JOY_HAT_DPAD_RIGHT_HAT) & JOY_HAT_DPAD_RIGHT) != 0;
#endif
	default:
		return false;
	}
}

} // namespace

bool IsJoystickButtonPressed(ControllerButtonNS::ControllerButton button)
{
	if (CurrentJoystick() == NULL)
		return false;
	if (IsJoystickHatButtonPressed(button))
		return true;
	const int joy_button = JoyButtonToControllerButton(button);
	return joy_button != -1 && SDL_JoystickGetButton(CurrentJoystick(), joy_button);
}

bool ProcessJoystickAxisMotion(const SDL_Event &event)
{
	if (event.type != SDL_JOYAXISMOTION)
		return false;
	switch (event.jaxis.axis) {
#ifdef JOY_AXIS_LEFTX
	case JOY_AXIS_LEFTX:
		leftStickXUnscaled = event.jaxis.value;
		leftStickNeedsScaling = true;
		break;
#endif
#ifdef JOY_AXIS_LEFTY
	case JOY_AXIS_LEFTY:
		leftStickYUnscaled = -event.jaxis.value;
		leftStickNeedsScaling = true;
		break;
#endif
#ifdef JOY_AXIS_RIGHTX
	case JOY_AXIS_RIGHTX:
		rightStickXUnscaled = event.jaxis.value;
		rightStickNeedsScaling = true;
		break;
#endif
#ifdef JOY_AXIS_RIGHTY
	case JOY_AXIS_RIGHTY:
		rightStickYUnscaled = -event.jaxis.value;
		rightStickNeedsScaling = true;
		break;
#endif
	default:
		return false;
	}
	return true;
}

static SDL_Joystick *current_joystick = NULL;

SDL_Joystick *CurrentJoystick()
{
	return current_joystick;
}

static int current_joystick_index = -1;

int CurrentJoystickIndex()
{
	return current_joystick_index;
}

void InitJoystick()
{
#if HAS_KBCTRL == 1
	sgbControllerActive = true;
#endif

	if (SDL_NumJoysticks() == 0) {
		current_joystick_index = -1;
#if HAS_KBCTRL == 0
		sgbControllerActive = false;
#endif
		return;
	}

	// Get the first available controller.
	for (int i = 0; i < SDL_NumJoysticks(); ++i) {
#ifndef USE_SDL1
		if (!SDL_IsGameController(i))
			continue;
#endif
		SDL_Log("Initializing joystick %d: %s", i, SDL_JoystickNameForIndex(i));
		current_joystick = SDL_JoystickOpen(i);
		if (current_joystick == NULL) {
			SDL_Log(SDL_GetError());
			continue;
		}
		current_joystick_index = i;
		sgbControllerActive = true;
		break;
	}
}

} // namespace dvl
