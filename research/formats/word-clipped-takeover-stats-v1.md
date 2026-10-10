# Clipped takeover diagnostics v1

Mode4 retains common64-byte MNMWRD01 records. Field4 is4, field8 counts actual
native main-plane commits, field9 original tail forwards, field10 zero(original
shadow comparisons never occur).

80-byte little-endian MNMWTK01 follows the MNMWCL01 field layout with changed
semantics: field6 native commits,7 clipped,8 interior,9 hidden,10 original
forwards,11 refusals,12 postcommit consistency errors,13 capture errors,14
retained samples,15 stopped,16 empty forwards,17 clipped samples,18 zero,19
reserved. Fields0..3 are magic/version1/length80; field4 installed,5 seen.

128-byte MNMWAT01 follows MNMWAD01 admission fields4..21. Fields15..20 now
count accepted native auxiliary/scalar/forward/scalar-clipped/forward-clipped/
auxiliary-clipped requests.22 counts unmasked or reserved-precision refusals;23
occupied next physical x87 push-slot refusals.24/25/26 count native admissions
with24/53/64-bit precision;27..31 reserved zero. Prefix version1/length128.

Samples retain MNMWRC01 version1 with mode4. Before/after are actual original
engine-canvas snapshots surrounding a native commit. Original equivalence
requires independent replay; these counters alone do not establish it.
