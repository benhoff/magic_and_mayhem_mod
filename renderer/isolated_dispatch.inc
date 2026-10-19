// Included by blit.cpp: all GL handles and validity stay on the service owner.
QByteArray GlBlitter::saveIsolatedState(){
    auto& p=*impl_;p.thread();p.flushCopies();Current current(p.context,p.surface);
    QByteArray bytes;QDataStream out(&bytes,QIODevice::WriteOnly);out.setVersion(QDataStream::Qt_6_0);
    wire::fields(out,wire::version,quint32(p.surfaces.size()));
    for(const auto& [id,s]:p.surfaces){
        Image pixels{s.width,s.height,std::vector<std::uint32_t>(std::size_t(s.width)*s.height)};
        p.gl.glActiveTexture(GL_TEXTURE0);p.gl.glBindTexture(GL_TEXTURE_2D,s.native);
        p.gl.glGetTexImage(GL_TEXTURE_2D,0,GL_RED_INTEGER,GL_UNSIGNED_INT,pixels.pixels.data());
        wire::fields(out,id,s.format,pixels,s.clipper,s.validity.serialized(),s.paletteColors,s.paletteKnown,s.coverage);
        ++p.counters.recoveryReadbacks;
    }
    ++p.counters.recoveryCheckpoints;wire::put(out,p.counters);p.check(true);
    if(bytes.size()>wire::maxPacket)throw std::runtime_error("Native recovery state exceeds budget");
    return bytes;
}
void GlBlitter::loadIsolatedState(const QByteArray& bytes){
    auto& p=*impl_;if(!p.surfaces.empty())throw std::runtime_error("Recovery import requires a fresh worker");
    QDataStream in(bytes);in.setVersion(QDataStream::Qt_6_0);quint32 version,count;wire::values(in,version,count);
    if(version!=wire::version || count>64)throw std::runtime_error("Invalid recovery state header");
    for(unsigned i=0;i<count;++i){
        SurfaceId id;PixelFormat format;Image pixels;ClipperState clip;std::vector<std::uint8_t> known,coverage;std::array<Rgb,256> palette;std::array<bool,256> paletteKnown;
        wire::values(in,id,format,pixels,clip,known,palette,paletteKnown,coverage);
        if(!id || id==UINT64_MAX || p.surfaces.count(id) || pixels.width<1 || pixels.height<1 || pixels.pixels.size()!=std::size_t(pixels.width)*pixels.height)throw std::runtime_error("Invalid recovery surface");
        const auto allocated=allocate(pixels.width,pixels.height,format);
        auto node=p.surfaces.extract(allocated);node.key()=id;auto& s=node.mapped();s.validity.restore(known);
        validateClipper(clip,s.width,s.height);s.clipper=clip;s.paletteColors=palette;s.paletteKnown=paletteKnown;s.coverage=std::move(coverage);
        Current current(p.context,p.surface);p.gl.glActiveTexture(GL_TEXTURE0);p.gl.glBindTexture(GL_TEXTURE_2D,s.native);
        p.gl.glTexSubImage2D(GL_TEXTURE_2D,0,0,0,s.width,s.height,GL_RED_INTEGER,GL_UNSIGNED_INT,pixels.pixels.data());
        if(s.palette){std::array<std::uint8_t,1024> colours{};for(unsigned n=0;n<256;++n){colours[n*4]=palette[n].red;colours[n*4+1]=palette[n].green;colours[n*4+2]=palette[n].blue;colours[n*4+3]=255;}
            p.gl.glBindTexture(GL_TEXTURE_2D,s.palette);p.gl.glTexSubImage2D(GL_TEXTURE_2D,0,0,0,256,1,GL_RGBA_INTEGER,GL_UNSIGNED_BYTE,colours.data());}
        p.surfaces.insert(std::move(node));reserveId(id);
    }
    wire::get(in,p.counters);wire::check(in);p.check(true);
}
QByteArray GlBlitter::dispatchIsolated(const QByteArray& request){
    if(isolated_)throw std::runtime_error("Recursive render process dispatch");
    Current dispatchContext(impl_->context,impl_->surface);
    QDataStream in(request);in.setVersion(QDataStream::Qt_6_0);wire::Op op;wire::get(in,op);QByteArray out;
    auto nativeFrame=[&](QImage pixels,PixelFormat format){
        if(format.bits!=16 || format.masks!=std::array<std::uint32_t,3>{0xf800,0x7e0,31})throw std::runtime_error("Process frame import requires RGB565");
        auto data=std::make_shared<GpuFrame::Data>();data->owner=impl_;data->dimensions=pixels.size();data->format=format;
        auto& p=*impl_;Current current(p.context,p.surface);p.gl.glActiveTexture(GL_TEXTURE0);
        data->texture=p.texture(GL_RGBA8UI,GL_RGBA_INTEGER,GL_UNSIGNED_BYTE,pixels.width(),pixels.height(),pixels.constBits());
        return GpuFrame(std::move(data));
    };
    switch(op){
case wire::Op::create:{SurfaceId requested;Image image;PixelFormat format;wire::values(in,requested,image,format);wire::check(in);const auto actual=create(image,format);auto node=impl_->surfaces.extract(actual);if(!requested || requested==UINT64_MAX || impl_->surfaces.count(requested))throw std::runtime_error("Invalid requested surface identity");node.key()=requested;impl_->surfaces.insert(std::move(node));reserveId(requested);break;}
case wire::Op::allocate:{SurfaceId requested;int width;int height;PixelFormat format;wire::values(in,requested,width,height,format);wire::check(in);const auto actual=allocate(width,height,format);auto node=impl_->surfaces.extract(actual);if(!requested || requested==UINT64_MAX || impl_->surfaces.count(requested))throw std::runtime_error("Invalid requested surface identity");node.key()=requested;impl_->surfaces.insert(std::move(node));reserveId(requested);break;}
case wire::Op::clone:{SurfaceId requested;SurfaceId source;wire::values(in,requested,source);wire::check(in);const auto actual=cloneSurface(source);auto node=impl_->surfaces.extract(actual);if(!requested || requested==UINT64_MAX || impl_->surfaces.count(requested))throw std::runtime_error("Invalid requested surface identity");node.key()=requested;impl_->surfaces.insert(std::move(node));reserveId(requested);break;}
case wire::Op::restore:{SurfaceId source;SurfaceId destination;wire::values(in,source,destination);wire::check(in);restoreSurface(source,destination);break;}
case wire::Op::rows:{SurfaceId id;Image plane;int first;int rows;wire::values(in,id,plane,first,rows);wire::check(in);updateRows(id,plane,first,rows);break;}
case wire::Op::regionRows:{SurfaceId id;int x;int y;Image plane;int first;int rows;wire::values(in,id,x,y,plane,first,rows);wire::check(in);updateRegionRows(id,x,y,plane,first,rows);break;}
case wire::Op::destroy:{SurfaceId id;wire::values(in,id);wire::check(in);serviceFrames_.erase(id);destroy(id);break;}
case wire::Op::update:{SurfaceId id;int x;int y;Image patch;wire::values(in,id,x,y,patch);wire::check(in);update(id,x,y,patch);break;}
case wire::Op::masked:{SurfaceId id;int x;int y;Image patch;std::vector<std::uint8_t> coverage;wire::values(in,id,x,y,patch,coverage);wire::check(in);updateMasked(id,x,y,patch,coverage);break;}
case wire::Op::covered:{SurfaceId id;int x;int y;Image patch;std::vector<std::uint8_t> coverage;wire::values(in,id,x,y,patch,coverage);wire::check(in);updateCoveredWords(id,x,y,patch,coverage);break;}
case wire::Op::dib:{SurfaceId id;DibInput dib;std::optional<std::vector<Rect>> dcRegions;wire::values(in,id,dib,dcRegions);wire::check(in);reloadDib(id,dib,dcRegions);break;}
case wire::Op::copy:{SurfaceId source;SurfaceId destination;Rect r;int x;int y;std::optional<std::uint32_t> key;std::optional<SurfaceId> mask;std::uint32_t keyMask;wire::values(in,source,destination,r,x,y,key,mask,keyMask);wire::check(in);copy(source,destination,r,x,y,key,mask,keyMask);break;}
case wire::Op::composite:{SurfaceId source;SurfaceId destination;Rect r;int x;int y;SurfaceId mask;SpriteComposite c;std::optional<std::array<std::uint16_t,256>> palette;wire::values(in,source,destination,r,x,y,mask,c,palette);wire::check(in);composite(source,destination,r,x,y,mask,c,palette?&*palette:nullptr);break;}
case wire::Op::colour:{SurfaceId destination;Image pixels;Rect source;int x;int y;SpriteComposite mode;wire::values(in,destination,pixels,source,x,y,mode);wire::check(in);colourRect(destination,pixels,source,x,y,mode);break;}
case wire::Op::additive:{SurfaceId destination;Rect r;std::array<std::uint16_t,3> channels;wire::values(in,destination,r,channels);wire::check(in);additiveRect(destination,r,channels);break;}
case wire::Op::invalidate:{SurfaceId id;wire::values(in,id);wire::check(in);invalidateContents(id);break;}
case wire::Op::clip:{SurfaceId id;ClipperState clipper;wire::values(in,id,clipper);wire::check(in);setClipper(id,clipper);break;}
case wire::Op::surfaceCopy:{SurfaceId source;SurfaceId destination;SurfaceCopyRequest request;std::optional<std::uint32_t> nativeKey;wire::values(in,source,destination,request,nativeKey);wire::check(in);out=wire::message(surfaceCopy(source,destination,request,nativeKey));break;}
case wire::Op::swap:{SurfaceId first;SurfaceId second;wire::values(in,first,second);wire::check(in);swapContents(first,second);break;}
case wire::Op::palette:{SurfaceId id;unsigned first;std::vector<Rgb> colors;wire::values(in,id,first,colors);wire::check(in);setPalette(id,first,colors);break;}
case wire::Op::read:{SurfaceId id;wire::values(in,id);wire::check(in);out=wire::message(read(id));break;}
case wire::Op::dimensions:{SurfaceId id;wire::values(in,id);wire::check(in);out=wire::message(dimensions(id));break;}
case wire::Op::present:{SurfaceId id;wire::values(in,id);wire::check(in);out=wire::message(present(id));break;}
case wire::Op::clipper:{SurfaceId id;wire::values(in,id);wire::check(in);out=wire::message(surfaceClipper(id));break;}
case wire::Op::coordinates:{SurfaceId pixels;std::optional<SurfaceId> source;SurfaceId destination;Rect r;int x;int y;std::optional<std::uint32_t> key;wire::values(in,pixels,source,destination,r,x,y,key);wire::check(in);copyCoordinates(pixels,source,destination,r,x,y,key);break;}
case wire::Op::driver:wire::check(in);out=wire::message(driver());break;
case wire::Op::stats:wire::check(in);out=wire::message(stats());break;
case wire::Op::checkpoint:wire::check(in);out=saveIsolatedState();break;
case wire::Op::load:{QByteArray bytes;wire::get(in,bytes);wire::check(in);loadIsolatedState(bytes);break;}
case wire::Op::frame:{
    SurfaceId id;wire::get(in,id);wire::check(in);auto frame=presentGpu(id);serviceFrames_[id]=frame;
    auto& p=*impl_;Current current(p.context,p.surface);QImage image(frame.size(),QImage::Format_RGBA8888);
    if(image.isNull())throw std::runtime_error("Cannot allocate isolated frame transport");
    p.gl.glActiveTexture(GL_TEXTURE0);p.gl.glBindTexture(GL_TEXTURE_2D,frame.data_->texture);
    p.gl.glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA_INTEGER,GL_UNSIGNED_BYTE,image.bits());++p.counters.recoveryReadbacks;
    out=wire::message(image,p.get(id).format);break;
}
case wire::Op::importFrame:{SurfaceId id;QImage image;PixelFormat format;wire::values(in,id,image,format);wire::check(in);auto frame=nativeFrame(std::move(image),format);importGpu565(id,frame);break;}
case wire::Op::mapped:{SurfaceId destination,ui,coordinates;QImage image;PixelFormat format;wire::values(in,destination,ui,coordinates,image,format);wire::check(in);auto frame=nativeFrame(std::move(image),format);assembleMappedWorld(destination,ui,coordinates,frame);break;}
default:throw std::runtime_error("Unknown render process operation");
    }
    impl_->flushCopies();impl_->check(true);return out;
}
