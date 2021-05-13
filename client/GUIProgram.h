#pragma once

/**
 * Prevents SDL from overriding main().
 */
#ifdef __MINGW32__
#undef main
#endif
