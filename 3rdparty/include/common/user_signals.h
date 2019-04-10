/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once
#include <signal.h>

/// Defines for specific signals

/// signal for backtracing all threads in the current process
#define SIGCALLSTACK SIGRTMIN

/// signal for backtracing another process
#define SIGINTERPROCESSCALLSTACK SIGCALLSTACK + 1
