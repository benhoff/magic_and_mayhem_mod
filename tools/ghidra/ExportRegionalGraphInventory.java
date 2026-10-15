// Re-export discovery with the verified navigation comparator, without saving the project.
// @category MagicMayhem
import ghidra.app.script.GhidraScript;
public class ExportRegionalGraphInventory extends GhidraScript {
 public void run() throws Exception {
  if(!"40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168".equals(currentProgram.getExecutableSHA256()))
   throw new IllegalStateException("Unsupported executable hash");
  var entry=toAddr(0x53d6f0);
  if(getFunctionAt(entry)==null){disassemble(entry);createFunction(entry,null);}
  var function=getFunctionAt(entry);
  if(function==null||function.getBody().getMaxAddress().getOffset()!=0x53d706)
   throw new IllegalStateException("Unexpected comparator extent");
  runScript("ExportBinaryInventory.java",getScriptArgs());
 }
}
