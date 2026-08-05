
#include "integrity.hpp"
#include "sha256.hpp"
#include "embedded_checksums.hpp"
#include "log.hpp"
#include <stdio.h>
#include <string.h>

int verify_module_integrity(const char *mod_dir) {
    int errors = 0;
    size_t count = sizeof(g_embedded_checksums) / sizeof(g_embedded_checksums[0]);

    if (count == 0) {
        log_warn("Security", "No embedded checksums present in binary segment.");
        return 0;
    }

    for (size_t i = 0; i < count; i++) {
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", mod_dir, g_embedded_checksums[i].rel_path);

        char computed_hex[65];
        if (sha256_file(full_path, computed_hex) != 0) {
            log_error("Security", "Failed to compute checksum for missing file: %s", g_embedded_checksums[i].rel_path);
            printf("[-] Missing file: %s\n", g_embedded_checksums[i].rel_path);
            errors++;
            continue;
        }

        if (strcmp(computed_hex, g_embedded_checksums[i].expected_sha256) != 0) {
            log_error("Security", "[TAMPERING DETECTED] %s SHA-256 mismatch!", g_embedded_checksums[i].rel_path);
            printf("[!] TAMPERING DETECTED: %s SHA-256 mismatch!\n", g_embedded_checksums[i].rel_path);
            printf("    Expected: %s\n", g_embedded_checksums[i].expected_sha256);
            printf("    Got:      %s\n", computed_hex);
            errors++;
        }
    }

    if (errors > 0) {
        log_error("Security", "CRITICAL: %d files failed embedded binary integrity check!", errors);
        printf("[!] CRITICAL: %d files failed embedded binary integrity check!\n", errors);
        return 1;
    }

    log_info("Security", "Embedded binary SHA-256 integrity verified successfully (0 mismatches).");
    printf("[+] Embedded binary SHA-256 integrity verified successfully (0 mismatches).\n");
    return 0;
}
