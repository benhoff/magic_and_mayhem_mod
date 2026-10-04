// Export menu routines and inbound references from a previously analyzed image.
// Function definitions are ephemeral in the required read-only session. No hooks.
// @category MagicMayhem
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.symbol.Reference;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;

public class ExportMenuObservation extends GhidraScript {
    public void run() throws Exception {
        if (!"40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168".equals(currentProgram.getExecutableSHA256()))
            throw new IllegalStateException("Unsupported executable hash");
        Path output = Paths.get(getScriptArgs()[0]);
        Path c = Files.createDirectory(output.resolve("c"));
        // Original analysis missed several callback targets. Define them only in
        // this read-only session, from pinned vtable/callback references.
        long[] entries = {0x4a6b70L,0x4a6e20L,0x4a6f60L,0x4a6e40L,0x4a7110L,
                          0x4a7570L,0x4a75c0L,0x4a7a80L,0x4a7b80L,0x4a7c90L,
                          0x4a7bc0L,0x4a7e40L,0x4a8240L,0x4a83a0L,0x4a7850L,0x557040L};
        for (long entry : entries) {
            if (getFunctionAt(toAddr(entry)) == null) {
                disassemble(toAddr(entry));
                if (createFunction(toAddr(entry), null) == null)
                    throw new IllegalStateException("Cannot define pinned callback " + Long.toHexString(entry));
            }
        }
        StringBuilder index = new StringBuilder("entry\tend_inclusive\tname\tstatus\n");
        StringBuilder refs = new StringBuilder("target\tfrom\towner\ttype\n");
        DecompInterface decompiler = new DecompInterface();
        try {
            if (!decompiler.openProgram(currentProgram)) throw new IllegalStateException(decompiler.getLastMessage());
            FunctionIterator functions = currentProgram.getFunctionManager().getFunctions(true);
            while (functions.hasNext()) {
                monitor.checkCancelled();
                Function f = functions.next();
                long va = f.getEntryPoint().getOffset();
                if (!((va >= 0x4a6900L && va < 0x4a8800L) || (va >= 0x4b2100L && va < 0x4b2d00L)
                       || (va >= 0x557000L && va < 0x557900L) || va == 0x5595d0L)) continue;
                DecompileResults result = decompiler.decompileFunction(f, 60, monitor);
                boolean ok = result.decompileCompleted() && result.getDecompiledFunction() != null;
                Files.writeString(c.resolve(f.getEntryPoint()+".c"),
                    "/* Inferred Ghidra pseudocode, not original source. Pinned No-CD build. */\n" +
                    (ok ? result.getDecompiledFunction().getC() : "/* FAILED */\n"), StandardOpenOption.CREATE_NEW);
                index.append(f.getEntryPoint()).append('\t').append(f.getBody().getMaxAddress()).append('\t')
                     .append(f.getName()).append('\t').append(ok ? "ok" : "failed").append('\n');
                for (Reference ref : getReferencesTo(f.getEntryPoint())) {
                    Function caller = getFunctionContaining(ref.getFromAddress());
                    refs.append(f.getEntryPoint()).append('\t').append(ref.getFromAddress()).append('\t')
                        .append(caller == null ? "data" : caller.getEntryPoint()).append('\t')
                        .append(ref.getReferenceType()).append('\n');
                }
            }
        } finally { decompiler.dispose(); }
        Files.writeString(output.resolve("functions.tsv"), index.toString(), StandardOpenOption.CREATE_NEW);
        Files.writeString(output.resolve("references.tsv"), refs.toString(), StandardOpenOption.CREATE_NEW);
        println("Menu observation export complete");
    }
}
