#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>
#include <thread>
#include <mutex>

#include "CRC32.hpp"
#include "IO.hpp"

/// @brief Переписывает последние 4 байта значением value
void replaceLastFourBytes(std::vector<char> &data, uint32_t value) {
  std::copy_n(reinterpret_cast<const char *>(&value), 4, data.end() - 4);
}

/**
 * @brief Формирует новый вектор с тем же CRC32, добавляя в конец оригинального
 * строку injection и дополнительные 4 байта
 * @details При формировании нового вектора последние 4 байта не несут полезной
 * нагрузки и подбираются таким образом, чтобы CRC32 нового и оригинального
 * вектора совпадали
 * @param original оригинальный вектор
 * @param injection произвольная строка, которая будет добавлена после данных
 * оригинального вектора
 * @return новый вектор
 */
bool hack(const std::vector<char> &original,const std::string &injection, size_t totalth, size_t nth, const char *path) {
  const uint32_t originalCrc32 = crc32(original.data(), original.size());

  std::vector<char> result(original.size() + injection.size() + 4);
  auto it = std::copy(original.begin(), original.end(), result.begin());
  std::copy(injection.begin(), injection.end(), it);

  /*
   * Внимание: код ниже крайне не оптимален.
   * В качестве доп. задания устраните избыточные вычисления
   */
  const size_t maxVal = std::numeric_limits<uint32_t>::max();

  size_t localmaxval = maxVal/totalth;

  for (size_t i = localmaxval*nth - localmaxval; i < localmaxval*nth; ++i) {
    // Заменяем последние четыре байта на значение i
    replaceLastFourBytes(result, uint32_t(i));
    // Вычисляем CRC32 текущего вектора result
    auto currentCrc32 = crc32(result.data(), result.size());

    if (currentCrc32 == originalCrc32) {
      std::cout << "Success\n";
      writeToFile(path, result);
      
      return 1;
    }
    // Отображаем прогресс
    if (i % 1000 == 0) {
      std::cout << "progress: "
                << static_cast<double>(i) / static_cast<double>(maxVal) << " thread: " << nth
                << std::endl;
    }
  }
  //throw std::logic_error("Can't hack");
  return 0;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Call with two args: " << argv[0]
              << " <input file> <output file>\n";
    return 1;
  }

  try {
    const std::vector<char> data = readFromFile(argv[1]);
    std::cout << std::thread::hardware_concurrency() << std::endl;
    for (size_t x = 1; x <= std::thread::hardware_concurrency(); x++){
      std::thread th(hack,data, "He-he-he", std::thread::hardware_concurrency(), x, argv[2]);
      th.join();
      const std::vector<char> data = readFromFile(argv[2]);
      if (data.size() > 1){
        break;
      }
    }
  } catch (std::exception &ex) {
    std::cerr << ex.what() << '\n';
    return 2;
  }
  return 0;
}
