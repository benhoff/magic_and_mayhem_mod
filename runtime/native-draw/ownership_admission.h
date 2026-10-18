/* Optional native-only ownership profile. Never admit a borrowed allocation or
 * a diagnostic writable-public-page mode as protected native backing. */
static u32 ns_ownership_required;
static void ns_ownership_admit(u32 mode){
 char flag[2];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_NATIVE_OWNERSHIP_REQUIRED",flag,2);
 if(!n){if(GetLastError()==203)return;ExitProcess(124);}
 if(n!=1 || flag[0]!='1')ExitProcess(124);
 char resident[2];
 if(mode!=3 || GetEnvironmentVariableA("MNM_PUBLIC_RESIDENT_WORLD",resident,2)!=1 || resident[0]!='1')ExitProcess(124);
 ns_ownership_required=1;
}
