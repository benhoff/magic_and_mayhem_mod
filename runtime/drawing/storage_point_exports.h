#pragma once
static const char* storage_point_export_name(u32 client){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_STORAGE_TRANSITIONS",flag,2);
 if(n==1&&flag[0]=='1')return client?"SceneDrawingClientStoragePointV2":"SceneDrawingStoragePointV2";
 return client?"SceneDrawingClientStoragePointV1":"SceneDrawingStoragePointV1";
}
