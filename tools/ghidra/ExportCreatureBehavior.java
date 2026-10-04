// Inferred pseudocode and references; run in a read-only analyzed project.
// @category MagicMayhem
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.nio.file.*;
public class ExportCreatureBehavior extends GhidraScript {
 public void run() throws Exception {
  if (!"40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168".equals(currentProgram.getExecutableSHA256()))
   throw new IllegalStateException("Unsupported executable hash");
  String[] args=getScriptArgs(); Path out=Paths.get(args[0]); Files.createDirectory(out);
  DecompInterface d=new DecompInterface();d.openProgram(currentProgram);
  for(int i=1;i<args.length;i++) {
   var a=toAddr(Long.parseLong(args[i],16));var f=getFunctionContaining(a);
   if(f==null){disassemble(a); f=createFunction(a, null);}
   if(f==null){println("NO FUNCTION "+a);continue;}
   var r=d.decompileFunction(f,60,monitor);
   var refs=currentProgram.getReferenceManager().getReferencesTo(f.getEntryPoint());
   StringBuilder s=new StringBuilder();while(refs.hasNext()){var ref=refs.next();var caller=getFunctionContaining(ref.getFromAddress());s.append(ref+" caller="+(caller==null?"data":caller.getEntryPoint())+"\n");}
   Files.writeString(out.resolve(f.getEntryPoint()+".refs"),s);
   if(r.decompileCompleted())Files.writeString(out.resolve(f.getEntryPoint()+".c"),r.getDecompiledFunction().getC());
   println(a+" in "+f.getEntryPoint()+" end "+f.getBody().getMaxAddress());
  }d.dispose();
 }
}
