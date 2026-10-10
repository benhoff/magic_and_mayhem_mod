# Native movie frame production

`NR.movie-frame-production` owns bounded Qt movie decoding to opaque owned RGBA
frames and full-sequence diagnostics. First-frame checks alone cannot validate
frame order, later conversion or completion of installed movie assets.

The native playback report now hashes every delivered RGBA row in frame order.
The decoder test timeout is bounded and configurable for full installed clips.
Native policy and original DirectShow equivalence remain separate: decoding an
AVI file does not establish legacy placement, control-byte behavior, sound-device
equivalence, native screen ownership through live return, or complete takeover.

The final [comparison](native-movie-frame-comparison-20261010.json) passes all
3,868 decoded frames and 888,459,264 pixels with zero channel differences. Five
installed AVIs and a changing, alpha-bearing synthetic clip run to completion.
The first owned frame remains unchanged after later frames; bounded cancellation
stops further delivery. Historical and failed media reports retain their hashes.

Installed Indeo4 YUV410P frames are normalized by the Qt FFmpeg backend to
YUV420P using its low-level bicubic swscale context. Native playback now selects
a deterministic CPU limited-range BT.601 conversion for that frame format,
including the software conversion's green-channel bias and nearest chroma
sample. RGBX/RGBA/BGRX/BGRA conversion copies original color bytes and forces
opaque alpha before hashing. Other color spaces/ranges retain Qt conversion
and remain outside the selected pixel comparison.

The independent reference decodes raw YUV410P using the FFmpeg CLI, applies
the declared low-level normalizer through libswscale, and calculates RGBA with
NumPy. No Qt/native frame or destination supplies the reference. Direct use of
the CLI scale filter applies extra chroma-location defaults and is a different
reference policy. The failed comparisons are retained, not relabeled passes.
AVI header frame-slot counts include empty chunks in several installed files;
actual independent decoded frame counts determine the comparison extent.

Primary conversion references: [Qt 6.11.2 FFmpeg video normalization](https://raw.githubusercontent.com/qt/qtmultimedia/v6.11.2/src/plugins/multimedia/ffmpeg/qffmpegvideobuffer.cpp),
[swscale context setup](https://raw.githubusercontent.com/qt/qtmultimedia/v6.11.2/src/plugins/multimedia/ffmpeg/qffmpeg.cpp),
and [software frame conversion](https://raw.githubusercontent.com/qt/qtmultimedia/v6.11.2/src/multimedia/video/qvideoframeconversionhelper.cpp).

The recorded cohort uses Qt6.11.2 and FFmpeg9.0.1 on this Linux test host.
Changing decoder/normalizer versions or the graphics backend needs a fresh
comparison; local source freshness alone does not establish cross-version or
Windows/physical-driver equivalence.
