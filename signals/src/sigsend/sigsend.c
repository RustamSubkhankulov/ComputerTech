#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

//---------------------------------------------------------

#include "../../inc/sigsend/sigsend.h"

//=========================================================

static int sigsend_snd_start_rt(void);
static int sigsend_rcv_start_rt(void);

static int sigsend_snd_rt(char val);
static int sigsend_rcv_rt(char* val_ptr);

static int sigsend_snd_end_rt(void);
static int sigsend_rcv_end_rt(void);

//---------------------------------------------------------

static int sigsend_snd_start_reg(void);
static int sigsend_rcv_start_reg(void);

static int sigsend_snd_reg(char val);
static int sigsend_rcv_reg(char* val_ptr);

static int sigsend_snd_end_reg(void);
static int sigsend_rcv_end_reg(void);

//=========================================================
// API FUNCTIONS
//=========================================================

int sigsend_snd_start(void)
{
    #ifdef RT_SIGS
        return sigsend_snd_start_rt();
    #endif 

    #ifdef REG_SIGS
        return sigsend_snd_start_reg();
    #endif 

    return 0;
}

//---------------------------------------------------------

int sigsend_rcv_start(void)
{
    #ifdef RT_SIGS
        return sigsend_rcv_start_rt();
    #endif 

    #ifdef REG_SIGS
        return sigsend_rcv_start_reg();
    #endif 

    return 0;
}

//---------------------------------------------------------

int sigsend_snd(char val)
{
    #ifdef RT_SIGS
        return sigsend_snd_rt(val);
    #endif 

    #ifdef REG_SIGS
        return sigsend_snd_reg();
    #endif 

    return 0;
}

//---------------------------------------------------------

int sigsend_rcv(char* val_ptr)
{
    #ifdef RT_SIGS
        return sigsend_rcv_rt(val_ptr);
    #endif 

    #ifdef REG_SIGS
        return sigsend_rcv_reg();
    #endif 

    return 0;
}

//---------------------------------------------------------

int sigsend_snd_end(void)
{
    #ifdef RT_SIGS
        return sigsend_snd_end_rt();
    #endif 

    #ifdef REG_SIGS
        return sigsend_snd_end_reg();
    #endif 

    return 0;
}

//---------------------------------------------------------

int sigsend_rcv_end(void)
{
    #ifdef RT_SIGS
        return sigsend_rcv_end_rt();
    #endif 

    #ifdef REG_SIGS
        return sigsend_rcv_end_reg();
    #endif 

    return 0;
}

//=========================================================
// REAL-TIME SIG FUNCTIONS
//=========================================================

static int sigsend_snd_start_rt(void)
{
    return 0;
}

//---------------------------------------------------------

static int sigsend_rcv_start_rt(void)
{
    // routine
}

//---------------------------------------------------------

static int sigsend_snd_rt(char val)
{
    // routine
}

//---------------------------------------------------------

static int sigsend_rcv_rt(char* val_ptr)
{
    assert(val_ptr);

    // routine
}

//---------------------------------------------------------

static int sigsend_snd_end_rt(void)
{
    return 0;
}

//---------------------------------------------------------

static int sigsend_rcv_end_rt(void)
{
    // routine
}

//=========================================================
// REGULAR SIG FUNCTIONS
//=========================================================

static int sigsend_snd_start_reg(void)
{

}

//---------------------------------------------------------

static int sigsend_rcv_start_reg(void)
{

}

//---------------------------------------------------------

static int sigsend_snd_reg(char val)
{

}

//---------------------------------------------------------

static int sigsend_rcv_reg(char* val_ptr)
{

}

//---------------------------------------------------------

static int sigsend_snd_end_reg(void)
{

}

//---------------------------------------------------------

static int sigsend_rcv_end_reg(void)
{
    
}