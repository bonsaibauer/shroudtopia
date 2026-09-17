# Bounded native-world diagnostic, client 2026-09-17

Experimental instrumentation, not a WorldApi provider or a finished world editor.
Default off. No captured engine pointers are dereferenced later or exposed to mods.

`worldDiagnostic: true` in loader configuration requests startup-only installation.
The installer verifies the known full executable SHA256; the loader checks client
name, PE timestamp/image size, original bytes, unique long signatures and absence
of an active world context. Configuration reload cannot arm a hook. Disable the
setting to stop recording; installed jumps remain inert until process shutdown,
avoiding hot unpatching while another thread could execute the trampoline.

Hooks:

- Cursor RVA `0x24aa1d`: common successor of selection-update branches, after the
  transform copies. Copies 160 bytes from R14+0x270 into a fixed mailbox.
- Terrain RVA `0x994810`: captures entry RCX/RDX/R8/R9, caller return address,
  fifth argument and thread ID. Does not dereference those argument pointers.
  A hit does not yet establish a building-block operation or a world commit.
- Building-place event RVA `0x3ebc82` and tear-down event RVA `0x3ebdb2`:
  replay the final owner-ID store first, then copy the complete 72-byte event
  while RBX is valid. Log position, quaternion, volume bounds, owner and tracking
  item. `material_feedback` is a MaterialFeedbackId, not a VoxelMaterialId.
- Voxel-write candidate RVA `0x996f90`: reached from the placement path
  `0x280f86 -> 0x3e2cd0 -> 0x3e337e`. Capture arguments and the first 72 bytes
  of argument 6, which the original reads, plus low bytes of arguments 7..9.
  This is still an unvalidated writer candidate, never invoked by the diagnostic.
  The image base is logged to resolve caller addresses after the process exits.

The ECS event constructors were found by instruction-validated references to
qualified type hashes `0xb6fcf706` and `0x474baeed`. The upstream actor-event
dispatch compares `0xde627227` (PlaceVoxelObjectEvent) at `0x28097b` and checks
its subtract flag. An emitted ECS event is not itself a command API.

The five signatures and displaced instruction boundaries can be verified with
`ShroudEdit/tools/verify-world-diagnostic-sites.py <game exe> <world_engine.cpp> <world_diagnostic.cpp>`.
The initial short writer signature was ambiguous; deployment uses a longer,
verified unique signature. No RIP-relative instructions/branches are displaced.

The payload makes no calls, performs no allocations/I/O, preserves modified GPRs
and RFLAGS, and replays complete overwritten instructions. It writes only its own
mailbox. Reader/writer share a non-blocking lock; busy samples are skipped. Logging
runs on the loader thread. This is a latest-sample trace with cumulative counters,
not a complete ordered event trace. Timestamps are drain times, not hook times.

Recording expires 300 seconds (five minutes) after the first observed sample, or
after ten minutes without any first sample. A late first sample still gets the full
five-minute window. Output is `shroudtopia-world-diagnostic-<pid>-<tick>.jsonl` next to
the game executable. No World service/capability is registered by this experiment.

## Checks performed

`build-world-probe.ps1` executes the payload in an isolated process with owned input:
cursor byte copy, thread ID, entry arguments, RAX/carry/direction preservation,
disabled/busy branches including an invalid input pointer that must not be read.
Additional tests cover both completed ECS events (including replayed owner store)
and the writer candidate's descriptor and stack arguments using owned buffers.
Loader Release build and platform API smoke test pass. Installer and restore were
tested in a fresh fixture. These tests do not establish in-game hook correctness.

## Deployment and test

Close game/server normally. Run `tools/install-world-diagnostic.ps1`; it backs up
and updates only winmm.dll and shroudtopia.json. No assets, mods or saves change.
Restore with `-RestoreBackup <printed backup path>` while the game is closed.

Use a disposable test world. First aim the hammer at ground, a wall and sky. Then
place and remove one ordinary building block. Finally perform one ordinary pickaxe
action for comparison. Report the action order. Inspect the arm/refusal log before
interpreting zero samples. Confirm game stability and distinguish terrain hits from
building operations before advancing to chunk reading or invoking any world writes.
