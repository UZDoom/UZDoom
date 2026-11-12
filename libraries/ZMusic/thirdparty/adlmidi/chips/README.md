# OPL2/OPL3 Chipset for C++

A bundle of various OPL2 and OPL3 emulators wrapped as unified C++ interface
with the polymorphism and resampling.

Primarily used with the libADLMIDI and OPL3 Bank Editor projects.

This module supports C++98 and higher. However, the use of YMFM emualtors
requires the support for C++14 standard by compilers.

# How to use
- Include the desired emulator's header (or multiple) into your project (for example "nuked_opl3.h")
- Have the pointer of type `OPLChipBase` that will hold the initialized emulator's instance.
- Use emulator through the API at the `OPLChipBase` pointer:
  - Set the output sample rate.
  - Set properties like enable or disable simulated full-panning stereo.
  - Write data to register addresses.
  - Generate the output using one of available methods: single sample, or stream.
- Delete chip by deleting the heap object via previous pointer. It's fine to use smart pointers.
- You can use different chip emulators through the same interface.
- Among emulators, you will find various interfaces over hardware outputs, they require additional setup like hardware address or COM port setup.

# License
- The API itself is licensed under LGPLv2.1+.
- Included chip emulators has various licenses:
  - **GPLv2+**:
    - DosBox OPL3 is licensed under GPLv2+.
    - LLE-OPL2 and LLE-OPL3 has the GPLv2+.
    - MAME OPL2 is licensed under GPLv2+.
  - **LGPLv2.1+**:
    - ESFMu is licensed under LGPLv2.1+.
    - Java OPL3 is licensed under LGPLv2.1+.
    - Nuked OPL3 emulator has LGPLv2.1+.
    - Nuked CQM emulator has LGPLv2.1+
  - **Permissive**:
    - Opal OPL3 is public domain.
    - YMFM emulators has BSD-3-Clause license.
