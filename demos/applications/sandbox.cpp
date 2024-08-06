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

#define LEDAMOUNT 5
std::array<std::array<hal::byte, 24>, LEDAMOUNT> array_LEDs;

void set_individual_LED(uint8_t LED_number, uint8_t R, uint8_t G, uint8_t B);
void set_all_LEDs(uint8_t R, uint8_t G, uint8_t B);
void set_LEDs_span(std::span<std::array<hal::byte, 24>> span_LEDs, uint8_t R, uint8_t G, uint8_t B);
void update_LEDs(hal::lpc40::spi& p_spi);

std::array<std::array<std::uint8_t, 5>, 3> array_colors = {{
  {255, 255, 0, 0, 160},
  {0, 255, 255, 0, 32},
  {0, 0, 0, 255, 240}
}};
  
std::array<std::array<std::uint8_t, 5>, 3> array_trailing = {{
  {255, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0}
}};

void application ()
{
    using namespace hal::literals;

    hal::cortex_m::dwt_counter steady_clock(
      hal::lpc40::get_frequency(hal::lpc40::peripheral::cpu));

    std::array<hal::byte, 32> uart_buffer{};
    hal::lpc40::uart uart0(0, uart_buffer);

    // Need to have data rate 2.5MHz minimum
    const hal::lpc40::spi::settings spi_settings = {2.5_MHz, {false}, {false}};
    hal::lpc40::spi spi2(2, spi_settings);
    hal::lpc40::output_pin chip_select(1, 10);
    chip_select.level(false);

    hal::print(uart0, "Starting Sandbox Application...\n");

    // Set RGB values for the first LED (index 0)
    //set_LEDs_span(std::span<std::array<hal::byte, 24>>{array_LEDs}, 255, 255, 255); 
    set_LEDs_span(std::span<std::array<hal::byte, 24>>{&array_LEDs[0], 2}, 255, 0, 255);
    //set_LEDs_span(std::span<std::array<hal::byte, 24>>{&array_LEDs[0], 1}, 0, 255, 0);

    update_LEDs(spi2);

    while (1) {
      using namespace std::literals;

      // for (int i = 0; i < 5; i++) {
      //   set_individual_LED(0, array_trailing[0][i], array_trailing[1][i], array_trailing[2][i]);
      //   set_individual_LED(1, array_trailing[0][(i+1)%5], array_trailing[1][(i+1)%5], array_trailing[2][(i+1)%5]);
      //   set_individual_LED(2, array_trailing[0][(i+2)%5], array_trailing[1][(i+2)%5], array_trailing[2][(i+2)%5]);
      //   set_individual_LED(3, array_trailing[0][(i+3)%5], array_trailing[1][(i+3)%5], array_trailing[2][(i+3)%5]);
      //   set_individual_LED(4, array_trailing[0][(i+4)%5], array_trailing[1][(i+4)%5], array_trailing[2][(i+4)%5]);
      //   update_LEDs(spi2);
      //   hal::delay(steady_clock, 100ms);
      // }
      hal::print(uart0, "Looped\n");
    }
}

void set_individual_LED(uint8_t LED_position, uint8_t R, uint8_t G, uint8_t B) {
  uint32_t color = G << 16 | R << 8 | B;

  int8_t j = 0;
  for (int8_t k = 23; k >= 0; k--) {
    if (((color >> k) & 0x01) == 1) {
      array_LEDs[LED_position][j] = 0xE0;
    } 
    else {
      array_LEDs[LED_position][j] = 0x80;
    }
    j++;
  }
}
void set_all_LEDs(uint8_t R, uint8_t G, uint8_t B) {
  uint32_t color = G << 16 | R << 8 | B;

  for (int8_t i = 0; i < LEDAMOUNT; i++) {
    int8_t j = 0;
    for (int8_t k = 23; k >= 0; k--) {
      if (((color >> k) & 0x01) == 1) {
        array_LEDs[i][j] = 0xE0;
      } 
      else {
        array_LEDs[i][j] = 0x80;
      }
      j++;
    }
  }
}

void set_LEDs_span(std::span<std::array<hal::byte, 24>> span_LEDs, uint8_t R, uint8_t G, uint8_t B) {
  uint32_t color = G << 16 | R << 8 | B;

  for (auto& arr: span_LEDs) {
    int8_t j = 0;
    for (int8_t k = 23; k >= 0; k--) {
      if (((color >> k) & 0x01) == 1) {
        arr[j] = 0xE0;
      } else {
        arr[j] = 0x80;
      }
      j++;
    }
  }
}

void update_LEDs(hal::lpc40::spi& p_spi) {
  for (int i = 0; i < LEDAMOUNT; i++) {
    hal::write(p_spi, array_LEDs[i]);
  }
}
