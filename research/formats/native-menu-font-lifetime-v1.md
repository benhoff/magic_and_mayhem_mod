# Native menu-font lifetime metadata, version1

`MNM_FONT_DC_LIFETIME=1` enables passive native Qt menu-font records on stderr.
Each bounded line is `MNM_QTFONT01 ` followed by a compact JSON object. These are
raw observations retained with the acquisition Qt log in SQLite, not a second
report store. Disabled execution emits no records and has no metadata consumer.

Every object has schema1, canonical positive decimal-string sequence and
host_ns, native player pid, GUI-thread boolean and kind. host_ns uses native
`steady_clock` nanoseconds, independently checked against host monotonic bounds
in controls. A maximum4096records and512distinct captions per root Show bound
output. Overflow is a failed proof. Generation/owner IDs are opaque monotonic
identities, never host addresses. Registration IDs remain local Qt identifiers.

The event grammar is create, four register records, zero or more reuse and
attach/paint/detach, four successful unregister records after all generation
owners detach, and destroy while QApplication is alive. Each catalog carries
its installation root and a SHA256of the concatenated four binary TTF digests.
Roles0..3name Heading800,Body800,Tooltip and yellow SFT inputs, respectively.
Fallback is explicit; installed acceptance refuses it. Component tests separately
allow the intentional all-files-absent fallback fixture.

Paint records observe only visible QLabel/QAbstractButton descendants owned by
that root. They include bounded caption text, widget/class, font role, selected
and resolved family, validity, size, actual Qt glyph map and global rectangle.
Deduplication includes geometry and resets on root Show. A Paint event selects
no font and returns false from the existing event filter. It is metadata about
the actual native UI path, not proof of original UI pixels or World text.

Installed analysis requires the tested player PID and staged asset root, a
single immutable content catalog matching current source-bound component tests,
peakfour registrations, observed reuse and complete retirement. Each required
caption joins a retained accessible widget name/ID/rectangle and screenshot hash.
Its paint must precede that screenshot and follow the preceding screenshot (or
player route start), within30seconds. Initial/restored Main headings, both Quick
body captions and both Spellbox tooltip-role buttons are required separately.
Yellow may be registered without a visible yellow caption; no draw credit follows.
Native outlines and Qt antialiasing remain an intentional presentation policy.
