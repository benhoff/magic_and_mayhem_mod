"""Independent expected presentation for the documented native RGB565 policy.

Original draw comparisons still use unchanged 16-bit words. RGBA expansion is
a separate native presentation check, as documented in surface-dib-ddraw.md.
"""


def rgb565_rgba(pixels):
    output = bytearray()
    for word in pixels:
        red, green, blue = (word >> 11) & 31, (word >> 5) & 63, word & 31
        output.extend(((red << 3) | (red >> 2), (green << 2) | (green >> 4),
                       (blue << 3) | (blue >> 2), 255))
    return bytes(output)
