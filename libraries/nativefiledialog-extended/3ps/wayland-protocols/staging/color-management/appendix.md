---
SPDX-FileCopyrightText: 2025 Collabora, Ltd.
SPDX-License-Identifier: MIT
---

Contents

[TOC]

# Wayland Color-Management Protocol

This document is an appendix to the [color-management][cm-spec]
protocol specification.

## Transfer functions (normative)

`wp_color_manager_v1` enumeration `transfer_function` lists a selection
of transfer functions describing display-referred image encoding between
normalized [electrical] $`E`$ and [optical] $`L`$ values. The function
definitions are as follows.

$`L`$
: Screen luminance in cd/m² in the assumed viewing environment.
Since the protocol uses the reference luminance level as a proxy for the
environment, these values must be interpreted in the context of the
reference luminance level.

$`L_W`$
: Screen luminance for peak white; the primary color volume maximum luminance.

$`L_B`$
: Screen luminance for black; the primary color volume minimum luminance.


### `bt1886`

```math
L = a\left(\max\left(E + b,0\right)\right)^\gamma
```

where $`\gamma = 2.4`$ and the parameters

```math
a = \left(L_W^{1/\gamma} - L_B^{1/\gamma}\right)^\gamma,
```

```math
b = \frac{L_B^{1/\gamma}}{L_W^{1/\gamma} - L_B^{1/\gamma}}.
```

The above are specified by [ITU-R BT.1886].

Note, that $`E < 0`$ and $`E > 1`$ are possible with limited range
quantization, as required by e.g. the calibration method in [ITU-R BT.814].

### `compound_power_2_4`

```math
O = \begin{cases}
\frac{E}{12.92}, & 0 \leq E < 0.04045\\
\left( \frac{E + 0.055}{1.055} \right)^{2.4}, & 0.04045 \leq E \leq 1
\end{cases}
```

The above is the IEC 61966-2-1 piece-wise transfer function,
as recorded in [Khronos Data Format Specification][KDFS] 1.4.0
Section 13.3, and restricted to the unit range.

```math
L = (L_W - L_B)O + L_B
```

### `gamma22`

```math
O = E^{2.2}, \quad 0 \leq E \leq 1
```

```math
L = (L_W - L_B)O + L_B
```

### `gamma28`

```math
O = E^{2.8}, \quad 0 \leq E \leq 1
```

```math
L = (L_W - L_B)O + L_B
```

### `ext_linear`

```math
O = E, \quad \forall E \in \Reals
```

```math
L = (L_W - L_B)O + L_B
```

### `st2084_pq`

```math
\begin{align*}
m_1 &=& \vphantom{\Bigg(} \frac{2610}{16384} &= 0.1593017578125\\
m_2 &=& \vphantom{\Bigg(} 128\left(\frac{2523}{4096}\right) &= 78.84375\\
c_1 = c_3 − c_2 + 1 &=& \vphantom{\Bigg(} \frac{3424}{4096} &= 0.8359375\\
c_2 &=& \vphantom{\Bigg(} 32\left(\frac{2413}{4096}\right) &= 18.8515625\\
c_3 &=& \vphantom{\Bigg(} 32\left(\frac{2392}{4096}\right) &= 18.6875
\end{align*}
```

```math
O = \left(
  \frac{\max\left[
    \left(E^\frac{1}{m_2} - c_1\right), 0
  \right]}{c_2 - c_3 E^\frac{1}{m_2}}
\right)^\frac{1}{m_1},
\quad 0 \leq E \leq 1
```

And the inverse

```math
E = \left( \frac{c_1 + c_2 O^{m_1}}{1 + c_3 O^{m_1}} \right)^{m_2},
\quad 0 \leq O \leq 1
```

The above are specified by [ITU-R BT.2100].

```math
L = 10'000\ \mathrm{cd/m²} \cdot O + L_B
```


[cm-spec]: https://gitlab.freedesktop.org/wayland/wayland-protocols/-/tree/main/staging/color-management
[electrical]: https://gitlab.freedesktop.org/pq/color-and-hdr/-/blob/main/doc/glossary.md#electrical-color-value
[optical]: https://gitlab.freedesktop.org/pq/color-and-hdr/-/blob/main/doc/glossary.md#optical-color-value
[ITU-R BT.814]: https://gitlab.freedesktop.org/pq/color-and-hdr/-/blob/main/doc/specs.md#itu-r-bt814
[ITU-R BT.1886]: https://gitlab.freedesktop.org/pq/color-and-hdr/-/blob/main/doc/specs.md#itu-r-bt1886
[ITU-R BT.2100]: https://gitlab.freedesktop.org/pq/color-and-hdr/-/blob/main/doc/specs.md#itu-r-bt2100
[KDFS]: https://registry.khronos.org/DataFormat/
