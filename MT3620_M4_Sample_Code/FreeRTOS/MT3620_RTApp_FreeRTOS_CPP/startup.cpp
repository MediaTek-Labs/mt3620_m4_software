#include <sys/cdefs.h>
#include "mt3620.h"

_Noreturn extern void CcMain(void);

#ifdef __cplusplus
extern "C" {
#endif

_Noreturn void RTCoreMain(void);

typedef void (*InitFunc)(void);
extern InitFunc __init_array_start;
extern InitFunc __init_array_end;

#ifdef __cplusplus
}
#endif

_Noreturn void RTCoreMain(void)
{
	// Setup vector table
	NVIC_SetupVectorTable();

	// Call global constructors
	for (InitFunc* func = &__init_array_start; func < &__init_array_end; ++func) (*func)();

	CcMain();
}
