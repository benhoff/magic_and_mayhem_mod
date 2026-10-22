
void __fastcall FUN_00545990(int param_1)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  uint uVar5;
  int *piVar6;
  uint uVar7;
  uint uVar8;
  int local_18;
  int local_8;
  int iStack_4;
  
  local_18 = *(int *)(param_1 + 0x1c00);
  uVar1 = DAT_006c549c - 1;
  if (-1 < local_18 + -1) {
    piVar6 = (int *)(param_1 + (local_18 + -1) * 4);
    do {
      iVar3 = *piVar6;
      uVar5 = *(uint *)(iVar3 + 0xc) >> 4;
      uVar7 = *(uint *)(iVar3 + 4) >> 5;
      uVar8 = *(uint *)(iVar3 + 8) >> 5;
      uVar2 = uVar5;
      if ((int)uVar1 <= (int)uVar5) {
        uVar2 = uVar1;
      }
      if ((*(byte *)(DAT_006c54dc + 8 +
                    ((&DAT_006cb8c2)[uVar2] + (&DAT_006cb942)[uVar8] + uVar7) * 0xc) & 3) <=
          DAT_00689995) {
        FUN_004f80f0(*(uint *)(iVar3 + 4),*(uint *)(iVar3 + 8),*(uint *)(iVar3 + 0xc),&local_8);
        iVar3 = (**(code **)(*(int *)*piVar6 + 4))();
        if ((((DAT_006e0008 <= iVar3 + local_8) && (local_8 < DAT_006a49b8)) &&
            (iVar3 = (**(code **)(*(int *)*piVar6 + 8))(), DAT_006cbb6c <= iVar3 + iStack_4)) &&
           (iStack_4 < DAT_00656618)) {
          uVar4 = FUN_004f7ff0(uVar7,uVar8);
          FUN_004ffd30(*piVar6,uVar7 * 0x20 + 0x10,uVar8 * 0x20 + 0x10,uVar5 * 0x10 + 1,local_8,
                       iStack_4,uVar4,0,0,8,0xffff8ad0,0,0xffff8ad0,0);
        }
      }
      piVar6 = piVar6 + -1;
      local_18 = local_18 + -1;
    } while (local_18 != 0);
  }
  return;
}

