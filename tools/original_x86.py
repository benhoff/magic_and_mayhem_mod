"""Read-only PE32 and entry-aligned x86 decoding using installed Capstone 5.

Recursive decoding is distinct from a linear sweep through embedded switch data.
An unresolved indirect edge remains unresolved; potential tables are candidates.
"""
import ctypes as C
import ctypes.util
import hashlib
import struct


class Instruction(C.Structure):
    _fields_ = [('id', C.c_uint), ('address', C.c_uint64), ('size', C.c_uint16),
                ('raw', C.c_ubyte * 24), ('mnemonic', C.c_char * 32),
                ('operands', C.c_char * 160), ('detail', C.c_void_p)]


class Decoder:
    def __init__(self):
        self.library = C.CDLL(ctypes.util.find_library('capstone'))
        self.library.cs_version.argtypes = [C.POINTER(C.c_int), C.POINTER(C.c_int)]
        major, minor = C.c_int(), C.c_int()
        self.library.cs_version(C.byref(major), C.byref(minor))
        if major.value != 5:
            raise ValueError('Require the reviewed Capstone 5 instruction ABI')
        self.version = [major.value, minor.value]
        self.library.cs_open.argtypes = [C.c_int, C.c_int, C.POINTER(C.c_size_t)]
        self.library.cs_close.argtypes = [C.POINTER(C.c_size_t)]
        self.library.cs_disasm.argtypes = [C.c_size_t, C.c_void_p, C.c_size_t,
            C.c_uint64, C.c_size_t, C.POINTER(C.POINTER(Instruction))]
        self.library.cs_disasm.restype = C.c_size_t
        self.library.cs_free.argtypes = [C.POINTER(Instruction), C.c_size_t]
        self.handle = C.c_size_t()
        if self.library.cs_open(3, 4, C.byref(self.handle)):
            raise ValueError('Cannot open x86-32 decoder')

    def close(self):
        if self.handle.value:
            self.library.cs_close(C.byref(self.handle))

    def decode(self, raw, address):
        pointer = C.POINTER(Instruction)()
        count = self.library.cs_disasm(self.handle, raw, len(raw), address, 1,
                                       C.byref(pointer))
        if not count:
            raise ValueError('Undecodable instruction at ' + hex(address))
        try:
            instruction = pointer[0]
            if instruction.address != address or not 1 <= instruction.size <= 15:
                raise ValueError('Decoder ABI or instruction extent disagreement')
            return dict(address=address, size=instruction.size,
                bytes=bytes(instruction.raw[:instruction.size]).hex(),
                mnemonic=instruction.mnemonic.decode(),
                operand=instruction.operands.decode())
        finally:
            self.library.cs_free(pointer, count)


class PE32:
    def __init__(self, raw):
        self.raw = raw
        self.sha256 = hashlib.sha256(raw).hexdigest()
        if len(raw) < 256 or raw[:2] != b'MZ':
            raise ValueError('Not a PE image')
        pe = self.u32(60)
        if self.bytes(pe, 4) != b'PE\0\0' or self.u16(pe + 4) != 0x14c:
            raise ValueError('Require PE32 x86')
        optional = pe + 24
        if self.u16(optional) != 0x10b:
            raise ValueError('Require PE32 optional header')
        self.base = self.u32(optional + 28)
        self.entry = self.base + self.u32(optional + 16)
        self.length = self.u32(optional + 56)
        self.sections = []
        table = optional + self.u16(pe + 20)
        for i in range(self.u16(pe + 6)):
            at = table + i * 40
            virtual, rva, stored, offset = struct.unpack('<4I', self.bytes(at + 8, 16))
            self.bytes(offset, stored)
            if rva + max(virtual, stored) > self.length:
                raise ValueError('Section outside image')
            self.sections.append(dict(name=self.bytes(at, 8).split(b'\0')[0].decode(),
                address=self.base + rva, virtual=virtual, stored=stored, offset=offset,
                executable=bool(self.u32(at + 36) & 0x20000000)))
        self.import_directory = self.base + self.u32(optional + 104)
        self.import_size = self.u32(optional + 108)

    def bytes(self, at, size):
        if not 0 <= at <= len(self.raw) or not 0 <= size <= len(self.raw) - at:
            raise ValueError('File range outside PE')
        return self.raw[at:at + size]

    def u16(self, at):
        return struct.unpack('<H', self.bytes(at, 2))[0]

    def u32(self, at):
        return struct.unpack('<I', self.bytes(at, 4))[0]

    def section(self, address):
        return next((s for s in self.sections if s['address'] <= address <
            s['address'] + max(s['stored'], s['virtual'])), None)

    def read(self, address, size, zero_fill=False):
        section = self.section(address)
        if section is None or address + size > section['address'] + max(section['stored'], section['virtual']):
            raise ValueError('Address outside section: ' + hex(address))
        delta = address - section['address']
        available = max(0, min(size, section['stored'] - delta))
        if available != size and not zero_fill:
            raise ValueError('Address is not file backed: ' + hex(address))
        stored = self.bytes(section['offset'] + delta, available) if available else b''
        return stored + bytes(size - available)

    def word(self, address):
        return struct.unpack('<I', self.read(address, 4))[0]

    def text(self, address):
        return bool((self.section(address) or {}).get('executable'))

    def string(self, address, limit=256):
        result = bytearray()
        for i in range(limit):
            value = self.read(address + i, 1)[0]
            if not value:
                return result.decode('ascii')
            result.append(value)
        raise ValueError('Unterminated image string')

    def imports(self):
        rows = []
        if not self.import_size:
            return rows
        for i in range(self.import_size // 20):
            lookup, _, _, name, iat = struct.unpack('<5I', self.read(self.import_directory + i * 20, 20))
            if not any((lookup, name, iat)):
                return rows
            dll = self.string(self.base + name)
            for j in range(4096):
                value = self.word(self.base + (lookup or iat) + j * 4)
                if not value:
                    break
                symbol = '#' + str(value & 0xffff) if value & 0x80000000 else self.string(self.base + value + 2)
                rows.append(dict(dll=dll, symbol=symbol, address=self.base + iat + j * 4))
            else:
                raise ValueError('Unterminated import table')
        raise ValueError('Unterminated import directory')
