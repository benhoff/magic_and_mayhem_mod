// Read-only, build-pinned legacy ANI/SPR reader and selected caller pseudocode.
// @category MagicMayhem
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import java.nio.file.*;
public class ExportLegacyAssetSupport extends GhidraScript {
 public void run() throws Exception {
  if (!"40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168".equals(currentProgram.getExecutableSHA256()))
   throw new IllegalStateException("Unsupported executable hash");
  String[] args=getScriptArgs();Path out=Paths.get(args[0]);Files.createDirectory(out);
  DecompInterface d=new DecompInterface();d.openProgram(currentProgram);
  try {
   for(int i=1;i<args.length;i++) {
    var a=toAddr(Long.parseLong(args[i],16));var f=getFunctionContaining(a);
    if(f==null) {disassemble(a); f=createFunction(a,null);}
    if(f==null) throw new IllegalStateException("Cannot identify function at "+a);
    var result=d.decompileFunction(f,60,monitor);
    if(!result.decompileCompleted()) throw new IllegalStateException("Decompile failed at "+a);
    Files.writeString(out.resolve(f.getEntryPoint()+".c"),result.getDecompiledFunction().getC());
    StringBuilder refs=new StringBuilder();var it=currentProgram.getReferenceManager().getReferencesTo(f.getEntryPoint());
    while(it.hasNext()) refs.append(it.next()).append("\n");
    Files.writeString(out.resolve(f.getEntryPoint()+".refs"),refs);
    println(a+" entry="+f.getEntryPoint()+" end="+f.getBody().getMaxAddress());
   }
  } finally {d.dispose();}
 }
}
