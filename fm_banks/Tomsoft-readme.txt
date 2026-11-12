Bank was imported by a hacky way from the Tomsoft's SegaMusic program
by TommyXie (Xie Rong Chun):
- the dummy MIDI file was created that contains all 128 instruments in GM order
- the Sega emulator playable BIN file was generated
- the GYM dump was generated from the playback of that dummy instrument
- OPN2 Bank Editor was used to scan GYM file for instruments and import all of
them.

The work woth done by Jean-Pierre Cimalando:
https://github.com/Wohlstand/OPN2BankEditor/issues/44

Then, the bank was tuned by Wohlstand:
- Corrected note offsets to align octaves of all instruments
- Merged with xg.wopn to provide the set of percussions.

License for this bank - MIT

To edit this bank and other banks in WOPL format, you can use this editor
which I created for that: https://github.com/Wohlstand/OPL3BankEditor

==============================================================================
This bank build by Jean-Pierre Cimalando, 2018-2022

==============================================================================
MIT License

Copyright (c) 2018-2022 Jean-Pierre Cimalando

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
