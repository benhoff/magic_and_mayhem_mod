
undefined8 __fastcall FUN_0059455c(undefined4 param_1,undefined4 param_2)

{
  undefined8 uVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  undefined4 uVar5;
  undefined4 in_EAX;
  int iVar6;
  int iVar7;
  undefined8 *puVar8;
  undefined8 *puVar9;
  
  iVar7 = DAT_006a2dc8;
  puVar8 = DAT_006e1f6c;
  puVar9 = DAT_00658174;
  iVar6 = DAT_006def50;
  do {
    do {
      uVar5 = *(undefined4 *)((int)puVar8 + 0x24);
      uVar1 = puVar8[3];
      *(undefined4 *)(puVar9 + 4) = *(undefined4 *)(puVar8 + 4);
      *(undefined4 *)((int)puVar9 + 0x24) = uVar5;
      uVar5 = *(undefined4 *)((int)puVar8 + 0x2c);
      uVar2 = puVar8[2];
      *(undefined4 *)(puVar9 + 5) = *(undefined4 *)(puVar8 + 5);
      *(undefined4 *)((int)puVar9 + 0x2c) = uVar5;
      uVar5 = *(undefined4 *)((int)puVar8 + 0x34);
      uVar3 = puVar8[1];
      *(undefined4 *)(puVar9 + 6) = *(undefined4 *)(puVar8 + 6);
      *(undefined4 *)((int)puVar9 + 0x34) = uVar5;
      uVar5 = *(undefined4 *)((int)puVar8 + 0x3c);
      uVar4 = *puVar8;
      *(undefined4 *)(puVar9 + 7) = *(undefined4 *)(puVar8 + 7);
      *(undefined4 *)((int)puVar9 + 0x3c) = uVar5;
      *puVar9 = uVar4;
      puVar9[1] = uVar3;
      puVar9[2] = uVar2;
      puVar9[3] = uVar1;
      puVar8 = puVar8 + 8;
      puVar9 = puVar9 + 8;
      iVar7 = iVar7 + -0x20;
    } while (iVar7 != 0);
    puVar8 = (undefined8 *)((int)puVar8 + DAT_006e1f70 * 4);
    iVar6 = iVar6 + -1;
    iVar7 = DAT_006a2dc8;
  } while (iVar6 != 0);
  return CONCAT44(param_2,in_EAX);
}

