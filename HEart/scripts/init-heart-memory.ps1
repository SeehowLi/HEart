param(
    [string]$ProjectRoot = ".",
    [string]$ProjectSlug = "",
    [string]$TargetLibrary = "",
    [string]$ProjectSummary = ""
)

$resolvedRoot = Resolve-Path -LiteralPath $ProjectRoot
$memoryRoot = Join-Path $resolvedRoot ".heart-memory"
$sessionsRoot = Join-Path $memoryRoot "sessions"
$today = Get-Date -Format "yyyy-MM-dd"

if ([string]::IsNullOrWhiteSpace($ProjectSlug)) {
    $ProjectSlug = Split-Path -Leaf $resolvedRoot
}

New-Item -ItemType Directory -Force -Path $memoryRoot | Out-Null
New-Item -ItemType Directory -Force -Path $sessionsRoot | Out-Null

$files = @{
    "README.md" = @"
# HEart Delegate Memory

This folder is the project-local delegate memory module for HEart-assisted CKKS/FHE work.

Use it as an append-only working log. It records the project's basic identity, rules, decisions, artifacts, open questions, and session history.

Files:

- `project.md`: project identity, target library, scope, and assumptions.
- `rules.md`: project-specific rules that HEart must obey while working here.
- `sessions/YYYY-MM-DD.md`: session log and handoff notes.
- `decisions.md`: stable design decisions and their rationale.
- `artifacts.md`: files created/modified and why.
- `open-questions.md`: unresolved facts, each marked `[需人工确认]` when applicable.
"@
    "project.md" = @"
# Project

| field | value |
|---|---|
| project_slug | $ProjectSlug |
| target_library | $TargetLibrary |
| summary | $ProjectSummary |
| created | $today |

## Scope

- 

## Inputs / Outputs

- Inputs:
- Outputs:

## Dynamic Range / Precision Assumptions

- [需人工确认]

## Source References

- HEart core:
- HEart library:
"@
    "rules.md" = @"
# Project Rules

These rules apply only to this project. They do not modify HEart core/libs.

## Target

- Target library: $TargetLibrary
- Use HEart core workflow before code: Design Note, scale/level schedule, key list, plaintext validation.

## Project-Specific Rules

- 

## Do Not

- Do not branch on ciphertext data.
- Do not implement CKKS code before Design Note confirmation.
- Do not change HEart core/libs from this project memory.
"@
    "decisions.md" = @"
# Decisions

| date | decision | rationale | sources |
|---|---|---|---|
"@
    "artifacts.md" = @"
# Artifacts

| date | path | action | note |
|---|---|---|---|
"@
    "open-questions.md" = @"
# Open Questions

| date | question | impact | owner/status |
|---|---|---|---|
"@
    (Join-Path "sessions" "$today.md") = @"
# HEart Session Log - $today

## Session

- Project: $ProjectSlug
- Target library: $TargetLibrary
- Task: $ProjectSummary
- Design Note status:
- Files changed:
- Validation:
- Next actions:
"@
}

foreach ($entry in $files.GetEnumerator()) {
    $path = Join-Path $memoryRoot $entry.Key
    if (-not (Test-Path -LiteralPath $path)) {
        Set-Content -LiteralPath $path -Value $entry.Value -Encoding UTF8
    }
}

Write-Output "HEart memory initialized: $memoryRoot"
