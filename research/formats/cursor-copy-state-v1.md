# Selected cursor COPY companion v1

`MNMCCS01`: fixed128-byte little-endian DWORD header followed by8192 rows of320 bytes. Zero unused tail is mandatory. This diagnostic stream is never rendering/input state.

Header: words2..5 = version1/header128/row320/capacity8192;6stored rows;7overflow;8attempted selected rows;9closed;10NoCD build40209ca7;11module base;12module extent;13closed COPY stored count;14COPY seen count;15COPY overflow-or-active-miss. Words16..31 are zero.

Rows0..43 exactly repeat a closed v2 COPY row, including its original call ID, for return RVAs18c488 and18cbb2. Remaining words:44tick;45cooperative HWND;46input coordinate mode(0none,1client,2desktop);47input publication sequence before;48active;49..50published point;51..52published extent;53sequence after;54saved-valid DWORD;55..58saved rectangle;59..60saved offsets;61cursor class;62cursor enabled;63deferred mode;64GetWindowRect success/65..68rectangle;69GetClientRect success/70..73rectangle;74native unhooked GetCursorPos success/75..76point;77publication coherent;78all pinned saved globals readable;79zero.

Saved globals are the pinned NoCD state at642c54,641a78..641a84,641aa0..641aa4,5e04c8,642c64 and6e1ee4. A coherent publication has equal even sequences. An incoherent or absent publication stays explicit; active and coherence do not prove lease admission. Window/native-pointer queries occur after COPY while the existing copy lease is held. They do not record the actual earlier hooked GetCursorPos return. No unverified pointer is dereferenced.

A closed strict v2 COPY census is required. Module/count identities and every selected row must match, in order, with no missing/extra rows. Original saved globals, published Qt input and native desktop position retain distinct meanings. Completeness of this companion is scoped to the two selected callers. It grants no pixel comparison, original-driver parity, coordinate-writer identity or whole-player acceptance.
