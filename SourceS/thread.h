#pragma once

namespace dvl {

typedef struct event_emul {
	SDL_mutex *mutex;
	SDL_cond *cond;
} event_emul;

event_emul *StartEvent();
void EndEvent(event_emul *event);
void SetEvent(event_emul *e);
void ResetEvent(event_emul *e);
int WaitForEvent(event_emul *e);
SDL_Thread *CreateThread(unsigned int (*handler)(void *), SDL_threadID *ThreadID);

} // namespace dvl
