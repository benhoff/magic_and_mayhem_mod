// Build-pinned, read-only font provider and ownership reference inspection.
// @category MagicMayhem
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import java.nio.file.*;
public class ExportFontProviders extends GhidraScript {
 public void run() throws Exception {
  if(!"40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168".equals(currentProgram.getExecutableSHA256()))throw new IllegalStateException("Unpinned executable");
  String[] args=getScriptArgs();Path out=Paths.get(args[0]);Files.createDirectory(out);
  DecompInterface d=new DecompInterface();d.openProgram(currentProgram);
  try {
   for(int i=1;i<args.length;i++) {
    var a=toAddr(Long.parseLong(args[i],16));StringBuilder refs=new StringBuilder();var it=currentProgram.getReferenceManager().getReferencesTo(a);
    while(it.hasNext()){var ref=it.next();var caller=getFunctionContaining(ref.getFromAddress());refs.append(ref).append(" owner=").append(caller==null?"unassigned":caller.getEntryPoint()).append("\n");}
    Files.writeString(out.resolve(a+".refs"),refs);
    if(a.getOffset()<0x5c0000L){var f=getFunctionAt(a);if(f==null){println("No discovered entry: "+a);continue;}var r=d.decompileFunction(f,60,monitor);if(!r.decompileCompleted())throw new IllegalStateException("Decompile failed: "+a);Files.writeString(out.resolve(a+".c"),r.getDecompiledFunction().getC());}
   }
  }finally{d.dispose();}
 }
}
