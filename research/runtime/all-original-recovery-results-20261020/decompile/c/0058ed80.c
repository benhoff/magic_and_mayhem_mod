
void __fastcall FUN_0058ed80(int param_1)

{
  uint *puVar1;
  uint *puVar2;
  uint *puVar3;
  int iVar4;
  int iVar5;
  bool bVar6;
  
  bVar6 = DAT_006e1f88 != 0;
  puVar1 = (uint *)FUN_0058b660();
  iVar5 = 0;
  puVar3 = puVar1;
  if (0 < *(int *)(param_1 + 0x14)) {
    do {
      iVar4 = 0;
      puVar2 = puVar3;
      if (0 < *(int *)(param_1 + 0x10) / 2) {
        do {
          puVar3 = puVar2 + 1;
          iVar4 = iVar4 + 1;
          *puVar2 = *puVar2 >> 1 & (-(uint)bVar6 & 0x2000200) + 0xfbef7bef;
          puVar2 = puVar3;
        } while (iVar4 < *(int *)(param_1 + 0x10) / 2);
      }
      iVar5 = iVar5 + 1;
    } while (iVar5 < *(int *)(param_1 + 0x14));
  }
  (**(code **)(**(int **)(param_1 + 4) + 0x80))(*(int **)(param_1 + 4),puVar1);
  return;
}

