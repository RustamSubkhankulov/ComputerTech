#pragma once

//=========================================================

// SIGSEND - lib, that transport data between 
//           processes using signals

//=========================================================

#include <signal.h>

//---------------------------------------------------------

#include "config.h"
#include "../../utility/err.h"

//=========================================================

// Functions to be called before using sigsend lib

int sigsend_snd_start(void);

int sigsend_rcv_start(void);

//---------------------------------------------------------

// Main send / receive operations

int sigsend_snd(char val);

int sigsend_rcv(char* val_ptr);

//---------------------------------------------------------

// Functions to be called at the end of sisgsend lib usage

int sigsend_snd_end(void);

int sigsend_rcv_end(void);

//---------------------------------------------------------

