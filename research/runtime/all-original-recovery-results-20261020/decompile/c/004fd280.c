
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004fd280(int param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  undefined4 *puVar5;
  
  *(undefined4 *)(param_1 + 0x1e5) = 0;
  if (*(int *)(param_1 + 0x99) != 0) {
    *(int *)(param_1 + 0x99) = *(int *)(param_1 + 0x99) + -1;
  }
  if (DAT_006e1f0c == 0) {
    DAT_006def68 = FUN_00506220;
  }
  else {
    DAT_006def68 = FUN_00491270;
  }
  DAT_006def6c = 0;
  DAT_006def64 = DAT_006def58;
  DAT_006def60 = 0;
  iVar1 = FUN_0052a710();
  while (iVar1 != 0) {
    *(undefined4 *)(iVar1 + 0x74) = 0;
    FUN_0051e260();
    iVar1 = FUN_0052a710();
  }
  *(int *)(param_1 + 0x1e9) = *(int *)(*(int *)(param_1 + 0x25) + 4) * -0x40;
  iVar1 = *(int *)(*(int *)(param_1 + 0x25) + 4) * -0x40;
  *(int *)(param_1 + 0x1f1) = iVar1;
  *(int *)(param_1 + 0x1ed) = iVar1;
  _DAT_006a4fd8 = 0;
  _DAT_006c3750 = 0;
  DAT_00657d40 = 0;
  FUN_004fcf50(param_1 + 0x11);
  *(undefined4 *)(param_1 + 0x35) = 1;
  if ((DAT_006a5134 == 2) && (DAT_006903e8 != 3)) {
    iVar1 = *(int *)(param_1 + 0x229) + DAT_00657d40 * 0x24;
    iVar2 = *(short *)(iVar1 + 0x10) + 0x20;
    if (iVar2 < 0x7f) {
      if (iVar2 < -0x7e) {
        iVar2 = -0x7f;
      }
    }
    else {
      iVar2 = 0x7f;
    }
    *(short *)(iVar1 + 0x10) = (short)iVar2;
    if ((DAT_00656600 != 0) && (*(int *)(DAT_00656600 + 0x174) != DAT_00644520)) {
      *(undefined2 *)(iVar1 + 0x12) = 2;
    }
  }
  FUN_004fff60();
  uVar4 = DAT_006e8178;
  if ((DAT_005e140c != '\0') &&
     (*(int *)(*(int *)(param_1 + 0x21) + 0x66ce) == 0 ||
      *(int *)(*(int *)(param_1 + 0x21) + 0x66ca) != 0)) {
    if (*(undefined4 **)(param_1 + 0x239) != (undefined4 *)0x0) {
      puVar5 = *(undefined4 **)(param_1 + 0x239);
      for (uVar3 = DAT_006e8178 >> 2; uVar3 != 0; uVar3 = uVar3 - 1) {
        *puVar5 = 0;
        puVar5 = puVar5 + 1;
      }
      for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
        *(undefined1 *)puVar5 = 0;
        puVar5 = (undefined4 *)((int)puVar5 + 1);
      }
    }
    FUN_005015f0();
    *(uint *)(*(int *)(param_1 + 0x21) + 0x66ce) =
         (uint)(*(int *)(param_1 + 0x231) != *(int *)(param_1 + 0x235));
  }
  FUN_0058d660();
  if (DAT_006e1f68 == 0) {
    iVar1 = FUN_0058b660();
    if (iVar1 == 0) goto LAB_004fd5af;
    FUN_0057dc20(DAT_006a49f4);
    FUN_005002a0();
    FUN_0057dc60(*(undefined4 *)(DAT_006c482c + 8),*(undefined4 *)(DAT_006c482c + 0xc));
    uVar4 = 0;
    if (*(int *)(param_1 + 0x1e5) != 0) {
      do {
        FUN_0058f9c0(iVar1,&DAT_006a49e0);
        uVar4 = uVar4 + 1;
      } while (uVar4 < *(uint *)(param_1 + 0x1e5));
    }
    iVar2 = *DAT_006a49e4;
  }
  else {
    FUN_0057dc20((-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0 + DAT_006e1f74 * 2);
    FUN_0057dc60(*(int *)(DAT_006c482c + 8) + DAT_006e1f70 * 2,
                 *(int *)(DAT_006c482c + 0xc) + DAT_006e1f74 * 2);
    FUN_005002a0();
    iVar1 = FUN_0058b660();
    if (iVar1 == 0) goto LAB_004fd5af;
    FUN_0057dc20(DAT_006a49f4);
    FUN_0057dc60(*(undefined4 *)(DAT_006c482c + 8),*(undefined4 *)(DAT_006c482c + 0xc));
    FUN_00594508();
    uVar4 = 0;
    if (*(int *)(param_1 + 0x1e5) != 0) {
      do {
        FUN_0058f9c0(iVar1,&DAT_006a49e0);
        uVar4 = uVar4 + 1;
      } while (uVar4 < *(uint *)(param_1 + 0x1e5));
    }
    iVar2 = *DAT_006a49e4;
  }
  (**(code **)(iVar2 + 0x80))(DAT_006a49e4,iVar1);
LAB_004fd5af:
  *(undefined4 *)(param_1 + 0x231) = 0;
  *(undefined4 *)(param_1 + 0x22d) = *(undefined4 *)(param_1 + 0x229);
  if (DAT_005e140c != '\0') {
    DAT_005e4558 = DAT_00689969;
    DAT_005e455c = DAT_0068996d;
    DAT_005e4560 = *(undefined4 *)(param_1 + 0x61);
    DAT_005e4564 = *(undefined4 *)(param_1 + 0x31);
    DAT_005e4568 = *(undefined4 *)(param_1 + 0x65);
  }
  FUN_0052ab80();
  if (*(int *)(param_1 + 0x221) != 0) {
    uVar4 = *(int *)(param_1 + 0x31) + 1U & 0x80000003;
    if ((int)uVar4 < 0) {
      uVar4 = (uVar4 - 1 | 0xfffffffc) + 1;
    }
    *(uint *)(param_1 + 0x31) = uVar4;
    *(uint *)(param_1 + 0x23d) = uVar4;
    *(undefined4 *)(param_1 + 0x35) = 0;
    *(undefined4 *)(param_1 + 0x99) = 3;
    uVar4 = 0;
    DAT_006e9850 = DAT_00689961;
    iVar1 = DAT_006def58;
    if (DAT_006df180 != 0) {
      do {
        if (*(int *)(iVar1 + 4) != 0) {
          FUN_00508a10();
        }
        uVar4 = uVar4 + 1;
        iVar1 = iVar1 + 0xe4b;
      } while (uVar4 < DAT_006df180);
    }
    DAT_005fed78 = DAT_00689961;
    uVar4 = 0;
    iVar1 = DAT_00689498;
    if (DAT_006898dc != 0) {
      do {
        if (*(int *)(iVar1 + 4) != 0) {
          FUN_004945f0();
        }
        uVar4 = uVar4 + 1;
        iVar1 = iVar1 + 0x22e;
      } while (uVar4 < DAT_006898dc);
    }
    FUN_00556330(DAT_00689961);
    *(undefined4 *)(param_1 + 0x221) = 0;
    return;
  }
  if (*(int *)(param_1 + 0x225) != 0) {
    iVar1 = *(int *)(param_1 + 0x31) + -1;
    if (iVar1 < 0) {
      iVar1 = *(int *)(param_1 + 0x31) + 3;
    }
    *(int *)(param_1 + 0x31) = iVar1;
    *(int *)(param_1 + 0x23d) = iVar1;
    *(undefined4 *)(param_1 + 0x35) = 0;
    *(undefined4 *)(param_1 + 0x99) = 3;
    uVar4 = 0;
    DAT_006e9850 = DAT_00689961;
    iVar1 = DAT_006def58;
    if (DAT_006df180 != 0) {
      do {
        if (*(int *)(iVar1 + 4) != 0) {
          FUN_00508a10();
        }
        uVar4 = uVar4 + 1;
        iVar1 = iVar1 + 0xe4b;
      } while (uVar4 < DAT_006df180);
    }
    DAT_005fed78 = DAT_00689961;
    uVar4 = 0;
    iVar1 = DAT_00689498;
    if (DAT_006898dc != 0) {
      do {
        if (*(int *)(iVar1 + 4) != 0) {
          FUN_004945f0();
        }
        uVar4 = uVar4 + 1;
        iVar1 = iVar1 + 0x22e;
      } while (uVar4 < DAT_006898dc);
    }
    FUN_00556330(DAT_00689961);
    *(undefined4 *)(param_1 + 0x225) = 0;
  }
  return;
}

