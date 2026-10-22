
void __thiscall
FUN_004a59e0(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5
            )

{
  int iVar1;
  char cVar2;
  
  iVar1 = FUN_004a58e0(param_2,1);
  cVar2 = (char)param_2;
  *(int *)(param_1 + 0x218) = *(int *)(param_1 + 0x218) + iVar1;
  if ((((cVar2 != ' ') && (cVar2 != '\t')) && (cVar2 != '\n')) && (cVar2 != '_')) {
    FUN_00581ec0(*(undefined4 *)(param_1 + 0x21c),param_3,param_4,param_5);
  }
  return;
}

