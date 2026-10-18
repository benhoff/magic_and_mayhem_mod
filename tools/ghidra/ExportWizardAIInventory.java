// Read-only discovery of wizard spell task entry bodies.
// @category MagicMayhem
import ghidra.app.script.GhidraScript;
public class ExportWizardAIInventory extends GhidraScript {
 public void run() throws Exception {
  if(!"40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168".equals(currentProgram.getExecutableSHA256())) throw new IllegalStateException("Unsupported executable");
  for(long value:new long[]{0x44ae70,0x4466f0,0x45d2b0,0x44f9d0,0x450780,0x44c560,0x4500c0,0x450ee0,0x45bb60}) {
   var entry=toAddr(value);
   if(getFunctionAt(entry)==null){disassemble(entry);createFunction(entry,null);}
   if(getFunctionAt(entry)==null)throw new IllegalStateException("Missing wizard function "+entry);
  }
  runScript("ExportBinaryInventory.java",getScriptArgs());
 }
}
