/**
 * @file appfat.cpp
 *
 * Implementation of error dialogs.
 */
#include "all.h"
#include "../3rdParty/Storm/Source/storm.h"
#ifndef _XBOX
#include <config.h>
#endif
#ifdef _XBOX
#include "xboxfuncs.h"
#endif

DEVILUTION_BEGIN_NAMESPACE

char sz_error_buf[256];
/** Set to true when a fatal error is encountered and the application should shut down. */
BOOL terminating;
/** Thread id of the last callee to FreeDlg(). */
int cleanup_thread_id;

/**
 * @brief Terminates the game and displays an error message box.
 */
void app_fatal(const char *pszFmt, ...)
{
	va_list va;

	va_start(va, pszFmt);
	FreeDlg();

	if (pszFmt)
		MsgBox(pszFmt, va);

	va_end(va);

#ifdef _XBOX
	CXBFunctions::QuitToDash();
#else
	exit(1);
#endif
}

/**
 * @brief Displays an error message box based on the given format string and variable argument list.
 */
void MsgBox(const char *pszFmt, va_list va)
{
	char text[256];

	vsnprintf(text, 256, pszFmt, va);

	UiErrorOkDialog("Error", text);
}

/**
 * @brief Cleans up after a fatal application error.
 */
void FreeDlg()
{
	if (terminating && cleanup_thread_id != SDL_GetThreadID(NULL))
		SDL_Delay(20000);

	terminating = TRUE;
	cleanup_thread_id = SDL_GetThreadID(NULL);

	if (gbMaxPlayers > 1) {
		if (SNetLeaveGame(3))
			SDL_Delay(2000);
	}

	SNetDestroy();
}

/**
 * @brief Displays a warning message box based on the given formatted error message.
 */
void DrawDlg(char *pszFmt, ...)
{
	char text[256];
	va_list va;

	va_start(va, pszFmt);
	vsnprintf(text, 256, pszFmt, va);
	va_end(va);

	UiErrorOkDialog(APP_NAME, text, false);
}

#ifdef _DEBUG
void assert_fail(int nLineNo, const char *pszFile, const char *pszFail)
{
	app_fatal("assertion failed (%d:%s)\n%s", nLineNo, pszFile, pszFail);
}
#endif

/**
 * @brief Terminates the game and displays an error dialog box based on the given dialog_id.
 */
void ErrDlg(const char *title, const char *error, char *log_file_path, int log_line_nr)
{
	char text[1024];

	FreeDlg();

	snprintf(text, 1024, "%s\n\nThe error occurred at: %s line %d", error, log_file_path, log_line_nr);

	UiErrorOkDialog(title, text);
	app_fatal(NULL);
}

/**
 * @brief Terminates the game with a file not found error dialog.
 */
void FileErrDlg(const char *error)
{
	char text[1024];

	FreeDlg();

	if (!error)
		error = "";
	snprintf(
	    text,
	    1024,
	    "Unable to open a required file.\n"
	    "\n"
	    "Verify that the MD5 of diabdat.mpq matches one of the following values\n"
	    "011bc6518e6166206231080a4440b373\n"
	    "68f049866b44688a7af65ba766bef75a\n"
	    "\n"
	    "The problem occurred when loading:\n%s",
	    error);

	UiErrorOkDialog("Data File Error", text);
	app_fatal(NULL);
}

/**
 * @brief Terminates the game with an insert CD error dialog.
 */
void InsertCDDlg(const char *fileName)
{
	char text[1024];
	snprintf(
	    text,
	    1024,
	    "Unable to open %s.\n"
	    "\n"
	    "Make sure that it is in the game folder and that the file name is in all lowercase.",
	    fileName);

	UiErrorOkDialog("Data File Error", text);
	app_fatal(NULL);
}

/**
 * @brief Terminates the game with a read-only directory error dialog.
 */
void DirErrorDlg(char *error)
{
	char text[1024];

	snprintf(text, 1024, "Unable to write to location:\n%s", error);

	UiErrorOkDialog("Read-Only Directory Error", text);
	app_fatal(NULL);
}

DEVILUTION_END_NAMESPACE
