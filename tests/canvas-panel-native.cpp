#include "../renderer/canvas_sequence.hpp"
#include <fstream>
#include <iostream>
int main(int argc, char **argv) try {
  if (argc != 3)
    throw std::runtime_error("Expected panel kind, output");
  mnm::render::CanvasSequence canvas;
  canvas.create(1, 16, 16);
  mnm::render::Image image{16, 16, {}};
  for (unsigned i = 0; i < 256; ++i)
    image.pixels.push_back(std::uint16_t(i * 1273 + 113));
  canvas.update(1, 0, 0, image);
  if (std::stoul(argv[1]) == 22 || std::stoul(argv[1]) == 26) {
    std::vector<std::uint16_t> colours;
    for (unsigned i = 0; i < 16; ++i) colours.push_back(std::uint16_t(i * 3137 + 211));
    std::vector<unsigned char> hidden(16);
    if (std::stoul(argv[1]) == 26) {
      canvas.create(2, 16, 16); canvas.update(2, 0, 0, image);
      for (unsigned i = 0; i < 16; ++i) hidden[i] = i % 3 == 0;
    }
    canvas.terrainMap(1, 8, 3, 4, 4, 3, 2, colours, hidden, std::stoul(argv[1]) == 26 ? 2 : 0);
  } else if (std::stoul(argv[1]) == 21) canvas.fade(1);
  else canvas.panel(1, {3, 4, 10, 11}, std::stoul(argv[1]), 40, true, true);
  std::ofstream out(argv[2], std::ios::binary);
  for (auto word : canvas.read(1).pixels) {
    out.put(char(word));
    out.put(char(word >> 8));
  }
  if (!out)
    throw std::runtime_error("Cannot write native fixture");
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
