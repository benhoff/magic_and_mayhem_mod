
/* WARNING: Removing unreachable block (ram,0x0054b46a) */
/* WARNING: Removing unreachable block (ram,0x0054b45e) */
/* WARNING: Removing unreachable block (ram,0x0054b476) */
/* WARNING: Removing unreachable block (ram,0x0054b3a4) */
/* WARNING: Removing unreachable block (ram,0x0054b3b0) */
/* WARNING: Removing unreachable block (ram,0x0054b3bc) */

void __thiscall FUN_0054b2d0(int *param_1,int param_2)

{
  byte bVar1;
  byte bVar2;
  ushort uVar3;
  int iVar4;
  byte *pbVar5;
  uint uVar6;
  uint uVar7;
  byte *pbVar8;
  ushort *puVar9;
  ushort *puVar10;
  int local_14;
  int local_10;
  
  if ((((*param_1 != 0) && (param_1[3] <= *(int *)(param_2 + 0x10))) &&
      (param_1[4] <= *(int *)(param_2 + 0x14))) && (iVar4 = FUN_0058b660(), iVar4 != 0)) {
    local_10 = 0;
    pbVar8 = (byte *)(*param_1 + 0x80);
    if (0 < param_1[4]) {
      do {
        local_14 = 0;
        pbVar5 = pbVar8;
        puVar9 = (ushort *)(local_10 * *(int *)(param_2 + 0x18) + iVar4);
        do {
          bVar1 = *pbVar5;
          pbVar8 = pbVar5 + 1;
          if ((bVar1 & 0xc0) == 0xc0) {
            bVar2 = *pbVar8;
            pbVar8 = pbVar5 + 2;
            local_14 = local_14 + (bVar1 & 0x3f);
            pbVar5 = (byte *)((uint)bVar2 * 3 + param_1[2]);
            if (DAT_006e1f88 == 0) {
              uVar3 = (ushort)(((*pbVar5 & 0xfff8) << 5 | pbVar5[1] & 0xfc) << 3);
            }
            else {
              uVar3 = (ushort)(((*pbVar5 & 0xf8) << 5 | pbVar5[1] & 0xf8) << 2);
            }
            uVar3 = uVar3 | pbVar5[2] >> 3;
            puVar10 = puVar9;
            if ((bVar1 & 0x3f) != 0) {
              uVar7 = (byte)((bVar1 & 0x3f) - 1) + 1;
              puVar10 = puVar9 + uVar7;
              for (uVar6 = uVar7 >> 1; uVar6 != 0; uVar6 = uVar6 - 1) {
                *(uint *)puVar9 = CONCAT22(uVar3,uVar3);
                puVar9 = puVar9 + 2;
              }
              for (uVar6 = (uint)((uVar7 & 1) != 0); uVar6 != 0; uVar6 = uVar6 - 1) {
                *puVar9 = uVar3;
                puVar9 = puVar9 + 1;
              }
            }
          }
          else {
            pbVar5 = (byte *)((uint)bVar1 * 3 + param_1[2]);
            if (DAT_006e1f88 == 0) {
              uVar3 = (ushort)(((*pbVar5 & 0xfff8) << 5 | pbVar5[1] & 0xfc) << 3);
            }
            else {
              uVar3 = (ushort)(((*pbVar5 & 0xf8) << 5 | pbVar5[1] & 0xf8) << 2);
            }
            puVar10 = puVar9 + 1;
            *puVar9 = uVar3 | pbVar5[2] >> 3;
            local_14 = local_14 + 1;
          }
          pbVar5 = pbVar8;
          puVar9 = puVar10;
        } while (local_14 < param_1[3]);
        local_10 = local_10 + 1;
      } while (local_10 < param_1[4]);
    }
    (**(code **)(**(int **)(param_2 + 4) + 0x80))(*(int **)(param_2 + 4),iVar4);
  }
  return;
}

