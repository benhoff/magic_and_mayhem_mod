
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ulonglong __thiscall
FUN_004284e0(int param_1,float param_2,int *param_3,float param_4,int param_5,undefined4 param_6,
            char param_7)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  float fVar3;
  float fVar4;
  double dVar5;
  float fVar6;
  char cVar7;
  undefined4 uVar8;
  int iVar9;
  uint uVar10;
  uint uVar11;
  undefined4 uVar12;
  int iVar13;
  int iVar14;
  int extraout_EDX;
  int *piVar15;
  float10 fVar16;
  undefined8 uVar17;
  undefined1 local_5;
  
  fVar6 = param_2;
  local_5 = 0;
  if (*(char *)(param_1 + 0x11fe0) == '\0') goto LAB_004285b5;
  *(undefined1 *)(param_1 + 0x11fe0) = 0;
  piVar15 = *(int **)(param_1 + 8);
  *(int **)(param_1 + 0x11fe1) = piVar15;
  *(undefined4 *)(param_1 + 0x11fe9) = 0;
  if (piVar15 != *(int **)(param_1 + 0xc)) {
    uVar1 = *(undefined4 *)(param_1 + 0x14);
    puVar2 = (undefined4 *)*piVar15;
    puVar2[2] = param_2;
    uVar8 = FUN_00407920();
    iVar9 = FUN_004079f0(uVar8);
    *(undefined4 *)((int)puVar2 + 0x27) = *(undefined4 *)(iVar9 + 0x174);
    FUN_00407b10();
    (**(code **)*puVar2)(fVar6,param_1 + 0x11c02,uVar1);
  }
  if (*(int *)((int)fVar6 + 0x39) != 0) goto LAB_004285b5;
  param_3[2] = param_3[2] + -1;
  uVar11 = param_3[1];
  iVar9 = param_3[2];
  if (DAT_006e1f64 == 0) {
LAB_004285a4:
    uVar10 = 0;
  }
  else {
    uVar10 = DAT_006e1f64 - 1;
    if (uVar10 != 0) goto LAB_004285aa;
    if (DAT_006a4910 == 0) goto LAB_004285a4;
    uVar17 = rdtsc();
    uVar11 = (int)uVar17 - (int)((ulonglong)uVar17 >> 0x20);
    uVar10 = uVar11 / DAT_006a4914;
    uVar11 = uVar11 % DAT_006a4914;
    iVar9 = *param_3 - uVar10;
    if (iVar9 < 1) goto LAB_004285aa;
  }
  if (iVar9 < 1) {
LAB_004285aa:
    return CONCAT44(uVar11,uVar10) & 0xffffffffffffff00;
  }
LAB_004285b5:
  piVar15 = *(int **)(param_1 + 0x11fe1);
  if (piVar15 != *(int **)(param_1 + 0xc)) {
    do {
      if (piVar15 != *(int **)(param_1 + 0x11fe1)) {
        puVar2 = (undefined4 *)*piVar15;
        uVar1 = *(undefined4 *)(param_1 + 0x14);
        puVar2[2] = fVar6;
        uVar8 = FUN_00407920();
        iVar9 = FUN_004079f0(uVar8);
        *(undefined4 *)((int)puVar2 + 0x27) = *(undefined4 *)(iVar9 + 0x174);
        uVar8 = DAT_005d854c;
        DAT_005d854c = 0;
        *(undefined4 *)((int)fVar6 + 0x41) = 0x1010101;
        *(undefined4 *)((int)fVar6 + 0x45) = 0x1010101;
        uVar12 = FUN_00407920();
        iVar9 = FUN_00407990(uVar12);
        while (iVar9 != 0) {
          FUN_00407b10();
          iVar9 = FUN_00407990(uVar12);
        }
        DAT_005d854c = uVar8;
        (**(code **)*puVar2)(fVar6,param_1 + 0x11c02,uVar1);
      }
      cVar7 = (**(code **)(*(int *)*piVar15 + 8))(&param_2,param_3,*(undefined4 *)(param_1 + 0x14));
      fVar4 = param_2;
      while (param_2 = fVar4, cVar7 != '\0') {
        if ((param_5 == 0) || (*(char *)(param_5 + 0x15) != '\0')) {
LAB_00428751:
          if ((param_7 == '\0') || (param_2 != (float)_DAT_005c5390)) {
            param_2 = param_2 + param_4;
            if (((*(char *)(param_1 + 0x11ddc) == '\0') || (param_2 == (float)_DAT_005c5390)) ||
               (*(float *)(param_1 + 0x11ddd) == (float)_DAT_005c5390)) {
              if (*(float *)(param_1 + 0x11ddd) <= param_2) {
                if (*(float *)(param_1 + 0x11ddd) < param_2) {
                  *(undefined4 *)(param_1 + 0x11de1) = 0;
                }
                iVar9 = *(int *)(param_1 + 0x11de1) + 1;
                *(int *)(param_1 + 0x11de1) = iVar9;
                uVar11 = FUN_0054e200();
                if ((int)((longlong)(ulonglong)(uVar11 & 0x7fffffff) % (longlong)iVar9) == 0) {
                  *(float *)(param_1 + 0x11ddd) = param_2;
                  if ((char)param_6 != '\0') {
                    (**(code **)(*(int *)*piVar15 + 0xc))();
                    *(int **)(param_1 + 0x11fe5) = piVar15;
                    local_5 = 1;
                  }
                }
              }
            }
            else {
              dVar5 = _DAT_005c5380;
              if ((float)_DAT_005c53a0 < param_2) {
                dVar5 = _DAT_005c5390;
              }
              fVar4 = (float)dVar5 + *(float *)(param_1 + 0x11fe9);
              *(float *)(param_1 + 0x11fe9) = fVar4;
              if (((fVar4 == (float)_DAT_005c5380) ||
                  (uVar11 = FUN_0054e200(),
                  (float)(uVar11 & 0x7fffffff) * _DAT_005c5398 <
                  (float)dVar5 / *(float *)(param_1 + 0x11fe9))) && ((char)param_6 != '\0')) {
                (**(code **)(*(int *)*piVar15 + 0xc))();
                *(int **)(param_1 + 0x11fe5) = piVar15;
                local_5 = 1;
                if (*(float *)(param_1 + 0x11ddd) < param_2) {
                  *(float *)(param_1 + 0x11ddd) = param_2;
                }
              }
            }
          }
        }
        else if ((*(char *)(param_5 + 0x1a) == '\0') || (fVar4 == (float)_DAT_005c5390)) {
          fVar3 = _DAT_005c5388 - fVar4 * *(float *)(param_5 + 0x1b);
          if (((float)_DAT_005c5380 < fVar3) &&
             (_DAT_005d8654 < (fVar4 - *(float *)(param_5 + 0x1b)) / fVar3)) goto LAB_00428751;
          if (*(float *)(param_5 + 0x1b) < (float)_DAT_005c5390) {
            FUN_0040a3b0();
            fVar16 = (float10)FUN_0059bcc0();
            if ((fVar16 < (float10)_DAT_005d8658) && ((float)_DAT_005c5380 < fVar4))
            goto LAB_00428751;
          }
        }
        cVar7 = (**(code **)(*(int *)*piVar15 + 8))
                          (&param_2,param_3,*(undefined4 *)(param_1 + 0x14));
        fVar4 = param_2;
      }
      uVar11 = param_3[1];
      iVar9 = param_3[2];
      if (DAT_006e1f64 == 0) {
LAB_0042891d:
        uVar10 = 0;
      }
      else {
        uVar10 = DAT_006e1f64 - 1;
        if (uVar10 != 0) goto LAB_00428936;
        if (DAT_006a4910 == 0) goto LAB_0042891d;
        uVar17 = rdtsc();
        uVar11 = (int)uVar17 - (int)((ulonglong)uVar17 >> 0x20);
        uVar10 = uVar11 / DAT_006a4914;
        uVar11 = uVar11 % DAT_006a4914;
        iVar9 = *param_3 - uVar10;
        if (iVar9 < 1) goto LAB_00428936;
      }
      if (iVar9 < 1) {
LAB_00428936:
        *(int **)(param_1 + 0x11fe1) = piVar15;
        return CONCAT44(uVar11,CONCAT31((int3)(uVar10 >> 8),local_5));
      }
      piVar15 = piVar15 + 1;
    } while (piVar15 != *(int **)(param_1 + 0xc));
  }
  *(undefined4 *)(param_1 + 0x11fe1) = 0;
  param_6 = DAT_005d854c;
  DAT_005d854c = 0;
  *(undefined4 *)((int)fVar6 + 0x41) = 0x1010101;
  *(undefined4 *)((int)fVar6 + 0x45) = 0x1010101;
  uVar17 = FUN_00407920();
  iVar9 = (int)((ulonglong)uVar17 >> 0x20);
  iVar13 = (int)uVar17;
  while (piVar15 = *(int **)((int)fVar6 + 0x49 + iVar13 * 4),
        piVar15 != *(int **)((int)fVar6 + 0x2d)) {
    iVar14 = *piVar15;
    *(int **)((int)fVar6 + 0x49 + iVar13 * 4) = piVar15 + 1;
    if (iVar14 == 0) goto LAB_004289ac;
    if ((*(int *)(iVar14 + 0x35) == 0) || (iVar9 = *(int *)(iVar14 + 0x89), DAT_005d854c <= iVar9))
    {
      if (iVar14 == 0) {
LAB_004289ac:
        *(undefined1 *)(iVar13 + 0x41 + (int)fVar6) = 1;
        if (iVar14 == 0) {
          DAT_005d854c = param_6;
          *(undefined1 *)(param_1 + 0x11fe0) = 1;
          return CONCAT44(iVar9,CONCAT31((int3)((uint)param_6 >> 8),local_5));
        }
      }
      FUN_00407b10();
      iVar9 = extraout_EDX;
    }
  }
  iVar14 = 0;
  goto LAB_004289ac;
}

