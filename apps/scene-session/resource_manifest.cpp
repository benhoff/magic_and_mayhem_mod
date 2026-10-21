#include "resource_manifest.hpp"
#include "../../assets/palette_lighting.hpp"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <cmath>
#include <set>
#include <stdexcept>
namespace mnm::scene {
namespace {
using O = QJsonObject;
void keys(const O &o, std::initializer_list<const char *> allowed) {
  for (auto i = o.begin(); i != o.end(); ++i) {
    bool found = false;
    for (auto k : allowed)
      found |= i.key() == k;
    if (!found)
      throw std::invalid_argument("Unknown scene manifest field: " +
                                  i.key().toStdString());
  }
}
O object(QJsonValue v) {
  if (!v.isObject())
    throw std::invalid_argument("Scene manifest expected object");
  return v.toObject();
}
QJsonArray array(QJsonValue v, std::size_t limit) {
  if (!v.isArray() || std::size_t(v.toArray().size()) > limit)
    throw std::invalid_argument("Scene manifest array bounds");
  return v.toArray();
}
std::string text(QJsonValue v) {
  if (!v.isString() || v.toString().isEmpty() || v.toString().size() > 4096)
    throw std::invalid_argument("Scene manifest string");
  return v.toString().toStdString();
}
int integer(QJsonValue v, int lo, int hi) {
  if (!v.isDouble() || !std::isfinite(v.toDouble()) ||
      v.toDouble() != std::floor(v.toDouble()) || v.toDouble() < lo ||
      v.toDouble() > hi)
    throw std::invalid_argument("Scene manifest integer bounds");
  return int(v.toDouble());
}
bool boolean(QJsonValue v) {
  if (!v.isBool())
    throw std::invalid_argument("Scene manifest boolean");
  return v.toBool();
}
assets::ResourceKind kind(const std::string &s) {
  if (s == "creature")
    return assets::ResourceKind::creature;
  if (s == "terrain")
    return assets::ResourceKind::terrain;
  if (s == "effect")
    return assets::ResourceKind::effect;
  if (s == "ui")
    return assets::ResourceKind::ui;
  throw std::invalid_argument("Scene resource kind");
}
game::Family family(const std::string &s) {
  if (s == "creature")
    return game::Family::creature;
  if (s == "effect")
    return game::Family::missileEffect;
  if (s == "object")
    return game::Family::mapLinked;
  throw std::invalid_argument("Scene family");
}
assets::ResourceId id(QJsonValue v) {
  auto o = object(v);
  keys(o, {"kind", "name"});
  return {kind(text(o["kind"])), text(o["name"])};
}
std::unique_ptr<assets::AssetFile> open(const assets::AssetStore &s,
                                        const std::string &p) {
  auto r = s.open(p);
  if (auto *e = std::get_if<assets::Error>(&r))
    throw std::runtime_error(e->detail);
  return std::get<std::unique_ptr<assets::AssetFile>>(std::move(r));
}
template <class T, class E> T decoded(std::variant<T, E> r) {
  if (auto *e = std::get_if<E>(&r))
    throw std::runtime_error(e->detail);
  return std::get<T>(std::move(r));
}
} // namespace
SceneManifest decodeSceneManifest(const assets::Bytes &bytes,
                                  const assets::AssetStore &store) {
  if (bytes.size() > 1024 * 1024)
    throw std::length_error("Scene manifest exceeds 1 MiB");
  QJsonParseError error;
  auto doc = QJsonDocument::fromJson(
      QByteArray(reinterpret_cast<const char *>(bytes.data()),
                 qsizetype(bytes.size())),
      &error);
  if (error.error != QJsonParseError::NoError || !doc.isObject())
    throw std::invalid_argument("Invalid scene manifest JSON");
  auto root = doc.object();
  keys(root, {"version", "map", "mps", "terrain", "terrainAnimation", "assets",
              "types", "viewport", "camera", "lighting", "palette",
              "visibility", "nativeAnimations"});
  if (integer(root["version"], 1, 1) != 1)
    throw std::invalid_argument("Scene manifest version");
  SceneManifest result;
  result.camera.span = 0;
  if (root.contains("map"))
    result.map = text(root["map"]);
  if (root.contains("mps"))
    result.mps = text(root["mps"]);
  result.terrain = id(root["terrain"]);
  if (root.contains("terrainAnimation"))
    result.terrainAnimation = text(root["terrainAnimation"]);
  std::set<assets::ResourceId> ids;
  for (auto v : array(root["assets"], 1024)) {
    auto o = object(v);
    keys(o, {"id", "image", "format", "animation", "terrainCatalog"});
    SceneAssetBinding b;
    b.id = id(o["id"]);
    b.recipe.image = text(o["image"]);
    if (!ids.insert(b.id).second)
      throw std::invalid_argument("Duplicate scene resource identity");
    if (o.contains("animation"))
      b.recipe.animation = text(o["animation"]);
    if (o.contains("terrainCatalog"))
      b.recipe.terrainCatalog = text(o["terrainCatalog"]);
    if (o.contains("format")) {
      const auto f = text(o["format"]);
      if (f == "sprite")
        b.recipe.format = assets::ResourceImageFormat::sprite;
      else if (f == "bmp")
        b.recipe.format = assets::ResourceImageFormat::bmp;
      else if (f == "pcx")
        b.recipe.format = assets::ResourceImageFormat::pcx;
      else if (f == "jpeg")
        b.recipe.format = assets::ResourceImageFormat::jpeg;
      else
        throw std::invalid_argument("Scene image format");
    }
    result.assets.push_back(std::move(b));
  }
  if (!ids.count(result.terrain))
    throw std::invalid_argument("Undeclared terrain resource");
  if (root.contains("types"))
    for (auto v : array(root["types"], 4096)) {
      auto o = object(v);
      keys(o, {"family", "type", "owner", "width", "directional", "layers"});
      SceneTypeProfile p;
      p.family = family(text(o["family"]));
      p.type = integer(o["type"], 0, 65535);
      if (o.contains("owner"))
        p.owner = std::uint32_t(integer(o["owner"], -1, 7));
      if (o.contains("width"))
        p.width = integer(o["width"], 1, 128);
      if (o.contains("directional"))
        p.directional = boolean(o["directional"]);
      std::set<unsigned> channels;
      for (auto item : array(o["layers"], 16)) {
        auto l = object(item);
        keys(l, {"channel", "resource", "sequence", "frame", "kind", "priority",
                 "lit", "livingOnly", "attachment"});
        SceneLayerSource layer;
        layer.channel = integer(l["channel"], 0, 15);
        if (!channels.insert(layer.channel).second)
          throw std::invalid_argument("Duplicate scene channel");
        layer.animation.resource = id(l["resource"]);
        if (!ids.count(layer.animation.resource))
          throw std::invalid_argument("Undeclared scene layer resource");
        if (l.contains("sequence"))
          layer.animation.sequence = integer(l["sequence"], 0, 65535);
        if (l.contains("frame")) {
          if (l.contains("sequence"))
            throw std::invalid_argument(
                "Static scene frame and ANI sequence conflict");
          layer.animation.fixedFrame = integer(l["frame"], 0, 65535);
        }
        if (l.contains("kind"))
          layer.kind = integer(l["kind"], 0, 2);
        if (l.contains("priority"))
          layer.priority = integer(l["priority"], 0, 255);
        if (l.contains("lit"))
          layer.lit = boolean(l["lit"]);
        if (l.contains("livingOnly"))
          layer.livingOnly = boolean(l["livingOnly"]);
        if (l.contains("attachment")) {
          auto a = text(l["attachment"]);
          if (a == "first")
            layer.attachment = reconstruction::AttachmentPoint::first;
          else if (a == "second")
            layer.attachment = reconstruction::AttachmentPoint::second;
          else
            throw std::invalid_argument("Scene attachment role");
        }
        p.layers.push_back(std::move(layer));
      }
      if (p.layers.empty() ||
          p.layers.front().channel !=
              (p.family == game::Family::missileEffect ? 3u : 0u))
        throw std::invalid_argument("Scene profile requires body channel");
      for (const auto &prior : result.types)
        if (prior.family == p.family && prior.type == p.type &&
            prior.owner == p.owner)
          throw std::invalid_argument("Duplicate scene type profile");
      result.types.push_back(std::move(p));
    }
  if (root.contains("viewport")) {
    auto v = array(root["viewport"], 2);
    if (v.size() != 2)
      throw std::invalid_argument("Scene viewport requires width,height");
    result.viewport = {0, 0, integer(v[0], 1, 2048), integer(v[1], 1, 2048)};
  }
  if (root.contains("camera")) {
    auto o = object(root["camera"]);
    keys(o, {"view", "column", "row", "cutLevel", "autoFocus"});
    if (o.contains("view"))
      result.camera.view = integer(o["view"], 0, 3);
    if (o.contains("column"))
      result.camera.column = integer(o["column"], -133, 133);
    if (o.contains("row"))
      result.camera.row = integer(o["row"], -133, 133);
    if (o.contains("cutLevel"))
      result.camera.cutLevel = integer(o["cutLevel"], 0, 32);
    if (o.contains("autoFocus"))
      result.autoFocus = boolean(o["autoFocus"]);
  }
  if (root.contains("visibility"))
    result.visibility = boolean(root["visibility"]);
  if (root.contains("lighting")) {
    auto o = object(root["lighting"]);
    keys(o, {"enabled", "player", "creatures", "objects", "effectConfig",
             "terrainConfig", "objectTypes"});
    if (o.contains("enabled"))
      result.useLighting = boolean(o["enabled"]);
    if (o.contains("player"))
      result.lightPlayer = integer(o["player"], -1, 7);
    if (o.contains("creatures"))
      result.creatureLights = boolean(o["creatures"]);
    if (o.contains("objects"))
      result.objectLightsEnabled = boolean(o["objects"]);
    if (o.contains("effectConfig")) {
      auto f = open(store, text(o["effectConfig"]));
      result.effectLights = decoded(assets::loadEffectLightingFields(*f));
    }
    if (o.contains("terrainConfig")) {
      auto f = open(store, text(o["terrainConfig"]));
      auto fields = decoded(assets::loadTerrainLightingFields(*f));
      result.lighting = reconstruction::applyTerrainLighting(
          result.lighting, fields.ambientLight, fields.lightRamp);
    }
    if (o.contains("objectTypes"))
      for (auto value : array(o["objectTypes"], 4096)) {
        auto t = object(value);
        keys(t, {"family", "type", "effectType"});
        result.objectLights.push_back(
            {family(text(t["family"])), unsigned(integer(t["type"], 0, 65535)),
             unsigned(integer(t["effectType"], 0, 88))});
      }
  }
  if (root.contains("palette")) {
    auto o = object(root["palette"]);
    keys(o,
         {"modes", "ordinal", "globalConfig", "effectProfile", "preferences"});
    if (o.contains("modes")) {
      result.paletteModes.clear();
      for (auto mode : array(o["modes"], 10))
        result.paletteModes.push_back(integer(mode, 0, 9));
    }
    if (o.contains("ordinal"))
      result.paletteOrdinal = integer(o["ordinal"], 0, 9);
    if (o.contains("globalConfig")) {
      auto f = open(store, text(o["globalConfig"]));
      auto fields = decoded(assets::loadPaletteLightingFields(*f));
      result.shading = reconstruction::applyPaletteLighting(
          result.shading, {fields.lightCurve, fields.colourFactor,
                           fields.lightPower, fields.colourPower});
    }
    if (o.contains("effectProfile")) {
      auto f = open(store, text(o["effectProfile"]));
      result.effects = paletteEffectProfile(decoded(assets::loadProfile(*f)));
    }
    if (o.contains("preferences")) {
      auto f = open(store, text(o["preferences"]));
      auto fields = decoded(assets::loadTerrainPalettePreference(*f));
      result.shading = reconstruction::applyTerrainPalettePreference(
          result.shading, fields.lightLevels);
    }
    if (result.paletteOrdinal &&
        *result.paletteOrdinal >= result.paletteModes.size())
      throw std::invalid_argument(
          "Scene palette ordinal outside declared chain");
  }
  if (root.contains("nativeAnimations"))
    for (auto v : array(root["nativeAnimations"], 1024)) {
      auto resource = id(v);
      if (!ids.count(resource))
        throw std::invalid_argument("Undeclared native animation resource");
      result.nativeAnimations.push_back(resource);
    }
  return result;
}
} // namespace mnm::scene
