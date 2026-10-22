#ifndef MNM_READABLE_EXTENT_H
#define MNM_READABLE_EXTENT_H
/* Caller supplies MNM_REGION_QUERY(address, seven_DWORD_region).
 * A proof is local to one synchronous source admission. It must never survive
 * an original callback, a queue boundary, or an external memory writer. */
static inline int mnm_readable_protection(unsigned protection) {
  if (protection & 0x100) return 0;
  switch (protection & 0xff) {
    case 2: case 4: case 8: case 0x20: case 0x40: case 0x80: return 1;
    default: return 0;
  }
}
static int mnm_readable_extent(unsigned p, unsigned n, unsigned *through) {
  unsigned end = p + n;
  if (through) *through = 0;
  if (!p || end < p) return 0;
  while (p < end) {
    unsigned m[7];
    if (MNM_REGION_QUERY(p,m) != 28 || m[4] != 0x1000 ||
        !mnm_readable_protection(m[5]) || m[0] > p ||
        m[3] > ~0u-m[0]) return 0;
    unsigned next = m[0] + m[3];
    if (next <= p) return 0;
    if (through) *through = next;
    p = next < end ? next : end;
  }
  return 1;
}
static inline int mnm_readable_tail(unsigned p, unsigned n, unsigned through) {
  if (!p || n > ~0u-p) return 0;
  if (through >= p && n <= through-p) return 1;
  if (through > p) return mnm_readable_extent(through,n-(through-p),0);
  return mnm_readable_extent(p,n,0);
}
/* A freshly queried entire region may cover several related inputs within
 * this same synchronous hook. Bounds are essential: an end alone cannot admit
 * a different pointer below the queried region. No original callback is allowed
 * between establishing and using this proof. */
static int mnm_readable_window(unsigned p,unsigned n,unsigned *base,unsigned *end) {
  unsigned m[7];*base=*end=0;
  if(!p||n>~0u-p||MNM_REGION_QUERY(p,m)!=28||m[4]!=0x1000||
     !mnm_readable_protection(m[5])||m[0]>p||m[3]>~0u-m[0])return 0;
  unsigned next=m[0]+m[3];if(next<=p)return 0;
  if(n<=next-p){*base=m[0];*end=next;return 1;}
  return mnm_readable_extent(p,n,0);
}
static int mnm_readable_in_window(unsigned p,unsigned n,unsigned base,unsigned end) {
  if(!p||n>~0u-p)return 0;
  if(base&&p>=base&&p<=end&&n<=end-p)return 1;
  return mnm_readable_extent(p,n,0);
}
/* Refreshable proof for several related pointers during one synchronous input
 * copy. Caller owns/reset these bounds for each admission; never retain them
 * across an original callback or external memory writer. */
static inline int mnm_readable_refresh_window(unsigned p,unsigned n,unsigned *base,unsigned *end) {
  if(!p||n>~0u-p){*base=*end=0;return 0;}
  if(*base&&p>=*base&&p<=*end&&n<=*end-p)return 1;
  return mnm_readable_window(p,n,base,end);
}
#endif
