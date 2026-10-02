// Reproducible, conservative markup for the no-CD route-request milestone.
// Modifies the analysis database only, never executable bytes.
// @category MagicMayhem

import ghidra.app.script.GhidraScript;
import ghidra.program.model.data.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.SourceType;
import java.util.ArrayList;
import java.util.List;

public class AnnotateRouteMilestone extends GhidraScript {
    private static final String HASH =
        "40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168";
    private final CategoryPath category = new CategoryPath("/MagicMayhem/NoCD");

    private DataType bytes(int length) {
        return new ArrayDataType(ByteDataType.dataType, length, 1);
    }

    private DataType pointer(DataType target) {
        return new PointerDataType(target, 4, currentProgram.getDataTypeManager());
    }

    private StructureDataType structure(String name, int size) {
        StructureDataType result = new StructureDataType(category, name, size);
        result.setPackingEnabled(false);
        return result;
    }

    private DataType persist(StructureDataType type) {
        return currentProgram.getDataTypeManager().resolve(type, DataTypeConflictHandler.REPLACE_HANDLER);
    }

    private Parameter parameter(String name, DataType type, int stackOffset) throws Exception {
        return new ParameterImpl(name, type, stackOffset, currentProgram, SourceType.USER_DEFINED);
    }

    private void signature(long address, String name, DataType result, String receiverName,
                           DataType receiver, String[] names, DataType[] types,
                           String evidence) throws Exception {
        Function function = getFunctionAt(toAddr(address));
        if (function == null) throw new IllegalStateException("Missing entry " + Long.toHexString(address));
        function.setName(name, SourceType.USER_DEFINED);
        List<Variable> parameters = new ArrayList<>();
        parameters.add(new ParameterImpl(receiverName, receiver, currentProgram.getRegister("ECX"),
                                        currentProgram, SourceType.USER_DEFINED));
        for (int index = 0; index < names.length; ++index) {
            parameters.add(parameter(names[index], types[index], 4 + index * 4));
        }
        VariableStorage returnStorage = result == VoidDataType.dataType
            ? VariableStorage.VOID_STORAGE
            : new VariableStorage(currentProgram, currentProgram.getRegister("EAX"));
        function.updateFunction("__thiscall",
            new ReturnParameterImpl(result, returnStorage, currentProgram), parameters,
            Function.FunctionUpdateType.CUSTOM_STORAGE, true, SourceType.USER_DEFINED);
        function.setStackPurgeSize(names.length * 4);
        // These tiny constructors write directly into caller-reserved stack slots.
        // Inlining analysis exposes that memory effect; no executable is patched.
        if (address == 0x40e290L || address == 0x40e8a0L || address == 0x40eeb0L) {
            function.setInline(true);
        }
        function.setComment(evidence + "\nNames are analyst assigned; see reconstruction/pathfinding/README.md.");
    }

    private void global(long address, String name, DataType type) throws Exception {
        clearListing(toAddr(address), toAddr(address + type.getLength() - 1));
        createData(toAddr(address), type);
        createLabel(toAddr(address), name, true);
    }

    @Override
    public void run() throws Exception {
        if (!HASH.equalsIgnoreCase(currentProgram.getExecutableSHA256()) ||
            currentProgram.getDefaultPointerSize() != 4 ||
            currentProgram.getImageBase().getOffset() != 0x400000L) {
            throw new IllegalArgumentException("Expected exact no-CD PE32 at image base 0x00400000");
        }
        // All sizes/offsets remain explicit. These are partial byte-layout prefixes.
        StructureDataType snapshot = structure("RouteSnapshotPrefix", 0x20c);
        snapshot.replaceAtOffset(0, IntegerDataType.dataType, 4, "target_x", "Coordinate label inferred");
        snapshot.replaceAtOffset(4, IntegerDataType.dataType, 4, "target_y", "Coordinate label inferred");
        snapshot.replaceAtOffset(8, IntegerDataType.dataType, 4, "target_z", "Coordinate label inferred");
        snapshot.replaceAtOffset(0xc, UnsignedIntegerDataType.dataType, 4, "unknown_0c", null);
        snapshot.replaceAtOffset(0x10, UnsignedIntegerDataType.dataType, 4, "waypoint_count",
                                 "Wrapper tests nonzero; search caps emitted entries at 16");
        snapshot.replaceAtOffset(0x14, bytes(0x1f8), 0x1f8, "unknown_14", null);
        DataType routeType = persist(snapshot);

        StructureDataType object = structure("ObjectPrefix", 0xd07);
        object.replaceAtOffset(0, bytes(0x96b), 0x96b, "unknown_000", null);
        object.replaceAtOffset(0x96b, routeType, 0x20c, "route", "Copied by 0x00512862");
        object.replaceAtOffset(0xb77, bytes(0x14), 0x14, "unknown_b77", null);
        object.replaceAtOffset(0xb8b, UnsignedIntegerDataType.dataType, 4, "route_present", null);
        object.replaceAtOffset(0xb8f, bytes(0x174), 0x174, "unknown_b8f", null);
        object.replaceAtOffset(0xd03, UnsignedIntegerDataType.dataType, 4, "unknown_d03", "Reset to zero");
        DataType objectType = persist(object);

        StructureDataType context = structure("RouteContextPrefix", 0x269);
        context.replaceAtOffset(0, routeType, 0x20c, "route", null);
        context.replaceAtOffset(0x20c, ByteDataType.dataType, 1, "unknown_route_flag",
                                "Aliases VA 0x00690354; wrapper sets 1 before search");
        context.replaceAtOffset(0x20d, bytes(0x5c), 0x5c, "unknown_20d", "Search state not yet recovered");
        DataType contextType = persist(context);
        DataType i32 = IntegerDataType.dataType;
        DataType u32 = UnsignedIntegerDataType.dataType;

        signature(0x40e290L, "normalize_x_coordinate", pointer(i32), "destination", pointer(i32),
            new String[]{"value"}, new DataType[]{i32}, "High: ECX destination, EAX same pointer, ret 4; positive dimension assumed.");
        signature(0x40e8a0L, "normalize_y_coordinate", pointer(i32), "destination", pointer(i32),
            new String[]{"value"}, new DataType[]{i32}, "High: ECX destination, EAX same pointer, ret 4; positive dimension assumed.");
        signature(0x40eeb0L, "copy_z_coordinate", pointer(i32), "destination", pointer(i32),
            new String[]{"value"}, new DataType[]{i32}, "High: one DWORD store, EAX destination, ret 4; z name inferred.");
        signature(0x512800L, "request_route", u32, "object", pointer(objectType),
            new String[]{"x", "y", "z", "unknown_argument"}, new DataType[]{i32, i32, i32, u32},
            "High: ECX object, four DWORD stack arguments, EAX normalized 0/1, ret 16. Coordinates inferred.");
        signature(0x54b800L, "search_route_candidate", VoidDataType.dataType, "context", pointer(contextType),
            new String[]{"object", "unknown_argument", "x", "y", "z", "remaining_budget"},
            new DataType[]{pointer(objectType), u32, i32, i32, i32, pointer(i32)},
            "Static stack tracing: ECX context, six DWORD stack arguments, ret 24. Search internals unresolved; return value unused.");

        global(0x5e174cL, "max_nodes_for_route_finding", i32);
        global(0x6c5494L, "map_x_dimension_inferred", i32);
        global(0x6c5498L, "map_y_dimension_inferred", i32);
        global(0x690148L, "route_scratch_prefix", contextType);
        createLabel(toAddr(0x690354L), "unknown_route_flag_690354", false);
        println("Applied hash-checked route milestone names, signatures and partial types.");
    }
}
