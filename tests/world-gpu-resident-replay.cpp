#include "../compat/legacy/world_rolling_session.hpp"
#include <QGuiApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
using namespace mnm;
std::vector<std::uint8_t> bytes(const QString &path) {
  QFile file(path);
  if(!file.open(QIODevice::ReadOnly)||file.size()>128*1024*1024)
    throw std::runtime_error("Unavailable bounded source corpus");
  const auto raw=file.readAll();return {raw.begin(),raw.end()};
}
int main(int argc,char **argv) try {
  QGuiApplication app(argc,argv);
  if(argc!=4)return 2;
  const auto source=legacy::decodeCanvasProducers(bytes(argv[1]));
  auto opened=assets::AssetStore::create(argv[2]);
  if(auto *error=std::get_if<assets::Error>(&opened))
    throw std::runtime_error(error->detail);
  const auto store=std::get<assets::AssetStore>(std::move(opened));
  render::GlBlitter renderer;
  std::uint64_t queues=0,words=0,rasters=0,deliveryReadbacks=0;
  {
    legacy::WorldRollingSession reference(renderer,store,true),resident(renderer,store,false,true);
    std::vector<legacy::CanvasProducer> pending;
    for(const auto &operation:source.operations) {
      pending.push_back(operation);
      if(operation.fields[2]!=12)continue;
      const auto packet=legacy::encodeRollingQueue(pending);
      const auto owned=legacy::decodeRollingQueue(packet);
      const auto expected=reference.consumeOwned(++queues,owned);
      const auto before=renderer.stats().nativeReadbacks;
      const auto frame=resident.consumeResident(queues,owned);
      deliveryReadbacks+=renderer.stats().nativeReadbacks-before;
      if(!frame.valid()||deliveryReadbacks||resident.worldReadbacks())
        throw std::runtime_error("Ordinary resident delivery exported GPU pixels");
      // This explicit read is a comparison diagnostic, outside normal delivery.
      const auto actual=resident.diagnosticReadResident(renderer);
      if(actual.width!=expected.width||actual.height!=expected.height||actual.pixels!=expected.pixels)
        throw std::runtime_error("Resident history differs at original queue "+std::to_string(queues));
      words+=actual.pixels.size();
      for(const auto &c:owned.operations)if(c.fields[2]==9)++rasters;
      pending.clear();
    }
    if(!pending.empty()||queues!=source.queues)
      throw std::runtime_error("Incomplete resident corpus");
  }
  if(renderer.stats().surfaces)
    throw std::runtime_error("Resident resources survived teardown");
  QFile report(argv[3]);
  const QJsonObject result{{"success",true},{"queues",qint64(queues)},
    {"words_compared",qint64(words)},{"world_rasters",qint64(rasters)},
    {"delivery_readbacks",qint64(deliveryReadbacks)},{"world_readbacks",0},
    {"diagnostic_readbacks",qint64(queues)},{"remaining_surfaces",0},
    {"original_destination_pixels_used_as_inputs",false},
    {"independent_original_equivalence",false},
    {"scope","Retained original source corpus, current native GPU residency versus independent native CPU composition. Original body suppression and independent original pixels are separate proofs."}};
  if(!report.open(QIODevice::WriteOnly|QIODevice::NewOnly)||report.write(QJsonDocument(result).toJson())<0)
    throw std::runtime_error("Cannot retain resident comparison");
  std::cout<<queues<<" retained original queues, "<<words<<" compared words, zero delivery readbacks\n";
  return 0;
} catch(const std::exception &error) {std::cerr<<error.what()<<'\n';return 1;}
