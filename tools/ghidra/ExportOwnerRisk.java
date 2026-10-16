// Read-only owner risk tolerance data and write references.
// @category MagicMayhem
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import java.nio.file.*;
import java.util.*;
public class ExportOwnerRisk extends GhidraScript {
 public void run() throws Exception {
  if(!"40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168".equals(currentProgram.getExecutableSHA256()))throw new IllegalStateException("build");
  Path out=Paths.get(getScriptArgs()[0]);Files.createDirectory(out);
  var address=toAddr(0x5faf1c);StringBuilder text=new StringBuilder();
  text.append("initial_word="+Integer.toUnsignedString(getInt(address))+"\n");
  Set<Long> entries=new TreeSet<>();
  for(var ref:getReferencesTo(address)){
   var f=getFunctionContaining(ref.getFromAddress());text.append(ref+" function="+(f==null?"unknown":f.getEntryPoint())+"\n");
   if(f!=null)entries.add(f.getEntryPoint().getOffset());
   var i=getInstructionAt(ref.getFromAddress());if(i!=null)text.append(i+"\n");
  }
  Files.writeString(out.resolve("risk.refs"),text);
  DecompInterface d=new DecompInterface();d.openProgram(currentProgram);
  for(long entry:entries){var f=getFunctionAt(toAddr(entry));var r=d.decompileFunction(f,60,monitor);if(r.decompileCompleted())Files.writeString(out.resolve(String.format("%08x.c",entry)),r.getDecompiledFunction().getC());}
  d.dispose();
 }
}
