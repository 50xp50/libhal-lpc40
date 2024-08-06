#include <array>
#include <span>

#include <libhal-armcortex/dwt_counter.hpp>
#include <libhal-lpc40/clock.hpp>
#include <libhal-lpc40/constants.hpp>
#include <libhal-lpc40/output_pin.hpp>
#include <libhal-lpc40/spi.hpp>
#include <libhal-lpc40/uart.hpp>
#include <libhal-util/serial.hpp>
#include <libhal-util/spi.hpp>
#include <libhal-util/steady_clock.hpp>

#include <libhal-lpc40/ws2812b.hpp>
#include <libhal/timeout.hpp>

template<std::size_t PixelCount>
struct ws2812b_spi_frame
{
  static constexpr std::size_t colors_available = 3;
  static constexpr std::size_t bytes_per_pixel_color = 8;
  static constexpr std::size_t array_length =
    PixelCount * colors_available * bytes_per_pixel_color;

  std::array<hal::byte, array_length> data;
};

struct rgb888
{
  std::uint8_t r;
  std::uint8_t g;
  std::uint8_t b;
};

template<std::size_t PixelCount>
void set_all_pixel(ws2812b_spi_frame<PixelCount>& p_frame, rgb888& p_pixel);
template<std::size_t PixelCount>
void set_range_pixel(ws2812b_spi_frame<PixelCount>& p_frame, rgb888 p_pixel, uint8_t p_start_pixel, uint8_t p_end_pixel) ;
void set_range_ws2812b_frame_pixel(std::span<hal::byte>& p_frame,rgb888 p_pixel);

void application ()
{
    using namespace hal::literals;

    // Setup counter
    hal::cortex_m::dwt_counter steady_clock(
      hal::lpc40::get_frequency(hal::lpc40::peripheral::cpu));

    // Setup UART buffer
    std::array<hal::byte, 32> uart_buffer{};
    hal::lpc40::uart uart0(0, uart_buffer);

    // Setup SPI line and chip select
    const hal::lpc40::spi::settings spi_settings = {2.5_MHz, {false}, {false}};
    hal::lpc40::spi spi2(2, spi_settings);
    hal::lpc40::output_pin chip_select(1, 10);
    chip_select.level(false);

    hal::print(uart0, "Starting Sandbox Application...\n");

    // Create ws2812b object
    hal::ws2812b ws2812b_driver(spi2, chip_select);
    
    // Create frame buffer 
    constexpr std::size_t PixelCount = 5;
    ws2812b_spi_frame<PixelCount> spi_frame;
    spi_frame.data.fill(0xE0);
    std::fill(spi_frame.data.begin(), spi_frame.data.begin() + 8, 0x80);

    // Define a color
    rgb888 redColor = {255, 0, 0};
    rgb888 purpleColor = {255, 0, 255};
    rgb888 greenColor = {0, 255, 0};


    // Set all pixels to the color
    set_all_pixel(spi_frame, redColor);
    set_range_pixel(spi_frame, purpleColor, 1, 3);
    std::span<hal::byte> frameSpan(spi_frame.data); // Create span from raw array
    std::span<hal::byte> subSpan = frameSpan.subspan(0,48); // Create subspan
    set_range_ws2812b_frame_pixel(subSpan, greenColor);

    // Update ws2812b with the spi_frame
    while (1) {
      using namespace std::literals;
      hal::print(uart0, "Flag 1\n");
      ws2812b_driver.update(spi_frame.data);
      hal::delay(steady_clock, 500ms);
      hal::print(uart0, "Looped\n");
    }
}

template<std::size_t PixelCount>
void set_all_pixel(ws2812b_spi_frame<PixelCount>& p_frame, rgb888& p_pixel) {
    uint32_t formatted_color_data = p_pixel.g << 16 | p_pixel.r << 8 | p_pixel.b;
    std::size_t starting_index_current_pixel, offset_amount_current_pixel;
    static constexpr uint32_t bytes_per_pixel = 24;

    for (std::size_t current_pixel = 0; current_pixel < PixelCount; current_pixel++) {
        starting_index_current_pixel = current_pixel * bytes_per_pixel;
        for (int8_t bit = 23; bit >= 0; bit--) {
            offset_amount_current_pixel = bytes_per_pixel - 1 - bit;
            if (((formatted_color_data >> bit) & 0x01) == 1) {
                p_frame.data[starting_index_current_pixel + offset_amount_current_pixel]= 0xE0;
            } 
            else {
                p_frame.data[starting_index_current_pixel + offset_amount_current_pixel]= 0x80;
            }
        }
    }
}

template<std::size_t PixelCount>
void set_range_pixel(ws2812b_spi_frame<PixelCount>& p_frame, rgb888 p_pixel, uint8_t p_start_pixel, uint8_t p_end_pixel) {
    uint32_t formatted_color_data = p_pixel.g << 16 | p_pixel.r << 8 | p_pixel.b;
    std::size_t starting_index_current_pixel, offset_amount_current_pixel;
    static constexpr uint32_t bytes_per_pixel = p_frame.colors_available * p_frame.bytes_per_pixel_color;

    for (std::size_t current_pixel = p_start_pixel; current_pixel <= p_end_pixel; current_pixel++) {
        starting_index_current_pixel = current_pixel * bytes_per_pixel;
        for (int8_t bit = 23; bit >= 0; bit--) {
            offset_amount_current_pixel = bytes_per_pixel - 1 - bit;
            if (((formatted_color_data >> bit) & 0x01) == 1) {
                p_frame.data[starting_index_current_pixel + offset_amount_current_pixel]= 0xE0;
            } 
            else {
                p_frame.data[starting_index_current_pixel + offset_amount_current_pixel]= 0x80;
            }
        }
    }
}

void set_range_ws2812b_frame_pixel(std::span<hal::byte>& p_frame, rgb888 p_pixel) {

    uint32_t bytes_per_pixel = 24;

    //if (p_frame.size() == 0 || p_frame.size() % bytes_per_pixel != 0 ) safe_throw

    uint32_t formatted_color_data = p_pixel.g << 16 | p_pixel.r << 8 | p_pixel.b;
    std::size_t starting_index_current_pixel, offset_amount_current_pixel;
    
    uint32_t pixel_count = p_frame.size() / bytes_per_pixel;


    for (std::size_t current_pixel = 0; current_pixel < pixel_count; current_pixel++) {
        starting_index_current_pixel = current_pixel * bytes_per_pixel;
        for (int8_t bit = 23; bit >= 0; bit--) {
            offset_amount_current_pixel = bytes_per_pixel - 1 - bit;
            if (((formatted_color_data >> bit) & 0x01) == 1) {
                p_frame[starting_index_current_pixel + offset_amount_current_pixel]= 0xE0;
            } 
            else {
                p_frame[starting_index_current_pixel + offset_amount_current_pixel]= 0x80;
            }
        }
    }
}
