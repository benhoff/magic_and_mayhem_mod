
void __fastcall FUN_004a2d70(int param_1)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  uint uVar4;
  int iVar5;
  int local_1c;
  int local_10;
  int local_c;
  int local_8;
  int local_4;
  
  if (*(int *)(param_1 + 0x84) != 0) {
    local_1c = 0;
    uVar4 = *(int *)(param_1 + 0x88) + 1;
    *(uint *)(param_1 + 0x88) = uVar4;
    if (0 < *(int *)(param_1 + 0x10)) {
      piVar3 = (int *)(param_1 + 0x14);
      do {
        iVar1 = piVar3[8];
        iVar2 = *piVar3;
        for (iVar5 = iVar1 + (uVar4 & 7); iVar5 < iVar2 + iVar1; iVar5 = iVar5 + 8) {
          local_8 = *(int *)(*(int *)(param_1 + 0x84) + iVar5 * 8);
          local_4 = *(int *)(*(int *)(param_1 + 0x84) + 4 + iVar5 * 8);
          if (DAT_006de6d5 == 0) {
            local_8 = (local_8 * 0x280) / 800;
            local_4 = (local_4 * 0x1e0) / 600;
          }
          local_c = local_4 + -1;
          local_10 = local_8 + -1;
          local_8 = local_8 + 1;
          local_4 = local_4 + 1;
          FUN_0058bac0(&local_10,(-(uint)(DAT_006e1f88 != 0) & 0xffff8000) + 0xffe0);
        }
        local_1c = local_1c + 1;
        piVar3 = piVar3 + 1;
      } while (local_1c < *(int *)(param_1 + 0x10));
    }
  }
  return;
}

