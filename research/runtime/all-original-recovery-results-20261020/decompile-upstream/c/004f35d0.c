
void __thiscall
FUN_004f35d0(int param_1,int *param_2,int param_3,int param_4,undefined4 param_5,undefined4 *param_6
            )

{
  ushort *puVar1;
  int iVar2;
  ushort uVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 extraout_ECX_01;
  int iVar7;
  int *piVar8;
  int iVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  uint local_10;
  
  iVar4 = (int)param_2;
  iVar5 = (&DAT_006cb8c2)[param_4] + (&DAT_006cb942)[param_3] + (int)param_2;
  uVar3 = *(ushort *)(DAT_006c54dc + iVar5 * 0xc + 10);
  if (((uVar3 & 0x80) != 0) || ((uVar3 & 0x100) != 0)) {
    iVar5 = *(int *)(DAT_0065660c + (uint)*(ushort *)(DAT_006c54dc + iVar5 * 0xc) * 0x164 + 0xac);
    param_4 = param_4 + -1;
    if (param_4 != 0) {
      param_2 = (int *)(param_1 + 0x6432 + param_4 * 4);
      do {
        if (*(int *)(DAT_0065660c + 0xac +
                    (uint)*(ushort *)
                           (*(int *)(param_1 + 0x4c) +
                           (*param_2 + *(int *)(param_1 + 0x64b2 + param_3 * 4) + iVar4) * 0xc) *
                    0x164) != iVar5) {
          param_4 = param_4 + 1;
          goto LAB_004f36c0;
        }
        param_4 = param_4 + -1;
        param_2 = param_2 + -1;
      } while (param_4 != 0);
      param_4 = 0;
    }
LAB_004f36c0:
    param_2 = &DAT_006cb8c2 + param_4;
    uVar6 = (uint)*(ushort *)
                   (DAT_006c54dc +
                   ((&DAT_006cb942)[param_3] + (&DAT_006cb8c2)[param_4] + iVar4) * 0xc);
    iVar7 = DAT_0065660c + uVar6 * 0x164;
    iVar9 = *(int *)(DAT_0065660c + 0x84 + uVar6 * 0x164);
    if (iVar9 == *(int *)(iVar7 + 0x88)) {
      iVar7 = 1;
    }
    else {
      iVar7 = (-(uint)(iVar9 != *(int *)(iVar7 + 0x8c)) & 2) + 2;
    }
    iVar5 = iVar5 * 0x10;
    local_10 = 0;
    iVar9 = param_4;
    if (*(int *)(iVar5 + DAT_006e1f84) * iVar7 != 0) {
      do {
        iVar2 = DAT_006c54dc + ((&DAT_006cb942)[param_3] + *param_2 + iVar4) * 0xc;
        FUN_004eb710(0);
        puVar1 = (ushort *)(iVar2 + 10);
        *puVar1 = *puVar1 & 0xfe7f;
        uVar13 = 1;
        uVar12 = extraout_ECX;
        FUN_0040eeb0(iVar9);
        uVar11 = extraout_ECX_00;
        FUN_0040e8a0(param_3);
        uVar10 = extraout_ECX_01;
        FUN_0040e290(iVar4);
        FUN_00540140(uVar10,uVar11,uVar12,uVar13);
        iVar9 = iVar9 + 1;
        param_2 = param_2 + 1;
        local_10 = local_10 + iVar7;
      } while (local_10 < (uint)(*(int *)(iVar5 + DAT_006e1f84) * iVar7));
    }
    piVar8 = &DAT_005e453c;
    do {
      iVar5 = piVar8[-1] + iVar4;
      iVar7 = *piVar8 + param_3;
      if (iVar5 < 0) {
        iVar5 = iVar5 + *(int *)(param_1 + 4);
      }
      if (*(int *)(param_1 + 4) <= iVar5) {
        iVar5 = iVar5 - *(int *)(param_1 + 4);
      }
      if (iVar7 < 0) {
        iVar7 = iVar7 + *(int *)(param_1 + 8);
      }
      if (*(int *)(param_1 + 8) <= iVar7) {
        iVar7 = iVar7 - *(int *)(param_1 + 8);
      }
      FUN_004f35d0(iVar5,iVar7,param_4,param_5,0);
      piVar8 = piVar8 + 2;
    } while ((int)piVar8 < 0x5e455c);
    if (param_6 != (undefined4 *)0x0) {
      *param_6 = 0x32d;
    }
  }
  return;
}

