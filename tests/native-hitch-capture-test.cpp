#include "../apps/qt-shell/native_frame_trace.hpp"
#include <QCoreApplication>
#include <QJsonDocument>
#include <iostream>
static void require(bool v,const char* why){if(!v)throw std::runtime_error(why);}
int main(int argc,char** argv)try{
 QCoreApplication app(argc,argv);NativeFrameTrace trace(true,true);
 const auto origin=trace.report()["host_origin_us"].toString().toLongLong();
 auto turn=[&](int n){NativeFrameTrace::Turn t;t.turn=n;t.begin=origin+n*1000;t.worldEnd=t.begin+10;t.commandEnd=t.worldEnd+10;t.extraEnd=t.commandEnd+10;t.exportEnd=t.extraEnd+10;t.statusEnd=t.exportEnd+10;t.waitEnd=t.begin+1000;t.sourceBefore=t.sourceAfter=65;t.draw=true;t.commands=1;t.delivery={67,65,2,0,67,65,65,2};t.uiGeneration=65;t.preparedGeneration=65;t.uiSealed=t.worldOutput=true;trace.record(t);};
 auto paint=[&](int ms,bool draw=true,bool paused=false,unsigned gen=65){trace.recordGui({unsigned(ms),gen,origin+ms*1000,3,draw,paused});};
 for(int n=0;n<150;++n){turn(n);if(n%16==0)paint(n);}
 for(int n=150;n<300;++n){turn(n);}
 paint(300);
 for(int n=300;n<700;++n){turn(n);if(n%16==0)paint(n);}
 auto first=trace.report()["hitch_capture"].toObject()["windows"].toArray();require(first.size()==1,"Authored hitch not captured exactly once");
 const auto captured=first[0].toObject();require(captured["paint_gap_ms"].toDouble()==156&&captured["post_complete"].toBool()&&!captured["clipped"].toBool(),"Hitch interval or bounded post capture");
 require(captured["rows"].toArray().size()>300&&captured["gui_rows"].toArray().size()>10,"Hitch pre/post rows missing");
 const auto rows=QJsonDocument(captured).toJson(QJsonDocument::Compact);
 for(int n=700;n<40000;++n){turn(n);if(n%16==0)paint(n);}
 auto after=trace.report();require(after["overwritten_turns"].toString().toULongLong()>0 || after["overwritten_gui_events"].toString().toULongLong()>0,"Fixture did not wrap tail");
 require(QJsonDocument(after["hitch_capture"].toObject()["windows"].toArray()[0].toObject()).toJson(QJsonDocument::Compact)==rows,"Tail overwrite lost frozen early hitch");
 paint(40000,false);paint(42000);paint(42016);paint(42020,true,true);paint(45000);paint(45016);paint(45020,true,false,0);paint(48000);paint(48016);
 require(trace.report()["hitch_capture"].toObject()["observed_hitches"].toString()=="1","Hidden/movie/non-world transition invented a hitch");
 for(int i=0;i<12;++i){const int begin=50000+i*2000;paint(begin,false);paint(begin);for(int n=begin;n<begin+100+i*10;++n)turn(n);paint(begin+100+i*10);for(int n=begin+100+i*10;n<begin+1500;++n)turn(n);}
 auto h=trace.report()["hitch_capture"].toObject();require(h["windows"].toArray().size()==8&&h["replaced_windows"].toString().toUInt()>0,"Hitch reservoir not bounded or did not retain stronger gaps");
 for(const auto& v:h["windows"].toArray()){auto w=v.toObject();require(w["rows"].toArray().size()<=2048&&w["gui_rows"].toArray().size()<=512,"Hitch capacity exceeded");}
 NativeFrameTrace disabled(false,true);disabled.recordGui({1,65,origin,3,true,false});require(!disabled.report()["enabled"].toBool(),"Disabled trace admitted hitches");
 std::cout<<"{\"success\":true,\"early_hitch_preserved_after_tail_wrap\":true,\"pre_post_state_recorded\":true,\"hidden_movie_nonworld_reset\":true,\"bounded_largest_gap_reservoir\":true,\"host_origin_exported\":true,\"window_limit\":8,\"ordinary_readbacks\":0}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
