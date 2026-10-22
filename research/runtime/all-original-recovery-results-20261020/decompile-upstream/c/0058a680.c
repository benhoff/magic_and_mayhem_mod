
int * __thiscall FUN_0058a680(int *param_1,uint param_2)

{
  if ((*param_1 != 0) && (param_2 < *(int *)(*param_1 + 0xc) - 1U)) {
    return (int *)(*(int *)(param_1[1] + param_2 * 4) + param_1[2]);
  }
  return param_1 + 3;
}

