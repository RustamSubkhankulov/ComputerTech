#pragma once 

// Configuration define - use RT signals instead of regular once
#define RT_SIGS

// Configuration define - do not use RT signals instead of regular once
// #define REG_SISG

#if defined(RT_SIGS) && defined (REG_SIGS)

    #error "Cannot use more than one type of signals"

#endif 

#if !defined(RT_SIGS) && !defined (REG_SIGS)

    #error "Need to choose one type of signals at least"

#endif 
