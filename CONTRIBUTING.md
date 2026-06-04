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

When contributing text strings, you will need to use Weblate.
- Engine text: https://hosted.weblate.org/engage/uzdoom/
- IWAD text: https://hosted.weblate.org/engage/doom-engine-games/

## Art Contributions

By submitting a pull request, you agree to follow these guidelines.

### Licensing

- **Creative Commons Compatibility**:
  All new contributions must be licensed under a Creative Commons license.
  This can include CC0 (public domain) work.
- **Ownership**:
  You certify that you created the asset yourself or have the legal right
  to contribute it under your chosen license.
- **Attribution**:
  All contributions must have an easily followed lineage.
  When editing files, no previous attribution will be removed.
- **Noncommercial Assets**:
  Assets bound by a noncommercial license will be placed in wadsrc_extra.
- **Derived Assets**:
  Derived assets must retain their original license and must only be used
  in the games from which they are originally derived from.
  Derived assets will be placed in wadsrc_extra.

### AI Generated Art

No AI generated art of any kind may be submitted. This includes but is not limited to:
- Art created by generative AI.
- Modifications or tracings of art created by generative AI.
- Art modified by generative AI (e.g. filters).

Art generated and modified through traditional algorithmic means is allowed. Asking for clarification is best if unsure. Any undisclosed use of generative AI will result in an organization-wide ban.

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

No AI generated code of any kind may be submitted. This includes but is not limited to:
- Code created by generative AI.
- Modifications of code created by generative AI.
- Code modified by generative AI (e.g. having it fix bugs in your code).
- Code created with generative AI assistance (i.e. having the generative AI tell you what code to create).
- Comments created with the assistance of generative AI.

Code generated and modified through traditional algorithmic means is allowed. Asking for clarification is best if unsure. Any undisclosed use of generative AI will result in an organization-wide ban.

When creating commit and pull request messages, authors are expected to use their own words and not generative AI. You may be asked to explain your contributions, and failure to do so will result in rejection.

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
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
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
