#include "../runtime/shadow/win32_min.h"
#include "../runtime/scene/sample_admission.h"
void start(void){
 if(!scene_sample_admitted(120,1,1,1,0,1)||!scene_sample_admitted(16,0,0,0,0,0)||!scene_sample_admitted(32,0,1,1,0,0)||!scene_sample_admitted(32,0,1,0,1,0))ExitProcess(201);
 if(scene_sample_admitted(120,0,1,1,0,1)||scene_sample_admitted(121,1,1,1,0,1)||scene_sample_admitted(120,1,0,1,0,1)||scene_sample_admitted(120,1,1,0,0,1)||scene_sample_admitted(120,1,1,1,0,0)||scene_sample_admitted(120,1,1,1,1,1)||scene_sample_admitted(0,0,0,0,0,0)||scene_sample_admitted(33,0,1,1,0,0))ExitProcess(202);ExitProcess(0);
}
