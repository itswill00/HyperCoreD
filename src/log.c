
#include "log.hpp"
#include <sys/time.h>

static const char *s_level_names[] = {
    "Info ",
    "State",
    "Warn ",
    "Error"
};

void log_write(log_level_t level, const char *tag, const char *fmt, ...) {
    FILE *f = fopen(LOG_PATH, "a");
    if (!f) return;

    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *tm_info = localtime(&tv.tv_sec);

    char tbuf[64];
    size_t len = strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", tm_info);
    snprintf(tbuf + len, sizeof(tbuf) - len, ".%03d", (int)(tv.tv_usec / 1000));

    const char *lvl_str = (level >= 0 && level <= LOG_LEVEL_Error) ? s_level_names[level] : "Info ";
    const char *tag_str = (tag && tag[0] != '\0') ? tag : "System";

    fprintf(f, "[%s] [%s] [%-8s] ", tbuf, lvl_str, tag_str);

    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    fprintf(f, "\n");
    va_end(args);

    fclose(f);
}

void rotate_log(void) {
    struct stat st;
    if (stat(LOG_PATH, &st) != 0 || st.st_size <= 102400) return;

    FILE *in = fopen(LOG_PATH, "r");
    if (!in) return;

    #define MAX_RING_LINES 250
    #define MAX_LINE_LEN 256
    static char s_ring_buffer[MAX_RING_LINES][MAX_LINE_LEN];

    char buf[MAX_LINE_LEN];
    int count = 0;

    while (fgets(buf, sizeof(buf), in)) {
        int slot = count % MAX_RING_LINES;
        strncpy(s_ring_buffer[slot], buf, MAX_LINE_LEN - 1);
        s_ring_buffer[slot][MAX_LINE_LEN - 1] = '\0';
        count++;
    }
    fclose(in);

    FILE *out = fopen(LOG_PATH, "w");
    if (!out) return;

    int total = (count < MAX_RING_LINES) ? count : MAX_RING_LINES;
    int start_idx = (count < MAX_RING_LINES) ? 0 : (count % MAX_RING_LINES);

    for (int i = 0; i < total; i++) {
        int slot = (start_idx + i) % MAX_RING_LINES;
        fputs(s_ring_buffer[slot], out);
    }
    fclose(out);
}
