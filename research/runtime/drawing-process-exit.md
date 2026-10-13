# Original process exit observation

This is native diagnostic policy, not recovered gameplay behavior. The existing
version12menu channel publishes the owner thread in its engine state. An
independent PE32 helper opens that thread with THREAD_QUERY_INFORMATION, obtains
its process ID, and opens that process with SYNCHRONIZE and
PROCESS_QUERY_LIMITED_INFORMATION. It does not inject, read target memory,
suspend or terminate the game. Thread/process IDs are launch-specific.

Microsoft documents [GetProcessIdOfThread](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocessidofthread)
and [GetExitCodeProcess](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getexitcodeprocess).
The implementation first requires wait0timeout258and queried STILL_ACTIVE259,
then retains the actual code only after the process handle signals. A terminated
process may itself return259; signaled wait distinguishes that from alive259.
The bounded wait is600,000msmaximum. Unknown APIs, identity mismatch, already
terminated process, timeout and failed queries refuse. Handles are private.

MNMDPX01has64byteheaderand64byterows of little-endian32bit scalars. Header
words0/1aremagic,2version1,3rowbytes64,4menu thread,5timeoutms,6–15zero.
Rows:0ordinal,1phase(1attached alive,2exit,3refused),2menu thread,3process ID,
4raw wait,5query BOOL,6raw exit code,7tick,8error,9–15zero. Admission requires
exactlyoneattach andoneexit row with stable identities, wait0andquery1. The
header/row extent and reserved words are checked; partial attachment is not exit
proof. Separate launcher codes remain separately recorded.

Capture attaches at the first observed Main state before gameplay. Before Quit,
it records the menu journal offset, verifies launcher liveness, sends the request
and requires original Main callback index4entry on the same owner thread after
that offset. The enabled gate additionally requires the exact API request entry
while observation/physical peer are live. Only an actual child exit0combined
with dispatch sets normal_original_quit_verified. Neither this bounded result
nor the separate reason3gate snapshot proves timer quiescence, import restoration
or complete shutdown/journey coverage. Fresh fixture/original execution pending.

Current evidence: owned fixture01passes actual exits0/37/259. Original16pilot03,
original32retry02and counter-only control02attach to the menu owner process while
alive, retain signaled exit0and actual MainQuit dispatch. These establish bounded
process/dispatch observation in this Wine11.16environment; no full timer/import
quiescence or original shutdown-body equivalence. Confidence: confirmed within
the recorded owned/live scopes; source/contract hashes remain per-result.
