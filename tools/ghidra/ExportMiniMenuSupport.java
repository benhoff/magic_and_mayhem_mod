// Read-only, pinned Mini Menu dispatch/lifecycle exports. Definitions exist only in discarded session.
// @category MagicMayhem
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import java.nio.file.*;
public class ExportMiniMenuSupport extends GhidraScript {
 public void run() throws Exception {
  if (!"40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168".equals(currentProgram.getExecutableSHA256())) throw new IllegalStateException("Unsupported build");
  Path out=Paths.get(getScriptArgs()[0]);Path c=Files.createDirectory(out.resolve("c"));
  for(long table:new long[]{0x5c6644L}) for(int i=0;i<14;i++) {
   long entry=Integer.toUnsignedLong(getInt(toAddr(table+4*i)));
   if(entry>=0x400000L&&entry<0x5a0000L&&getFunctionAt(toAddr(entry))==null){disassemble(toAddr(entry));createFunction(toAddr(entry),null);}
  }
  for(long entry:new long[]{0x4b2100L,0x4b23f0L,0x4b24f0L,0x4b25e0L,0x557040L,0x557130L,0x5571e0L,0x5595d0L}) if(getFunctionAt(toAddr(entry))==null){disassemble(toAddr(entry));createFunction(toAddr(entry),null);}
  DecompInterface d=new DecompInterface();d.openProgram(currentProgram);
  try {
   FunctionIterator functions=currentProgram.getFunctionManager().getFunctions(true);
   while(functions.hasNext()) {
    Function f=functions.next();long va=f.getEntryPoint().getOffset();
    if(!((va>=0x4b1d80L&&va<0x4b2730L)||(va>=0x557000L&&va<0x559850L)))continue;
    var result=d.decompileFunction(f,60,monitor);if(!result.decompileCompleted())throw new IllegalStateException("Failed "+f.getEntryPoint());
    Files.writeString(c.resolve(f.getEntryPoint()+".c"),result.getDecompiledFunction().getC());
   }
  } finally {d.dispose();}
 }
}
