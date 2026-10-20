// Reuse the independently encoded RLE/wire fixtures, not its additive oracle.
#define main additive_fixture_unused_main
#include "additive-sprite-test.cpp"
#undef main
static Fixture unitRuns(){
    auto f=fixture(256,256,false);f.encoded.assign(40+f.height*8,0);
    word(f.encoded,4,f.width);word(f.encoded,8,f.height);
    for(int y=0;y<f.height;++y){
        word(f.encoded,40+y*8,f.encoded.size());
        for(int x=0;x<f.width;++x){f.encoded.push_back(0);f.encoded.push_back(1);}
    }
    for(int y=0;y<f.height;++y){word(f.encoded,44+y*8,f.encoded.size());
        f.encoded.insert(f.encoded.end(),f.indices.begin()+y*f.width,f.indices.begin()+(y+1)*f.width);}
    word(f.encoded,0,f.encoded.size());return f;
}
static std::vector<unsigned char> endpoints(const Fixture& f){
    std::vector<unsigned char> mask(f.mask.size(),0);
    auto get=[&](std::size_t at){return unsigned(f.encoded.at(at))|unsigned(f.encoded.at(at+1))<<8|unsigned(f.encoded.at(at+2))<<16|unsigned(f.encoded.at(at+3))<<24;};
    for(int y=0;y<f.height;++y){unsigned x=0,at=get(40+y*8);
        while(x<unsigned(f.width)){x+=f.encoded.at(at++);if(x==unsigned(f.width))break;
            const auto count=f.encoded.at(at++);require(count&&x+count<=unsigned(f.width),"Invalid oracle RLE");
            mask[y*f.width+x]=mask[y*f.width+x+count-1]=1;x+=count;}}
    return mask;
}
static void outlineApply(render::Image& canvas,const Fixture& f,int x,int y,render::Rect clip,const render::SpriteColourTable& table){
    x-=f.originX;y-=f.originY;const bool clipped=x<clip.left||x+f.width>clip.right;
    const auto edge=endpoints(f);
    for(int row=0;row<f.height;++row)for(int col=0;col<f.width;++col){
        const auto index=row*f.width+col;const int dx=x+col,dy=y+row;
        if(!f.mask[index]||dx<clip.left||dx>=clip.right||dy<clip.top||dy>=clip.bottom)continue;
        auto& d=canvas.pixels[dy*canvas.width+dx];const auto s=table[f.indices[index]];
        if(clipped)d=((s>>1)&0x7bef)+((d>>1)&0x7bef);
        else if(edge[index])d=s;
    }
}
static void outlineRecord(QByteArray& wire,const Fixture& f,int x,int y,render::Rect clip,const render::SpriteColourTable& palette){
    const auto at=wire.size();record(wire,f,x,y,clip,palette);
    put(wire,at+4,MNM_WORLD_RUN_OUTLINE);put(wire,at+52,14);
}
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);require(argc==2,"Expected fresh outline prefix");const QString prefix=QString::fromLocal8Bit(argv[1]);
    const auto units=unitRuns(),dense=fixture(256,256,false),holes=fixture(17,9,true);
    const auto sprBytes=spriteFile({units,dense,holes});QTemporaryDir directory;require(directory.isValid(),"Outline root unavailable");
    const auto root=std::filesystem::path(directory.path().toStdString());write(root,"outline.spr",sprBytes);
    auto input=store(root);assets::ResourceManager resources(input);legacy::SnapshotResources bindings(input,resources);
    bindings.add({assets::ResourceKind::ui,"outline"},"outline.spr",QCryptographicHash::hash(bytes(sprBytes),QCryptographicHash::Sha256).toHex());
    render::GlBlitter renderer;unsigned cases=0;
    auto check=[&](QByteArray wire,const render::Image& before,const render::Image& expected,bool atlas){
        const auto frame=legacy::decodeWorldFrame(wire);auto draws=legacy::worldDisplay(frame,bindings);
        if(atlas){legacy::WorldPreparation job(input);job.plan(wire);auto plan=std::get<legacy::WorldPreparationPlan>(await(job));
            require(plan.files.size()==1,"Outline lost resource plan");resources.bind(plan.files[0].id,spr(plan.files[0].path));
            job.prepare({resources.request(plan.files[0].id)},16*1024*1024,8);
            const auto ready=std::get<legacy::PreparedWorld>(await(job));require(ready.draws.size()==draws.size(),"Outline worker lost draws");
            for(std::size_t i=0;i<draws.size();++i)require(ready.draws[i].composite.mode==draws[i].composite.mode,"Outline worker changed branch");
            job.cancel();}
        render::SceneLimits limits;limits.cache.indexedAtlas=atlas;
        render::SceneRenderer scene(renderer,resources,before,limits);const auto reads=renderer.stats().nativeReadbacks;
        scene.draw(draws);require(scene.presentGpu().valid()&&renderer.stats().nativeReadbacks==reads,"Outline reads back pixels");
        const auto actual=scene.read();require(actual.pixels==expected.pixels,"Outline endpoint/clip/palette/order differs");
        const auto name=prefix+"-"+QString::number(cases++);save(name+".bin",wire);save(name+".before.565",pixels(before));save(name+".native.565",pixels(actual));
    };
    render::SpriteColourTable palette;for(unsigned i=0;i<256;++i)palette[i]=(i*7919+0x1234)&65535;
    palette[0]=0;
    for(const auto* f:{&units,&dense})for(bool atlas:{false,true}){
        auto wire=envelope(256,256);outlineRecord(wire,*f,0,0,{0,0,256,256},palette);seal(wire,1);
        auto before=background(256,256),expected=before;outlineApply(expected,*f,0,0,{0,0,256,256},palette);check(wire,before,expected,atlas);
        for(const auto& mutation:std::vector<std::pair<unsigned,unsigned>>{{84,10},{84,0},{116,0},{132,13},{132,15},{172,1}}){auto bad=wire;put(bad,mutation.first,mutation.second);rejected([&]{legacy::decodeWorldFrame(bad);});}
    }
    // Every source channel value against every RGB565 destination, on the
    // horizontally clipped path as well as endpoint copying.
    for(unsigned shade=0;shade<64;++shade){
        palette.fill(((shade&31)<<11)|(shade<<5)|(shade&31));
        for(bool clipped:{false,true}){const render::Rect clip=clipped?render::Rect{1,0,256,256}:render::Rect{0,0,256,256};
            auto wire=envelope(256,256);outlineRecord(wire,units,0,0,clip,palette);seal(wire,1);
            constexpr int shades[]{-127,-1,0,1,127};put(wire,124,shades[shade%5]);put(wire,136,shade%3);
            auto before=background(256,256),expected=before;outlineApply(expected,units,0,0,clip,palette);check(wire,before,expected,shade==0);}
    }
    for(unsigned i=0;i<256;++i)palette[i]=(i*7919+0x1234)&65535;
    palette[0]=0;
    for(bool narrow:{false,true})for(bool atlas:{false,true}){
        const render::Rect clip=narrow?render::Rect{4,3,29,16}:render::Rect{0,0,37,19};
        auto wire=envelope(37,19);auto before=background(37,19),expected=before;unsigned count=0;
        for(int y:{-12,-1,0,2,7,12,19})for(int x:{-20,-1,0,2,7,18,21,37}){
            outlineRecord(wire,holes,x,y,clip,palette);outlineApply(expected,holes,x,y,clip,palette);++count;}
        seal(wire,count);check(wire,before,expected,atlas);
    }
    // Equal decoded pixels with different encoded runs have different outline
    // semantics, so a visual rebind must retain that distinction.
    const auto decoded=std::get<assets::Sprite>(assets::decodeSprite(sprBytes));
    require(legacy::spriteVisualIdentity(decoded,decoded.frames[0])!=legacy::spriteVisualIdentity(decoded,decoded.frames[1]),"Outline run identity collapsed");
    // Endpoint bit2 is still ordinary nonzero coverage for displacement
    // admission. Transparent tail padding must not make a valid draw fail.
    {
        auto source=renderer.create({2,1,{0,0}},render::spriteFormat);
        auto mask=renderer.create({2,1,{3,0}},{8,{}});
        auto canvas=renderer.create({2,1,{7,9}},render::spriteFormat);
        render::SpriteComposite displacement;displacement.mode=render::CompositeMode::displace;
        displacement.rowPeriod=1;displacement.rowOffsets[0]=1;
        renderer.composite(source,canvas,{0,0,2,1},0,0,mask,displacement);
        require(renderer.read(canvas).pixels==std::vector<std::uint32_t>({9,9}),"Endpoint coverage lost safe displacement proof");
        renderer.update(mask,0,0,{2,1,{3,3}});
        rejected([&]{renderer.composite(source,canvas,{0,0,2,1},0,0,mask,displacement);});
        require(renderer.read(canvas).pixels==std::vector<std::uint32_t>({9,9}),"Unsafe displacement changed pixels");
        renderer.destroy(canvas);renderer.destroy(mask);renderer.destroy(source);
    }
    require(renderer.stats().surfaces==0,"Outline GPU lifetime leak");
    std::cout<<cases<<" outline fixtures pass\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
