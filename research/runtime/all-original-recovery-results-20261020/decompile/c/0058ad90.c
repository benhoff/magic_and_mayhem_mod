
void __thiscall FUN_0058ad90(undefined4 *param_1,int *param_2)

{
  int *piVar1;
  undefined4 *puVar2;
  int *piVar3;
  undefined1 uVar4;
  int iVar5;
  undefined4 uVar6;
  undefined1 *puVar7;
  int *piStack_424;
  int *piStack_420;
  undefined4 uStack_41c;
  undefined4 uStack_418;
  int iStack_24;
  int iStack_20;
  int iStack_18;
  
  if (0x7f < DAT_006f8154) {
    return;
  }
  uStack_418 = 0;
  piStack_420 = param_2;
  piStack_424 = DAT_006f98c8;
  uStack_41c = param_1 + 1;
  iVar5 = (**(code **)(*DAT_006f98c8 + 0x18))();
  if (iVar5 != 0) {
    FUN_00484c00(iVar5);
  }
  puVar2 = (undefined4 *)param_1[1];
  piVar1 = param_1 + 2;
  iVar5 = (**(code **)*puVar2)(puVar2,&DAT_005c7990,piVar1);
  if (iVar5 != 0) {
    FUN_00484c00(iVar5);
    if (DAT_006f98c8 != (int *)0x0) {
      while (DAT_006f6950 != 0) {
        FUN_0058b3e0();
      }
      if (DAT_006f68d8 == 0) {
        if (DAT_006f98d0 != (int *)0x0) {
          (**(code **)(*DAT_006f98d0 + 8))(DAT_006f98d0);
          DAT_006f98d0 = (int *)0x0;
          if (DAT_006f68d8 != 0) goto LAB_0058ae45;
        }
      }
      else {
LAB_0058ae45:
        (**(code **)(*DAT_006f98c8 + 0x4c))(DAT_006f98c8);
        DAT_006f68d8 = 0;
      }
      (**(code **)(*DAT_006f98c4 + 8))(DAT_006f98c4);
      (**(code **)(*DAT_006f98c8 + 8))(DAT_006f98c8);
      (**(code **)(*DAT_006f98cc + 8))(DAT_006f98cc);
      DAT_006f98c4 = (int *)0x0;
      DAT_006f98c8 = (int *)0x0;
      DAT_006f98cc = (int *)0x0;
      DAT_006f98c0 = (undefined4 *)0x0;
    }
    DestroyWindow(DAT_00656614);
  }
  if (DAT_006e1f1c != 0) {
    iVar5 = (**(code **)(*(int *)*piVar1 + 0x94))((int *)*piVar1,0);
  }
  if (iVar5 == -0x7789fe3e) {
    piVar3 = (int *)*piVar1;
    if ((piVar3 != (int *)0x0) && (iVar5 = (**(code **)(*piVar3 + 0x6c))(piVar3), iVar5 == 0)) {
      if (DAT_006de6dd != 0) {
        piStack_424 = (int *)(uint)*(ushort *)(param_1 + 0xb);
        piStack_420 = piStack_424;
        (**(code **)(*(int *)*piVar1 + 0x74))((int *)*piVar1,8,&piStack_424);
      }
      puVar2 = (undefined4 *)param_1[10];
      if (puVar2 == (undefined4 *)0xffffffff) {
        FUN_004a2f90();
      }
      else if (puVar2 != (undefined4 *)0x0) {
        (**(code **)*puVar2)(0);
      }
    }
LAB_0058af15:
    if (DAT_006f98c8 != (int *)0x0) {
      while (DAT_006f6950 != 0) {
        FUN_0058b3e0();
      }
      if (DAT_006f68d8 == 0) {
        if (DAT_006f98d0 != (int *)0x0) {
          (**(code **)(*DAT_006f98d0 + 8))(DAT_006f98d0);
          DAT_006f98d0 = (int *)0x0;
          if (DAT_006f68d8 != 0) goto LAB_0058af60;
        }
      }
      else {
LAB_0058af60:
        (**(code **)(*DAT_006f98c8 + 0x4c))(DAT_006f98c8);
        DAT_006f68d8 = 0;
      }
      (**(code **)(*DAT_006f98c4 + 8))(DAT_006f98c4);
      (**(code **)(*DAT_006f98c8 + 8))(DAT_006f98c8);
      (**(code **)(*DAT_006f98cc + 8))(DAT_006f98cc);
      DAT_006f98c4 = (int *)0x0;
      DAT_006f98c8 = (int *)0x0;
      DAT_006f98cc = (int *)0x0;
      DAT_006f98c0 = (undefined4 *)0x0;
    }
    DestroyWindow(DAT_00656614);
  }
  else if (iVar5 != 0) goto LAB_0058af15;
  param_1[7] = DAT_006f8154;
  (&DAT_006f6950)[DAT_006f8154] = (int)param_1;
  DAT_006f8154 = DAT_006f8154 + 1;
  if ((*(int *)(iStack_18 + 4) == 1) && (*(int *)(iStack_18 + 0x68) == 0x200)) {
    DAT_006f98c0 = param_1;
    if (DAT_006f8174 != 0) {
      iVar5 = 0;
      puVar7 = (undefined1 *)((int)&uStack_41c + 1);
      do {
        uVar4 = (undefined1)iVar5;
        puVar7[-1] = uVar4;
        *puVar7 = uVar4;
        puVar7[1] = uVar4;
        iVar5 = iVar5 + 1;
        puVar7 = puVar7 + 4;
      } while (iVar5 < 0x100);
      iVar5 = (**(code **)(*DAT_006f98c8 + 0x14))(DAT_006f98c8,4,&uStack_41c,param_1 + 3,0);
      if (iVar5 != 0) {
        if (DAT_006f98c8 != (int *)0x0) {
          while (DAT_006f6950 != 0) {
            FUN_0058b3e0();
          }
          if (DAT_006f68d8 == 0) {
            if (DAT_006f98d0 != (int *)0x0) {
              (**(code **)(*DAT_006f98d0 + 8))(DAT_006f98d0);
              DAT_006f98d0 = (int *)0x0;
              if (DAT_006f68d8 != 0) goto LAB_0058b087;
            }
          }
          else {
LAB_0058b087:
            (**(code **)(*DAT_006f98c8 + 0x4c))(DAT_006f98c8);
            DAT_006f68d8 = 0;
          }
          (**(code **)(*DAT_006f98c4 + 8))(DAT_006f98c4);
          (**(code **)(*DAT_006f98c8 + 8))(DAT_006f98c8);
          (**(code **)(*DAT_006f98cc + 8))(DAT_006f98cc);
          DAT_006f98c4 = (int *)0x0;
          DAT_006f98c8 = (int *)0x0;
          DAT_006f98cc = (int *)0x0;
          DAT_006f98c0 = (undefined4 *)0x0;
        }
        DestroyWindow(DAT_00656614);
      }
      (**(code **)(*(int *)*piVar1 + 0x7c))((int *)*piVar1,param_1[3]);
    }
    if ((DAT_006f68d8 == 0) &&
       (iVar5 = (**(code **)(*(int *)*piVar1 + 0x70))((int *)*piVar1,DAT_006f98d0), iVar5 != 0)) {
      FUN_00484c00(iVar5);
    }
  }
  iVar5 = (**(code **)(*(int *)param_1[1] + 0x58))((int *)param_1[1],iStack_18);
  if (iVar5 == 0) goto LAB_0058b1d7;
  FUN_00484c00(iVar5);
  if (DAT_006f98c8 != (int *)0x0) {
    while (DAT_006f6950 != 0) {
      FUN_0058b3e0();
    }
    if (DAT_006f68d8 == 0) {
      if (DAT_006f98d0 != (int *)0x0) {
        (**(code **)(*DAT_006f98d0 + 8))(DAT_006f98d0);
        DAT_006f98d0 = (int *)0x0;
        if (DAT_006f68d8 != 0) goto LAB_0058b181;
      }
    }
    else {
LAB_0058b181:
      (**(code **)(*DAT_006f98c8 + 0x4c))(DAT_006f98c8);
      DAT_006f68d8 = 0;
    }
    (**(code **)(*DAT_006f98c4 + 8))(DAT_006f98c4);
    (**(code **)(*DAT_006f98c8 + 8))(DAT_006f98c8);
    (**(code **)(*DAT_006f98cc + 8))(DAT_006f98cc);
    DAT_006f98c4 = (int *)0x0;
    DAT_006f98c8 = (int *)0x0;
    DAT_006f98cc = (int *)0x0;
    DAT_006f98c0 = (undefined4 *)0x0;
  }
  DestroyWindow(DAT_00656614);
LAB_0058b1d7:
  if (*(int *)(iStack_18 + 0xc) == 0) {
    iVar5 = FUN_0058b270();
    if (iVar5 == 0) {
      iVar5 = GetSystemMetrics(0);
    }
    else {
      iVar5 = (*DAT_006f98a0)();
    }
    param_1[4] = iVar5;
    iVar5 = FUN_0058b270();
    if (iVar5 == 0) {
      iVar5 = GetSystemMetrics(1);
      param_1[5] = iVar5;
      iStack_24 = iStack_20;
    }
    else {
      uVar6 = (*DAT_006f98a0)();
      param_1[5] = uVar6;
    }
  }
  else {
    param_1[4] = *(int *)(iStack_18 + 0xc);
    param_1[5] = *(undefined4 *)(iStack_18 + 8);
    iStack_24 = iStack_18;
  }
  if ((*(uint *)(iStack_24 + 0x68) & 0x4000) == 0) {
    *param_1 = 0;
  }
  else {
    *param_1 = 1;
  }
  *(undefined4 *)((int)param_1 + 0x2e) = 0;
  *(undefined2 *)(param_1 + 0xb) = 0;
  param_1[10] = 0;
  *(int *)((int)param_1 + 0x2e) = param_1[5] * param_1[4] * 2;
  return;
}

