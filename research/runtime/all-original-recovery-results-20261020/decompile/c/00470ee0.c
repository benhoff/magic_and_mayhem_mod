
void __fastcall FUN_00470ee0(int param_1)

{
  int *piVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005bf53b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  FUN_0057dbd0();
  uVar6 = 0;
  iVar5 = DAT_006def58;
  if (DAT_006df180 != 0) {
    do {
      if ((*(int *)(iVar5 + 4) != 0) &&
         ((*(int *)(iVar5 + 0x68) != 0 || (*(int *)(iVar5 + 0x70) != 0)))) {
        FUN_005074f0();
      }
      uVar6 = uVar6 + 1;
      iVar5 = iVar5 + 0xe4b;
    } while (uVar6 < DAT_006df180);
  }
  DAT_006df17c = 0;
  FUN_005075b0();
  FUN_004dadb0();
  thunk_FUN_00572870();
  FUN_004db980(s_cfg_battletooltip_cfg_005d9f64);
  FUN_004dc5e0();
  if ((DAT_006e1f0c == 0) && (FUN_004da260(), DAT_006e1f0c == 0)) {
    FUN_004dc8b0();
    FUN_004dd1e0();
  }
  FUN_00470d40();
  if (DAT_00644520 == -1) {
    DAT_00644524 = 0;
    iVar5 = DAT_00644524;
    do {
      do {
        DAT_00644524 = iVar5;
        if (7 < iVar5) goto LAB_00471039;
        DAT_00644524 = iVar5 + 1;
        if (iVar5 == -1) {
          piVar1 = (int *)0x0;
        }
        else {
          piVar1 = &DAT_00643a80 + iVar5 * 0x55;
        }
        iVar5 = DAT_00644524;
      } while (*piVar1 == 0);
      if (piVar1 == (int *)0x0) goto LAB_00471039;
      iVar4 = FUN_00466870((int)(piVar1 + -0x190ea0) / 0x154);
      iVar5 = DAT_00644524;
    } while (iVar4 == 0);
    FUN_004f7ae0(iVar4);
  }
LAB_00471039:
  iVar5 = FUN_0057d310(s_sprites_battlebuttons_spr_005da1dc,2);
  if (iVar5 != 0) {
    local_1c = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x1d6;
    local_14 = *(int *)(**(int **)(param_1 + 0x100c3) + 4 + *(int *)(param_1 + 0x100c7)) + local_1c;
    local_18 = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x184;
    local_10 = *(int *)(**(int **)(param_1 + 0x100c3) + 8 + *(int *)(param_1 + 0x100c7)) + local_18;
    if (*(int *)(param_1 + 0x100b7) == 0) {
      puVar2 = (undefined4 *)FUN_00597850(0x10);
      if (puVar2 == (undefined4 *)0x0) {
        puVar2 = (undefined4 *)0x0;
      }
      else {
        puVar2[1] = &LAB_0046fc50;
        puVar2[2] = 0;
        puVar2[3] = param_1;
        *puVar2 = &PTR_LAB_005c5e10;
      }
      *(undefined4 **)(param_1 + 0x100b7) = puVar2;
    }
    if (*(int *)(param_1 + 0x100b3) == 0) {
      iVar5 = FUN_00597850(0x7a);
      local_4 = 0;
      if (iVar5 == 0) {
        uVar3 = 0;
      }
      else {
        uVar3 = FUN_004cb4b0();
      }
      local_4 = 0xffffffff;
      *(undefined4 *)(param_1 + 0x100b3) = uVar3;
    }
    FUN_004ca9b0(&DAT_006a49e0);
    FUN_004cb810(param_1 + 0x100bb);
    *(undefined4 *)(*(int *)(param_1 + 0x100b3) + 0x35) = 0xfe;
    if (*(int *)(param_1 + 0x100b3) != 0) {
      iVar5 = -1;
      if (DAT_00689999 == 3) {
        iVar5 = 1;
      }
      iVar4 = DAT_00689995;
      if (DAT_00689999 < DAT_00689995) {
        iVar4 = DAT_00689999;
      }
      if (iVar4 < 1) {
        iVar4 = 1;
      }
      if (DAT_00689999 < iVar4) {
        iVar4 = DAT_00689999;
      }
      if (iVar4 < 1) {
        iVar4 = 1;
      }
      FUN_004cb7d0(iVar4 + iVar5,iVar4 + iVar5,0xffffffff);
    }
    FUN_004cb0b0(0,&local_1c,*(undefined4 *)(param_1 + 0x100b7),0,1);
    FUN_004d3b40();
  }
  ExceptionList = local_c;
  return;
}

