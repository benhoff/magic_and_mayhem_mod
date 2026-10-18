/* Optional diagnostic refusal policy. Resolver coverage is not a global route
 * sandbox: direct driver calls, XCB/DRI and front-buffer writers remain outside. */
static int presentation_route_strict(void){
 const char* value=getenv("MNM_DRAW_STRICT_ROUTES");
 if(!value)return 0;
 if(strcmp(value,"1"))gl_refuse("Invalid MNM_DRAW_STRICT_ROUTES");
 return 1;
}
static int presentation_route_alternate(const char* name){
 return !strcmp(name,"eglSwapBuffersWithDamageKHR")||!strcmp(name,"eglSwapBuffersWithDamageEXT")||
  !strcmp(name,"eglPostSubBufferNV")||!strcmp(name,"eglSwapBuffersRegionNOK")||
  !strcmp(name,"eglSwapBuffersRegion2NOK")||!strcmp(name,"glXCopySubBufferMESA")||
  !strcmp(name,"glXSwapBuffersMscOML");
}
static int presentation_route_selected(const char* name){
 return presentation_route_alternate(name)||!strcmp(name,"glXSwapBuffers")||
  !strcmp(name,"glXGetProcAddress")||!strcmp(name,"glXGetProcAddressARB")||
  !strcmp(name,"eglSwapBuffers")||!strcmp(name,"eglGetProcAddress")||
  !strcmp(name,"glBlitFramebuffer")||!strcmp(name,"glBlitFramebufferEXT");
}
static void presentation_route_lookup(const char* name,int next){
 if(!presentation_route_selected(name)||!presentation_route_strict())return;
 if(next)gl_refuse("Strict routes reject selected RTLD_NEXT lookup");
 if(presentation_route_alternate(name))gl_refuse("Strict routes reject alternate presentation lookup");
}
