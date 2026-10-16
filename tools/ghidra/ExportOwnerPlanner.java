// Read-only owner task factory references and their initialization bodies.
// @category MagicMayhem
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import java.nio.file.*;
import java.util.*;
public class ExportOwnerPlanner extends GhidraScript {
 public void run() throws Exception {
  if (!"40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168".equals(currentProgram.getExecutableSHA256())) throw new IllegalStateException("build");
  Path out=Paths.get(getScriptArgs()[0]); Files.createDirectory(out);
  DecompInterface d=new DecompInterface();d.openProgram(currentProgram);
  Set<Long> functions=new TreeSet<>(); StringBuilder text=new StringBuilder();
  for(long a=0x5d8474;a<0x5d8574;a+=4) {
   long p=Integer.toUnsignedLong(getInt(toAddr(a))); if(p==0)break;
   text.append(String.format("factory %x -> %x\n",a,p));
   for(var ref:getReferencesTo(toAddr(p))) {
    var f=getFunctionContaining(ref.getFromAddress());text.append(ref+" function="+(f==null?"none":f.getEntryPoint())+"\n");
    if(f!=null)functions.add(f.getEntryPoint().getOffset());
    else {
     var ins=getInstructionAt(ref.getFromAddress());
     for(int i=0;i<10 && ins!=null;i++,ins=ins.getNext()) {
      text.append(ins.getAddress()+" "+ins+"\n");
      for(var call:ins.getReferencesFrom())if(call.getReferenceType().isCall()) {
       var callee=getFunctionAt(call.getToAddress());if(callee!=null)functions.add(callee.getEntryPoint().getOffset());
      }
     }
    }
   }
  }
  Files.writeString(out.resolve("factories.refs"),text);
  for(long a:functions) {
   var r=d.decompileFunction(getFunctionAt(toAddr(a)),60,monitor);
   if(r.decompileCompleted())Files.writeString(out.resolve(Long.toHexString(a)+".c"),r.getDecompiledFunction().getC());
  }
  d.dispose();
 }
}
