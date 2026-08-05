
#include "sha256.hpp"
#include <stdio.h>
#include <string.h>

int sha256_file(const char *filepath, char out_hex[65]) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "sha256sum '%s' 2>/dev/null", filepath);
    FILE *fp = popen(cmd, "r");
    if (!fp) return -1;

    char buf[128] = "";
    if (fgets(buf, sizeof(buf), fp) != NULL) {
        pclose(fp);
        if (strlen(buf) >= 64) {
            strncpy(out_hex, buf, 64);
            out_hex[64] = '\0';
            return 0;
        }
    } else {
        pclose(fp);
    }
    return -1;
}
