#ifndef CLI_H_
#define CLI_H_

// NOTE: we assume here that app_state has been initialized with defaults
#include "app_state.h"

void handle_args(int argc, char *argv[], AppState *app_state); 



#endif // CLI_H_
