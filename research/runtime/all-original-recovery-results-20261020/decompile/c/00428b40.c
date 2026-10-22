
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00428b40(int param_1)

{
  int *piVar1;
  int *piVar2;
  undefined4 uVar3;
  bool bVar4;
  int iVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  undefined4 uVar8;
  int iVar9;
  float *pfVar10;
  float10 fVar11;
  undefined4 uStack_28;
  undefined4 uStack_24;
  undefined1 auStack_20 [4];
  undefined1 auStack_1c [4];
  undefined1 auStack_18 [4];
  undefined1 auStack_14 [8];
  undefined4 uStack_c;
  undefined1 auStack_8 [8];
  
  piVar1 = *(int **)(param_1 + 0xbe);
  piVar2 = (int *)*piVar1;
  do {
    if (piVar2 == piVar1) {
      return;
    }
    piVar1 = (int *)piVar2[4];
    bVar4 = false;
    if ((*(char *)(piVar1[2] + 4) == '\0') ||
       ((float)_DAT_005c5390 <= *(float *)((int)piVar1 + 0x1b))) {
LAB_00428c17:
      if ((*(char *)(param_1 + 0x18) != '\0') && (DAT_006e1f0c == 0)) goto LAB_00428c41;
      if (piVar1[4] <= *(int *)(param_1 + 0x14)) {
        (**(code **)(*piVar1 + 0x18))();
        goto LAB_00428c41;
      }
      if ((char)piVar1[5] != '\0') goto LAB_00428c41;
    }
    else {
      FUN_00466870(*(undefined4 *)((int)piVar1 + 0x27));
      fVar11 = (float10)FUN_0059bcc0();
      if ((float10)_DAT_005d8658 <= fVar11) goto LAB_00428c17;
LAB_00428c41:
      bVar4 = true;
    }
    FUN_0042dd00();
    if (bVar4) {
      if ((*(char *)(piVar1[2] + 4) != '\0') && (-1 < *(int *)((int)piVar1 + 0x23))) {
        iVar5 = FUN_00466870(*(undefined4 *)((int)piVar1 + 0x27));
        if (iVar5 == 0) {
          fVar11 = (float10)_DAT_005c5380;
        }
        else {
          fVar11 = (float10)*(int *)(iVar5 + 0x922) / (float10)(*(int *)(iVar5 + 0x926) + 1);
        }
        pfVar10 = (float *)&DAT_005c5348;
        iVar5 = 0;
        do {
          if ((float)((((float10)*(float *)((int)piVar1 + 0x2b) + (float10)_DAT_005c5378) *
                      ((float10)(*(int *)((int)piVar1 + 0x2f) + 1) /
                      (float10)(*(int *)((int)piVar1 + 0x33) + 1))) /
                     (fVar11 + (float10)_DAT_005c5378)) <= *pfVar10) break;
          pfVar10 = pfVar10 + 1;
          iVar5 = iVar5 + 1;
        } while ((int)pfVar10 < 0x5c5350);
        FUN_00477040(*(undefined4 *)((int)piVar1 + 0x23),iVar5);
      }
      iVar5 = (int)piVar1 + 0x1f;
      puVar6 = (undefined4 *)FUN_0042d8c0(auStack_8,iVar5);
      uVar3 = *puVar6;
      uStack_c = puVar6[1];
      uStack_28 = 0;
      FUN_00431a40(uVar3,uStack_c,&uStack_28,uStack_24);
      FUN_0042b6f0(auStack_20,uVar3,uStack_c);
      puVar6 = (undefined4 *)FUN_0042e520(auStack_1c,iVar5);
      puVar7 = (undefined4 *)FUN_0042d8a0(auStack_18,iVar5);
      uVar3 = *puVar6;
      uVar8 = *puVar7;
      uStack_24 = 0;
      FUN_0042fb00(uVar8,uVar3,&uStack_24);
      FUN_0042b6f0(auStack_14,uVar8,uVar3);
      *(undefined4 *)(piVar1[2] + 5) = 0;
      uVar3 = DAT_005d854c;
      DAT_005d854c = 3;
      uVar8 = FUN_00407920();
      iVar5 = FUN_004079f0(uVar8);
      iVar9 = FUN_004079f0(uVar8);
      while (iVar9 != 0) {
        iVar9 = FUN_004079f0(uVar8);
      }
      DAT_005d854c = uVar3;
      if (iVar5 != 0) {
        FUN_00416fe0(piVar1[2]);
      }
      if (piVar1 != (int *)0x0) {
        FUN_00418050();
        FUN_00597870(piVar1);
      }
    }
    piVar1 = *(int **)(param_1 + 0xbe);
  } while( true );
}

