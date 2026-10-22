
void FUN_0058d660(void)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 *puVar4;
  
  iVar3 = 0;
  iVar1 = DAT_006f98d4;
  do {
    if (iVar1 == 0) {
      return;
    }
    if (0 < iVar1) {
      puVar4 = &DAT_006f6b78;
      iVar2 = iVar1;
      do {
        if (((puVar4[-10] != 0) && (iVar2 = iVar2 + -1, iVar3 == puVar4[-9])) &&
           ((iVar1 = (**(code **)(**(int **)(puVar4[-7] + 8) + 0x34))(*(int **)(puVar4[-7] + 8),1),
            iVar1 == 0 || (iVar1 == -0x7789fdc1)))) {
          iVar1 = puVar4[-6];
          DAT_006f98d8 = 1;
          if (iVar1 == 0) {
            FUN_0058c8a0(puVar4[-7],puVar4 + -5,puVar4[-1],*puVar4);
LAB_0058d70e:
            puVar4[-10] = 0;
            DAT_006f98d4 = DAT_006f98d4 + -1;
          }
          else {
            if (iVar1 == 1) {
              FUN_0058c140(puVar4[-7],puVar4[-1],*puVar4);
              goto LAB_0058d70e;
            }
            if (iVar1 == 2) {
              FUN_0058c4a0(puVar4[-7],puVar4 + -5,puVar4[-1],*puVar4);
              goto LAB_0058d70e;
            }
          }
          DAT_006f98d8 = 0;
        }
        puVar4 = puVar4 + 0xb;
        iVar1 = DAT_006f98d4;
      } while (0 < iVar2);
    }
    iVar3 = iVar3 + 1;
    if (2 < iVar3) {
      return;
    }
  } while( true );
}

