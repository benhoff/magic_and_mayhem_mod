// Read-only, pinned setup/map dispatch exports. Definitions exist only in discarded session.
// @category MagicMayhem
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import java.nio.file.*;
public class ExportSpellMenuSupport extends GhidraScript {
 public void run() throws Exception {
  if (!"40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168".equals(currentProgram.getExecutableSHA256())) throw new IllegalStateException("Unsupported build");
  Path out=Paths.get(getScriptArgs()[0]);Path c=Files.createDirectory(out.resolve("c"));
  for(long table:new long[]{0x5c775cL}) for(int i=0;i<14;i++) {
   long entry=Integer.toUnsignedLong(getInt(toAddr(table+4*i)));
   if(entry>=0x400000L&&entry<0x5a0000L&&getFunctionAt(toAddr(entry))==null){disassemble(toAddr(entry));createFunction(toAddr(entry),null);}
  }
  for(long entry:new long[]{0x576380L,0x5765d0L,0x578cd0L,0x579710L,0x579d30L,0x5799c0L,0x590b70L}) if(getFunctionAt(toAddr(entry))==null){disassemble(toAddr(entry));createFunction(toAddr(entry),null);}
  DecompInterface d=new DecompInterface();d.openProgram(currentProgram);
  try {
   FunctionIterator functions=currentProgram.getFunctionManager().getFunctions(true);
   while(functions.hasNext()) {
    Function f=functions.next();long va=f.getEntryPoint().getOffset();
    if(!((va>=0x5751b0L&&va<0x579e00L)||(va>=0x58fc00L&&va<0x591800L)))continue;
    var result=d.decompileFunction(f,60,monitor);if(!result.decompileCompleted())throw new IllegalStateException("Failed "+f.getEntryPoint());
    Files.writeString(c.resolve(f.getEntryPoint()+".c"),result.getDecompiledFunction().getC());
   }
  } finally {d.dispose();}
 }
}
