#ifndef EMBEDDED_CHECKSUMS_HPP
#define EMBEDDED_CHECKSUMS_HPP

typedef struct {
    const char *rel_path;
    const char *expected_sha256;
} file_checksum_t;

static const file_checksum_t g_embedded_checksums[] = {
    { "post-fs-data.sh", "08f4818bacf22eb061098631930934e5e5822ad9dc5f99fe7875af48ba1e7d40" },
    { "service.sh", "f22365b95de7bacfc5708daa6e7257d100d8c06bd3e53f7da6d6867329a27504" },
    { "system.prop", "47c1c0edbf58137f6bbe62c34b29de2347ce6b8a1a792db508061785571eb196" },
    { "module.prop", "5dc81d0fa65bc6fabd866ddd911aeb4c42284a31eedc6709bf06bcde8f51d676" },
    { "update.json", "af4adb573acb11ddb8309b2d6d9979db68d412a9b2df9115786e52352006dbe6" },
    { "changelog.md", "9dd0a4fb72b3bed83b1743369b9f6eae6218bf755f91d3bda4f558f94219970c" },
    { "NOTICE.md", "43a0ad79380851e3ee0403f5c6d3badfc840a7716d581a3e48d60f86d1d1d771" },
    { "uninstall.sh", "d744ee35a8c5a6a7f5f9d727655579aab4035efa698d2a00cfbb4948a6fd8d1f" },
    { "webroot/index.html", "32e6e411d7c7b979e39b4faa9ac5ee168d15bff1df19b2f660ffea140d997621" },
    { "banner.jpg", "7c529f60727b6cce0736f8df7fe7d14087fb15da15f97ff20cb289456c62def6" },
};

#endif /* EMBEDDED_CHECKSUMS_HPP */
