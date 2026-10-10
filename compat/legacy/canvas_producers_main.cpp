#include "canvas_producers.hpp"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
namespace {
QByteArray read(const QString &name, qint64 maximum = 128 * 1024 * 1024) {
  QFile f(name);
  if (!f.open(QIODevice::ReadOnly) || f.size() > maximum)
    throw std::runtime_error("Cannot read bounded native input: " +
                             name.toStdString());
  return f.readAll();
}
std::vector<std::uint8_t> bytes(const QByteArray &b) {
  return {b.begin(), b.end()};
}
void save(const QString &name, const QByteArray &b) {
  QFile f(name);
  if (f.exists() || !f.open(QIODevice::WriteOnly | QIODevice::NewOnly) ||
      f.write(b) != b.size())
    throw std::runtime_error("Cannot write native output: " +
                             name.toStdString());
}
} // namespace
int main(int argc, char **argv) {
  QCoreApplication app(argc, argv);
  if (argc != 4) {
    std::cerr << "Usage: mnm-canvas-producers-preview INPUT.bin SOURCES.json "
                 "OUTPUT_DIRECTORY\n";
    return 2;
  }
  QJsonObject report;
  QJsonArray checkpoints;
  unsigned sequence = 0;
  try {
    const auto input = read(QString::fromLocal8Bit(argv[1]), 512 * 1024 * 1024);
    const auto stream = mnm::legacy::decodeCanvasProducers(bytes(input));
    const auto sourcesPath = QString::fromLocal8Bit(argv[2]);
    const auto document = QJsonDocument::fromJson(read(sourcesPath));
    if (!document.isObject())
      throw std::invalid_argument("Invalid native source bindings");
    const auto sources = document.object();
    QDir root(QFileInfo(sourcesPath).absolutePath());
    QDir out(QString::fromLocal8Bit(argv[3]));
    if (!out.exists() && !QDir().mkpath(out.absolutePath()))
      throw std::runtime_error("Cannot create native output directory");
    mnm::legacy::CanvasProducerReplay replay([&](const std::string &name) {
      const auto binding =
          sources.value(QString::fromStdString(name)).toObject();
      const auto path = binding.value("file").toString();
      if (path.isEmpty() || path.contains('/') || path.contains('\\') ||
          path == "." || path == "..")
        throw std::invalid_argument("Invalid closed source binding");
      auto source = read(root.filePath(path));
      if (QCryptographicHash::hash(source, QCryptographicHash::Sha256)
              .toHex() != binding.value("sha256").toString().toLatin1())
        throw std::invalid_argument("Encoded source fingerprint changed");
      return bytes(source);
    });
    for (const auto &c : stream.operations) {
      sequence = c.fields[1];
      replay.apply(c);
      if (c.fields[2] == 10) {
        QJsonObject checkpoint{{"sequence", int(sequence)},
                               {"oracle", int(c.fields[14])},
                               {"canvas", int(c.fields[3])}};
        try {
          auto image = replay.read(c.fields[3]);
          if (image.width != int(c.fields[5]) ||
              image.height != int(c.fields[6]))
            throw std::runtime_error("Native checkpoint extent changed");
          QByteArray raw;
          raw.reserve(image.pixels.size() * 2);
          for (auto word : image.pixels) {
            raw.append(char(word));
            raw.append(char(word >> 8));
          }
          auto name =
              QString("native-%1.565").arg(c.fields[14], 4, 10, QChar('0'));
          save(out.filePath(name), raw);
          checkpoint["path"] = name;
          checkpoint["sha256"] = QString::fromLatin1(
              QCryptographicHash::hash(raw, QCryptographicHash::Sha256)
                  .toHex());
          checkpoint["completed"] = true;
        } catch (const std::exception &e) {
          checkpoint["completed"] = false;
          checkpoint["error"] = e.what();
        }
        checkpoints.append(checkpoint);
      }
    }
    report["success"] = true;
    report["queues"] = int(stream.queues);
    report["records"] = int(sequence);
  } catch (const std::exception &e) {
    report["success"] = false;
    report["failed_sequence"] = int(sequence);
    report["error"] = e.what();
  }
  report["original_pixels_used_as_native_inputs"] = false;
  report["checkpoints"] = checkpoints;
  QDir out(QString::fromLocal8Bit(argv[3]));
  QDir().mkpath(out.absolutePath());
  try {
    save(out.filePath("native-report.json"), QJsonDocument(report).toJson());
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
  std::cout
      << QJsonDocument(report).toJson(QJsonDocument::Compact).toStdString()
      << '\n';
  return report.value("success").toBool() ? 0 : 1;
}
