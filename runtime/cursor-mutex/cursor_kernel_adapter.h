#pragma once
/* Native diagnostic state phases only. Owning module serializes each state
 * phase. Duplicate/Wait/Release/Close calls occur BETWEEN phases, outside locks.
 * Publication requires original constructor return still owns the new handle. */
#include "cursor_kernel_adapter_core.h"
static int mnm_cursor_kernel_publish(MnmCursorKernelAdapter* adapter,unsigned role,
 void* original,void* duplicate,unsigned identity){
 if(adapter->fault||role>1)return 0;
 return mnm_cursor_lifetime_publish(role?&adapter->inner:&adapter->outer,original,duplicate,identity);
}
static int mnm_cursor_kernel_retire(MnmCursorKernelAdapter* adapter,unsigned role,
 void* original,unsigned identity){
 if(role>1)return 0;
 return mnm_cursor_lifetime_retire(role?&adapter->inner:&adapter->outer,original,identity);
}
static void* mnm_cursor_kernel_close_begin(MnmCursorKernelAdapter* adapter,unsigned role){
 if(role>1)return 0;
 return mnm_cursor_lifetime_close_begin(role?&adapter->inner:&adapter->outer);
}
static int mnm_cursor_kernel_close_end(MnmCursorKernelAdapter* adapter,unsigned role,
 void* duplicate,unsigned identity,unsigned result){
 if(role>1)return 0;
 return mnm_cursor_lifetime_close_end(role?&adapter->inner:&adapter->outer,duplicate,identity,result);
}
