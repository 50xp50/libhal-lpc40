#pragma once

#include <array>

#include <libhal/output_pin.hpp>
#include <libhal/spi.hpp>

namespace hal
{

class ws2812b
{
public:
  ws2812b(hal::spi& p_spi, hal::output_pin& p_chip_select);

  template<std::size_t PixelCount>
  void update(std::array<hal::byte,PixelCount>& p_frame)
  {
    update(std::span<hal::byte>(p_frame));
  }

private:
  hal::spi* m_spi;
  hal::output_pin* m_chip_select;

  void update(std::span<hal::byte> p_data);
};
}  // namespace hal