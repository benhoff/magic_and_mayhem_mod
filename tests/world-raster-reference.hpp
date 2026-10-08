// Precise admitted original raster entry, with a closed effective colour/row table.
static unsigned originalRaster(const Bytes& record,std::uintptr_t entry,std::vector<std::uint16_t>& pixels){
 if(record.size()<136||u32(record,8)!=9||u32(record,0)!=record.size())throw std::runtime_error("Closed raster envelope");
 const auto width=u32(record,20),height=u32(record,24),size=u32(record,76),payload=u32(record,80),backend=u32(record,60),mode=u32(record,56),indexed=u32(record,68);
 if(!width||!height||width>2048||height>2048||pixels.size()!=std::size_t(width)*height||size<40||record.size()!=96+size+payload||u32(record,96)!=size||u32(record,124)||indexed>1||payload!=(mode==3?64u:indexed?512u:0u))throw std::runtime_error("Closed source/extent");
 const std::pair<std::uintptr_t,unsigned> entries[]={{0x595677,0},{0x5947b2,1},{0x59521a,1},{0x595b47,2},{0x59603e,2},{0x57de00,3},{0x57ec90,4},{0x57f0f0,5},{0x57f5f0,6},{0x5806f0,7},{0x596490,9},{0x5968a4,9},{0x57e540,10}};
 bool allowed=false;for(auto [address,kind]:entries)if(entry==address&&backend==kind)allowed=true;
 if(!allowed)throw std::runtime_error("Precise raster entry/backend");
 const unsigned char generic[]={0x55,0x8b,0xec};const unsigned char wrapper[]={0x83,0xec};
 if(std::memcmp(reinterpret_cast<void*>(entry),entry==0x57de00||backend<=2||backend==9?generic:wrapper,entry==0x57de00||backend<=2||backend==9?3:2))throw std::runtime_error("Precise raster prefix");
 Bytes frame(record.begin()+96,record.begin()+96+size);std::vector<std::uint32_t> palette(3+256,0);
 if(indexed&&mode!=3){const auto step=backend==2?4u:2u;for(unsigned i=0;i<256;++i){const auto word=unsigned(record[96+size+i*2])|unsigned(record[97+size+i*2])<<8;std::memcpy(reinterpret_cast<unsigned char*>(palette.data()+3)+i*step,&word,2);}const auto pointer=std::uint32_t(reinterpret_cast<std::uintptr_t>(palette.data()));std::memcpy(frame.data()+28,&pointer,4);}
 else {const std::uint32_t marker=indexed?0u:UINT32_MAX;std::memcpy(frame.data()+28,&marker,4);}
 global(0x658174,reinterpret_cast<std::uintptr_t>(pixels.data()));global(0x6a2dc8,width);global(0x6e1f68,0);global(0x6e1f88,0);
 for(unsigned i=0;i<4;++i)global(i==0?0x6e0008:i==1?0x6cbb6c:i==2?0x6a49b8:0x656618,u32(record,40+i*4));
 const int x=std::int32_t(u32(record,32)),y=std::int32_t(u32(record,36));
 if(backend==9){global(0x6c4830,UINT32_MAX);for(unsigned j=0;j<16;++j)global(0x656640+j*4,u32(record,96+size+j*4));}
 if(backend<=3||backend==9){using Draw=unsigned (*)(void*,int,int,int,int);return reinterpret_cast<Draw>(entry)(frame.data(),x,y,0,0);}
 if(backend==10){using Shadow=unsigned (__attribute__((fastcall)) *)(void*,int,int);return reinterpret_cast<Shadow>(entry)(frame.data(),x,y);}
 if(backend==7){
  const auto period=u32(record,64),amplitude=period/2;if(!period||period>16||payload!=64)throw std::runtime_error("Wave table");
  global(0x5f14d0,0);for(unsigned j=0;j<16;++j)global(0x5f14d0+(amplitude*16+j)*4,u32(record,96+size+j*4));
  const auto left=std::int64_t(x)-std::int32_t(u32(frame,12));bool clipped=left<std::int32_t(u32(record,40))||left+u32(frame,4)>std::int32_t(u32(record,48));unsigned phase=0;
  if(clipped){unsigned matches=0;for(unsigned q=0;q<16;++q){bool same=true;for(unsigned j=0;j<16;++j){int n=8-int((j+q)&15);if(unsigned(n<0?-n:n)!=u32(record,96+size+j*4))same=false;}if(same){phase=q;++matches;}}if(matches!=1)throw std::runtime_error("Clipped wave phase");}
  global(0x6c4830,phase);using Wave=unsigned (__attribute__((fastcall)) *)(void*,int,int,int);return reinterpret_cast<Wave>(entry)(frame.data(),x,y,amplitude);
 }
 using Blend=unsigned (__attribute__((fastcall)) *)(void*,int,int,int,int);return reinterpret_cast<Blend>(entry)(frame.data(),x,y,0,0);
}
