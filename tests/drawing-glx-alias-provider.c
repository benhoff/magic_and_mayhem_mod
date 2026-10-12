/* Independent fake driver exports used only to test exact function ownership. */
typedef void(*Function)(void);
static unsigned calls;
void glXSwapBuffers(void* display,unsigned long drawable){(void)display;(void)drawable;++calls;}
unsigned mnm_provider_calls(void){return calls;}
Function glXGetProcAddressARB(const unsigned char* name){(void)name;return (Function)glXSwapBuffers;}
Function glXGetProcAddress(const unsigned char* name){(void)name;return (Function)glXSwapBuffers;}
