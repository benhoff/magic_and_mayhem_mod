
undefined4 __thiscall
FUN_004ed8f0(undefined4 *param_1,uint param_2,uint param_3,int param_4,undefined4 param_5,
            undefined4 param_6)

{
  int iVar1;
  undefined2 *puVar2;
  int *piVar3;
  uint uVar4;
  int iVar5;
  
  if (((0x2a < param_2) && (0x2a < param_3)) && (param_4 != 0)) {
    FUN_004ecbe0();
    param_1[1] = param_2;
    param_1[4] = param_2 * param_3;
    iVar5 = param_2 * param_3 * param_4;
    param_1[2] = param_3;
    param_1[0x11] = 0xffffffff;
    param_1[0x12] = 0xffffffff;
    *param_1 = 6;
    param_1[3] = param_4;
    param_1[0x10] = 0x200;
    param_1[5] = iVar5;
    param_1[6] = param_5;
    param_1[7] = param_6;
    param_1[0xf] = 0;
    param_1[0xe] = 0;
    param_1[0xd] = 0;
    param_1[0xc] = 0;
    param_1[0xb] = 0;
    param_1[10] = 0;
    param_1[9] = 0;
    param_1[8] = 0;
    iVar1 = FUN_00597850(iVar5 * 0xc);
    if (iVar1 == 0) {
      iVar1 = 0;
    }
    else if (-1 < iVar5 + -1) {
      puVar2 = (undefined2 *)(iVar1 + 4);
      do {
        puVar2[-2] = 0xffff;
        puVar2[-1] = 0xffff;
        *puVar2 = 0xffff;
        puVar2[1] = 0xffff;
        puVar2[2] = 0;
        puVar2[3] = 0;
        puVar2 = puVar2 + 6;
        iVar5 = iVar5 + -1;
      } while (iVar5 != 0);
    }
    param_1[0x13] = iVar1;
    iVar1 = 0;
    if (0 < (int)param_1[3]) {
      piVar3 = (int *)((int)param_1 + 0x6432);
      do {
        *piVar3 = param_1[4] * iVar1;
        iVar1 = iVar1 + 1;
        piVar3 = piVar3 + 1;
      } while (iVar1 < (int)param_1[3]);
    }
    iVar1 = 0;
    if (0 < (int)param_1[2]) {
      piVar3 = (int *)((int)param_1 + 0x64b2);
      do {
        *piVar3 = iVar1 * param_1[1];
        iVar1 = iVar1 + 1;
        piVar3 = piVar3 + 1;
      } while (iVar1 < (int)param_1[2]);
    }
    puVar2 = (undefined2 *)param_1[0x13];
    uVar4 = 0;
    if (param_1[5] != 0) {
      do {
        *puVar2 = 0;
        puVar2[5] = 0;
        puVar2[2] = 0xffff;
        puVar2[3] = 0xffff;
        puVar2[1] = 0xffff;
        puVar2 = puVar2 + 6;
        uVar4 = uVar4 + 1;
      } while (uVar4 < (uint)param_1[5]);
    }
    FUN_004ecdd0();
    FUN_00545570(DAT_006c549c << 4);
    return 1;
  }
  MessageBoxA((HWND)0x0,s_Tried_to_create_a_map_below_the_m_005e456c,s_MAP_SIZE_ERROR_005e459c,
              0x42030);
  return 0;
}

