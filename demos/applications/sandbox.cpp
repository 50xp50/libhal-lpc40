#include <array>

#include <libhal-armcortex/dwt_counter.hpp>
#include <libhal-lpc40/clock.hpp>
#include <libhal-lpc40/constants.hpp>
#include <libhal-lpc40/output_pin.hpp>
#include <libhal-lpc40/spi.hpp>
#include <libhal-lpc40/uart.hpp>
#include <libhal-util/serial.hpp>
#include <libhal-util/spi.hpp>
#include <libhal-util/steady_clock.hpp>

void setLED(uint8_t R, uint8_t G, uint8_t B, hal::lpc40::spi& p_spi, uint8_t r, uint8_t g, uint8_t b) {

  uint32_t color = G << 16 | R << 8 | B;
  uint32_t color2 = g << 16 | r << 8 | b;
  std::array<hal::byte, 24> dataLED;
  std::array<hal::byte, 24> dataLED2;

  int8_t j = 0;
  for (int8_t i = 23; i >= 0; i--) {
    if (((color >> i) & 0x01) == 1) {
      dataLED[j] = 0xE0;
    } 
    else {
      dataLED[j] = 0x80;
    }
    j++;
  }

  j = 0;
  for (int8_t i = 23; i >= 0; i--) {
    if (((color2 >> i) &0x01) == 1) {
      dataLED2[j] = 0xE0;
    } 
    else {
      dataLED2[j] = 0x80;
    }
    j++;
  }

  //p_spi.transfer(dataLED, buffer);
  hal::write(p_spi, dataLED);
  hal::write(p_spi, dataLED2);
  hal::write(p_spi, dataLED);
  hal::write(p_spi, dataLED2);
  hal::write(p_spi, dataLED);
}

void application ()
{
    using namespace hal::literals;

    hal::cortex_m::dwt_counter steady_clock(
      hal::lpc40::get_frequency(hal::lpc40::peripheral::cpu));

    std::array<hal::byte, 32> uart_buffer{};
    hal::lpc40::uart uart0(0, uart_buffer);

    // Need to have data rate 2.5MHz minimum
    const hal::lpc40::spi::settings spi_settings = {.clock_rate = 2.5_MHz};
    hal::lpc40::spi spi2(2, spi_settings);
    hal::lpc40::output_pin chip_select(1, 10);
    chip_select.level(false);

    hal::print(uart0, "Starting Sandbox Application...\n");

    while (1) {
      using namespace std::literals;

      setLED(255, 0, 0, spi2, 0, 255, 0);
      hal::delay(steady_clock, 1000ms);
      // setLED(0, 0, 255, spi2, 255, 0, 0);
      // hal::delay(steady_clock, 1000ms);
      
      hal::print(uart0, "Looped\n");
    }
}


