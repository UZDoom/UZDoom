.. Copyright 2022 Red Hat, Inc.

.. contents::

Rec ITU-T H.273
===============

Rec ITU-T H.273 (short H.273) is a Recommendation from the International
Telecommunication Union (ITU) with the title "Coding-independent code points for
video signal type identification". All published versions can be found at
https://www.itu.int/rec/T-REC-H.273/en.

For a quick introduction to Rec ITU-T H.273 see
https://gitlab.freedesktop.org/pq/color-and-hdr/-/blob/main/doc/cicp_h273.md.

Code point and pixel format compatibility
=========================================

Certain color representation metadata requires the selected code point to be
compatible with the buffer's pixel format. Which code points are compatible with
which pixel formats depends on the type of metadata.

All ``Chroma420SampleLocType`` chroma location code points are compatible with
4:2:0 subsampled pixel formats. Using a pixel format which is not 4:2:0
subsampled in a commit where a ``Chroma420SampleLocType`` code point is set
results in a protocol error. Clients can unset all code points again by
destroying the wp_color_representation_surface_v1, when they switch to such
formats.

The matrix coefficients' code point and pixel format compatibility is harder to
determine and depends on the specific code point.

The ``MatrixCoefficients`` code points are defined by "Rec ITU-T H.273
Coding-independent code points for video signal type identification". This
document further defines equations which describe how a tristimulus value can be
transformed. Which equations can be applied depends on which
``MatrixCoefficients`` code point is selected and if the ``VideoFullRangeFlag``
is set or not. By applying the applicable equations on a tristimulus value one
or more color encodings can be inferred. This color encoding has three channels
and each of those channels must map to the pixel format of the surface's buffer.
In Rec ITU-T H.273 (07/21) those channels are either R, G and B or Y, Cb and Cr.

Equations numbers used in the examples below are taken from Rec ITU-T H.273
(07/21) and might change in future versions.

For example code point 0: equations 11-13 transform the tristimulus values E\
:sub:`R`, E\ :sub:`G`, E\ :sub:`B` to a non-linear encoding E'\ :sub:`R`, E'\
:sub:`G`, E'\ :sub:`B`. Those can be transformed to an RGB encoding with
equations 20-22 (if the ``VideoFullRangeFlag`` is not set) or 26-28 (if the
``VideoFullRangeFlag`` is set). A YCbCr encoding can be inferred from the RGB
encoding with equations 41-43.

Therefore the code point 0 is compatible only with pixel formats which contain
the RGB or YCbCr color channels. The pixel formats may additionally carry unused
bits, alpha or other channels.

For example code point 1: apply equations 11-13, 38-40 and either 23-25 or 29-31
(depending on the ``VideoFullRangeFlag``) to arrive at the YCbCr encoding. An
RGB encoding cannot be inferred from the applicable equations.

Therefore code point 1 is is compatible only with pixel formats which contain
the YCbCr color channels.

MatrixCoefficients usage
========================

Note that the ``MatrixCoefficients`` equations as defined by Rec ITU-T H.273
describe how the client transforms the tristimulus values to an encoding which
ends up in the buffer it sends to the compositor. Compositors will use the
inverse steps, including the transfer characteristics which are not defined by
this protocol to convert the encoding back to tristimulus values with color
primaries which are also not defined by this protocol.

Some ``MatrixCoefficients`` code points require applying formulas or infering
constants from the transfer characteristics or color primaries of the image.
Compositors should not advertise support for such code points if the client
can't communicate the transfer characteristics and color primaries to the
compositor. Communicating those when needed is left for other Wayland extensions
to be used in conjunction with color-representation.