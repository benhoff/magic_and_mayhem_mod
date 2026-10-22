
undefined4 __thiscall FUN_004ca0c0(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  undefined4 *puVar5;
  int iVar6;
  undefined4 *puVar7;
  
  if ((param_2 != 0) && (*(int *)(param_1 + 0x848) != 0)) {
    while ((*(int *)(param_1 + 0x844) < *(int *)(param_1 + 0x83c) ||
           (uVar4 = FUN_004a6970(),
           (uint)(*(int *)(param_1 + 0x844) - *(int *)(param_1 + 0x83c)) <= uVar4))) {
      piVar1 = *(int **)(param_1 + 0x834);
      iVar6 = piVar1[2];
      iVar2 = piVar1[1];
      if (iVar6 < iVar2) {
        puVar5 = (undefined4 *)(iVar6 * 0x10 + *piVar1);
        *(undefined4 *)(param_1 + 0x838) = *puVar5;
        *(undefined4 *)(param_1 + 0x83c) = puVar5[1];
        *(undefined4 *)(param_1 + 0x840) = puVar5[2];
        *(undefined4 *)(param_1 + 0x844) = puVar5[3];
        piVar1[2] = piVar1[2] + 1;
      }
      uVar4 = (uint)(iVar6 < iVar2);
      *(uint *)(param_1 + 0x848) = uVar4;
      if (uVar4 == 0) break;
      *(undefined4 *)(param_1 + 0x80c) = *(undefined4 *)(param_1 + 0x838);
    }
    if ((*(int *)(param_1 + 0x848) != 0) && (iVar6 = FUN_0058b660(), iVar6 != 0)) {
      FUN_0057dc20(*(undefined4 *)(*(int *)(param_1 + 0x84c) + 0x14));
      FUN_0057dc60((-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                   (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
      puVar5 = (undefined4 *)(param_1 + 0x838);
      FUN_004a5ce0(param_2,puVar5,0,0,0,0x10,0);
      iVar2 = DAT_006b061c;
      *(int *)(param_1 + 0x83c) = DAT_006b061c;
      if ((*(int *)(param_1 + 0x844) < iVar2) ||
         (uVar4 = FUN_004a6970(),
         (uint)(*(int *)(param_1 + 0x844) - *(int *)(param_1 + 0x83c)) <= uVar4)) {
        piVar1 = *(int **)(param_1 + 0x834);
        iVar2 = piVar1[2];
        iVar3 = piVar1[1];
        if (iVar2 < iVar3) {
          puVar7 = (undefined4 *)(iVar2 * 0x10 + *piVar1);
          *puVar5 = *puVar7;
          *(undefined4 *)(param_1 + 0x83c) = puVar7[1];
          *(undefined4 *)(param_1 + 0x840) = puVar7[2];
          *(undefined4 *)(param_1 + 0x844) = puVar7[3];
          piVar1[2] = piVar1[2] + 1;
        }
        uVar4 = (uint)(iVar2 < iVar3);
        *(uint *)(param_1 + 0x848) = uVar4;
        if (uVar4 != 0) {
          *(undefined4 *)(param_1 + 0x80c) = *puVar5;
        }
      }
      piVar1 = *(int **)(*(int *)(param_1 + 0x84c) + 4);
      (**(code **)(*piVar1 + 0x80))(piVar1,iVar6);
      return 1;
    }
  }
  return 0;
}

