
void __thiscall FUN_004efc80(int param_1,int param_2)

{
  int *piVar1;
  ushort uVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  int *piVar7;
  
  iVar6 = 0;
  if (DAT_006e1ee8 != 0) {
    iVar6 = FUN_0059cae2(s_objects_plc_005e4a30,&DAT_005dc824);
    if (iVar6 != 0) {
      FUN_0059d50b(iVar6,s_Object_Placement_log_005e4a18);
      FUN_0059d50b(iVar6,s____________________005e4a00);
      FUN_0059d50b(iVar6,s_map_generation_seed___d_005e49e4,DAT_00656624);
      FUN_0059d50b(iVar6,s_object_placement_seed___d_005e49c8,param_2);
    }
  }
  piVar7 = *(int **)(param_1 + 100);
  *(undefined4 *)(param_1 + 0x7f8) = 0;
  FUN_00532f00();
  param_2 = 0;
LAB_004efcfa:
  do {
    if (*(int *)(param_1 + 0x68) <= param_2) {
LAB_004efee5:
      if (DAT_006e1ee8 != 0) {
        FUN_0059c4c3(iVar6);
      }
      return;
    }
    if ((piVar7[3] == 5) && (piVar7[4] != 0x3d)) {
      if (piVar7[4] == 0x13) {
        FUN_00533250(piVar7);
      }
      if ((((DAT_0068991c == 0) || (piVar7[4] < 0x35)) || (0x38 < piVar7[4])) &&
         (iVar4 = (&DAT_006cb8c2)[piVar7[2]] + (&DAT_006cb942)[piVar7[1]] + *piVar7,
         *(short *)(DAT_006c54dc + 6 + iVar4 * 0xc) == -1)) {
        uVar2 = *(ushort *)(DAT_006c54dc + iVar4 * 0xc);
        if ((uVar2 != 0) && (*(int *)(DAT_0065660c + 0x94 + (uint)uVar2 * 0x164) == 0x10))
        goto LAB_004efee5;
        uVar5 = FUN_0054e200();
        if (DAT_006e1f30 != 0) {
          FUN_00485b10(s_c__Projects_Chaos_Map_cpp_005e49ac,0xba9);
        }
        if (piVar7[5] < (int)(((uVar5 & 0x7fffffff) % 99 + 1) - (&DAT_006c5a70)[piVar7[4]])) {
          if (DAT_006e1ee8 != 0) {
            FUN_0059d50b(iVar6,s_Object_missed__i_Position____i___005e4958,piVar7[4],*piVar7,
                         piVar7[1],piVar7[2]);
          }
        }
        else {
          iVar4 = FUN_00544cf0();
          if (iVar4 != 0) {
            FUN_00502aa0(*piVar7,piVar7[1],piVar7[2]);
            FUN_00543970(piVar7[4]);
            if (DAT_006e1ee8 != 0) {
              FUN_0059d50b(iVar6,s_Object_Deplaoyed__i_Position_____005e4980,piVar7[4],*piVar7,
                           piVar7[1],piVar7[2]);
            }
            iVar3 = piVar7[4];
            if (iVar3 == 0x32) {
              *(int *)(iVar4 + 0x6a) = piVar7[6];
              piVar1 = piVar7 + 7;
              piVar7 = piVar7 + 10;
              param_2 = param_2 + 1;
              *(int *)(iVar4 + 0x6e) = *piVar1;
              goto LAB_004efcfa;
            }
            if ((0x34 < iVar3) && (iVar3 < 0x39)) {
              piVar1 = piVar7 + 6;
              piVar7 = piVar7 + 10;
              *(int *)(iVar4 + 0x6a) = *piVar1;
              param_2 = param_2 + 1;
              goto LAB_004efcfa;
            }
            if (iVar3 == 0x13) {
              piVar7 = piVar7 + 10;
              *(int *)(param_1 + 0x7f8) = *(int *)(param_1 + 0x7f8) + 1;
              param_2 = param_2 + 1;
              goto LAB_004efcfa;
            }
          }
        }
      }
    }
    piVar7 = piVar7 + 10;
    param_2 = param_2 + 1;
  } while( true );
}

