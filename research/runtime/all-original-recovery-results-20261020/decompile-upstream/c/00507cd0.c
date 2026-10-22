
void __fastcall FUN_00507cd0(int param_1)

{
  ushort uVar1;
  uint uVar2;
  int *piVar3;
  int iVar4;
  int iVar5;
  uint local_c;
  uint local_4;
  
  if (*(int *)(param_1 + 0xe4) != 0) {
    uVar2 = FUN_0054e200();
    if (DAT_006e1f30 != 0) {
      FUN_00485b10(s_c__Projects_Chaos_Mapelem_cpp_005e9c10,0xa93);
    }
    if ((uint)(DAT_006bc985 * 100) /
        (uint)(*(int *)(*(int *)(param_1 + 0xac) + 0x2c) + DAT_006bc985) <
        (uVar2 & 0x7fffffff) % 100) {
      if (*(int *)(param_1 + 0x114) != 0) {
        *(undefined4 *)(param_1 + 0x114) = 0;
      }
      if (*(int *)(param_1 + 0x120) != 0) {
        FUN_004de090(0,1);
        *(undefined4 *)(param_1 + 0x120) = 0;
      }
    }
    if (*(int *)(param_1 + 0xf8) == 0) {
      FUN_005076f0(-DAT_006bcad6,*(undefined4 *)(param_1 + 0x11c),0,1);
    }
  }
  if (*(int *)(*(int *)(param_1 + 0xac) + 8) == 2) {
    local_c = 0xc;
    piVar3 = &DAT_005e4eb0;
  }
  else {
    local_c = 8;
    piVar3 = &DAT_005e4e70;
  }
  local_4 = 0;
  if (local_c != 0) {
    do {
      iVar5 = 0;
      iVar4 = FUN_0040e650(*piVar3 + *(int *)(param_1 + 8));
      uVar2 = piVar3[1] + *(int *)(param_1 + 0xc);
      if ((int)uVar2 < 0) {
        do {
          uVar2 = uVar2 + DAT_006c5498;
        } while ((int)uVar2 < 0);
      }
      else {
        for (; DAT_006c5498 <= uVar2; uVar2 = uVar2 - DAT_006c5498) {
        }
      }
      uVar1 = *(ushort *)
               (DAT_006c54dc +
                ((&DAT_006cb8c2)[*(int *)(param_1 + 0x10)] + (&DAT_006cb942)[uVar2] + iVar4) * 0xc +
               4);
      if (uVar1 != 0xffff) {
        if (uVar1 < DAT_006def5c) {
          iVar5 = (uint)uVar1 * 0xe4b + DAT_006def58;
        }
        uVar2 = FUN_0054e200();
        if (DAT_006e1f30 != 0) {
          FUN_00485b10(s_c__Projects_Chaos_Mapelem_cpp_005e9c10,0xaae);
        }
        if ((*(int *)(iVar5 + 0x114) == 0) &&
           ((uVar2 & 0x7fffffff) % 100 <
            (uint)(DAT_006bc985 * 100) /
            (uint)(*(int *)(*(int *)(param_1 + 0xac) + 0x14) + DAT_006bc985))) {
          FUN_00515f70(*(undefined4 *)(param_1 + 0x11c));
          *(int *)(param_1 + 0x118) = DAT_006c4830 + 0x32;
          return;
        }
      }
      local_4 = local_4 + 1;
      piVar3 = piVar3 + 2;
    } while (local_4 < local_c);
  }
  *(int *)(param_1 + 0x118) = DAT_006c4830 + 0x32;
  return;
}

