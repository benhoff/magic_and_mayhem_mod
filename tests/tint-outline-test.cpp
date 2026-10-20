#define main additive_fixture_unused_main
#include "additive-sprite-test.cpp"
#undef main
static void tintRecord(QByteArray& wire,const Fixture& frame,int x,int y,render::Rect clip,
                       const render::SpriteColourTable& palette,int shade,std::array<unsigned,3> rgb){
    const auto at=wire.size();record(wire,frame,x,y,clip,palette);
    put(wire,at,64+frame.encoded.size()+524);put(wire,at+4,MNM_WORLD_TINT_OUTLINE);
    put(wire,at+40,524);put(wire,at+44,shade);put(wire,at+52,15);
    for(auto value:rgb){const auto offset=wire.size();wire.append(QByteArray(4,0));put(wire,offset,value);}
}
static unsigned tintExpected(int shade,const std::array<unsigned,3>& rgb){
    std::array<unsigned,3> v;
    for(unsigned i=0;i<3;++i)v[i]=std::min(255u,(rgb[i]&255u)*unsigned(127+shade)/127u);
    return ((v[0]/8)<<11)|((v[1]/4)<<5)|(v[2]/8);
}
static void tintApply(render::Image& canvas,const Fixture& f,int x,int y,render::Rect clip,
                      const render::SpriteColourTable& palette,int shade,std::array<unsigned,3> rgb){
    x-=f.originX;y-=f.originY;const bool clipped=x<clip.left||x+f.width>clip.right;
    auto get=[&](unsigned at){return unsigned(f.encoded.at(at))|unsigned(f.encoded.at(at+1))<<8|unsigned(f.encoded.at(at+2))<<16|unsigned(f.encoded.at(at+3))<<24;};
    for(int row=0;row<f.height;++row){unsigned col=0,at=get(40+row*8);
        while(col<unsigned(f.width)){col+=f.encoded.at(at++);if(col==unsigned(f.width))break;
            const auto count=f.encoded.at(at++);require(count&&col+count<=unsigned(f.width),"Invalid tint RLE");
            for(unsigned index=0;index<count;++index){const int dx=x+col+index,dy=y+row;
                if(dx<clip.left||dx>=clip.right||dy<clip.top||dy>=clip.bottom)continue;
                auto& d=canvas.pixels[dy*canvas.width+dx];
                if(clipped){const auto s=palette[f.indices[row*f.width+col+index]];d=((s>>1)&0x7bef)+((d>>1)&0x7bef);}
                else if(index==0){
                    const auto value=tintExpected(shade,rgb);const auto base=std::int64_t(dy)*canvas.width+x+col;
                    for(auto pixel:{base-1,base})canvas.pixels.at(pixel)=value;
                    if(count>1)for(auto pixel:{base+count-2,base+count-1})canvas.pixels.at(pixel)=value;
                }
            }col+=count;
        }
    }
}
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);require(argc==2,"Expected fresh tint prefix");const QString prefix=QString::fromLocal8Bit(argv[1]);
    const auto point=fixture(1,1,false),holes=fixture(17,9,true);const auto sprite=spriteFile({point,holes});
    QTemporaryDir directory;require(directory.isValid(),"Tint root unavailable");const auto root=std::filesystem::path(directory.path().toStdString());write(root,"tint.spr",sprite);
    auto input=store(root);assets::ResourceManager resources(input);legacy::SnapshotResources bindings(input,resources);
    bindings.add({assets::ResourceKind::ui,"tint"},"tint.spr",QCryptographicHash::hash(bytes(sprite),QCryptographicHash::Sha256).toHex());
    render::GlBlitter renderer;unsigned cases=0;
    auto check=[&](const QByteArray& wire,const render::Image& before,const render::Image& expected,bool atlas){
        const auto frame=legacy::decodeWorldFrame(wire);const auto draws=legacy::worldDisplay(frame,bindings);
        if(atlas){legacy::WorldPreparation job(input);job.plan(wire);auto plan=std::get<legacy::WorldPreparationPlan>(await(job));
            require(plan.files.size()==1,"Tint lost resource plan");resources.bind(plan.files[0].id,spr(plan.files[0].path));job.prepare({resources.request(plan.files[0].id)},16*1024*1024,8);
            const auto ready=std::get<legacy::PreparedWorld>(await(job));require(ready.draws.size()==draws.size(),"Tint worker lost draws");
            for(std::size_t i=0;i<draws.size();++i)require(ready.draws[i].colours==draws[i].colours&&ready.draws[i].composite.mode==draws[i].composite.mode,"Tint worker changed colours or branch");
            job.cancel();}
        render::SceneLimits limits;limits.cache.indexedAtlas=atlas;render::SceneRenderer scene(renderer,resources,before,limits);
        const auto reads=renderer.stats().nativeReadbacks;scene.draw(draws);
        require(scene.presentGpu().valid()&&renderer.stats().nativeReadbacks==reads,"Tint drawing reads back pixels");
        const auto actual=scene.read();require(actual.pixels==expected.pixels,"Tint arithmetic, coverage, clipping or order differs");
        const auto name=prefix+"-"+QString::number(cases++);save(name+".bin",wire);save(name+".before.565",pixels(before));save(name+".native.565",pixels(actual));
    };
    render::SpriteColourTable palette;for(unsigned i=0;i<256;++i)palette[i]=(i*7919+0x1234)&65535;
    palette[0]=0;
    // All255 bounded shades and every low-byte value in each RGB input.
    for(int shade=-127;shade<=127;++shade){auto wire=envelope(258,1);auto before=background(258,1),expected=before;
        for(unsigned i=0;i<256;++i){const std::array<unsigned,3> rgb{0x12340000u|i,0x98760000u|((i*73+18)&255),0xabcdef00u|((i*17+201)&255)};
            tintRecord(wire,point,i+1,0,{0,0,258,1},palette,shade,rgb);tintApply(expected,point,i+1,0,{0,0,258,1},palette,shade,rgb);}
        seal(wire,256);check(wire,before,expected,shade%2==0);
        if(shade==0){for(const auto& mutation:std::vector<std::pair<unsigned,unsigned>>{{32,259},{84,11},{124,unsigned(-128)},{124,128},{128,1},{132,14},{136,1},{120,512}}){auto bad=wire;put(bad,mutation.first,mutation.second);rejected([&]{legacy::decodeWorldFrame(bad);});}}
    }
    for(bool narrow:{false,true})for(bool atlas:{false,true}){const render::Rect clip=narrow?render::Rect{4,3,29,16}:render::Rect{0,0,37,19};
        auto wire=envelope(37,19);auto before=background(37,19),expected=before;unsigned count=0;
        for(int shade:{-127,-1,0,1,127})for(int y:{-12,-1,0,2,7,12,19})for(int x:{-20,-1,0,2,7,18,21,37}){
            const std::array<unsigned,3> rgb{0xffff12f8,0xabcdefad,0x12345671};tintRecord(wire,holes,x,y,clip,palette,shade,rgb);tintApply(expected,holes,x,y,clip,palette,shade,rgb);++count;}
        seal(wire,count);check(wire,before,expected,atlas);
    }
    {
        auto before=background(2,2);render::SceneRenderer scene(renderer,resources,before);
        auto unsafe=envelope(2,2);tintRecord(unsafe,point,0,0,{0,0,2,2},palette,0,{255,0,0});seal(unsafe,1);
        rejected([&]{scene.draw(legacy::worldDisplay(legacy::decodeWorldFrame(unsafe),bindings));});
        auto safe=envelope(2,2);tintRecord(safe,point,0,1,{0,1,2,2},palette,0,{255,0,0});seal(safe,1);
        auto expected=before;expected.pixels[1]=expected.pixels[2]=0xf800;
        scene.draw(legacy::worldDisplay(legacy::decodeWorldFrame(safe),bindings));
        require(scene.read().pixels==expected.pixels,"Tint pair did not cross the row/clip boundary");
        check(safe,before,expected,true);
        rejected([&]{scene.draw(legacy::worldDisplay(legacy::decodeWorldFrame(unsafe),bindings));});
        require(scene.read().pixels==expected.pixels,"Unsafe tint admission changed the previous canvas");
    }
    {
        // Backend9 consumes RLE coverage even for an indexed source. Its actual
        // cdecl distortion entry and normalized table must survive a top clip.
        auto wire=envelope(37,19);record(wire,holes,7,-4,{0,0,37,19},palette);wire.chop(512);
        put(wire,80,64+holes.encoded.size()+64);put(wire,84,MNM_WORLD_WAVE);put(wire,120,64);put(wire,132,9);put(wire,140,16);
        for(unsigned row=0;row<16;++row){const auto at=wire.size();wire.append(QByteArray(4,0));put(wire,at,row%7);}
        seal(wire,1);auto before=background(37,19),expected=before;
        for(int row=0;row<holes.height;++row)for(int col=0;col<holes.width;++col){
            const int x=4+col,y=-2+row;if(y<0||y>=19||!holes.mask[row*holes.width+col])continue;
            expected.pixels[y*37+x]=before.pixels[y*37+x+unsigned(y)%16%7];
        }
        check(wire,before,expected,true);
    }
    require(renderer.stats().surfaces==0,"Tint GPU lifetime leak");std::cout<<cases<<" tint fixtures pass\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
