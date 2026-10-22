
void __fastcall FUN_004dcab0(int param_1)

{
  undefined4 uVar1;
  short sVar2;
  int iVar3;
  int *piVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  int local_34;
  int local_28;
  int local_20;
  undefined4 local_1c;
  int local_18;
  int local_14;
  int iStack_10;
  int iStack_c;
  
  if (*(int *)(param_1 + 0x6d) == 0) {
    FUN_0058bc10((-(uint)(DAT_006e1f88 != 0) & 0xffffd2a0) + 0x5ac0);
  }
  else {
    FUN_0054b2d0(param_1 + 0x81);
  }
  iVar3 = FUN_0058b660();
  if (iVar3 != 0) {
    local_20 = 0x10;
    local_1c = 0x10;
    local_18 = *(int *)(param_1 + 0x91) + -0x10;
    local_14 = *(int *)(param_1 + 0x95) + -0x10;
    FUN_0057dc20(*(int *)(param_1 + 0x95));
    FUN_0057dc60(*(undefined4 *)(param_1 + 0x91),*(undefined4 *)(param_1 + 0x95));
    local_34 = param_1 + 0xdf;
    piVar4 = (int *)(param_1 + 0x118);
    iVar6 = 0;
    do {
      uVar1 = local_1c;
      if (piVar4[-3] != -1) {
        uVar5 = piVar4[3] + *piVar4 * 2;
        if (*(uint *)(*(int *)(param_1 + 0xb3) + 0xc) < uVar5) {
          uVar5 = 0;
        }
        iVar7 = *(int *)(*(int *)(param_1 + 0xbb) + uVar5 * 4) + *(int *)(param_1 + 0xbf);
        local_28 = ((*(int *)(param_1 + 0x91) - 0x20U) / 3) * iVar6 + 0x10;
        local_20 = local_28;
        if (*(int *)(iVar7 + 0x1c) == -1) {
          (*DAT_006903dc)(iVar7,local_28,local_1c);
        }
        else {
          sVar2 = (*DAT_006c4e3c)(iVar7,local_28,local_1c,0,0);
          if (sVar2 == 1) {
            FUN_0057de00(iVar7,local_28,uVar1,0,0);
          }
        }
        local_20 = local_20 + *(int *)(iVar7 + 4);
        FUN_004a5ce0(local_34,&local_20,0,0,0,0x10,0);
      }
      iVar6 = iVar6 + 1;
      piVar4 = piVar4 + 1;
      local_34 = local_34 + 0xf;
    } while (iVar6 < 3);
    iVar6 = 0;
    local_20 = 0x14;
    local_1c = 0x30;
    if (*(char *)(param_1 + 0x29) != '\0') {
      iVar6 = FUN_004a5ce0(param_1 + 0x29,&local_20,0,0,0,0,0);
    }
    (**(code **)(**(int **)(param_1 + 0x85) + 0x80))(*(int **)(param_1 + 0x85),iVar3);
    iStack_c = iVar3 + 0x18;
    local_18 = iVar6 + 1 + local_28;
    iStack_10 = local_18 + 2;
    local_14 = iVar3;
    FUN_0058e2c0(&local_18);
  }
  return;
}

