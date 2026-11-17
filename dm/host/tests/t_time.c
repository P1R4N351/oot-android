// Clock snapshot arithmetic (hle_time.c, from PR #36): CalculateSpanBetween and CalculateStandardUserSystemClockDifferenceByUser looked up by their mangled names in the real swforce
// registrations, run on both host snapshot entries and on edited copies
#include "../hle_time.c"
#include "../hle_net.c"   // DM_ONLINE: nifm proxy setting and nsd name resolution
#include <mach-o/getsect.h>

void dm_guest_sleep_ns(int64_t ns) { (void)ns; }   // hle_net.c (socket Poll) links these
void *guest_sym(const char *n) { (void)n; return NULL; }
uint64_t guest_ns(void) { return clock_gettime_nsec_np(CLOCK_MONOTONIC); }
static int checks, fails;
extern const struct mach_header_64 _mh_execute_header;
#define CHECK(c, msg) do { checks++; if (!(c)) { fails++; fprintf(stderr, "FAIL: %s\n", msg); } } while (0)
static void *hook(const char *name) {
    unsigned long bytes = 0;
    const hle_force_ent *e = (const hle_force_ent *)getsectiondata(&_mh_execute_header, "__DATA", "swforce", &bytes);
    void *fn = NULL; unsigned count = 0;
    for (size_t i = 0; e && i < bytes / sizeof *e; i++) if (!strcmp(e[i].name, name)) { count++; fn = (void *)e[i].fn; }
    CHECK(count == 1 && fn, name);
    return count == 1 ? fn : NULL;
}
static void put64(uint8_t *s, int off, int64_t v) { memcpy(s + off, &v, 8); }
static int64_t get64(const uint8_t *s, int off) { int64_t v; memcpy(&v, s + off, 8); return v; }
int main(void) {
    Result (*snap)(uint8_t *) = hook("_ZN2nn4time13ClockSnapshot29CreateWithStandardSystemClockEPS1_");
    Result (*adj_snap)(uint8_t *) = hook("_ZN2nn4time13ClockSnapshot31CreateWithAdjustableSystemClockEPS1_");
    unsigned (*span)(int64_t *, const uint8_t *, const uint8_t *) = hook("_ZN2nn4time20CalculateSpanBetweenEPNS_8TimeSpanERKNS0_13ClockSnapshotES5_");
    int64_t (*udiff)(const uint8_t *, const uint8_t *) = hook("_ZN2nn4time48CalculateStandardUserSystemClockDifferenceByUserERKNS0_13ClockSnapshotES3_");
    if (!snap || !adj_snap || !span || !udiff) { printf("time: %d checks, %d failed\n", checks, fails); return 1; }
    uint8_t a[0xd0], b[0xd0], c[0xd0]; int64_t ts = -1;
    // Before the first snapshot: h_time_GetCurrentTime reads DM_FIXED_TIME once and caches it
    setenv("DM_FIXED_TIME", "1700000000", 1);
    setenv("DM_ONLINE", "1", 1);   // read once: the network clock then follows the user clock
    setenv("TZ", "UTC", 1); tzset();
    {   // DM_ONLINE=1: the network clock reads the user clock and is accurate (offline both fail)
        Result (*net)(int64_t *) = hook("_ZN2nn4time26StandardNetworkSystemClock14GetCurrentTimeEPNS0_9PosixTimeE");
        unsigned (*acc)(void) = hook("_ZN2nn4time26StandardNetworkSystemClock20IsAccuracySufficientEv");
        int64_t nt = 0;
        CHECK(net && net(&nt) == RES_OK && nt == 1700000000, "online: the network clock is the user clock");
        CHECK(acc && acc() == 1, "online: the network clock is accurate");
    }
    {   // DM_ONLINE=1: the proxy query succeeds with no proxy, the nsd resolution keeps the name (offline both fail)
        Result (*proxy)(void *) = hook("_ZN2nn4nifm22GetCurrentProxySettingEPNS0_12ProxySettingE");
        Result (*resex)(void *, const void *) = hook("_ZN2nn3nsd9ResolveExEPNS0_4FqdnERKS1_");
        Result (*res)(void *, const void *) = hook("_ZN2nn3nsd7ResolveEPNS0_4FqdnERKS1_");
        uint8_t ps[0xaa]; memset(ps, 0xaa, sizeof ps);
        CHECK(proxy && proxy(ps) == RES_OK && ps[0] == 0 && ps[0xa9] == 0, "online: no proxy, buffer cleared");
        char in[256] = "receive-lp1.dg.srv.nintendo.net", out[256]; memset(out, 0x7f, sizeof out);
        CHECK(resex && resex(out, in) == RES_OK && !strcmp(out, in) && out[255] == 0, "online: ResolveEx keeps the name");
        CHECK(res && res(out, in) == RES_OK && !strcmp(out, in), "online: Resolve keeps the name");
        CHECK(resex && resex(NULL, in) != RES_OK, "online: ResolveEx without a buffer fails");
    }
    CHECK(snap(a) == RES_OK, "snapshot a");
    CHECK(a[0xcd] == 1, "standard snapshot type is 1");
    // Both entries must stay inside the SDK's 0xd0-byte buffer, including the type near its end.
    uint8_t guarded[0xf0]; memset(guarded, 0xa5, sizeof guarded);
    uint8_t *adjustable = guarded + 0x10;
    CHECK(snap(adjustable) == RES_OK, "standard snapshot in guarded buffer");
    int standard_guards_ok = 1;
    for (int i = 0; i < 0x10; i++) standard_guards_ok &= guarded[i] == 0xa5 && guarded[0xe0 + i] == 0xa5;
    CHECK(standard_guards_ok, "standard snapshot writes only its 0xd0 bytes");
    memset(guarded, 0xa5, sizeof guarded);
    CHECK(adj_snap(adjustable) == RES_OK, "adjustable snapshot");
    int guards_ok = 1;
    for (int i = 0; i < 0x10; i++) guards_ok &= guarded[i] == 0xa5 && guarded[0xe0 + i] == 0xa5;
    CHECK(guards_ok, "adjustable snapshot writes only its 0xd0 bytes");
    CHECK(adjustable[0xcd] == 2, "adjustable snapshot type is 2");
    CHECK(get64(adjustable, 0x40) == 1700000000, "adjustable user time follows the host clock");
    const CalendarTime *cal = (const CalendarTime *)(adjustable + 0x50);
    CHECK(cal->year == 2023 && cal->month == 11 && cal->day == 14 && cal->hour == 22 && cal->minute == 13 && cal->second == 20,
          "adjustable snapshot has the UTC calendar for the fixed time");
    uint8_t zero[0x20] = {0};
    CHECK(!memcmp(adjustable + 0x20, zero, sizeof zero) && get64(adjustable, 0x48) == 0,
          "offline snapshot has no network clock context or time");
    CHECK(!memcmp(a + 0x98, adjustable + 0x98, 16), "both snapshots use the same steady clock source");
    CHECK(udiff(a, adjustable) == 0, "changing snapshot type does not report a user clock change");
    CHECK(!strcmp((const char *)adjustable + 0xa8, "UTC"), "adjustable snapshot carries the host time zone");
    memcpy(b, a, sizeof b); b[0xcd] = 2;
    CHECK(span(&ts, a, b) == 1 && ts == 0, "changing only the type leaves the elapsed span at zero");
    unsetenv("DM_FIXED_TIME");
    CHECK(span(&ts, a, a) == 1 && ts == 0, "span of a snapshot with itself is 0");
    memcpy(b, a, sizeof b); put64(b, 0x90, get64(a, 0x90) + 90);   // b: 90 s of steady clock later
    CHECK(span(&ts, a, b) == 1 && ts == 90 * 1000000000LL, "span from the steady time points, in nanoseconds");
    CHECK(span(&ts, b, a) == 1 && ts == -90 * 1000000000LL, "span backwards is negative");
    memcpy(c, b, sizeof c); c[0x98] ^= 1; ts = 1234;   // c: another steady clock source, no network time
    CHECK(span(&ts, a, c) == 0 && ts == 1234, "different sources and no network time: false, output untouched");
    put64(a, 0x48, 1000); put64(c, 0x48, 1600);   // with network times the span comes from them
    CHECK(span(&ts, a, c) == 1 && ts == 600 * 1000000000LL, "different sources: span from the network times");
    CHECK(snap(a) == RES_OK && snap(b) == RES_OK && udiff(a, b) == 0, "two host snapshots in a row: the user did not move the clock");
    memcpy(b, a, sizeof b); put64(b, 0x00, get64(a, 0x00) + 3600);   // b: the user clock offset one hour later
    CHECK(udiff(a, b) == 3600 * 1000000000LL, "user clock moved an hour");
    b[0x10] ^= 1; CHECK(udiff(a, b) == 0, "user contexts of different steady sources: 0");
    b[0x10] ^= 1; a[0xcc] = b[0xcc] = 1; CHECK(udiff(a, b) == 0, "automatic correction on in both: 0");
    a[0xcc] = 0; CHECK(udiff(a, b) == 3600 * 1000000000LL, "automatic correction in one only: the difference");
    printf("time: %d checks, %d failed\n", checks, fails);
    return fails != 0;
}
