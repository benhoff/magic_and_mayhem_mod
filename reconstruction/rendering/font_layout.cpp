#include "font_layout.hpp"
#include <algorithm>
#include <stdexcept>

namespace mnm::reconstruction::rendering {
namespace {
std::int32_t signedWord(std::uint32_t n) {
  return static_cast<std::int32_t>(n<0x80000000u ? std::int64_t(n) : std::int64_t(n)-0x100000000LL);
}
std::int32_t add(std::int32_t a,std::int32_t b) { return signedWord(std::uint32_t(a)+std::uint32_t(b)); }
std::int32_t sub(std::int32_t a,std::int32_t b) { return signedWord(std::uint32_t(a)-std::uint32_t(b)); }
}
FontLayoutResult fontLineLayout(const ByteFont &font,const FontLayoutInput &in) {
  if (font.rows>128 || in.cursor.trailing.size()!=font.rows ||
      std::uint64_t(font.rows)*font.glyphCount>font.contours.size())
    throw std::invalid_argument("Incomplete font layout contours");
  FontLayoutResult out;out.cursor=in.cursor;
  const bool measure=in.kind==FontLayoutKind::measure;
  const bool limited=in.kind==FontLayoutKind::heightLimited;
  if (in.nullSource) {
    if (!measure) throw std::invalid_argument("Null drawing source");
    return out;
  }
  if (in.text.empty() || in.text.size()>65536 || in.text.back()!=0 ||
      std::find(in.text.begin(),in.text.end()-1,0)!=in.text.end()-1)
    throw std::invalid_argument("Expected bounded terminated byte string");
  const auto &r=in.bounds;
  const auto height=std::uint32_t(r.bottom)-std::uint32_t(r.top);
  if (measure && height<std::uint32_t(add(in.ascent,in.descent))) return out;
  if (in.text.size()==1) {
    if (in.kind==FontLayoutKind::rectangle) return out;
    throw std::invalid_argument("Empty source has unsafe original look-behind");
  }
  auto &cursor=out.cursor;
  auto reset=[&] {std::fill_n(cursor.trailing.begin(),std::min<std::size_t>(32,font.rows),0);};
  auto byte=[&](std::int64_t at) {
    if (at<0 || std::uint64_t(at)>=in.text.size()) throw std::invalid_argument("Original text source overrun");
    return in.text[static_cast<std::size_t>(at)];
  };
  auto advance=[&](std::uint8_t c,bool update) {return std::uint32_t(fontByteAdvance(font,cursor,c,update,in.punctuationMode));};
  const auto halfWidth=sub(r.right,r.left)/2;
  const auto halfHeight=sub(r.bottom,r.top)/2;
  cursor.x=limited ? in.firstX : r.left;
  if (!measure) cursor.y=add(r.top,in.ascent);
  reset();
  std::uint32_t width=0,maxWidth=0,count=0,start=0;
  std::int64_t at=0,next=0;
  bool longWord=false;
  std::vector<std::uint32_t> advances;
  const std::size_t capacity=in.kind==FontLayoutKind::rectangle ? 512 : 100;
  auto line=[&](std::uint32_t n) {
    if (out.lines.size()>=256) throw std::invalid_argument("Original line table overrun");
    std::int32_t x=limited && out.lines.empty() ? in.firstX : r.left;
    if (in.flags&4) x=add(sub(r.left,signedWord(width>>1)),halfWidth);
    else if (in.flags&2) x=sub(r.right,signedWord(width));
    out.lines.push_back({x,n,start});
    maxWidth=std::max(maxWidth,width);
  };
  for (;;) {
    const auto step=advance(byte(at),true);
    width+=step;
    if (in.flags&16) {
      if (count>=capacity) throw std::invalid_argument("Original advance stack overrun");
      if (advances.size()<=count) advances.resize(count+1);
      advances[count]=step;
    }
    next=at+1;
    cursor.x=add(cursor.x,signedWord(step));
    auto prospective=std::uint32_t(cursor.x);
    const auto look=byte(next);
    if (look!=32 && look!=10 && look!=9) prospective+=advance(look==0 ? 32 : look,false);
    auto n=count+1;
    if (((in.flags&16) && std::uint32_t(r.right)<prospective) || byte(at)==10) {
      if (byte(at)==10) {
        if (at>1) width+=advance(byte(at-1),true);
      } else {
        width-=advances.at(count);
        n=count;
        --at;
        while (n!=0) {
          const auto b=byte(at);
          if (b==32 || b==9 || at==start) break;
          width-=advances.at(n-1);
          --n;--at;
        }
        if (n==0) longWord=true;
      }
      if (longWord) {
        do {
          const auto b=byte(at);
          if (b==0) throw std::invalid_argument("Original long word scan crosses terminator");
          width+=advance(b,true);++n;++at;
        } while (byte(at)!=32 && byte(at)!=9);
      }
      line(n);
      cursor.x=r.left;reset();
      next=at+1;start=static_cast<std::uint32_t>(next);
      count=0;n=0;width=0;
    }
    const bool heightStop=limited &&
      (std::uint32_t(out.lines.size()+1)*std::uint32_t(in.lineHeight)>height);
    const auto finalPointer=heightStop ? at : next;
    if (heightStop || byte(next)==0 || out.lines.size()>=255) {
      width+=advance(byte(finalPointer-1),true);
      line(n);out.width=measure ? maxWidth : width;
      break;
    }
    count=n;at=next;
  }
  if (measure) return out;
  if (in.flags&8) {
    const auto extent=std::uint32_t(add(in.ascent,in.descent))*std::uint32_t(out.lines.size());
    cursor.y=add(cursor.y,sub(halfHeight,signedWord(extent>>1)));
  }
  for (auto &l:out.lines) {
    cursor.x=l.x;reset();
    for (std::uint32_t n=0;n<l.count;++n) {
      const auto b=byte(l.source++);out.consumed=l.source;
      if ((b>31 && std::uint32_t(b)<font.glyphCount+33u) || b==10 || b==9) {
        const auto glyph=fontByteGlyph(font,cursor,b,in.punctuationMode);
        if (glyph.frame) out.glyphs.push_back(glyph);
      }
    }
    cursor.y=add(cursor.y,in.lineHeight);
  }
  return out;
}
}
