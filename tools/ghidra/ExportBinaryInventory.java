// Export discovery metadata without treating decompilation as recovered behavior.
// @category MagicMayhem
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.AddressRange;
import ghidra.program.model.address.AddressRangeIterator;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.mem.MemoryBlock;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;

public class ExportBinaryInventory extends GhidraScript {
    private String quote(String value) {
        return "\"" + value.replace("\\", "\\\\").replace("\"", "\\\"")
            .replace("\n", "\\n").replace("\r", "\\r").replace("\t", "\\t") + "\"";
    }
    private String address(ghidra.program.model.address.Address value) {
        return quote("0x" + value.toString());
    }
    @Override public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length != 1) throw new IllegalArgumentException("Expected output JSON path");
        StringBuilder out = new StringBuilder("{\"schema\":1,\"functions\":[");
        FunctionIterator functions = currentProgram.getFunctionManager().getFunctions(true);
        boolean first = true;
        while (functions.hasNext()) {
            monitor.checkCancelled();
            Function function = functions.next();
            if (function.isExternal()) continue;
            if (!first) out.append(','); first = false;
            out.append("{\"entry\":").append(address(function.getEntryPoint()))
                .append(",\"name\":").append(quote(function.getName()))
                .append(",\"thunk\":").append(function.isThunk()).append(",\"ranges\":[");
            AddressRangeIterator ranges = function.getBody().getAddressRanges();
            boolean firstRange = true;
            while (ranges.hasNext()) {
                AddressRange range = ranges.next();
                if (!firstRange) out.append(','); firstRange = false;
                out.append('[').append(address(range.getMinAddress())).append(',')
                    .append(quote("0x" + Long.toHexString(range.getMaxAddress().getOffset() + 1))).append(']');
            }
            out.append("],\"data_references\":[");
            ReferenceIterator refs = currentProgram.getReferenceManager().getReferencesTo(function.getEntryPoint());
            boolean firstRef = true;
            while (refs.hasNext()) {
                Reference ref = refs.next();
                if (!ref.getReferenceType().isData()) continue;
                if (!firstRef) out.append(','); firstRef = false;
                out.append(address(ref.getFromAddress()));
            }
            out.append("]}");
        }
        out.append("],\"flows\":[");
        InstructionIterator instructions = currentProgram.getListing().getInstructions(true);
        first = true;
        while (instructions.hasNext()) {
            monitor.checkCancelled();
            Instruction instruction = instructions.next();
            MemoryBlock block = currentProgram.getMemory().getBlock(instruction.getAddress());
            if (block == null || !block.isExecute()) continue;
            ghidra.program.model.symbol.FlowType flow = instruction.getFlowType();
            if (!flow.isCall() && !flow.isJump()) continue;
            if (!first) out.append(','); first = false;
            Function owner = currentProgram.getFunctionManager().getFunctionContaining(instruction.getAddress());
            out.append("{\"site\":").append(address(instruction.getAddress()))
                .append(",\"owner\":").append(owner == null ? "null" : address(owner.getEntryPoint()))
                .append(",\"kind\":").append(quote(flow.isCall() ? "call" : "jump"))
                .append(",\"computed\":").append(flow.isComputed()).append(",\"targets\":[");
            boolean firstTarget = true;
            for (ghidra.program.model.address.Address target : instruction.getFlows()) {
                if (!firstTarget) out.append(','); firstTarget = false;
                out.append(address(target));
            }
            out.append("],\"references\":["); boolean firstReference = true;
            for (Reference ref : instruction.getReferencesFrom()) {
                if (!firstReference) out.append(','); firstReference = false;
                out.append("{\"target\":").append(address(ref.getToAddress()))
                    .append(",\"type\":").append(quote(ref.getReferenceType().toString())).append('}');
            }
            out.append("],\"instruction\":").append(quote(instruction.toString())).append('}');
        }
        out.append("]}");
        Files.writeString(Path.of(args[0]), out.toString() + "\n", StandardCharsets.UTF_8, StandardOpenOption.CREATE_NEW);
        println("Binary discovery inventory exported; indirect targets remain provisional.");
    }
}
