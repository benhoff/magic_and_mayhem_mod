
undefined8 __fastcall FUN_00594508(undefined4 param_1,undefined4 param_2)

{
  undefined4 uVar1;
  undefined4 in_EAX;
  int iVar2;
  int iVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  
  iVar3 = DAT_006a2dc8;
  puVar4 = DAT_006e1f6c;
  puVar5 = DAT_00658174;
  iVar2 = DAT_006def50;
  do {
    do {
      uVar1 = puVar4[1];
      *puVar5 = *puVar4;
      puVar5[1] = uVar1;
      puVar4 = puVar4 + 2;
      puVar5 = puVar5 + 2;
      iVar3 = iVar3 + -4;
    } while (iVar3 != 0);
    puVar4 = puVar4 + DAT_006e1f70;
    iVar2 = iVar2 + -1;
    iVar3 = DAT_006a2dc8;
  } while (iVar2 != 0);
  return CONCAT44(param_2,in_EAX);
}

