
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004fcf50(int param_1,int *param_2)

{
  int iVar1;
  char cVar2;
  undefined4 uVar3;
  int *piVar4;
  int *piVar5;
  int iVar6;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  int local_4;
  
  if (DAT_006de4d8 != 0) {
    FUN_0058bac0(param_2,(-(uint)(DAT_006e1f88 != 0) & 0xffff8000) + 0xffe0);
  }
  DAT_00656600 = 0;
  DAT_00657d40 = 0;
  DAT_006dbea4 = 0;
  _DAT_006a4fd8 = 0;
  DAT_006c5488 = -1;
  DAT_006c5484 = 0xffffffff;
  DAT_006cbb64 = 0xffffffff;
  _DAT_006c3750 = 0;
  FUN_0057dc60(param_2[2],param_2[3]);
  iVar6 = DAT_00641fbc;
  local_c = param_2[1] + 0x10;
  local_10 = *param_2 + 0x20;
  local_8 = param_2[2] + 0x20;
  local_4 = param_2[3] + 0x50;
  *(undefined4 *)(param_1 + 0x91) = (&DAT_00641ab8)[DAT_00641fbc * 10];
  *(undefined4 *)(param_1 + 0x95) = *(undefined4 *)(&DAT_00641abc + iVar6 * 0x28);
  *(undefined4 *)(param_1 + 0xad) = *(undefined4 *)(*(int *)(param_1 + 0x21) + 0x7f0);
  *(undefined4 *)(param_1 + 0x1f5) = 0;
  *(undefined4 *)(param_1 + 0x1fd) = 0;
  *(undefined4 *)(param_1 + 0x219) = 0;
  *(undefined4 *)(param_1 + 0x205) = 0;
  *(int *)(param_1 + 0xa1) = *(int *)(*(int *)(param_1 + 0x25) + 4) << 5;
  *(int *)(param_1 + 0xa5) = *(int *)(*(int *)(param_1 + 0x25) + 8) << 5;
  DAT_005e41ac = (undefined4)(0x100 / (longlong)DAT_006de6e5);
  iVar6 = 0;
  piVar4 = &DAT_006e818c;
  do {
    piVar5 = piVar4 + 1;
    iVar1 = iVar6 / 100;
    iVar6 = iVar6 + 0x52;
    *piVar4 = iVar1;
    piVar4 = piVar5;
  } while ((int)piVar5 < 0x6e895c);
  DAT_005e41b0 = 1;
  if (((((*(int *)(*(int *)(param_1 + 0x21) + 0x66ca) == 0) && (DAT_005e4558 == DAT_00689969)) &&
       (DAT_005e455c == DAT_0068996d)) &&
      ((DAT_005e4560 == *(int *)(param_1 + 0x61) && (DAT_005e4564 == *(int *)(param_1 + 0x31))))) &&
     (*(int *)(param_1 + 0x65) == DAT_005e4568)) {
    DAT_005e41b0 = 0;
  }
  else {
    *(undefined4 *)(*(int *)(param_1 + 0x21) + 0x66ce) = 0;
  }
  switch(*(undefined4 *)(param_1 + 0x31)) {
  case 0:
    FUN_004f83f0(&local_10);
    break;
  case 1:
    FUN_004fbe00(&local_10);
    break;
  case 2:
    FUN_004fc3f0(&local_10);
    break;
  case 3:
    FUN_004fc930(&local_10);
  }
  FUN_00545990();
  _DAT_006db796 = 0;
  FUN_004e0f50(&DAT_006d7796);
  DAT_006a5134 = 0;
  iVar6 = *(int *)(param_1 + 0x1e9);
  iVar1 = *(int *)(param_1 + 0x1f1);
  if (((iVar1 < iVar6) && (*(int *)(param_1 + 0x1ed) < iVar6)) && (DAT_00656600 != 0)) {
    DAT_006a5134 = 2;
  }
  else if (((iVar6 < iVar1) && (*(int *)(param_1 + 0x1ed) < iVar1)) && (DAT_006dbea4 != 0)) {
    DAT_006a5134 = 3;
  }
  else if (DAT_006c5488 != -1) {
    DAT_006a5134 = 1;
  }
  local_18 = (&DAT_00641ab8)[DAT_00641fbc * 10];
  local_14 = *(int *)(&DAT_00641abc + DAT_00641fbc * 0x28);
  if (((DAT_006c4e65 == 0) || (local_18 < DAT_006c4e51)) ||
     ((DAT_006c4e59 <= local_18 || ((local_14 < DAT_006c4e55 || (DAT_006c4e5d <= local_14)))))) {
    piVar4 = &local_18;
    FUN_00463cb0(piVar4);
    cVar2 = FUN_00464290(piVar4);
    if (cVar2 != '\0') goto LAB_004fd24a;
    uVar3 = FUN_004a3c70();
  }
  else {
    FUN_00466870(DAT_00644520);
    uVar3 = FUN_0057cf40();
  }
  FUN_004a5850(uVar3);
LAB_004fd24a:
  *(undefined4 *)(*(int *)(param_1 + 0x21) + 0x66ca) = 0;
  FUN_004ff1d0();
  return;
}

