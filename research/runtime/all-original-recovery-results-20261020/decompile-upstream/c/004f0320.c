
void __fastcall FUN_004f0320(int param_1)

{
  int iVar1;
  uint uVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  undefined4 uVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  undefined4 local_74;
  uint local_70;
  undefined4 local_6c;
  int local_68;
  int local_64;
  int local_60;
  int local_5c;
  undefined4 local_58;
  undefined4 local_54 [16];
  undefined4 local_14;
  undefined4 local_10;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_005c2318;
  local_c = ExceptionList;
  local_14 = 0;
  local_10 = 0;
  local_4 = 0;
  local_74 = 0;
  local_70 = 0;
  local_6c = 0;
  local_68 = 0;
  local_60 = 0;
  ExceptionList = &local_c;
  local_5c = param_1;
  if ((DAT_006e1ee8 != 0) &&
     (ExceptionList = &local_c, iVar1 = FUN_0059cae2(s_Creature_plc_005e4ab4,&DAT_005dc824),
     local_60 = iVar1, iVar1 != 0)) {
    FUN_0059d50b(iVar1,s_CreaturesPlacement_log_005e4a9c);
    FUN_0059d50b(iVar1,s____________________005e47c0);
  }
  puVar6 = *(undefined4 **)(param_1 + 100);
  local_64 = 0;
  while ((local_64 < *(int *)(param_1 + 0x68) && (local_68 < *(int *)(param_1 + 0x804)))) {
    if (puVar6[3] == 4) {
      uVar2 = FUN_0054e200();
      if (DAT_006e1f30 != 0) {
        FUN_00485b10(s_c__Projects_Chaos_Map_cpp_005e49ac,0xc8d);
      }
      if (((int)(((uVar2 & 0x7fffffff) % 99 + 1) - (&DAT_006c5a00)[puVar6[4]]) <= (int)puVar6[5]) &&
         (puVar3 = (undefined4 *)FUN_0052a640(), puVar3 != (undefined4 *)0x0)) {
        local_74 = FUN_0040e650(*puVar6);
        local_70 = puVar6[1];
        if ((int)local_70 < 0) {
          do {
            local_70 = local_70 + DAT_006c5498;
          } while ((int)local_70 < 0);
        }
        else {
          for (; DAT_006c5498 <= local_70; local_70 = local_70 - DAT_006c5498) {
          }
        }
        local_6c = puVar6[2];
        local_58 = *puVar3;
        iVar1 = puVar6[4] * 0x5c9;
        puVar4 = (undefined4 *)
                 FUN_0040e250(*(undefined4 *)((int)&DAT_006a5f8c + iVar1),
                              *(undefined4 *)((int)&DAT_006a5f88 + iVar1),
                              *(undefined4 *)(&DAT_006a5fbc + iVar1),
                              *(undefined4 *)((int)&DAT_006a5fc4 + iVar1),0,
                              *(undefined4 *)((int)&DAT_006a5f80 + puVar6[4] * 0x5c9),0);
        puVar7 = local_54;
        for (iVar1 = 5; iVar1 != 0; iVar1 = iVar1 + -1) {
          *puVar7 = *puVar4;
          puVar4 = puVar4 + 1;
          puVar7 = puVar7 + 1;
        }
        *(undefined2 *)puVar7 = *(undefined2 *)puVar4;
        iVar1 = FUN_004f5230(local_58,local_54,&local_74,&local_70,&local_6c);
        param_1 = local_5c;
        if (iVar1 != 0) {
          iVar1 = puVar6[4];
          if (DAT_006dbe9d == '\0') {
            uVar5 = *(undefined4 *)(&DAT_005c6fd4 + iVar1 * 4);
          }
          else {
            uVar5 = *(undefined4 *)(&DAT_005c7044 + iVar1 * 4);
          }
          iVar1 = FUN_005055a0(iVar1,uVar5);
          if (iVar1 == 0) {
            MessageBoxA((HWND)0x0,s_Unable_to_load_creature_file_005d95a0,
                        s_DEPLOYMENT_ERROR_005d95c0,0x42030);
            DestroyWindow(DAT_00656614);
          }
          FUN_005070e0(local_74,local_70,local_6c,puVar6[4]);
          iVar1 = puVar6[6];
          if ((iVar1 < 0) || (7 < iVar1)) {
            MessageBoxA((HWND)0x0,s_creature_army_index_out_of_range_005e4a68,
                        s_DEPLOYMENT_ERROR_005d95c0,0x42030);
            iVar1 = 0;
          }
          FUN_00506290(puVar6[4],iVar1,0);
          if (DAT_006e1ee8 != 0) {
            FUN_0059d50b(local_60,s_Creature_Deployed__i_Position_____005e4a3c,puVar6[4],local_74,
                         local_70,local_6c);
          }
          *(undefined4 *)((int)puVar3 + 0xbcf) = local_74;
          *(uint *)((int)puVar3 + 0xbd3) = local_70;
          *(undefined4 *)((int)puVar3 + 0xbd7) = local_6c;
          *(undefined4 *)((int)puVar3 + 0xbdb) = puVar6[7];
          *(undefined4 *)((int)puVar3 + 0xbdf) = puVar6[8];
          puVar3[0x1c1] = puVar6[9];
          *(undefined4 *)((int)puVar3 + 0xbe3) =
               *(undefined4 *)(&DAT_006a6501 + puVar3[0x2a] * 0x5c9);
          *(undefined4 *)((int)puVar3 + 0xbe7) =
               *(undefined4 *)(&DAT_006a6505 + puVar3[0x2a] * 0x5c9);
          *(undefined4 *)((int)puVar3 + 0xbeb) = 0;
          *(undefined4 *)((int)puVar3 + 0xbef) = 0;
          if (puVar3[0x1c1] != 0) {
            *(undefined4 *)((int)puVar3 + 0xbef) = 5;
          }
          local_68 = local_68 + 1;
          param_1 = local_5c;
        }
      }
    }
    local_64 = local_64 + 1;
    puVar6 = puVar6 + 10;
  }
  if (DAT_006e1ee8 != 0) {
    FUN_0059c4c3(local_60);
  }
  local_4 = 0xffffffff;
  FUN_00532f00();
  ExceptionList = local_c;
  return;
}

