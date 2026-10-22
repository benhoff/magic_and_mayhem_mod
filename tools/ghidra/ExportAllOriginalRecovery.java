// Read-only in-memory discovery/decompilation for all five recovery contracts.
// @category MagicMayhem
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.Function;
import java.nio.file.*;
import java.util.*;
public class ExportAllOriginalRecovery extends GhidraScript {
    private final String PIN="40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168";
    public void run() throws Exception {
        if(!PIN.equals(currentProgram.getExecutableSHA256()))throw new IllegalStateException("Original build differs");
        String[] args=getScriptArgs();Path out=Paths.get(args[0]);Files.createDirectory(out.resolve("c"));
        var selected=new TreeMap<Long,Function>();var log=new StringBuilder();
        for(String line:Files.readAllLines(Paths.get(args[1]))){
            String[] fields=line.split(" ");long value=Long.parseLong(fields[1],16);var address=toAddr(value);
            Function f;
            if(fields[0].equals("caller"))f=getFunctionContaining(address);
            else {
                f=getFunctionAt(address);
                if(f==null){
                    var owner=getFunctionContaining(address);
                    if(owner!=null){log.append("overlap ").append(line).append(" owner ").append(owner.getEntryPoint()).append("\n");continue;}
                    disassemble(address);f=createFunction(address,null);
                }
            }
            if(f==null){log.append("unresolved ").append(line).append("\n");continue;}
            selected.put(f.getEntryPoint().getOffset(),f);
        }
        var functions=currentProgram.getFunctionManager().getFunctions(true);
        while(functions.hasNext()){
            var f=functions.next();long entry=f.getEntryPoint().getOffset();
            if(args.length<3&&((entry>=0x545500L&&entry<0x54ad40L)||(entry>=0x58a500L&&entry<0x58f500L)))selected.put(entry,f);
        }
        var decompiler=new DecompInterface();decompiler.openProgram(currentProgram);
        try{
            int i=0;
            for(var f:selected.values()){
                monitor.checkCancelled();var result=decompiler.decompileFunction(f,45,monitor);
                if(result.decompileCompleted())Files.writeString(out.resolve("c").resolve(f.getEntryPoint()+".c"),result.getDecompiledFunction().getC());
                else log.append("decompile-failed ").append(f.getEntryPoint()).append(" ").append(result.getErrorMessage()).append("\n");
                log.append("function ").append(f.getEntryPoint()).append(" ").append(f.getBody().getNumAddresses()).append(" ").append(result.decompileCompleted()).append("\n");
                println("Recovery export "+(++i)+"/"+selected.size()+" "+f.getEntryPoint());
            }
        }finally{decompiler.dispose();}
        Files.writeString(out.resolve("discovery-notes.txt"),log.toString());
        var refs=new StringBuilder();for(long value:new long[]{0x6e1f68L,0x6574c8L,0x54e170L}){
            var references=currentProgram.getReferenceManager().getReferencesTo(toAddr(value));
            while(references.hasNext())refs.append(Long.toHexString(value)).append(" ").append(references.next()).append("\n");
        }
        Files.writeString(out.resolve("global-references.txt"),refs.toString());
        runScript("ExportBinaryInventory.java",new String[]{out.resolve("discovery.json").toString()});
    }
}
