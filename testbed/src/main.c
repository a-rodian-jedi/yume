#include <core/logger.h>
#include <core/asserts.h>
#include <platform/platform.h>

int main() {
    YFATAL("A test fatal error: %s", "Gregorski stinks");
    YERROR("A test error error: %f", 3.14f);
    YWARN("A test warn error: %s", "Jared's feet smell of Doritos");
    YINFO("A test info error: %f", 3.14f);
    YDEBUG("A test debug error: %f", 3.14f);
    YTRACE("A test trace error: %f", 3.14f);

    // YASSERT(FALSE);

    platform_state state;
    if (platform_startup(&state, "Yume Engine Testbed", 100, 100, 1280, 720)) {
        while(TRUE) {
            platform_pump_messages(&state);
        }
    }

    platform_shutdown(&state);

    return 0;
}