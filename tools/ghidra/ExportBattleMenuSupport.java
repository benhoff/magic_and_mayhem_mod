// Read-only, pinned setup/map dispatch exports. Definitions exist only in discarded session.
// @category MagicMayhem
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import java.nio.file.*;
public class ExportBattleMenuSupport extends GhidraScript {
 public void run() throws Exception {
  if (!"40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168".equals(currentProgram.getExecutableSHA256())) throw new IllegalStateException("Unsupported build");
  Path out=Paths.get(getScriptArgs()[0]);Path c=Files.createDirectory(out.resolve("c"));
  for(long table:new long[]{0x5c6534L,0x5c6564L,0x5c6940L}) for(int i=0;i<14;i++) {
   long entry=Integer.toUnsignedLong(getInt(toAddr(table+4*i)));
   if(entry>=0x400000L&&entry<0x5a0000L&&getFunctionAt(toAddr(entry))==null){disassemble(toAddr(entry));createFunction(toAddr(entry),null);}
  }
  for(long entry:new long[]{0x4ad5a0L,0x4ad6a0L,0x4ad6f0L,0x4ad860L,0x4ae670L,0x4bbbf0L,0x4bbca0L,0x4bbd80L,0x4b7b40L,0x4b8090L,0x4cde80L,0x4cdf40L,0x4d1df0L,0x4d1ed0L,0x4d1060L,0x4d19f0L,0x4d1360L,0x4d1e50L}) if(getFunctionAt(toAddr(entry))==null){disassemble(toAddr(entry));createFunction(toAddr(entry),null);}
  DecompInterface d=new DecompInterface();d.openProgram(currentProgram);
  try {
   FunctionIterator functions=currentProgram.getFunctionManager().getFunctions(true);
   while(functions.hasNext()) {
    Function f=functions.next();long va=f.getEntryPoint().getOffset();
    if(!((va>=0x4abf20L&&va<0x4af400L)||(va>=0x4bb2d0L&&va<0x4bbef0L)||va==0x4b7b40L||va==0x4b8090L||(va>=0x4cda00L&&va<0x4ce000L)||(va>=0x4d1000L&&va<0x4d2400L)))continue;
    var result=d.decompileFunction(f,60,monitor);if(!result.decompileCompleted())throw new IllegalStateException("Failed "+f.getEntryPoint());
    Files.writeString(c.resolve(f.getEntryPoint()+".c"),result.getDecompiledFunction().getC());
   }
  } finally {d.dispose();}
 }
}
