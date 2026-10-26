#include "../compat/legacy/drawing_scene.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <algorithm>
#include <iostream>
using namespace mnm;
using namespace resource_test;
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);require(argc==3,"Expected owned input directory and new native output directory");
 const auto inputs=std::filesystem::path(argv[1]),outputs=std::filesystem::path(argv[2]);
 require(!std::filesystem::exists(outputs),"Refusing output overwrite");std::filesystem::create_directory(outputs);
 QTemporaryDir empty;require(empty.isValid(),"Asset-free fixture root");const auto assets=store(std::filesystem::path(empty.path().toStdString()));render::GlBlitter renderer;
 std::vector<std::filesystem::path> paths;for(const auto& e:std::filesystem::directory_iterator(inputs))if(e.path().extension()==".bin")paths.push_back(e.path());std::sort(paths.begin(),paths.end());require(!paths.empty()&&paths.size()<=256,"Fixture count");
 for(const auto& path:paths){
  require(std::filesystem::file_size(path)<=128*1024,"Fixture input budget");std::ifstream f(path,std::ios::binary);Bytes bytes((std::istreambuf_iterator<char>(f)),{});
  const auto stream=legacy::decodeCanvasProducers(bytes,false);require(stream.operations.size()>=5&&stream.operations.back().fields[2]==12,"Expected bounded World fixture");
  const auto& created=stream.operations.front().fields;require(created[2]==1,"Missing fixture canvas");
  legacy::DrawingScene scene(renderer,assets,[](const std::string&)->Bytes{throw std::runtime_error("Colour fixture requested an asset");});
  for(const auto& command:stream.operations)scene.apply(command);
  scene.seal({stream.operations.back().fields[1],1,created[3],created[5],created[6]});const auto image=scene.read();
  std::ofstream out(outputs/path.filename(),std::ios::binary);for(auto v:image.pixels){out.put(char(v));out.put(char(v>>8));}require(bool(out),"Native output write");
 }
 for(unsigned bad=0;bad<5;++bad){
  legacy::DrawingScene scene(renderer,assets,[](const std::string&)->Bytes{throw std::runtime_error("Unexpected asset");});
  auto submit=[&](legacy::CanvasProducer c){c.fields[1]=scene.sequence()+1;c.fields[0]=96+c.bytes().size();c.fields[18]=c.bytes().size();scene.apply(c);};
  legacy::CanvasProducer c{};c.fields[2]=1;c.fields[3]=1;c.fields[5]=c.fields[6]=bad==4?2048:8;submit(c);
  if(bad==3){c.fields[3]=2;submit(c);c.fields[3]=1;}
  c.fields[2]=5;c.fields[12]=c.fields[5];c.fields[13]=c.fields[6];c.fields[14]=0x1234;submit(c);
  c.fields[2]=11;c.fields[14]=1;submit(c);c.fields[2]=5;c.fields[14]=0x4321;
  if(bad==0)c.fields[14]=65536;
  if(bad==1)c.fields[10]=c.fields[12]+1;
  if(bad==2)c.payload={1};
  if(bad==3)c.fields[3]=2;
  if(bad==4)for(unsigned n=0;n<12;++n)submit(c);
  rejected([&]{submit(c);});rejected([&]{scene.read();});
 }
 require(renderer.stats().surfaces==0,"World fixture leaked native surfaces");std::cout<<"{\"success\":true,\"cases\":"<<paths.size()<<",\"refusals\":5,\"remaining_surfaces\":0,\"original_oracles_read\":false}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
