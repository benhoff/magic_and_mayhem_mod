
void __fastcall FUN_004ddd60(int param_1)

{
  int iVar1;
  short sVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  
  iVar4 = *(int *)(param_1 + 0x3a);
  if (iVar4 != 0) {
    uVar3 = ((iVar4 - *(int *)(param_1 + 0x3e)) * 0x5c) / iVar4;
    if (0x5a < (int)uVar3) {
      uVar3 = 0x5b;
    }
    if (uVar3 != *(uint *)(param_1 + 0x42)) {
      iVar4 = FUN_0058b660();
      if (iVar4 != 0) {
        FUN_0057dc20(*(undefined4 *)(param_1 + 0x1c));
        FUN_0057dc60(0x28,0x28);
        uVar5 = uVar3;
        if (*(uint *)(DAT_006e2090 + 0xc) < uVar3) {
          uVar5 = 0;
        }
        iVar1 = *(int *)(DAT_006e2098 + uVar5 * 4);
        iVar6 = iVar1 + DAT_006e209c;
        if (*(int *)(iVar1 + 0x1c + DAT_006e209c) == -1) {
          (*DAT_006903dc)(iVar6);
        }
        else {
          sVar2 = (*DAT_006c4e3c)(iVar6,0,0,0,0);
          if (sVar2 == 1) {
            FUN_0057de00(iVar6,0,0,0,0);
          }
        }
        (**(code **)(**(int **)(param_1 + 0xc) + 0x80))(*(int **)(param_1 + 0xc),iVar4);
        *(uint *)(param_1 + 0x42) = uVar3;
      }
    }
  }
  return;
}

