// Read-only re-export of newly recovered owner planner function boundaries.
// @category MagicMayhem
import ghidra.app.script.GhidraScript;
public class ExportOwnerPlannerInventory extends GhidraScript {
 public void run() throws Exception {
  if(!"40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168".equals(currentProgram.getExecutableSHA256()))throw new IllegalStateException("Unsupported executable");
  for(long value:new long[]{0x53d6f0,0x415640,0x415670,0x419900,0x4261b0,0x431be0,0x431ce0,0x432fc0,0x4330c0,0x4379d0,0x438a00,0x439e80,0x43a000,0x43b290,0x43b4c0,0x43c3c0,0x43c600}) {
   var entry=toAddr(value);if(getFunctionAt(entry)==null){disassemble(entry);createFunction(entry,null);}
   if(getFunctionAt(entry)==null)throw new IllegalStateException("Missing planner function "+entry);
  }
  runScript("ExportBinaryInventory.java",getScriptArgs());
 }
}
