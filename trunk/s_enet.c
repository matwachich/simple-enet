#include <stdio.h>

#include "enet/enet.h"
#include "s_enet.h"

/* Init/Shutdown */
/** \brief
 *
 * \return
 */
int         SENET_API   SE_Startup ()
{
    if (enet_initialize() != 0) {
        return 0;
    }
    return 1;
}

/** \brief
 *
 * \return
 */
void        SENET_API   SE_Shutdown ()
{
    enet_deinitialize();
}

inline void debug (const char* data)
{
    printf(data);
    fflush(NULL);
}
