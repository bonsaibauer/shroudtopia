# Native prop placement investigation, 2026-09-18

## Observed in build 28, process 24292

F9 captured one prop. F8 called RVA 3E1120 and returned 1024, then 1025.
Neither operation passed the subsequent template/tracking/position check.
The live object lookup hook at RVA 23A159 exposed manager 3006CB092C0.
ReadProcessMemory enumeration of that manager showed entity 1024 named
3_MotherFlame and 1025 named Scatter_Loot_Greenhillzone_tree_01_fallen_branch_01_a.
Thus those return values must not be logged as verified workbench entity IDs.

## Binary evidence

- 282380 is the normal prop update. Its row is fetched before 282496;
  the input-event filtering begins at 2824B2. The old hook 282965 is inside
  the accepted construction-event path and cannot service idle F8 requests.
- 280790 is a different update function. Build 28 injected the prop request
  into its 280887 hook using that function's execution view.
- Normal placement calls 3E2500, which calls 3E1120 before feedback 3EBB70.
- 3E1120 gets the command writer through 8D0F90 and calls 876FB0.
- 876FB0 forwards to 877050. That function reserves bytes via 875E10,
  allocates a counter value and serializes the template and components.
  The return therefore confirms a queued command, not a materialized entity.
- 3EDC80 clears entries from a five-slot history structure. It must not be
  described or invoked speculatively as a general ECS command-buffer flush.

## Build 29 change and limitations

Move prop creation to 282496 in the native prop update, before event filtering.
Keep voxel snapshot collection at 280887. Resolve the owner from the execution
view using the addressing performed by 8D3490. Do not compare prop context
identity with the independently pinned voxel-world pointer.

This addresses a demonstrated context mismatch; whether it explains the lost
commands remains a live-test hypothesis. Build success is not game validation.
Replacement/removal and undo are not yet verified. Do not claim overall mod
completion or guaranteed rollback when a queued prop has not been accounted for.

Read-only helper: tools/live-hook-record.cpp locates installed E9 hooks and
copies their diagnostic mailbox. tools/live-entity-reader.cpp enumerates the
specified live manager. Neither tool writes process memory.
