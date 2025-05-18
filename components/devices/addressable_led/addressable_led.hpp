/**
 * @brief LED Strip Controller for ESP32 RMT Peripheral
 * @author Igor Naskin
 * @date 09.05.2025
 */

#pragma once

#include <vector>
#include "hal/gpio_types.h"
#include "driver/rmt_tx.h"
#include "color.hpp"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include <cstdint>


namespace addressable_led {
    static const char *TAG = "addressable_led";
};

/**
 * @enum LedType
 * @brief Supported types of addressable LED strips
 */
enum class LedType {
    WS2812B,
};

/**
 * @enum Rating
 * @brief Performance configuration for RMT peripheral
 */
enum class Rating {
    DEFAULT,    ///< Balanced performance and memory usage
    PERFOMANCE, ///< Higher performance with increased memory usage
};

/**
 * @struct LedTypeSpecific
 * @tparam Type The LED type to specialize for
 * @brief Provides type-specific traits for different LED strips
 * 
 * Specializations should define:
 * - Data transmission timing properties
 * - Color format
 */
template<LedType Type>
struct LedTypeSpecific;

/**
 * @brief Specialization for WS2812B LED strips
 */
template<>
struct LedTypeSpecific<LedType::WS2812B> {
    static constexpr bool msbFirst = true; ///< Data transmission order
    static constexpr float T0H_us = 0.3f;  ///< Duration of '0' bit high signal (μs)
    static constexpr float T0L_us = 0.9f;  ///< Duration of '0' bit low signal (μs)
    static constexpr float T1H_us = 0.9f;  ///< Duration of '1' bit high signal (μs)
    static constexpr float T1L_us = 0.3f;  ///< Duration of '1' bit low signal (μs)

    using ColorFormat = color::CGRB; ///< Green-Red-Blue color format
};

template<LedType Type>
class AddresableLED {
public:
    /**
     * @brief Construct a new LED Strip controller
     * @param ledCount Number of LEDs in the strip
     * @param connPin GPIO pin connected to LED data line
     * @param rating Performance configuration
     */
    AddresableLED(const std::size_t ledCount, const gpio_num_t connPin, const Rating rating = Rating::DEFAULT);

    /**
     * @brief Set global brightness level
     * @param level Brightness value (0-255)
     * @note Applies to all subsequent color operations
     */
    void setBrightness(uint8_t level);

    /**
     * @brief Set color of a single LED
     * @param color Color in CRGB format
     * @param ledIndex Index of LED to set (0-based)
     * @return esp_err_t
     * @retval ESP_ERR_INVALID_SIZE if ledIndex is out of bounds
     */
    esp_err_t setColor(const color::CRGB& color, size_t ledIndex);

    /**
     * @brief Set color for a range of LEDs
     * @param color Color in CRGB format
     * @param startIndex First LED to set (0-based)
     * @param count Number of LEDs to set
     * @return esp_err_t ESP_OK on success, error code on failure
     * @retval ESP_ERR_INVALID_SIZE if range is invalid
     */
    esp_err_t setColor(const color::CRGB& color, size_t startIndex, size_t count);

    /**
     * @brief Turn off all LEDs
     * @note Uses the LED type's Black color definition
     */
    void clear(void);

    /**
     * @brief Push color data to LED strip
     * @return esp_err_t ESP_OK on success, error code on failure
     * @retval RSLT_ERR_TIMEOUT if RMT peripheral is busy
     * @retval ESP_FAIL if transmission fails
     */
    esp_err_t update(void);

private:
    /**
     * @brief Apply current brightness to all LEDs
     */
    void applyBrightness(void);

    /**
     * @brief RMT encoder structure for LED protocol
     */
    struct RmtLedStripEncoder {
        rmt_encoder_t base;            ///< Base encoder interface
        rmt_encoder_t* bytes_encoder;  ///< Bytes encoder handle
        rmt_encoder_t* copy_encoder;   ///< Copy encoder handle
        int state;                     ///< Current encoder state
        rmt_symbol_word_t reset_code;  ///< Reset code timing
    };

    /**
     * @brief Create RMT encoder for LED protocol
     * @param encoder Encoder structure to initialize
     * @param resolutionHz RMT resolution in Hz
     * @return esp_err_t ESP_OK on success, error code on failure
     */
    static esp_err_t create_encoder(RmtLedStripEncoder* encoder, uint32_t resolutionHz);

    /**
     * @brief RMT encoder callback function
     * @param encoder Encoder handle
     * @param channel RMT channel handle
     * @param primary_data Data to encode
     * @param data_size Size of data
     * @param ret_state Encoder state
     * @return size_t Number of symbols encoded
     */
    static size_t encode_led_strip(rmt_encoder_t* encoder, rmt_channel_handle_t channel, 
                                 const void* primary_data, size_t data_size, 
                                 rmt_encode_state_t* ret_state);

    /**
     * @brief Delete RMT encoder
     * @param encoder Encoder to delete
     * @return esp_err_t ESP_OK on success
     */
    static esp_err_t delete_encoder(rmt_encoder_t* encoder);

    /**
     * @brief Reset RMT encoder
     * @param encoder Encoder to reset
     * @return esp_err_t ESP_OK on success
     */
    static esp_err_t reset_encoder(rmt_encoder_t* encoder);

    rmt_encoder_handle_t ledEncoder_ = nullptr;  ///< RMT encoder handle
    rmt_channel_handle_t ledChannel_ = nullptr;  ///< RMT channel handle
    std::vector<typename LedTypeSpecific<Type>::ColorFormat> leds_; ///< LED color buffer
    uint8_t brightness_ = 255; ///< Current brightness level (0-255)
};


/* ================== Implementation of template methods =================== */

template<LedType Type>
AddresableLED<Type>::AddresableLED(const std::size_t ledCount, const gpio_num_t connPin, const Rating rating) {
    size_t rmtMemoryBlockSize;
    size_t rmtTransactionQueueDepth;
    const uint32_t rmtResolutionHz = 10'000'000; // makes uS resolution which is sufficient for WS2812B
    switch (rating) {
        case Rating::PERFOMANCE:
            rmtMemoryBlockSize = 128;
            rmtTransactionQueueDepth = 8;
            break;
        case Rating::DEFAULT:
            rmtMemoryBlockSize = 64;
            rmtTransactionQueueDepth = 4;
            break;
        default:
            rmtMemoryBlockSize = 64;
            rmtTransactionQueueDepth = 4;
            ESP_LOGW(addressable_led::TAG, "unknown rmt rating, using DEFAULT settings");
    }
    
    const rmt_tx_channel_config_t rmtTxChConfig = {
        .gpio_num = connPin,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        /* Increase the block size can make the LED less flickering*/
        .resolution_hz = rmtResolutionHz,
        .mem_block_symbols = rmtMemoryBlockSize,
        /* Set the number of transactions that can be pending in the background*/
        .trans_queue_depth = rmtTransactionQueueDepth,
        .intr_priority = 0,
        .flags = {},
    };

    ESP_ERROR_CHECK(rmt_new_tx_channel(&rmtTxChConfig, &ledChannel_));
    ESP_LOGI(addressable_led::TAG, "create RMT TX channel");

    RmtLedStripEncoder* led_encoder = static_cast<RmtLedStripEncoder*>(rmt_alloc_encoder_mem(sizeof(RmtLedStripEncoder)));
    ESP_ERROR_CHECK(create_encoder(led_encoder, rmtResolutionHz));
    ledEncoder_ = &led_encoder->base;
    ESP_LOGI(addressable_led::TAG, "install led strip encoder");

    ESP_ERROR_CHECK(rmt_enable(ledChannel_));
    ESP_LOGI(addressable_led::TAG, "enable RMT TX channel");

    leds_.resize(ledCount);
    leds_.shrink_to_fit();

    /* Set full brightness */
    setBrightness(255);
    clear();
}

template<LedType Type>
void AddresableLED<Type>::setBrightness(uint8_t level)  {
    brightness_ = level;
    applyBrightness();
    ESP_LOGI(addressable_led::TAG, "brightness set to %d [0 .. 255]", brightness_);
}

template<LedType Type>
esp_err_t AddresableLED<Type>::setColor(const color::CRGB& color, size_t ledIndex) {
    if (ledIndex >= leds_.size()) {
        ESP_LOGE(addressable_led::TAG, "setColor: invalid led index passed");
        return ESP_ERR_INVALID_SIZE;
    }

    leds_[ledIndex] = color.toColor<typename LedTypeSpecific<Type>::ColorFormat>();
    applyBrightness();

    return ESP_OK;
}

template<LedType Type>
esp_err_t AddresableLED<Type>::setColor(const color::CRGB& color, size_t startIndex, size_t count) {
    if (startIndex + count >= leds_.size()) {
        return ESP_ERR_INVALID_SIZE;
    }

    const size_t EndIndex = startIndex + count;

    for (size_t i = startIndex; i < EndIndex; i++) {
        leds_[i] = color.toColor<typename LedTypeSpecific<Type>::ColorFormat>();
    }
    applyBrightness();

    return ESP_OK;
}

template<LedType Type>
void AddresableLED<Type>::clear(void) {
    for (auto& led : leds_) {
        led = LedTypeSpecific<Type>::ColorFormat::Black;
    }
}

template<LedType Type>
esp_err_t AddresableLED<Type>::update(void) {
    if (rmt_tx_wait_all_done(ledChannel_, pdMS_TO_TICKS(1000)) != ESP_OK) {
        ESP_LOGI(addressable_led::TAG, "looks like rmt got stuck - rmt busy for too long");
        return ESP_ERR_TIMEOUT;
    }

    const rmt_transmit_config_t txConfig = {
        .loop_count = 0,
        .flags = {},
    };

    const uint8_t ColorsCount = 3;
    if (rmt_transmit(ledChannel_, ledEncoder_, leds_.data(), leds_.size() * ColorsCount, &txConfig) != ESP_OK) {
        ESP_LOGI(addressable_led::TAG, "unable to update buffer");
        return ESP_FAIL;
    }

    ESP_LOGI(addressable_led::TAG, "buffer updated");
    
    return ESP_OK;
}

template<LedType Type>
void AddresableLED<Type>::applyBrightness(void) {
    const uint16_t brightness__ = brightness_;
    for (auto& led : leds_) {
        led.r = (led.r * brightness__) / 255;
        led.g = (led.g * brightness__) / 255;
        led.b = (led.b * brightness__) / 255;
    }
}

template<LedType Type>
esp_err_t AddresableLED<Type>::create_encoder(RmtLedStripEncoder* encoder, uint32_t resolutionHz) {  
    encoder->base.encode = &AddresableLED<Type>::encode_led_strip;
    encoder->base.del = &AddresableLED<Type>::delete_encoder;
    encoder->base.reset = &AddresableLED<Type>::reset_encoder;
    
    const uint32_t UsInSec = 1'000'000;
    const uint16_t TicksPerUs = resolutionHz / UsInSec;
    // Configure based on LED type traits
    rmt_bytes_encoder_config_t bytes_encoder_config = {
        .bit0 = {
            .duration0 = static_cast<uint16_t>(LedTypeSpecific<Type>::T0H_us * TicksPerUs),
            .level0 = 1,
            .duration1 = static_cast<uint16_t>(LedTypeSpecific<Type>::T0L_us * TicksPerUs),
            .level1 = 0,
        },
        .bit1 = {
            .duration0 = static_cast<uint16_t>(LedTypeSpecific<Type>::T1H_us * TicksPerUs),
            .level0 = 1,
            .duration1 = static_cast<uint16_t>(LedTypeSpecific<Type>::T1L_us * TicksPerUs),
            .level1 = 0,
        },
        .flags = {
            .msb_first = LedTypeSpecific<Type>::msbFirst,
        }
    };

    rmt_copy_encoder_config_t copy_encoder_config = {};
    
    ESP_RETURN_ON_ERROR(rmt_new_bytes_encoder(&bytes_encoder_config, &encoder->bytes_encoder), 
                       addressable_led::TAG, "create bytes encoder failed");
    ESP_RETURN_ON_ERROR(rmt_new_copy_encoder(&copy_encoder_config, &encoder->copy_encoder), 
                       addressable_led::TAG, "create copy encoder failed");
    
    const uint16_t TickForReset = TicksPerUs * 50 / 2; // 50us reset code
    encoder->reset_code = {
        .duration0 = TickForReset,
        .level0 = 0,
        .duration1 = TickForReset,
        .level1 = 0
    };
    
    return ESP_OK;
}

template<LedType Type>
size_t AddresableLED<Type>::encode_led_strip(rmt_encoder_t* encoder, rmt_channel_handle_t channel, const void* primary_data, size_t data_size, rmt_encode_state_t* ret_state) {
    RmtLedStripEncoder* led_encoder = reinterpret_cast<RmtLedStripEncoder*>(encoder);
    size_t encoded_symbols = 0;
    rmt_encode_state_t session_state = RMT_ENCODING_RESET;
    rmt_encode_state_t state = RMT_ENCODING_RESET;
    
    switch (led_encoder->state) {
    case 0: // send RGB data
        encoded_symbols += led_encoder->bytes_encoder->encode(led_encoder->bytes_encoder, channel, 
                                                            primary_data, data_size, &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            led_encoder->state = 1;
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state = static_cast<rmt_encode_state_t>(state | RMT_ENCODING_MEM_FULL);
            goto out;
        }
        // fall-through
    case 1: // send reset code
        encoded_symbols += led_encoder->copy_encoder->encode(led_encoder->copy_encoder, channel,
                                                           &led_encoder->reset_code,
                                                           sizeof(led_encoder->reset_code),
                                                           &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            led_encoder->state = RMT_ENCODING_RESET;
            state = static_cast<rmt_encode_state_t>(state | RMT_ENCODING_COMPLETE);
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state = static_cast<rmt_encode_state_t>(state | RMT_ENCODING_MEM_FULL);
            goto out;
        }
    }
out:
    *ret_state = state;
    return encoded_symbols;
}

template<LedType Type>
esp_err_t AddresableLED<Type>::delete_encoder(rmt_encoder_t* encoder) {
    RmtLedStripEncoder* led_encoder = reinterpret_cast<RmtLedStripEncoder*>(encoder);
    rmt_del_encoder(led_encoder->bytes_encoder);
    rmt_del_encoder(led_encoder->copy_encoder);
    free(led_encoder);
    return ESP_OK;
}

template<LedType Type>
esp_err_t AddresableLED<Type>::reset_encoder(rmt_encoder_t* encoder) {
    RmtLedStripEncoder* led_encoder = reinterpret_cast<RmtLedStripEncoder*>(encoder);
    rmt_encoder_reset(led_encoder->bytes_encoder);
    rmt_encoder_reset(led_encoder->copy_encoder);
    led_encoder->state = RMT_ENCODING_RESET;
    return ESP_OK;
}