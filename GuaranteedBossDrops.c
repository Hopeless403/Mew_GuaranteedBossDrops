/*
 * GuaranteedBossDrops.c
 *
 * Build:
 * - version without rare guarantee:
 *   cl /LD /O2 /GS- /W3 /D_CRT_SECURE_NO_WARNINGS GuaranteedBossDrops.c / Fe : GuaranteedBossDrops.dll
 * - version with rare guarantee:
 *   cl /LD /O2 /GS- /W3 /D_CRT_SECURE_NO_WARNINGS /DFORCE_RARE GuaranteedBossDrops.c /Fe:GuaranteedBossDrops.dll
 */

#include "GuaranteedBossDrops.h"
#include "mewjector.h"
#include <stdarg.h>

#define MOD_NAME "GuaranteedBossDrops"

/* ===================================================================
 *  Globals
 * =================================================================== */

static MewjectorAPI mj;
static UINT_PTR g_gameBase = 0;

/* Trampolines */
static fn_boss_on_death g_origBossOnDeath = NULL;
static fn_rng_roll      g_origRNGRoll = NULL;

/* Shared state */
static int rngHookActive = 0;

/* Simple queue */
#define MAX_ROLLS 16
static double nextRngRolls[MAX_ROLLS];
static int queueStart = 0;
static int queueEnd = 0;

/* ===================================================================
 *  Helpers
 * =================================================================== */

static void EnqueueRoll(double v)
{
    if ((queueEnd + 1) % MAX_ROLLS != queueStart) {
        nextRngRolls[queueEnd] = v;
        queueEnd = (queueEnd + 1) % MAX_ROLLS;
    }
}

static int HasRoll(void)
{
    return queueStart != queueEnd;
}

static double DequeueRoll(void)
{
    double v = nextRngRolls[queueStart];
    queueStart = (queueStart + 1) % MAX_ROLLS;
    return v;
}

/* ===================================================================
 *  Logging
 * =================================================================== */

static void Log(const char* fmt, ...)
{
    if (!mj.Log) return;

    char buffer[512];

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);

    mj.Log(MOD_NAME, "%s", buffer);
}

/* ===================================================================
 *  Hook
 * =================================================================== */

static void __fastcall HookBossRewardOnDeath(void* reward, void* a2, void* a3)
{
    /* reward + 0xC8 is a bool */
    int willFire = !(*(unsigned char*)((unsigned char*)reward + 0xC8));
    int getRare = 1;

    Log("BossReward::OnDeath(%d)", !willFire);

    if (willFire) {
        EnqueueRoll(0.0);                 // force drop
#ifdef FORCE_RARE
        EnqueueRoll(getRare ? 0.0 : 1.0); // force rare
#endif
    }

    rngHookActive = 1;

    if (g_origBossOnDeath)
        g_origBossOnDeath(reward, a2, a3);

    rngHookActive = 0;
}

static double __fastcall HookRNGRoll(double a1, unsigned long long a2)
{
    double p = 0.0;

    if (g_origRNGRoll)
        p = g_origRNGRoll(a1, a2);

    if (rngHookActive)
        Log("-> Rolled probability %f", p);

    if (HasRoll()) {
        double newVal = DequeueRoll();
        Log("-> Replacing result with %f", newVal);
        p = newVal;
    }

    return p;
}

/* ===================================================================
 *  Initialization
 * =================================================================== */

static void Initialize(void)
{
    int hookCount = 0;
    void* trampoline = NULL;

    if (!MJ_Resolve(&mj)) {
        return;
    }
    mj.Log(MOD_NAME, "Initializing (API v%d)...", mj.GetVersion());

    g_gameBase = mj.GetGameBase();

    /* Resolve game functions */
    /* // */

    /* Hook 1: BossReward::OnDeath */
    trampoline = NULL;
    if (mj.InstallHook(RVA_BOSS_ONDEATH, 15,
        (void*)HookBossRewardOnDeath,
        &trampoline, 20, MOD_NAME)) {

        g_origBossOnDeath = (fn_boss_on_death)trampoline;
        Log("Hook BossReward::OnDeath installed");
    }
    else {
        mj.Log(MOD_NAME, "  FAILED: Hook 1 (BossReward::OnDeath)");
    }
    /* Hook 2: RollWithLuck */
    trampoline = NULL;
    if (mj.InstallHook(RVA_RNG_ROLL, 18,
        (void*)HookRNGRoll,
        &trampoline, 20, MOD_NAME)) {

        g_origRNGRoll = (fn_rng_roll)trampoline;
        Log("Hook RNGRoll installed");
    }
    else {
        mj.Log(MOD_NAME, "  FAILED: Hook 2 (RNGRoll)");
    }
}

/* ===================================================================
 *  DllMain
 * =================================================================== */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        if (MJ_Resolve(&mj)) {
            mj.Log(MOD_NAME, "Loading!");
        }
        Initialize();
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        if (MJ_Resolve(&mj)) {
            mj.Log(MOD_NAME, "Unloading!");
        }
    }

    return TRUE;
}
