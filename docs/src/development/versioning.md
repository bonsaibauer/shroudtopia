# Versioning and Releases

```text
2.3.5-0041
│ │ │  └──── Build number
│ │ └─────── Patch version
│ └───────── Minor version
└─────────── Major version
```

| Part | Meaning | Increase when |
|---|---|---|
| Major | Incompatible product version | Public ABI, manifest, or core contract breaks |
| Minor | Compatible feature version | A compatible API function, service, or module is added |
| Patch | Compatible correction | Behavior is fixed without adding a public function |
| Build | CI execution number | Each automated build |

## Naming

| Item | Format |
|---|---|
| `VERSION` | `MAJOR.MINOR.PATCH` |
| Branch | `MAJOR.MINOR.PATCH` |
| Release tag | `vMAJOR.MINOR.PATCH` |
| Artifact | `shroudtopia-MAJOR.MINOR.PATCH-BUILD.zip` |

The build number is not part of the Git tag. GitHub Actions uses its run number.
API ABI versions, service versions, and mod versions remain independent.

## Release workflow

1. Set the product version once in `VERSION`.
2. Use the matching branch name.
3. Run `build.ps1` and verify all checks.
4. Tag the verified commit as `v<VERSION>`.
5. Publish the already tested CI artifact.
