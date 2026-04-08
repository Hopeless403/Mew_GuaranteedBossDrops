/*
 * GuaranteedBossDrops.c
 *
 * Build:
 * - version without rare guarantee:
 *   cl /LD /O2 /GS- /W3 /D_CRT_SECURE_NO_WARNINGS GuaranteedBossDrops.c / Fe : GuaranteedBossDrops.dll
 * - version with rare guarantee:
 *   cl /LD /O2 /GS- /W3 /D_CRT_SECURE_NO_WARNINGS /DFORCE_RARE GuaranteedBossDrops.c /Fe:GuaranteedBossDrops.dll
 */

#ifndef GuaranteedBossDrops_H
#define GuaranteedBossDrops_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ===================================================================
 *  RVAs — offsets from game base
 * =================================================================== */

 /* Hook : BossRewards::OnDeath and RNG_Roll */
#define RVA_BOSS_ONDEATH	 0x7d5b10
#define RVA_RNG_ROLL         0x943300

 /* ===================================================================
  *  Function pointer typedefs
  * =================================================================== */

typedef void   (*fn_boss_on_death)(void* reward, void* a2, void* a3);
typedef double (*fn_rng_roll)(double a1, unsigned long long a2);

#endif /* GuaranteedBossDrops_H */
