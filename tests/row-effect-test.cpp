#define main additive_fixture_unused_main
#include "additive-sprite-test.cpp"
#undef main
struct RowState {unsigned amplitude,phase;std::array<unsigned,32> table;};
static std::vector<int> effectiveRows(const Fixture& f,int x,int y,render::Rect clip,bool sample,const RowState& state){
    x-=f.originX;y-=f.originY;const bool clipped=x<clip.left||x+f.width>clip.right;
    std::vector<int> rows;
    for(int row=0;row<f.height;++row){const auto phase=state.phase+unsigned(y+row);
        if(clipped){const int centre=sample?8:int(state.amplitude);const auto remainder=sample?phase&15u:phase%(state.amplitude*2);
            rows.push_back(std::abs(centre-int(remainder)));}
        else rows.push_back(int(state.table[phase%(state.amplitude*2)]));
    }
    return rows;
}
static void rowRecord(QByteArray& wire,const Fixture& frame,int x,int y,render::Rect clip,const render::SpriteColourTable& palette,
                      bool sample,const RowState& state){
    const auto at=wire.size();record(wire,frame,x,y,clip,palette);wire.chop(512);
    const auto offsets=effectiveRows(frame,x,y,clip,sample,state);
    put(wire,at,64+frame.encoded.size()+(sample?0:512)+offsets.size()*4);
    put(wire,at+4,sample?MNM_WORLD_DESTINATION_ROWS:MNM_WORLD_SOURCE_ROW_SHIFT);
    put(wire,at+40,(sample?0:512)+offsets.size()*4);put(wire,at+48,state.phase);put(wire,at+52,sample?7:16);put(wire,at+60,frame.height);
    if(!sample)for(auto pixel:palette){wire.append(char(pixel));wire.append(char(pixel>>8));}
    for(auto offset:offsets){const auto where=wire.size();wire.append(QByteArray(4,0));put(wire,where,unsigned(offset));}
}
static void rowApply(render::Image& canvas,const Fixture& f,int x,int y,render::Rect clip,const render::SpriteColourTable& palette,
                     bool sample,const RowState& state){
    const auto rows=effectiveRows(f,x,y,clip,sample,state);const auto before=canvas.pixels;
    x-=f.originX;y-=f.originY;const bool clipped=x<clip.left||x+f.width>clip.right;
    auto get=[&](unsigned at){return unsigned(f.encoded.at(at))|unsigned(f.encoded.at(at+1))<<8|unsigned(f.encoded.at(at+2))<<16|unsigned(f.encoded.at(at+3))<<24;};
    for(int row=0;row<f.height;++row){const int dy=y+row;if(dy<clip.top||dy>=clip.bottom)continue;
        unsigned cursor=0,at=get(40+row*8);
        while(cursor<unsigned(f.width)){cursor+=f.encoded.at(at++);if(cursor==unsigned(f.width))break;
            const auto length=f.encoded.at(at++);require(length&&cursor+length<=unsigned(f.width),"Invalid row oracle RLE");
            int first=int(cursor),last=int(cursor+length);cursor+=length;
            if(sample&&clipped){const int left=std::max(0,clip.left-x),right=std::min(f.width,clip.right-x);
                if(first<left){if(last<=left||last>right)continue;first=left;}
                else if(last>=right)continue;
            }else{first=std::max(first,clip.left-x);last=std::min(last,clip.right-x);}
            for(int col=first;col<last;++col){const auto base=std::size_t(dy)*canvas.width+x+col;
                if(sample)canvas.pixels.at(base)=before.at(base+rows[row]);
                else canvas.pixels.at(base+rows[row])=palette[f.indices[row*f.width+col]];
            }
        }
    }
}
static QByteArray rowStates(const std::vector<RowState>& states){
    QByteArray bytes("MNMSHFT1",8);bytes.append(QByteArray(4,0));put(bytes,8,states.size());
    for(const auto& state:states){auto at=bytes.size();bytes.append(QByteArray(132,0));put(bytes,at,state.amplitude);
        for(unsigned i=0;i<32;++i)put(bytes,at+4+i*4,state.table[i]);}
    return bytes;
}
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);require(argc==2,"Expected fresh row-effect output prefix");const QString prefix=QString::fromLocal8Bit(argv[1]);
    const auto sparse=fixture(17,9,true),dense=fixture(256,9,false),point=fixture(1,1,false);const auto source=spriteFile({sparse,dense,point});
    QTemporaryDir directory;require(directory.isValid(),"Row-effect root unavailable");const auto root=std::filesystem::path(directory.path().toStdString());write(root,"rows.spr",source);
    auto input=store(root);assets::ResourceManager resources(input);legacy::SnapshotResources bindings(input,resources);
    bindings.add({assets::ResourceKind::ui,"rows"},"rows.spr",QCryptographicHash::hash(bytes(source),QCryptographicHash::Sha256).toHex());
    render::SpriteColourTable palette;for(unsigned i=0;i<256;++i)palette[i]=(i*7919+0x1234)&65535;
    palette[0]=0;render::GlBlitter renderer;unsigned cases=0;
    auto check=[&](const QByteArray& wire,const std::vector<RowState>& states,const render::Image& before,const render::Image& expected,bool atlas){
        const auto frame=legacy::decodeWorldFrame(wire);const auto draws=legacy::worldDisplay(frame,bindings);
        if(atlas){legacy::WorldPreparation job(input);job.plan(wire);const auto plan=std::get<legacy::WorldPreparationPlan>(await(job));
            require(plan.files.size()==1,"Row effect lost resource plan");resources.bind(plan.files[0].id,spr(plan.files[0].path));
            job.prepare({resources.request(plan.files[0].id)},16*1024*1024,8);const auto ready=std::get<legacy::PreparedWorld>(await(job));
            require(ready.draws.size()==draws.size(),"Row worker lost draws");
            for(std::size_t i=0;i<draws.size();++i)require(ready.draws[i].rowEffect&&ready.draws[i].rowEffect->offsets==draws[i].rowEffect->offsets&&ready.draws[i].rowEffect->kind==draws[i].rowEffect->kind,"Row worker changed effect");
            job.cancel();}
        render::SceneLimits limits;limits.cache.indexedAtlas=atlas;render::SceneRenderer scene(renderer,resources,before,limits);
        const auto reads=renderer.stats().nativeReadbacks;scene.draw(draws);
        require(scene.presentGpu().valid()&&renderer.stats().nativeReadbacks==reads,"Row-effect drawing reads pixels back");
        const auto actual=scene.read();require(actual.pixels==expected.pixels,"Row effect source/sampling, clip, wrapping or order differs");
        const auto name=prefix+"-"+QString::number(cases++);save(name+".bin",wire);save(name+".bin.rows",rowStates(states));save(name+".before.565",pixels(before));save(name+".native.565",pixels(actual));
    };
    const unsigned phases[]{0,1,15,31,0xfffffff8u};
    for(bool sample:{false,true})for(unsigned amplitude=sample?9:1;amplitude<=16;++amplitude)for(unsigned phase:phases)for(unsigned geometry=0;geometry<4;++geometry){
        RowState state{amplitude,phase,{}};for(unsigned i=0;i<32;++i)state.table[i]=(i*7+3)%(amplitude*2+1);
        const bool narrow=geometry>=2;const render::Rect clip=narrow?render::Rect{8,3,64,27}:render::Rect{0,0,83,31};
        const int left=geometry==0?5:geometry==1?63:geometry==2?4:58,top=geometry==0?3:geometry==1?12:geometry==2?-2:18;
        auto wire=envelope(83,31);rowRecord(wire,sparse,left+sparse.originX,top+sparse.originY,clip,palette,sample,state);seal(wire,1);
        auto before=background(83,31),expected=before;rowApply(expected,sparse,left+sparse.originX,top+sparse.originY,clip,palette,sample,state);
        check(wire,{state},before,expected,(amplitude+geometry+phase)%2==0);
        if(phase==0&&geometry==0&&amplitude==(sample?9u:1u)){
            for(const auto& mutation:std::vector<std::pair<unsigned,unsigned>>{{32,84},{84,13},{132,sample?16u:7u},{140,8},{std::uint32_t(144+sparse.encoded.size()+(sample?0:512)),33}}){auto bad=wire;put(bad,mutation.first,mutation.second);rejected([&]{legacy::decodeWorldFrame(bad);});}
        }
    }
    for(bool sample:{false,true}){RowState state{16,0xfffffff8u,{}};for(unsigned i=0;i<32;++i)state.table[i]=i;
        auto wire=envelope(300,19);rowRecord(wire,dense,1,2,{0,0,300,19},palette,sample,state);seal(wire,1);
        auto before=background(300,19),expected=before;rowApply(expected,dense,1,2,{0,0,300,19},palette,sample,state);check(wire,{state},before,expected,true);}
    for(bool firstSample:{false,true}){RowState state{12,3,{}};for(unsigned i=0;i<32;++i)state.table[i]=(i*5)%25;
        auto wire=envelope(83,31);auto before=background(83,31),expected=before;
        rowRecord(wire,sparse,8,4,{0,0,83,31},palette,firstSample,state);rowApply(expected,sparse,8,4,{0,0,83,31},palette,firstSample,state);
        rowRecord(wire,sparse,10,5,{0,0,83,31},palette,true,state);rowApply(expected,sparse,10,5,{0,0,83,31},palette,true,state);
        seal(wire,2);check(wire,{state,state},before,expected,true);}
    for(bool sample:{false,true}){RowState state{16,1,{}};state.table.fill(32);
        auto wire=envelope(1,40);rowRecord(wire,point,0,1,{0,0,1,40},palette,sample,state);seal(wire,1);
        auto before=background(1,40),expected=before;rowApply(expected,point,0,1,{0,0,1,40},palette,sample,state);
        check(wire,{state},before,expected,true);}
    // A covered last-row shift must fail before changing an admitted canvas.
    for(bool sample:{false,true}){RowState state{16,0,{}};state.table.fill(32);
        auto before=background(4,2);render::SceneRenderer scene(renderer,resources,before);scene.draw({});const auto previous=scene.read().pixels;
        auto wire=envelope(4,2);rowRecord(wire,point,3,1,{0,0,4,2},palette,sample,state);seal(wire,1);
        rejected([&]{scene.draw(legacy::worldDisplay(legacy::decodeWorldFrame(wire),bindings));});
        require(scene.read().pixels==previous,"Rejected row span changed completed pixels");}
    require(cases==486,"Incomplete row-effect matrix");require(renderer.stats().surfaces==0,"Row-effect GPU lifetime leak");
    std::cout<<cases<<" row-effect fixtures pass\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
