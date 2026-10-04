#include "wizard.hpp"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <stdexcept>

using namespace mnm::assets;
namespace {
QString string(const std::string& s) {return QString::fromLatin1(s.data(),static_cast<qsizetype>(s.size()));}
template<class T> QJsonValue optional(const std::optional<T>& v) {return v ? QJsonValue(*v) : QJsonValue(QJsonValue::Null);}
template<> QJsonValue optional(const std::optional<std::string>& v) {return v ? QJsonValue(string(*v)) : QJsonValue(QJsonValue::Null);}
QJsonObject stats(const WizardStats& s) {
    QJsonObject r{{"name",string(s.name)},{"wizardanimfile",optional(s.animationFile)}};
    const std::pair<const char*,std::optional<std::int32_t> WizardStats::*> fields[]={
        {"maxhealth",&WizardStats::maxHealth},{"maxmana",&WizardStats::maxMana},
        {"intelligence",&WizardStats::intelligence},{"magicresistance",&WizardStats::magicResistance},
        {"controllimit",&WizardStats::controlLimit},{"combatmodifier",&WizardStats::combatModifier},
        {"difficultymodifier",&WizardStats::difficultyModifier},{"aggressiveattack",&WizardStats::aggressiveAttack},
        {"cautiousattack",&WizardStats::cautiousAttack},{"occupypowerpoint",&WizardStats::occupyPowerPoint},
        {"collectmana",&WizardStats::collectMana},{"evadedetection",&WizardStats::evadeDetection},
        {"scout",&WizardStats::scout},{"retreat",&WizardStats::retreat},
        {"collectfood",&WizardStats::collectFood},{"protectwizard",&WizardStats::protectWizard},
        {"starttalismans_law",&WizardStats::startTalismansLaw},{"starttalismans_neutral",&WizardStats::startTalismansNeutral},
        {"starttalismans_chaos",&WizardStats::startTalismansChaos}};
    for(const auto& f:fields) r[f.first]=optional(s.*f.second);
    return r;
}
QJsonObject action(const WizardAction& a) {
    QJsonObject r{{"valid",optional(a.valid)},{"nodetype",optional(a.nodeType)},
        {"targetnodetype",optional(a.targetNodeType)},{"destination",optional(a.destination)},{"spelltarget",optional(a.spellTarget)}};
    const std::pair<const char*,std::optional<std::int32_t> WizardAction::*> fields[]={
        {"begintime",&WizardAction::beginTime},{"endtime",&WizardAction::endTime},
        {"dependancy",&WizardAction::dependency},{"waitingtime",&WizardAction::waitingTime},
        {"rating",&WizardAction::rating},{"numberofactions",&WizardAction::numberOfActions},
        {"health",&WizardAction::health},{"mana",&WizardAction::mana},
        {"sectionid",&WizardAction::sectionId},{"spelltype",&WizardAction::spellType},
        {"spelltargetid",&WizardAction::spellTargetId},{"xposition",&WizardAction::xPosition},
        {"yposition",&WizardAction::yPosition},{"zposition",&WizardAction::zPosition},
        {"xtargetposition",&WizardAction::xTargetPosition},{"ytargetposition",&WizardAction::yTargetPosition},
        {"ztargetposition",&WizardAction::zTargetPosition}};
    for(const auto& f:fields) r[f.first]=optional(a.*f.second);
    return r;
}
QJsonObject selection(const std::map<std::uint32_t,bool>& m) {QJsonObject r;for(const auto& e:m) r[QString::number(e.first)]=e.second;return r;}
}
int main(int argc,char** argv) try {
    QCoreApplication app(argc,argv);
    if(argc!=3) throw std::runtime_error("Usage: mnm-wizard-inspect ROOT PATH.wzd");
    auto configured=AssetStore::create(argv[1]);
    if(const auto* error=std::get_if<Error>(&configured)) throw std::runtime_error(error->detail);
    const auto store=std::get<AssetStore>(std::move(configured));auto opened=store.open(argv[2]);
    if(const auto* error=std::get_if<Error>(&opened)) throw std::runtime_error(error->detail);
    auto file=std::get<std::unique_ptr<AssetFile>>(std::move(opened));auto result=loadWizard(*file);file.reset();
    if(const auto* error=std::get_if<PersistenceError>(&result)) throw std::runtime_error(error->detail+" at byte "+std::to_string(error->offset));
    const auto& w=std::get<WizardDefinition>(result);QJsonObject items,actions,config;
    for(const auto& item:w.items) items[QString::number(item.first)]=QJsonObject{{"hasit",optional(item.second.hasIt)},{"researched",optional(item.second.researched)}};
    for(const auto& a:w.actions) actions[QString::number(a.first)]=action(a.second);
    for(const auto& section:w.config.sections) {QJsonObject fields;for(const auto& f:section.second) fields[string(f.first)]=string(f.second);config[string(section.first)]=fields;}
    const auto hash=QCryptographicHash::hash(QByteArray(reinterpret_cast<const char*>(w.source.data()),static_cast<qsizetype>(w.source.size())),QCryptographicHash::Sha256);
    const QJsonObject out{{"general",stats(w.stats)},{"validwizardfile",optional(w.validWizardFile)},
        {"items",items},{"start_spells",selection(w.startSpells)},{"start_objects",selection(w.startObjects)},
        {"start_magic_items",selection(w.startMagicItems)},{"actions",actions},{"config",config},
        {"source_sha256",QString::fromLatin1(hash.toHex())}};
    std::cout<<QJsonDocument(out).toJson().constData();return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 2;}
