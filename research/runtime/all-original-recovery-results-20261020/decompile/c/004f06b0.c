
void __fastcall FUN_004f06b0(int param_1)

{
  ushort uVar1;
  uint uVar2;
  int iVar3;
  int *piVar4;
  int iVar5;
  int iVar6;
  int *piVar7;
  undefined4 *puVar8;
  undefined4 *puVar9;
  int local_33c;
  int local_334;
  int local_330;
  int local_32c;
  int local_328;
  int *local_324;
  int local_320 [200];
  
  iVar6 = 0;
  local_32c = 0;
  local_328 = 0;
  local_334 = 0;
  local_33c = 0;
  if ((DAT_006e1ee8 != 0) &&
     (local_32c = FUN_0059cae2(s_region_plc_005e4b14,&DAT_005dc824), local_32c != 0)) {
    FUN_0059d50b(local_32c,s_Region_of_power_Placement_log_005e4af4);
    FUN_0059d50b(local_32c,s____________________005e47c0);
  }
  if (0xe < *(int *)(param_1 + 0x7f4)) {
    *(undefined4 *)(param_1 + 0x7f4) = 0xf;
  }
  puVar9 = (undefined4 *)(param_1 + 0x84);
  for (iVar3 = 0x4b; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar9 = 0;
    puVar9 = puVar9 + 1;
  }
  piVar7 = *(int **)(param_1 + 100);
  local_324 = local_320 + 1;
  piVar4 = (int *)(param_1 + 0x88);
  for (local_330 = 0; local_330 < *(int *)(param_1 + 0x68); local_330 = local_330 + 1) {
    if ((piVar7[3] == 5) && (piVar7[4] == 0x3d)) {
      iVar3 = (&DAT_006cb942)[piVar7[1]] + (&DAT_006cb8c2)[piVar7[2]] + *piVar7;
      iVar6 = local_33c;
      if (*(short *)(DAT_006c54dc + 6 + iVar3 * 0xc) == -1) {
        uVar1 = *(ushort *)(DAT_006c54dc + iVar3 * 0xc);
        if ((uVar1 != 0) && (*(int *)(DAT_0065660c + 0x94 + (uint)uVar1 * 0x164) == 0x10)) break;
        if ((piVar7[5] == 100) && ((DAT_00689920 != 2 && (DAT_00689920 != 1)))) {
          FUN_004ecc80(*piVar7,piVar7[1],piVar7[2],piVar7[6]);
          piVar4[-1] = *piVar7;
          *piVar4 = piVar7[1];
          piVar4[1] = piVar7[2];
          piVar4[2] = 0;
          piVar4[3] = -1;
          iVar6 = local_33c + 1;
          piVar4 = piVar4 + 5;
          local_33c = iVar6;
          if (*(int *)(param_1 + 0x7f4) <= iVar6) break;
        }
        else {
          local_334 = local_334 + piVar7[5];
          local_324[-1] = local_334;
          *local_324 = local_330;
          local_328 = local_328 + 1;
          local_324 = local_324 + 2;
        }
        iVar6 = local_33c;
        if (DAT_006e1ee8 != 0) {
          FUN_0059d50b(local_32c,s_Region_power_deployed__i_Positio_005e4ac4,piVar7[4],*piVar7,
                       piVar7[1],piVar7[2]);
        }
      }
    }
    piVar7 = piVar7 + 10;
  }
  if (iVar6 < *(int *)(param_1 + 0x7f4)) {
    puVar9 = (undefined4 *)(param_1 + 0x88 + iVar6 * 0x14);
    do {
      if ((local_334 == 0) || (local_328 == 0)) break;
      uVar2 = FUN_0054e200();
      if (DAT_006e1f30 != 0) {
        FUN_00485b10(s_c__Projects_Chaos_Map_cpp_005e49ac,0xd2b);
      }
      iVar3 = 0;
      if (0 < local_328) {
        piVar4 = local_320;
        do {
          if ((int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)local_334) < *piVar4) {
            local_320[iVar3 * 2] = -1;
            iVar5 = local_320[iVar3 * 2 + 1];
            if ((iVar5 < 0) || (*(int *)(param_1 + 0x68) <= iVar5)) {
              iVar5 = 0;
            }
            else {
              iVar5 = *(int *)(param_1 + 100) + iVar5 * 0x28;
            }
            local_334 = local_334 - *(int *)(iVar5 + 0x14);
            break;
          }
          iVar3 = iVar3 + 1;
          piVar4 = piVar4 + 2;
        } while (iVar3 < local_328);
      }
      iVar3 = local_320[iVar3 * 2 + 1];
      if ((iVar3 < 0) || (*(int *)(param_1 + 0x68) <= iVar3)) {
        puVar8 = (undefined4 *)0x0;
      }
      else {
        puVar8 = (undefined4 *)(*(int *)(param_1 + 100) + iVar3 * 0x28);
      }
      FUN_004ecc80(*puVar8,puVar8[1],puVar8[2],puVar8[6]);
      iVar6 = iVar6 + 1;
      puVar9[-1] = *puVar8;
      *puVar9 = puVar8[1];
      puVar9[1] = puVar8[2];
      puVar9[2] = 0;
      puVar9[3] = 0xffffffff;
      puVar9 = puVar9 + 5;
    } while (iVar6 < *(int *)(param_1 + 0x7f4));
  }
  if (DAT_006e1ee8 != 0) {
    FUN_0059c4c3(local_32c);
  }
  if (iVar6 != *(int *)(param_1 + 0x7f4)) {
    *(int *)(param_1 + 0x7f4) = iVar6;
  }
  return;
}

