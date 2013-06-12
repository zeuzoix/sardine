#ifndef _TWI_DEBUG_H_
#define _TWI_DEBUG_H_
   
typedef int(*TWI_DEBUG_PFN)(char *pc_str);

extern TWI_DEBUG_PFN gpfn_twi_debug;

#ifdef DEBUG
#define TWI_DEBUG(x)  {                   \
      if(NULL != gpfn_twi_debug)          \
      {                                   \
         (void)gpfn_twi_debug(x);         \
      }                                   \
   }
#else
#define TWI_DEBUG(x)  
#endif

#endif
