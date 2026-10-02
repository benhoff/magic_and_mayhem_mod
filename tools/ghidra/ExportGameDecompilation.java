// Export inferred C for selected no-CD routines, or all discovered functions.
// Run after headless auto-analysis; outputs go to a fresh evidence directory.
// @category MagicMayhem

import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.ArrayList;
import java.util.List;

public class ExportGameDecompilation extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length != 2) {
            throw new IllegalArgumentException("Expected output directory and routes/all");
        }
        Path output = Paths.get(args[0]);
        List<Function> functions = new ArrayList<>();
        if (args[1].equals("all")) {
            FunctionIterator iterator = currentProgram.getFunctionManager().getFunctions(true);
            while (iterator.hasNext()) {
                Function function = iterator.next();
                if (!function.isExternal()) functions.add(function);
            }
        } else if (args[1].equals("routes")) {
            long[] entries = {0x512800L, 0x54b800L, 0x4ebae0L, 0x4ec780L,
                              0x40e290L, 0x40e8a0L, 0x40eeb0L};
            for (long entry : entries) {
                Function function = getFunctionAt(toAddr(entry));
                if (function == null) {
                    throw new IllegalStateException("No function at " + Long.toHexString(entry));
                }
                functions.add(function);
            }
        } else {
            throw new IllegalArgumentException("Unknown export mode");
        }
        Path cDirectory = Files.createDirectory(output.resolve("c"));
        DecompInterface decompiler = new DecompInterface();
        int succeeded = 0;
        int failed = 0;
        StringBuilder index = new StringBuilder("address\tname\tstatus\tfile\n");
        try {
            if (!decompiler.openProgram(currentProgram)) {
                throw new IllegalStateException(decompiler.getLastMessage());
            }
            for (Function function : functions) {
                monitor.checkCancelled();
                String address = function.getEntryPoint().toString();
                String filename = address + ".c";
                DecompileResults result = decompiler.decompileFunction(function, 60, monitor);
                boolean ok = result.decompileCompleted() && result.getDecompiledFunction() != null;
                String code = ok ? result.getDecompiledFunction().getC()
                                 : "/* DECOMPILATION FAILED */\n";
                Files.writeString(cDirectory.resolve(filename),
                    "/* Ghidra inferred pseudocode; not original or buildable source.\n" +
                    " * Entry VA: " + address + "; see manifest.json for executable hash. */\n" + code,
                    StandardCharsets.UTF_8, StandardOpenOption.CREATE_NEW);
                index.append(address).append('\t').append(function.getName()).append('\t')
                     .append(ok ? "ok" : "failed").append('\t').append("c/")
                     .append(filename).append('\n');
                if (ok) succeeded++;
                else {
                    failed++;
                    printerr(address + ": " + result.getErrorMessage());
                }
            }
        } finally {
            decompiler.dispose();
        }
        Files.writeString(output.resolve("functions.tsv"), index.toString(),
                          StandardCharsets.UTF_8, StandardOpenOption.CREATE_NEW);
        Files.writeString(output.resolve("export-summary.json"),
                          "{\"succeeded\":" + succeeded + ",\"failed\":" + failed + "}\n",
                          StandardCharsets.UTF_8, StandardOpenOption.CREATE_NEW);
        println("Exported " + succeeded + " functions; " + failed + " failures.");
    }
}
