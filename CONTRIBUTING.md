# 🌈 Contributors Guide

This document outlines the requirements for contributing to UZDoom.

## General Guidelines

In order to keep a healthy community, please follow these basic points:
- **Language**:
  English is used as a common language in the project.
- **Respect**:
  Remember that we are all people. Give the benefit of the doubt,
  as text-based communication is prone to misunderstanding.
- **Inclusivity**:
  Do not use slurs.
  Do not discriminate against people based on their identity.
- **Constructive Feedback**:
  When reviewing code or discussing features, focus on the work rather
  than the person.

## Issue Reporting

When reporting an issue to UZDoom, please keep the following in mind:
- **Scope**:
  Keep separate issues in separate issue reports.
- **Research**:
  Check that the issue has not been reported already.
- **Verify**:
  Test that the issue has not been fixed in a development build.
- **Reproducibility**:
  Ensure issues are easily reproducible with clear steps.
- **Context**:
  Ensure all relevant information is included.
  (OS, hardware, logs, or specific WADs used)

## Feature Requests

When requesting an idea to UZDoom, please keep the following in mind:
- **Research**:
  Check that the feature has not already been suggested or implemented.
- **Utility**:
  Give a concise use-case.
  (How does this benefit the player or modder?)
- **Expectations**:
  Remember that this is a passion project produced for free.
  Features are implemented based on developer interest and availability.

## Pull Requests

We favour contributions that align with our core values:
- **Accessibility**:
  Ensuring doom is accessible to all.
- **Agency**:
  Keeping control in the player's hand.
- **Stability**:
  Maintaining backwards compatibility.

## Art Contributions

By submitting a pull request, you agree to follow these guidelines.

### Licensing

- **Creative Commons Compatibility**:
  All new contributions must be licensed under a Creative Commons license.
  This can include CC0 (public domain) work.
- **Ownership**:
  You certify that you created the asset yourself or have the legal right
  to contribute it under the your chosen license.
- **Attribution**:
  All contributions must have an easily followed lineage.
  When editing files, no previous attribution will be removed.
- **Noncommercial Assets**:
  Assets bound by a noncommercial license, will be placed in wadsrc_extra.
- **Derived Assets**:
  Derived assets must retain their original license and must only be used
  in the games from which they are originally derived from.
  Derived assets will be placed in wadsrc_extra.

### AI Generated Art

Absolutely no AI generated art may be submitted to this project.

## Code Contributions

By submitting a pull request, you agree to follow these guidelines.

### Licensing

UZDoom is licensed under GPLv3 or later.
To ensure the project remains legally sound,
all contributors must adhere to the following:

- **GPL Compatibility**:
  All new contributions must be licensed as GPLv3+.
  If importing third-party code, it must be under a GPL-compatible license.
- **Header Requirement**:
  Every new source file must include the standard GPLv3+ boilerplate
  provided below.
- **Ownership**:
  You certify that you wrote the code yourself or have the legal right to
  contribute it under the GPLv3+ license.
- **Attribution**:
  All contributions must have an easily followed lineage.
  When editing files, no previous attribution will be removed.
- **Comment History**:
  This is an old project with many old jokes and notes.
  An effort will be made to preserve these, even if they are not required.

### AI Generated Code

As LLMs are trained on unknown scraped data, no code generated entirely
by an LLM may be submitted. AI may be used as a brainstorming or
debugging aid, but all submitted code must be fundamentally authored
and understood by the contributor to ensure it meets our quality and
licensing standards. Code generated through a traditional mechanical
processes is not considered AI generated.

### Testing

Code quality is maintained through verification.
No pull request will be merged unless:

- **Compilation**:
  The code builds without errors on all supported platforms.
  No significant new warnings will be emitted during compilation.
  Silencing warnings through compiler directives is often not a valid fix.
- **Verification**:
  You have tested the changes in-game to ensure they function as intended.
  Your changes should not break existing features or engine stability.

### Source File Template

All new source files should (but are not required to) follow this structure
to maintain consistency and self-documentation.

```cpp
/*
** filename.ext
**
** A short description of what this file does
**
**---------------------------------------------------------------------------
**
** Copyright 2025 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**---------------------------------------------------------------------------
**
** An optional, longer description, covering whatever else you need to
** describe in order for people to contribute to this file. This should
** contain more of the "why" of the code, and less of the "what"
*/

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//
//
//==========================================================================
```
