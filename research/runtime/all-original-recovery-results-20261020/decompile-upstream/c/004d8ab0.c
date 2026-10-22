
void __thiscall FUN_004d8ab0(int param_1,undefined4 param_2)

{
  char cVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  char *pcVar6;
  undefined4 *puVar7;
  char *pcVar8;
  char *pcVar9;
  undefined4 uVar10;
  undefined4 local_214;
  int local_210;
  undefined4 local_20c;
  int local_208;
  int local_204;
  char local_200 [4];
  char local_1fc [4];
  char local_1f8 [4];
  char local_1f4 [4];
  char local_1f0 [2];
  undefined1 local_1ee;
  undefined4 local_1ed [59];
  char local_100 [256];
  
  if (*(int *)(param_1 + 0x15e) != 0) {
    local_200 = (char  [4])s_Sprites_printed__005e2fe4._0_4_;
    local_1fc = (char  [4])s_Sprites_printed__005e2fe4._4_4_;
    local_1f8 = (char  [4])s_Sprites_printed__005e2fe4._8_4_;
    local_1f4 = (char  [4])s_Sprites_printed__005e2fe4._12_4_;
    local_1f0 = (char  [2])s_Sprites_printed__005e2fe4._16_2_;
    iVar4 = *(int *)(param_1 + 0x15a);
    local_1ee = DAT_005e2ff6;
    puVar7 = local_1ed;
    for (iVar2 = 0x3b; iVar2 != 0; iVar2 = iVar2 + -1) {
      *puVar7 = 0;
      puVar7 = puVar7 + 1;
    }
    *(undefined1 *)puVar7 = 0;
    iVar2 = iVar4 + -2;
    if (*(int *)(param_1 + 4) != 0) {
      local_208 = (iVar4 + -1) * *(int *)(param_1 + 0x156);
      local_210 = *(int *)(param_1 + 0x156) * iVar2;
      local_214 = 0;
      local_20c = 0x194;
      if (DAT_006de6dd == 0) {
        uVar10 = 0;
      }
      else {
        uVar10 = 0x1f;
      }
      FUN_0058bac0(&local_214,uVar10);
      local_204 = FUN_0058b660();
      if (local_204 != 0) {
        FUN_0057dc20(*(undefined4 *)(param_1 + 0x138));
        FUN_0057dc60(local_20c,local_208);
        FUN_0059de2f(param_2,local_100,10);
        uVar3 = 0xffffffff;
        pcVar6 = local_100;
        do {
          pcVar9 = pcVar6;
          if (uVar3 == 0) break;
          uVar3 = uVar3 - 1;
          pcVar9 = pcVar6 + 1;
          cVar1 = *pcVar6;
          pcVar6 = pcVar9;
        } while (cVar1 != '\0');
        uVar3 = ~uVar3;
        iVar4 = -1;
        pcVar6 = local_200;
        do {
          pcVar8 = pcVar6;
          if (iVar4 == 0) break;
          iVar4 = iVar4 + -1;
          pcVar8 = pcVar6 + 1;
          cVar1 = *pcVar6;
          pcVar6 = pcVar8;
        } while (cVar1 != '\0');
        pcVar6 = pcVar9 + -uVar3;
        pcVar9 = pcVar8 + -1;
        for (uVar5 = uVar3 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
          *(undefined4 *)pcVar9 = *(undefined4 *)pcVar6;
          pcVar6 = pcVar6 + 4;
          pcVar9 = pcVar9 + 4;
        }
        for (uVar3 = uVar3 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
          *pcVar9 = *pcVar6;
          pcVar6 = pcVar6 + 1;
          pcVar9 = pcVar9 + 1;
        }
        uVar3 = FUN_004a5ce0(local_200,&local_214,0,0,0,0,0);
        *(undefined4 *)(*(int *)(param_1 + 0xc) + iVar2 * 4) = 100;
        (**(code **)(**(int **)(param_1 + 0x128) + 0x80))(*(int **)(param_1 + 0x128),local_204);
        iVar4 = iVar2 * 0x10;
        *(undefined4 *)(iVar4 + *(int *)(param_1 + 0x10)) = 0;
        *(int *)(iVar4 + 4 + *(int *)(param_1 + 0x10)) = iVar2 * *(int *)(param_1 + 0x156);
        *(uint *)(iVar4 + 8 + *(int *)(param_1 + 0x10)) = uVar3 + 3 & 0xfffffffc;
        iVar4 = iVar4 + *(int *)(param_1 + 0x10);
        *(int *)(iVar4 + 0xc) = *(int *)(iVar4 + 4) + *(int *)(param_1 + 0x156);
        *(uint *)(*(int *)(param_1 + 0x14) + iVar2 * 4) =
             (((-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280) / 2 - (uVar3 >> 1)) + 3 & 0xfffffffc;
      }
    }
  }
  return;
}

