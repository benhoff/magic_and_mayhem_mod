
bool __thiscall FUN_0058d320(int param_1,char *param_2)

{
  bool bVar1;
  void *pvVar2;
  
  pvVar2 = (void *)FUN_0058b660();
  bVar1 = false;
  if (pvVar2 != (void *)0x0) {
    bVar1 = jpeg_read(param_2,pvVar2,*(int *)(param_1 + 0x10),*(int *)(param_1 + 0x18),
                      *(int *)(param_1 + 0x14),DAT_006e1f88);
    (**(code **)(**(int **)(param_1 + 4) + 0x80))(*(int **)(param_1 + 4),pvVar2);
  }
  return bVar1;
}

