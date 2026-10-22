
/* WARNING: Removing unreachable block (ram,0x0054a023) */
/* WARNING: Removing unreachable block (ram,0x00549fc6) */
/* WARNING: Removing unreachable block (ram,0x0054a001) */
/* WARNING: Removing unreachable block (ram,0x0054a0b8) */

void __fastcall
FUN_00549e80(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5
            ,undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9,
            int param_10)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 *puVar4;
  uint uVar5;
  int local_28;
  int local_24;
  int local_20;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c29a3;
  local_c = ExceptionList;
  local_28 = 0;
  if ((((param_1 != 0) && (param_1 != 0x18)) && (param_1 != 0x19)) && (param_1 != 0x1a)) {
    ExceptionList = &local_c;
    FUN_0054a680();
    FUN_0054a5f0();
    local_20 = *(int *)(&DAT_006f2174 + param_1 * 4);
    if (param_10 != 0) {
      if (*(int *)(param_10 + 4) == 0) {
        local_28 = 0;
      }
      else {
        local_28 = (*(int *)(param_10 + 8) - *(int *)(param_10 + 4)) / 0x1c;
      }
      if (local_20 <= local_28) {
        local_20 = local_28;
      }
    }
    local_24 = 0;
    if (0 < local_20) {
      do {
        if ((param_10 == 0) || (local_28 < 1)) {
          uVar5 = FUN_0054e200();
          iVar1 = (uVar5 & 0x7fffffff) % 6 + 2;
          uVar5 = FUN_0054e200();
          if ((uVar5 & 1) != 0) {
            iVar1 = -iVar1;
          }
          uVar5 = FUN_0054e200();
          iVar2 = (uVar5 & 0x7fffffff) % 6 + 2;
          uVar5 = FUN_0054e200();
          if ((uVar5 & 1) != 0) {
            iVar2 = -iVar2;
          }
          uVar5 = FUN_0054e200();
          iVar3 = (uVar5 & 7) + 5;
        }
        else {
          local_28 = local_28 + -1;
          iVar1 = __ftol();
          iVar2 = __ftol();
          iVar3 = __ftol();
        }
        puVar4 = (undefined4 *)FUN_00597850(0x82);
        local_4 = 0;
        if (puVar4 == (undefined4 *)0x0) {
          puVar4 = (undefined4 *)0x0;
        }
        else {
          FUN_00548650(param_2,param_3,param_4,iVar1,iVar2,iVar3);
          *(undefined4 *)((int)puVar4 + 0x5a) = 0;
          local_4 = CONCAT31(local_4._1_3_,1);
          iVar1 = DAT_005e1424 / 10;
          *(undefined4 *)((int)puVar4 + 0x72) = param_9;
          *(int *)((int)puVar4 + 0x5e) = iVar1;
          *(int *)((int)puVar4 + 0x76) = param_1;
          *(int *)((int)puVar4 + 0x7a) = local_24;
          uVar5 = FUN_0054e200();
          *(uint *)((int)puVar4 + 0x7e) = uVar5 & 7;
          *puVar4 = &PTR_FUN_005c752c;
          FUN_0054a5f0();
        }
        local_4 = 0xffffffff;
        if (puVar4 != (undefined4 *)0x0) {
          *(undefined4 *)((int)puVar4 + 0x62) = param_5;
          *(undefined4 *)((int)puVar4 + 0x6e) = param_8;
          *(undefined4 *)((int)puVar4 + 0x66) = param_6;
          *(undefined4 *)((int)puVar4 + 0x6a) = param_7;
          iVar1 = FUN_005457e0(puVar4);
          if (iVar1 == 0) {
            (**(code **)*puVar4)(1);
          }
        }
        local_24 = local_24 + 1;
      } while (local_24 < local_20);
    }
  }
  ExceptionList = local_c;
  return;
}

