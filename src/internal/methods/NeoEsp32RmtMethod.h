/*-------------------------------------------------------------------------
NeoPixel library helper functions for Esp32.

A BIG thanks to Andreas Merkle for the investigation and implementation of
a workaround to the GCC bug that drops method attributes from template methods

Written by Michael C. Miller.

I invest time and resources providing this open source code,
please support me by donating (see https://github.com/Makuna/NeoPixelBus)

-------------------------------------------------------------------------
This file is part of the Makuna/NeoPixelBus library.

NeoPixelBus is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

NeoPixelBus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with NeoPixel.  If not, see
<http://www.gnu.org/licenses/>.
-------------------------------------------------------------------------*/

#pragma once

#ifdef ARDUINO_ARCH_ESP32

extern "C"
{
#include <rom/gpio.h>
}

// Version check must come BEFORE including any RMT headers
// to avoid deprecated legacy header warnings in ESP-IDF 5.x
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#define NEOPIXELBUS_RMT_NEW_API 1
#include <driver/rmt_tx.h>
#include <driver/rmt_common.h>
#else
#define NEOPIXELBUS_RMT_NEW_API 0
#include <driver/rmt.h>
#endif

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 3, 0)
#define NEOPIXELBUS_RMT_INT_FLAGS (ESP_INTR_FLAG_LOWMED)
#else
#define NEOPIXELBUS_RMT_INT_FLAGS (ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL1)
#endif

// ============================================================================
// LEGACY API (ESP-IDF < 5.0) - uses rmt_item32_t based translation
// ============================================================================
#if !NEOPIXELBUS_RMT_NEW_API

class NeoEsp32RmtSpeedBase
{
public:
    const static uint8_t RmtClockDivider = 2;

    inline constexpr static uint32_t FromNs(uint32_t ns)
    {
        return ns / NsPerRmtTick;
    }

    inline constexpr static uint32_t Item32Val(uint16_t nsHigh, uint16_t nsLow)
    {
        return (FromNs(nsLow) << 16) | (1 << 15) | (FromNs(nsHigh));
    }

    const static rmt_idle_level_t IdleLevel = RMT_IDLE_LEVEL_LOW;

protected:
    const static uint32_t RmtCpu = 80000000L;
    const static uint32_t NsPerSecond = 1000000000L;
    const static uint32_t RmtTicksPerSecond = (RmtCpu / RmtClockDivider);
    const static uint32_t NsPerRmtTick = (NsPerSecond / RmtTicksPerSecond);
};

class NeoEsp32RmtInvertedSpeedBase : public NeoEsp32RmtSpeedBase
{
public:
    inline constexpr static uint32_t Item32Val(uint16_t nsHigh, uint16_t nsLow)
    {
        return (FromNs(nsLow) << 16) | (1 << 31) | (FromNs(nsHigh));
    }

    const static rmt_idle_level_t IdleLevel = RMT_IDLE_LEVEL_HIGH;
};

#define NEOPIXELBUS_DECLARE_SPEED(className, baseClass, bit0High, bit0Low, bit1High, bit1Low, resetNs) \
class className : public baseClass \
{ \
public: \
    const static uint32_t RmtBit0 = Item32Val(bit0High, bit0Low); \
    const static uint32_t RmtBit1 = Item32Val(bit1High, bit1Low); \
    const static uint16_t RmtDurationReset = FromNs(resetNs); \
    static void IRAM_ATTR Translate(const void* src, rmt_item32_t* dest, size_t src_size, size_t wanted_num, size_t* translated_size, size_t* item_num); \
};

NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtSpeedWs2811, NeoEsp32RmtSpeedBase, 300, 950, 900, 350, 300000)
NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtSpeedWs2812x, NeoEsp32RmtSpeedBase, 400, 850, 800, 450, 300000)
NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtSpeedWs2805, NeoEsp32RmtSpeedBase, 300, 790, 790, 300, 300000)
NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtSpeedSk6812, NeoEsp32RmtSpeedBase, 400, 850, 800, 450, 80000)
NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtSpeedTm1814, NeoEsp32RmtInvertedSpeedBase, 360, 890, 720, 530, 200000)
NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtSpeedTm1829, NeoEsp32RmtInvertedSpeedBase, 300, 900, 800, 400, 200000)
NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtSpeedTm1914, NeoEsp32RmtInvertedSpeedBase, 360, 890, 720, 530, 200000)
NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtSpeed800Kbps, NeoEsp32RmtSpeedBase, 400, 850, 800, 450, 50000)
NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtSpeed400Kbps, NeoEsp32RmtSpeedBase, 800, 1700, 1600, 900, 50000)
NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtSpeedApa106, NeoEsp32RmtSpeedBase, 350, 1350, 1350, 350, 50000)
NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtSpeedTx1812, NeoEsp32RmtSpeedBase, 300, 600, 600, 300, 80000)
NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtSpeedGs1903, NeoEsp32RmtSpeedBase, 300, 900, 900, 300, 40000)

NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtInvertedSpeedWs2811, NeoEsp32RmtInvertedSpeedBase, 300, 950, 900, 350, 300000)
NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtInvertedSpeedWs2812x, NeoEsp32RmtInvertedSpeedBase, 400, 850, 800, 450, 300000)
NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtInvertedSpeedWs2805, NeoEsp32RmtInvertedSpeedBase, 300, 790, 790, 300, 300000)
NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtInvertedSpeedSk6812, NeoEsp32RmtInvertedSpeedBase, 400, 850, 800, 450, 80000)
NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtInvertedSpeedTm1814, NeoEsp32RmtInvertedSpeedBase, 360, 890, 720, 530, 200000)
NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtInvertedSpeedTm1829, NeoEsp32RmtInvertedSpeedBase, 300, 900, 800, 400, 200000)
NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtInvertedSpeedTm1914, NeoEsp32RmtInvertedSpeedBase, 360, 890, 720, 530, 200000)
NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtInvertedSpeed800Kbps, NeoEsp32RmtInvertedSpeedBase, 400, 850, 800, 450, 50000)
NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtInvertedSpeed400Kbps, NeoEsp32RmtInvertedSpeedBase, 800, 1700, 1600, 900, 50000)
NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtInvertedSpeedApa106, NeoEsp32RmtInvertedSpeedBase, 350, 1350, 1350, 350, 50000)
NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtInvertedSpeedTx1812, NeoEsp32RmtInvertedSpeedBase, 300, 600, 600, 300, 80000)
NEOPIXELBUS_DECLARE_SPEED(NeoEsp32RmtInvertedSpeedGs1903, NeoEsp32RmtInvertedSpeedBase, 300, 900, 900, 300, 40000)

class NeoEsp32RmtChannel0
{
public:
    NeoEsp32RmtChannel0() {};
    const static rmt_channel_t RmtChannelNumber = RMT_CHANNEL_0;
};

class NeoEsp32RmtChannel1
{
public:
    NeoEsp32RmtChannel1() {};
    const static rmt_channel_t RmtChannelNumber = RMT_CHANNEL_1;
};

class NeoEsp32RmtChannel2
{
public:
    NeoEsp32RmtChannel2() {};
    const static rmt_channel_t RmtChannelNumber = RMT_CHANNEL_2;
};

class NeoEsp32RmtChannel3
{
public:
    NeoEsp32RmtChannel3() {};
    const static rmt_channel_t RmtChannelNumber = RMT_CHANNEL_3;
};

#if !defined(CONFIG_IDF_TARGET_ESP32S2) && !defined(CONFIG_IDF_TARGET_ESP32C3) && !defined(CONFIG_IDF_TARGET_ESP32S3)

class NeoEsp32RmtChannel4
{
public:
    NeoEsp32RmtChannel4() {};
    const static rmt_channel_t RmtChannelNumber = RMT_CHANNEL_4;
};

class NeoEsp32RmtChannel5
{
public:
    NeoEsp32RmtChannel5() {};
    const static rmt_channel_t RmtChannelNumber = RMT_CHANNEL_5;
};

class NeoEsp32RmtChannel6
{
public:
    NeoEsp32RmtChannel6() {};
    const static rmt_channel_t RmtChannelNumber = RMT_CHANNEL_6;
};

class NeoEsp32RmtChannel7
{
public:
    NeoEsp32RmtChannel7() {};
    const static rmt_channel_t RmtChannelNumber = RMT_CHANNEL_7;
};

#endif

class NeoEsp32RmtChannelN
{
public:
    NeoEsp32RmtChannelN(NeoBusChannel channel) :
        RmtChannelNumber(static_cast<rmt_channel_t>(channel))
    {
    }
    NeoEsp32RmtChannelN() = delete;
    const rmt_channel_t RmtChannelNumber;
};

template<typename T_SPEED, typename T_CHANNEL> class NeoEsp32RmtMethodBase
{
public:
    typedef NeoNoSettings SettingsObject;

    NeoEsp32RmtMethodBase(uint8_t pin, uint16_t pixelCount, size_t elementSize, size_t settingsSize)  :
        _sizeData(pixelCount * elementSize + settingsSize),
        _pin(pin)
    {
    }

    NeoEsp32RmtMethodBase(uint8_t pin, uint16_t pixelCount, size_t elementSize, size_t settingsSize, NeoBusChannel channel) :
        _sizeData(pixelCount* elementSize + settingsSize),
        _pin(pin),
        _channel(channel)
    {
    }

    ~NeoEsp32RmtMethodBase()
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT(rmt_wait_tx_done(_channel.RmtChannelNumber, 10000 / portTICK_PERIOD_MS));
        ESP_ERROR_CHECK(rmt_driver_uninstall(_channel.RmtChannelNumber));
        gpio_matrix_out(_pin, SIG_GPIO_OUT_IDX, false, false);
        pinMode(_pin, INPUT);
        free(_dataEditing);
        free(_dataSending);
    }

    bool IsReadyToUpdate() const
    {
        return (ESP_OK == ESP_ERROR_CHECK_WITHOUT_ABORT_SILENT_TIMEOUT(rmt_wait_tx_done(_channel.RmtChannelNumber, 0)));
    }

    bool Initialize()
    {
        _dataEditing = static_cast<uint8_t*>(malloc(_sizeData));
        if (!_dataEditing) return false;

        _dataSending = static_cast<uint8_t*>(malloc(_sizeData));
        if (!_dataSending)
        {
            free(_dataEditing);
            _dataEditing = nullptr;
            return false;
        }

        rmt_config_t config = {};
        config.rmt_mode = RMT_MODE_TX;
        config.channel = _channel.RmtChannelNumber;
        config.gpio_num = static_cast<gpio_num_t>(_pin);
        config.mem_block_num = 1;
        config.tx_config.loop_en = false;
        config.tx_config.idle_output_en = true;
        config.tx_config.idle_level = T_SPEED::IdleLevel;
        config.tx_config.carrier_en = false;
        config.tx_config.carrier_level = RMT_CARRIER_LEVEL_LOW;
        config.clk_div = T_SPEED::RmtClockDivider;

        ESP_ERROR_CHECK(rmt_config(&config));
        ESP_ERROR_CHECK(rmt_driver_install(_channel.RmtChannelNumber, 0, NEOPIXELBUS_RMT_INT_FLAGS));
        ESP_ERROR_CHECK(rmt_translator_init(_channel.RmtChannelNumber, T_SPEED::Translate));
        return true;
    }

    void Update(bool maintainBufferConsistency)
    {
        if (ESP_OK == ESP_ERROR_CHECK_WITHOUT_ABORT(rmt_wait_tx_done(_channel.RmtChannelNumber, 10000 / portTICK_PERIOD_MS)))
        {
            ESP_ERROR_CHECK_WITHOUT_ABORT(rmt_write_sample(_channel.RmtChannelNumber, _dataEditing, _sizeData, false));
            if (maintainBufferConsistency)
            {
                memcpy(_dataSending, _dataEditing, _sizeData);
            }
            std::swap(_dataSending, _dataEditing);
        }
    }

    bool AlwaysUpdate() { return false; }
    bool SwapBuffers()
    {
        std::swap(_dataSending, _dataEditing);
        return true;
    }
    uint8_t* getData() const { return _dataEditing; }
    size_t getDataSize() const { return _sizeData; }
    void applySettings([[maybe_unused]] const SettingsObject& settings) {}

private:
    const size_t  _sizeData;
    const uint8_t _pin;
    const T_CHANNEL _channel;
    uint8_t*  _dataEditing;
    uint8_t*  _dataSending;
};

// ============================================================================
// NEW API (ESP-IDF >= 5.0) - uses byte-based RMT encoding
// ============================================================================
#else // NEOPIXELBUS_RMT_NEW_API

class NeoEsp32RmtSpeedBase
{
public:
    const static uint8_t RmtClockDivider = 2;
    const static uint32_t RmtCpu = 80000000L;
    const static uint32_t NsPerSecond = 1000000000L;
    const static uint32_t NsPerRmtTick = (NsPerSecond / (RmtCpu / RmtClockDivider));

    inline constexpr static uint32_t FromNs(uint32_t ns)
    {
        return ns / NsPerRmtTick;
    }

    inline constexpr static uint32_t BitTiming(uint16_t nsHigh, uint16_t nsLow)
    {
        return (FromNs(nsLow) << 16) | FromNs(nsHigh);
    }

    const static bool Inverted = false;
    static constexpr uint32_t ResolutionHz = RmtCpu / RmtClockDivider;
};

class NeoEsp32RmtInvertedSpeedBase : public NeoEsp32RmtSpeedBase
{
public:
    const static bool Inverted = true;
};

#define NEOPIXELBUS_DECLARE_SPEED_NEW(className, baseClass, bit0High, bit0Low, bit1High, bit1Low, resetNs) \
class className : public baseClass \
{ \
public: \
    const static uint32_t Bit0 = BitTiming(bit0High, bit0Low); \
    const static uint32_t Bit1 = BitTiming(bit1High, bit1Low); \
};

NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtSpeedWs2811, NeoEsp32RmtSpeedBase, 300, 950, 900, 350, 300000)
NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtSpeedWs2812x, NeoEsp32RmtSpeedBase, 400, 850, 800, 450, 300000)
NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtSpeedWs2805, NeoEsp32RmtSpeedBase, 300, 790, 790, 300, 300000)
NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtSpeedSk6812, NeoEsp32RmtSpeedBase, 400, 850, 800, 450, 80000)
NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtSpeedTm1814, NeoEsp32RmtInvertedSpeedBase, 360, 890, 720, 530, 200000)
NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtSpeedTm1829, NeoEsp32RmtInvertedSpeedBase, 300, 900, 800, 400, 200000)
NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtSpeedTm1914, NeoEsp32RmtInvertedSpeedBase, 360, 890, 720, 530, 200000)
NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtSpeed800Kbps, NeoEsp32RmtSpeedBase, 400, 850, 800, 450, 50000)
NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtSpeed400Kbps, NeoEsp32RmtSpeedBase, 800, 1700, 1600, 900, 50000)
NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtSpeedApa106, NeoEsp32RmtSpeedBase, 350, 1350, 1350, 350, 50000)
NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtSpeedTx1812, NeoEsp32RmtSpeedBase, 300, 600, 600, 300, 80000)
NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtSpeedGs1903, NeoEsp32RmtSpeedBase, 300, 900, 900, 300, 40000)

NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtInvertedSpeedWs2811, NeoEsp32RmtInvertedSpeedBase, 300, 950, 900, 350, 300000)
NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtInvertedSpeedWs2812x, NeoEsp32RmtInvertedSpeedBase, 400, 850, 800, 450, 300000)
NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtInvertedSpeedWs2805, NeoEsp32RmtInvertedSpeedBase, 300, 790, 790, 300, 300000)
NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtInvertedSpeedSk6812, NeoEsp32RmtInvertedSpeedBase, 400, 850, 800, 450, 80000)
NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtInvertedSpeedTm1814, NeoEsp32RmtInvertedSpeedBase, 360, 890, 720, 530, 200000)
NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtInvertedSpeedTm1829, NeoEsp32RmtInvertedSpeedBase, 300, 900, 800, 400, 200000)
NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtInvertedSpeedTm1914, NeoEsp32RmtInvertedSpeedBase, 360, 890, 720, 530, 200000)
NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtInvertedSpeed800Kbps, NeoEsp32RmtInvertedSpeedBase, 400, 850, 800, 450, 50000)
NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtInvertedSpeed400Kbps, NeoEsp32RmtInvertedSpeedBase, 800, 1700, 1600, 900, 50000)
NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtInvertedSpeedApa106, NeoEsp32RmtInvertedSpeedBase, 350, 1350, 1350, 350, 50000)
NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtInvertedSpeedTx1812, NeoEsp32RmtInvertedSpeedBase, 300, 600, 600, 300, 80000)
NEOPIXELBUS_DECLARE_SPEED_NEW(NeoEsp32RmtInvertedSpeedGs1903, NeoEsp32RmtInvertedSpeedBase, 300, 900, 900, 300, 40000)

class NeoEsp32RmtChannelBase
{
public:
    NeoEsp32RmtChannelBase() : _channelHandle(nullptr) {}
    rmt_channel_handle_t getHandle() const { return _channelHandle; }
    void setHandle(rmt_channel_handle_t handle) { _channelHandle = handle; }
protected:
    rmt_channel_handle_t _channelHandle;
};

class NeoEsp32RmtChannel0 : public NeoEsp32RmtChannelBase
{
public:
    NeoEsp32RmtChannel0() : _channelIndex(0) {}
    const uint8_t _channelIndex;
};

class NeoEsp32RmtChannel1 : public NeoEsp32RmtChannelBase
{
public:
    NeoEsp32RmtChannel1() : _channelIndex(1) {}
    const uint8_t _channelIndex;
};

class NeoEsp32RmtChannel2 : public NeoEsp32RmtChannelBase
{
public:
    NeoEsp32RmtChannel2() : _channelIndex(2) {}
    const uint8_t _channelIndex;
};

class NeoEsp32RmtChannel3 : public NeoEsp32RmtChannelBase
{
public:
    NeoEsp32RmtChannel3() : _channelIndex(3) {}
    const uint8_t _channelIndex;
};

#if !defined(CONFIG_IDF_TARGET_ESP32S2) && !defined(CONFIG_IDF_TARGET_ESP32C3) && !defined(CONFIG_IDF_TARGET_ESP32S3)

class NeoEsp32RmtChannel4 : public NeoEsp32RmtChannelBase
{
public:
    NeoEsp32RmtChannel4() : _channelIndex(4) {}
    const uint8_t _channelIndex;
};

class NeoEsp32RmtChannel5 : public NeoEsp32RmtChannelBase
{
public:
    NeoEsp32RmtChannel5() : _channelIndex(5) {}
    const uint8_t _channelIndex;
};

class NeoEsp32RmtChannel6 : public NeoEsp32RmtChannelBase
{
public:
    NeoEsp32RmtChannel6() : _channelIndex(6) {}
    const uint8_t _channelIndex;
};

class NeoEsp32RmtChannel7 : public NeoEsp32RmtChannelBase
{
public:
    NeoEsp32RmtChannel7() : _channelIndex(7) {}
    const uint8_t _channelIndex;
};

#endif

class NeoEsp32RmtChannelN : public NeoEsp32RmtChannelBase
{
public:
    NeoEsp32RmtChannelN(NeoBusChannel channel) : _channelIndex(static_cast<uint8_t>(channel)) {}
    const uint8_t _channelIndex;
};

template<typename T_SPEED, typename T_CHANNEL> class NeoEsp32RmtMethodBase
{
public:
    typedef NeoNoSettings SettingsObject;

    NeoEsp32RmtMethodBase(uint8_t pin, uint16_t pixelCount, size_t elementSize, size_t settingsSize)  :
        _sizeData(pixelCount * elementSize + settingsSize),
        _pin(pin),
        _channel(),
        _encoder(nullptr),
        _isTransmitting(false),
        _initialized(false),
        _dataEditing(nullptr),
        _dataSending(nullptr)
    {
    }

    NeoEsp32RmtMethodBase(uint8_t pin, uint16_t pixelCount, size_t elementSize, size_t settingsSize, NeoBusChannel channel) :
        _sizeData(pixelCount* elementSize + settingsSize),
        _pin(pin),
        _channel(channel),
        _encoder(nullptr),
        _isTransmitting(false),
        _initialized(false),
        _dataEditing(nullptr),
        _dataSending(nullptr)
    {
    }

    ~NeoEsp32RmtMethodBase()
    {
        if (_channel.getHandle())
        {
            ESP_ERROR_CHECK_WITHOUT_ABORT(rmt_tx_wait_all_done(_channel.getHandle(), 10000 / portTICK_PERIOD_MS));
            if (_encoder)
            {
                ESP_ERROR_CHECK(rmt_del_encoder(_encoder));
            }
            ESP_ERROR_CHECK(rmt_del_channel(_channel.getHandle()));
        }
        gpio_matrix_out(_pin, SIG_GPIO_OUT_IDX, false, false);
        pinMode(_pin, INPUT);
        free(_dataEditing);
        free(_dataSending);
    }

    bool IsReadyToUpdate() const
    {
        return !_isTransmitting;
    }

    bool Initialize()
    {
        _dataEditing = static_cast<uint8_t*>(malloc(_sizeData));
        if (!_dataEditing) return false;

        _dataSending = static_cast<uint8_t*>(malloc(_sizeData));
        if (!_dataSending)
        {
            free(_dataEditing);
            _dataEditing = nullptr;
            return false;
        }

        rmt_tx_channel_config_t tx_config = {};
        tx_config.gpio_num = static_cast<gpio_num_t>(_pin);
        tx_config.resolution_hz = T_SPEED::ResolutionHz;
        tx_config.mem_block_symbols = 64;
        tx_config.trans_queue_depth = 1;

        rmt_channel_handle_t handle;
        esp_err_t err = rmt_new_tx_channel(&tx_config, &handle);
        if (err != ESP_OK)
        {
            free(_dataEditing);
            free(_dataSending);
            _dataEditing = nullptr;
            _dataSending = nullptr;
            return false;
        }
        _channel.setHandle(handle);

        rmt_bytes_encoder_config_t byte_encoder_config = {};
        byte_encoder_config.bit0.val = T_SPEED::Bit0;
        byte_encoder_config.bit1.val = T_SPEED::Bit1;
        err = rmt_new_bytes_encoder(&byte_encoder_config, &_encoder);
        if (err != ESP_OK)
        {
            ESP_ERROR_CHECK(rmt_del_channel(_channel.getHandle()));
            _channel.setHandle(nullptr);
            free(_dataEditing);
            free(_dataSending);
            _dataEditing = nullptr;
            _dataSending = nullptr;
            return false;
        }

        rmt_tx_event_callbacks_t callbacks = {};
        callbacks.on_trans_done = &NeoEsp32RmtTransDoneCallbackWrapper::template callback<NeoEsp32RmtMethodBase<T_SPEED, T_CHANNEL>>;
        err = rmt_tx_register_event_callbacks(_channel.getHandle(), &callbacks, this);
        if (err != ESP_OK)
        {
            ESP_ERROR_CHECK(rmt_del_encoder(_encoder));
            ESP_ERROR_CHECK(rmt_del_channel(_channel.getHandle()));
            _channel.setHandle(nullptr);
            _encoder = nullptr;
            free(_dataEditing);
            free(_dataSending);
            _dataEditing = nullptr;
            _dataSending = nullptr;
            return false;
        }

        ESP_ERROR_CHECK(rmt_enable(_channel.getHandle()));

        _initialized = true;
        return true;
    }

    void Update(bool maintainBufferConsistency)
    {
        if (_isTransmitting || !_initialized)
        {
            return;
        }

        if (maintainBufferConsistency)
        {
            memcpy(_dataSending, _dataEditing, _sizeData);
        }

        rmt_transmit_config_t transmit_config = {};
        transmit_config.loop_count = 0;

        _isTransmitting = true;

        ESP_ERROR_CHECK(rmt_transmit(_channel.getHandle(),
            _encoder,
            _dataEditing,
            _sizeData,
            &transmit_config));

        std::swap(_dataSending, _dataEditing);
    }

    bool AlwaysUpdate() { return false; }
    bool SwapBuffers()
    {
        std::swap(_dataSending, _dataEditing);
        return true;
    }
    uint8_t* getData() const { return _dataEditing; }
    size_t getDataSize() const { return _sizeData; }
    void applySettings([[maybe_unused]] const SettingsObject& settings) {}

    void IRAM_ATTR markTransmitDone()
    {
        _isTransmitting = false;
    }

    // Note: _isTransmitting is written in ISR (on the same CPU core) and read in
    // main thread. The volatile qualifier provides visibility, but on ESP-IDF
    // the callback runs in a critical section providing sufficient ordering.
    // For cross-core access or stronger guarantees, atomic operations would be needed.

private:
    struct NeoEsp32RmtTransDoneCallbackWrapper
    {
        template<typename T>
        static bool IRAM_ATTR callback(rmt_channel_t* channel, const rmt_tx_done_event_data_t* edata, void* user_data)
        {
            (void)channel;
            (void)edata;
            auto* instance = static_cast<T*>(user_data);
            instance->markTransmitDone();
            return true;
        }
    };

    const size_t  _sizeData;
    const uint8_t _pin;
    T_CHANNEL _channel;
    rmt_encoder_handle_t _encoder;
    volatile bool _isTransmitting;
    bool _initialized;
    uint8_t*  _dataEditing;
    uint8_t*  _dataSending;
};

#endif // NEOPIXELBUS_RMT_NEW_API

// normal
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2811, NeoEsp32RmtChannelN> NeoEsp32RmtNWs2811Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2812x, NeoEsp32RmtChannelN> NeoEsp32RmtNWs2812xMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2812x, NeoEsp32RmtChannelN> NeoEsp32RmtNWs2816Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2805, NeoEsp32RmtChannelN> NeoEsp32RmtNWs2805Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedSk6812, NeoEsp32RmtChannelN> NeoEsp32RmtNSk6812Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1814, NeoEsp32RmtChannelN> NeoEsp32RmtNTm1814Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1829, NeoEsp32RmtChannelN> NeoEsp32RmtNTm1829Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1914, NeoEsp32RmtChannelN> NeoEsp32RmtNTm1914Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedApa106, NeoEsp32RmtChannelN> NeoEsp32RmtNApa106Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTx1812, NeoEsp32RmtChannelN> NeoEsp32RmtNTx1812Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedGs1903, NeoEsp32RmtChannelN> NeoEsp32RmtNGs1903Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeed800Kbps, NeoEsp32RmtChannelN> NeoEsp32RmtN800KbpsMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeed400Kbps, NeoEsp32RmtChannelN> NeoEsp32RmtN400KbpsMethod;
typedef NeoEsp32RmtNWs2805Method NeoEsp32RmtNWs2814Method;

typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2811, NeoEsp32RmtChannel0> NeoEsp32Rmt0Ws2811Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2812x, NeoEsp32RmtChannel0> NeoEsp32Rmt0Ws2812xMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2812x, NeoEsp32RmtChannel0> NeoEsp32Rmt0Ws2816Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2805, NeoEsp32RmtChannel0> NeoEsp32Rmt0Ws2805Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedSk6812, NeoEsp32RmtChannel0> NeoEsp32Rmt0Sk6812Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1814, NeoEsp32RmtChannel0> NeoEsp32Rmt0Tm1814Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1829, NeoEsp32RmtChannel0> NeoEsp32Rmt0Tm1829Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1914, NeoEsp32RmtChannel0> NeoEsp32Rmt0Tm1914Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedApa106, NeoEsp32RmtChannel0> NeoEsp32Rmt0Apa106Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTx1812, NeoEsp32RmtChannel0> NeoEsp32Rmt0Tx1812Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedGs1903, NeoEsp32RmtChannel0> NeoEsp32Rmt0Gs1903Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeed800Kbps, NeoEsp32RmtChannel0> NeoEsp32Rmt0800KbpsMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeed400Kbps, NeoEsp32RmtChannel0> NeoEsp32Rmt0400KbpsMethod;
typedef NeoEsp32Rmt0Ws2805Method NeoEsp32Rmt0Ws2814Method;

typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2811, NeoEsp32RmtChannel1> NeoEsp32Rmt1Ws2811Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2812x, NeoEsp32RmtChannel1> NeoEsp32Rmt1Ws2812xMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2812x, NeoEsp32RmtChannel1> NeoEsp32Rmt1Ws2816Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2805, NeoEsp32RmtChannel1> NeoEsp32Rmt1Ws2805Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedSk6812, NeoEsp32RmtChannel1> NeoEsp32Rmt1Sk6812Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1814, NeoEsp32RmtChannel1> NeoEsp32Rmt1Tm1814Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1829, NeoEsp32RmtChannel1> NeoEsp32Rmt1Tm1829Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1914, NeoEsp32RmtChannel1> NeoEsp32Rmt1Tm1914Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedApa106, NeoEsp32RmtChannel1> NeoEsp32Rmt1Apa106Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTx1812, NeoEsp32RmtChannel1> NeoEsp32Rmt1Tx1812Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedGs1903, NeoEsp32RmtChannel1> NeoEsp32Rmt1Gs1903Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeed800Kbps, NeoEsp32RmtChannel1> NeoEsp32Rmt1800KbpsMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeed400Kbps, NeoEsp32RmtChannel1> NeoEsp32Rmt1400KbpsMethod;
typedef NeoEsp32Rmt1Ws2805Method NeoEsp32Rmt1Ws2814Method;

#if !defined(CONFIG_IDF_TARGET_ESP32C3)

typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2811, NeoEsp32RmtChannel2> NeoEsp32Rmt2Ws2811Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2812x, NeoEsp32RmtChannel2> NeoEsp32Rmt2Ws2812xMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2812x, NeoEsp32RmtChannel2> NeoEsp32Rmt2Ws2816Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2805, NeoEsp32RmtChannel2> NeoEsp32Rmt2Ws2805Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedSk6812, NeoEsp32RmtChannel2>  NeoEsp32Rmt2Sk6812Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1814, NeoEsp32RmtChannel2> NeoEsp32Rmt2Tm1814Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1829, NeoEsp32RmtChannel2> NeoEsp32Rmt2Tm1829Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1914, NeoEsp32RmtChannel2> NeoEsp32Rmt2Tm1914Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedApa106, NeoEsp32RmtChannel2> NeoEsp32Rmt2Apa106Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTx1812, NeoEsp32RmtChannel2> NeoEsp32Rmt2Tx1812Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedGs1903, NeoEsp32RmtChannel2> NeoEsp32Rmt2Gs1903Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeed800Kbps, NeoEsp32RmtChannel2> NeoEsp32Rmt2800KbpsMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeed400Kbps, NeoEsp32RmtChannel2> NeoEsp32Rmt2400KbpsMethod;
typedef NeoEsp32Rmt2Ws2805Method NeoEsp32Rmt2Ws2814Method;

typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2811, NeoEsp32RmtChannel3> NeoEsp32Rmt3Ws2811Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2812x, NeoEsp32RmtChannel3> NeoEsp32Rmt3Ws2812xMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2812x, NeoEsp32RmtChannel3> NeoEsp32Rmt3Ws2816Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2805, NeoEsp32RmtChannel3> NeoEsp32Rmt3Ws2805Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedSk6812, NeoEsp32RmtChannel3>  NeoEsp32Rmt3Sk6812Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1814, NeoEsp32RmtChannel3> NeoEsp32Rmt3Tm1814Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1829, NeoEsp32RmtChannel3> NeoEsp32Rmt3Tm1829Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1914, NeoEsp32RmtChannel3> NeoEsp32Rmt3Tm1914Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedApa106, NeoEsp32RmtChannel3> NeoEsp32Rmt3Apa106Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTx1812, NeoEsp32RmtChannel3> NeoEsp32Rmt3Tx1812Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedGs1903, NeoEsp32RmtChannel3> NeoEsp32Rmt3Gs1903Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeed800Kbps, NeoEsp32RmtChannel3> NeoEsp32Rmt3800KbpsMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeed400Kbps, NeoEsp32RmtChannel3> NeoEsp32Rmt3400KbpsMethod;
typedef NeoEsp32Rmt3Ws2805Method NeoEsp32Rmt3Ws2814Method;

#if !defined(CONFIG_IDF_TARGET_ESP32S2) && !defined(CONFIG_IDF_TARGET_ESP32S3)

typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2811, NeoEsp32RmtChannel4> NeoEsp32Rmt4Ws2811Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2812x, NeoEsp32RmtChannel4> NeoEsp32Rmt4Ws2812xMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2812x, NeoEsp32RmtChannel4> NeoEsp32Rmt4Ws2816Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2805, NeoEsp32RmtChannel4> NeoEsp32Rmt4Ws2805Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedSk6812, NeoEsp32RmtChannel4>  NeoEsp32Rmt4Sk6812Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1814, NeoEsp32RmtChannel4> NeoEsp32Rmt4Tm1814Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1829, NeoEsp32RmtChannel4> NeoEsp32Rmt4Tm1829Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1914, NeoEsp32RmtChannel4> NeoEsp32Rmt4Tm1914Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedApa106, NeoEsp32RmtChannel4> NeoEsp32Rmt4Apa106Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTx1812, NeoEsp32RmtChannel4> NeoEsp32Rmt4Tx1812Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedGs1903, NeoEsp32RmtChannel4> NeoEsp32Rmt4Gs1903Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeed800Kbps, NeoEsp32RmtChannel4> NeoEsp32Rmt4800KbpsMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeed400Kbps, NeoEsp32RmtChannel4> NeoEsp32Rmt4400KbpsMethod;
typedef NeoEsp32Rmt4Ws2805Method NeoEsp32Rmt4Ws2814Method;

typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2811, NeoEsp32RmtChannel5> NeoEsp32Rmt5Ws2811Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2812x, NeoEsp32RmtChannel5> NeoEsp32Rmt5Ws2812xMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2812x, NeoEsp32RmtChannel5> NeoEsp32Rmt5Ws2816Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2805, NeoEsp32RmtChannel5> NeoEsp32Rmt5Ws2805Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedSk6812, NeoEsp32RmtChannel5>  NeoEsp32Rmt5Sk6812Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1814, NeoEsp32RmtChannel5> NeoEsp32Rmt5Tm1814Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1829, NeoEsp32RmtChannel5> NeoEsp32Rmt5Tm1829Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1914, NeoEsp32RmtChannel5> NeoEsp32Rmt5Tm1914Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedApa106, NeoEsp32RmtChannel5> NeoEsp32Rmt5Apa106Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTx1812, NeoEsp32RmtChannel5> NeoEsp32Rmt5Tx1812Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedGs1903, NeoEsp32RmtChannel5> NeoEsp32Rmt5Gs1903Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeed800Kbps, NeoEsp32RmtChannel5> NeoEsp32Rmt5800KbpsMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeed400Kbps, NeoEsp32RmtChannel5> NeoEsp32Rmt5400KbpsMethod;
typedef NeoEsp32Rmt5Ws2805Method NeoEsp32Rmt5Ws2814Method;

typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2811, NeoEsp32RmtChannel6> NeoEsp32Rmt6Ws2811Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2812x, NeoEsp32RmtChannel6> NeoEsp32Rmt6Ws2812xMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2812x, NeoEsp32RmtChannel6> NeoEsp32Rmt6Ws2816Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2805, NeoEsp32RmtChannel6> NeoEsp32Rmt6Ws2805Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedSk6812, NeoEsp32RmtChannel6>  NeoEsp32Rmt6Sk6812Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1814, NeoEsp32RmtChannel6> NeoEsp32Rmt6Tm1814Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1829, NeoEsp32RmtChannel6> NeoEsp32Rmt6Tm1829Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1914, NeoEsp32RmtChannel6> NeoEsp32Rmt6Tm1914Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedApa106, NeoEsp32RmtChannel6> NeoEsp32Rmt6Apa106Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTx1812, NeoEsp32RmtChannel6> NeoEsp32Rmt6Tx1812Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedGs1903, NeoEsp32RmtChannel6> NeoEsp32Rmt6Gs1903Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeed800Kbps, NeoEsp32RmtChannel6> NeoEsp32Rmt6800KbpsMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeed400Kbps, NeoEsp32RmtChannel6> NeoEsp32Rmt6400KbpsMethod;
typedef NeoEsp32Rmt6Ws2805Method NeoEsp32Rmt6Ws2814Method;

typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2811, NeoEsp32RmtChannel7> NeoEsp32Rmt7Ws2811Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2812x, NeoEsp32RmtChannel7> NeoEsp32Rmt7Ws2812xMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2812x, NeoEsp32RmtChannel7> NeoEsp32Rmt7Ws2816Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedWs2805, NeoEsp32RmtChannel7> NeoEsp32Rmt7Ws2805Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedSk6812, NeoEsp32RmtChannel7>  NeoEsp32Rmt7Sk6812Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1814, NeoEsp32RmtChannel7> NeoEsp32Rmt7Tm1814Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1829, NeoEsp32RmtChannel7> NeoEsp32Rmt7Tm1829Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTm1914, NeoEsp32RmtChannel7> NeoEsp32Rmt7Tm1914Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedApa106, NeoEsp32RmtChannel7> NeoEsp32Rmt7Apa106Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedTx1812, NeoEsp32RmtChannel7> NeoEsp32Rmt7Tx1812Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeedGs1903, NeoEsp32RmtChannel7> NeoEsp32Rmt7Gs1903Method;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeed800Kbps, NeoEsp32RmtChannel7> NeoEsp32Rmt7800KbpsMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtSpeed400Kbps, NeoEsp32RmtChannel7> NeoEsp32Rmt7400KbpsMethod;
typedef NeoEsp32Rmt7Ws2805Method NeoEsp32Rmt7Ws2814Method;

#endif // !defined(CONFIG_IDF_TARGET_ESP32S2) && !defined(CONFIG_IDF_TARGET_ESP32S3)
#endif // !defined(CONFIG_IDF_TARGET_ESP32C3)

// inverted
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2811, NeoEsp32RmtChannelN> NeoEsp32RmtNWs2811InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2812x, NeoEsp32RmtChannelN> NeoEsp32RmtNWs2812xInvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2812x, NeoEsp32RmtChannelN> NeoEsp32RmtNWs2816InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2805, NeoEsp32RmtChannelN> NeoEsp32RmtNWs2805InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedSk6812, NeoEsp32RmtChannelN> NeoEsp32RmtNSk6812InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1814, NeoEsp32RmtChannelN> NeoEsp32RmtNTm1814InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1829, NeoEsp32RmtChannelN> NeoEsp32RmtNTm1829InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1914, NeoEsp32RmtChannelN> NeoEsp32RmtNTm1914InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedApa106, NeoEsp32RmtChannelN> NeoEsp32RmtNApa106InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTx1812, NeoEsp32RmtChannelN> NeoEsp32RmtNTx1812InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedGs1903, NeoEsp32RmtChannelN> NeoEsp32RmtNGs1903InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeed800Kbps, NeoEsp32RmtChannelN> NeoEsp32RmtN800KbpsInvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeed400Kbps, NeoEsp32RmtChannelN> NeoEsp32RmtN400KbpsInvertedMethod;
typedef NeoEsp32RmtNWs2805InvertedMethod NeoEsp32RmtNWs2814InvertedMethod;

typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2811, NeoEsp32RmtChannel0> NeoEsp32Rmt0Ws2811InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2812x, NeoEsp32RmtChannel0> NeoEsp32Rmt0Ws2812xInvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2812x, NeoEsp32RmtChannel0> NeoEsp32Rmt0Ws2816InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2805, NeoEsp32RmtChannel0> NeoEsp32Rmt0Ws2805InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedSk6812, NeoEsp32RmtChannel0> NeoEsp32Rmt0Sk6812InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1814, NeoEsp32RmtChannel0> NeoEsp32Rmt0Tm1814InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1829, NeoEsp32RmtChannel0> NeoEsp32Rmt0Tm1829InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1914, NeoEsp32RmtChannel0> NeoEsp32Rmt0Tm1914InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedApa106, NeoEsp32RmtChannel0> NeoEsp32Rmt0Apa106InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTx1812, NeoEsp32RmtChannel0> NeoEsp32Rmt0Tx1812InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedGs1903, NeoEsp32RmtChannel0> NeoEsp32Rmt0Gs1903InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeed800Kbps, NeoEsp32RmtChannel0> NeoEsp32Rmt0800KbpsInvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeed400Kbps, NeoEsp32RmtChannel0> NeoEsp32Rmt0400KbpsInvertedMethod;
typedef NeoEsp32Rmt0Ws2805InvertedMethod NeoEsp32Rmt0Ws2814InvertedMethod;

typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2811, NeoEsp32RmtChannel1> NeoEsp32Rmt1Ws2811InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2812x, NeoEsp32RmtChannel1> NeoEsp32Rmt1Ws2812xInvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2812x, NeoEsp32RmtChannel1> NeoEsp32Rmt1Ws2816InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2805, NeoEsp32RmtChannel1> NeoEsp32Rmt1Ws2805InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedSk6812, NeoEsp32RmtChannel1> NeoEsp32Rmt1Sk6812InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1814, NeoEsp32RmtChannel1> NeoEsp32Rmt1Tm1814InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1829, NeoEsp32RmtChannel1> NeoEsp32Rmt1Tm1829InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1914, NeoEsp32RmtChannel1> NeoEsp32Rmt1Tm1914InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedApa106, NeoEsp32RmtChannel1> NeoEsp32Rmt1Apa106InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTx1812, NeoEsp32RmtChannel1> NeoEsp32Rmt1Tx1812InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedGs1903, NeoEsp32RmtChannel1> NeoEsp32Rmt1Gs1903InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeed800Kbps, NeoEsp32RmtChannel1> NeoEsp32Rmt1800KbpsInvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeed400Kbps, NeoEsp32RmtChannel1> NeoEsp32Rmt1400KbpsInvertedMethod;
typedef NeoEsp32Rmt1Ws2805InvertedMethod NeoEsp32Rmt1Ws2814InvertedMethod;

#if !defined(CONFIG_IDF_TARGET_ESP32C3)

typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2811, NeoEsp32RmtChannel2> NeoEsp32Rmt2Ws2811InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2812x, NeoEsp32RmtChannel2> NeoEsp32Rmt2Ws2812xInvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2812x, NeoEsp32RmtChannel2> NeoEsp32Rmt2Ws2816InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2805, NeoEsp32RmtChannel2> NeoEsp32Rmt2Ws2805InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedSk6812, NeoEsp32RmtChannel2>  NeoEsp32Rmt2Sk6812InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1814, NeoEsp32RmtChannel2> NeoEsp32Rmt2Tm1814InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1829, NeoEsp32RmtChannel2> NeoEsp32Rmt2Tm1829InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1914, NeoEsp32RmtChannel2> NeoEsp32Rmt2Tm1914InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedApa106, NeoEsp32RmtChannel2> NeoEsp32Rmt2Apa106InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTx1812, NeoEsp32RmtChannel2> NeoEsp32Rmt2Tx1812InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedGs1903, NeoEsp32RmtChannel2> NeoEsp32Rmt2Gs1903InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeed800Kbps, NeoEsp32RmtChannel2> NeoEsp32Rmt2800KbpsInvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeed400Kbps, NeoEsp32RmtChannel2> NeoEsp32Rmt2400KbpsInvertedMethod;
typedef NeoEsp32Rmt2Ws2805InvertedMethod NeoEsp32Rmt2Ws2814InvertedMethod;

typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2811, NeoEsp32RmtChannel3> NeoEsp32Rmt3Ws2811InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2812x, NeoEsp32RmtChannel3> NeoEsp32Rmt3Ws2812xInvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2805, NeoEsp32RmtChannel3> NeoEsp32Rmt3Ws2805InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2812x, NeoEsp32RmtChannel3> NeoEsp32Rmt3Ws2816InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedSk6812, NeoEsp32RmtChannel3>  NeoEsp32Rmt3Sk6812InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1814, NeoEsp32RmtChannel3> NeoEsp32Rmt3Tm1814InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1829, NeoEsp32RmtChannel3> NeoEsp32Rmt3Tm1829InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1914, NeoEsp32RmtChannel3> NeoEsp32Rmt3Tm1914InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedApa106, NeoEsp32RmtChannel3> NeoEsp32Rmt3Apa106InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTx1812, NeoEsp32RmtChannel3> NeoEsp32Rmt3Tx1812InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedGs1903, NeoEsp32RmtChannel3> NeoEsp32Rmt3Gs1903InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeed800Kbps, NeoEsp32RmtChannel3> NeoEsp32Rmt3800KbpsInvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeed400Kbps, NeoEsp32RmtChannel3> NeoEsp32Rmt3400KbpsInvertedMethod;
typedef NeoEsp32Rmt3Ws2805InvertedMethod NeoEsp32Rmt3Ws2814InvertedMethod;

#if !defined(CONFIG_IDF_TARGET_ESP32S2) && !defined(CONFIG_IDF_TARGET_ESP32S3)

typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2811, NeoEsp32RmtChannel4> NeoEsp32Rmt4Ws2811InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2812x, NeoEsp32RmtChannel4> NeoEsp32Rmt4Ws2812xInvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2812x, NeoEsp32RmtChannel4> NeoEsp32Rmt4Ws2816InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2805, NeoEsp32RmtChannel4> NeoEsp32Rmt4Ws2805InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedSk6812, NeoEsp32RmtChannel4>  NeoEsp32Rmt4Sk6812InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1814, NeoEsp32RmtChannel4> NeoEsp32Rmt4Tm1814InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1829, NeoEsp32RmtChannel4> NeoEsp32Rmt4Tm1829InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1914, NeoEsp32RmtChannel4> NeoEsp32Rmt4Tm1914InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedApa106, NeoEsp32RmtChannel4> NeoEsp32Rmt4Apa106InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTx1812, NeoEsp32RmtChannel4> NeoEsp32Rmt4Tx1812InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedGs1903, NeoEsp32RmtChannel4> NeoEsp32Rmt4Gs1903InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeed800Kbps, NeoEsp32RmtChannel4> NeoEsp32Rmt4800KbpsInvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeed400Kbps, NeoEsp32RmtChannel4> NeoEsp32Rmt4400KbpsInvertedMethod;
typedef NeoEsp32Rmt4Ws2805InvertedMethod NeoEsp32Rmt4Ws2814InvertedMethod;

typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2811, NeoEsp32RmtChannel5> NeoEsp32Rmt5Ws2811InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2812x, NeoEsp32RmtChannel5> NeoEsp32Rmt5Ws2812xInvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2812x, NeoEsp32RmtChannel5> NeoEsp32Rmt5Ws2816InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2805, NeoEsp32RmtChannel5> NeoEsp32Rmt5Ws2805InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedSk6812, NeoEsp32RmtChannel5>  NeoEsp32Rmt5Sk6812InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1814, NeoEsp32RmtChannel5> NeoEsp32Rmt5Tm1814InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1829, NeoEsp32RmtChannel5> NeoEsp32Rmt5Tm1829InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1914, NeoEsp32RmtChannel5> NeoEsp32Rmt5Tm1914InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedApa106, NeoEsp32RmtChannel5> NeoEsp32Rmt5Apa106InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTx1812, NeoEsp32RmtChannel5> NeoEsp32Rmt5Tx1812InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedGs1903, NeoEsp32RmtChannel5> NeoEsp32Rmt5Gs1903InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeed800Kbps, NeoEsp32RmtChannel5> NeoEsp32Rmt5800KbpsInvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeed400Kbps, NeoEsp32RmtChannel5> NeoEsp32Rmt5400KbpsInvertedMethod;
typedef NeoEsp32Rmt5Ws2805InvertedMethod NeoEsp32Rmt5Ws2814InvertedMethod;

typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2811, NeoEsp32RmtChannel6> NeoEsp32Rmt6Ws2811InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2812x, NeoEsp32RmtChannel6> NeoEsp32Rmt6Ws2812xInvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2812x, NeoEsp32RmtChannel6> NeoEsp32Rmt6Ws2816InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2805, NeoEsp32RmtChannel6> NeoEsp32Rmt6Ws2805InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedSk6812, NeoEsp32RmtChannel6>  NeoEsp32Rmt6Sk6812InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1814, NeoEsp32RmtChannel6> NeoEsp32Rmt6Tm1814InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1829, NeoEsp32RmtChannel6> NeoEsp32Rmt6Tm1829InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1914, NeoEsp32RmtChannel6> NeoEsp32Rmt6Tm1914InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedApa106, NeoEsp32RmtChannel6> NeoEsp32Rmt6Apa106InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTx1812, NeoEsp32RmtChannel6> NeoEsp32Rmt6Tx1812InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedGs1903, NeoEsp32RmtChannel6> NeoEsp32Rmt6Gs1903InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeed800Kbps, NeoEsp32RmtChannel6> NeoEsp32Rmt6800KbpsInvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeed400Kbps, NeoEsp32RmtChannel6> NeoEsp32Rmt6400KbpsInvertedMethod;
typedef NeoEsp32Rmt6Ws2805InvertedMethod NeoEsp32Rmt6Ws2814InvertedMethod;

typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2811, NeoEsp32RmtChannel7> NeoEsp32Rmt7Ws2811InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2812x, NeoEsp32RmtChannel7> NeoEsp32Rmt7Ws2812xInvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2812x, NeoEsp32RmtChannel7> NeoEsp32Rmt7Ws2816InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedWs2805, NeoEsp32RmtChannel7> NeoEsp32Rmt7Ws2805InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedSk6812, NeoEsp32RmtChannel7>  NeoEsp32Rmt7Sk6812InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1814, NeoEsp32RmtChannel7> NeoEsp32Rmt7Tm1814InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1829, NeoEsp32RmtChannel7> NeoEsp32Rmt7Tm1829InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTm1914, NeoEsp32RmtChannel7> NeoEsp32Rmt7Tm1914InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedApa106, NeoEsp32RmtChannel7> NeoEsp32Rmt7Apa106InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedTx1812, NeoEsp32RmtChannel7> NeoEsp32Rmt7Tx1812InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeedGs1903, NeoEsp32RmtChannel7> NeoEsp32Rmt7Gs1903InvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeed800Kbps, NeoEsp32RmtChannel7> NeoEsp32Rmt7800KbpsInvertedMethod;
typedef NeoEsp32RmtMethodBase<NeoEsp32RmtInvertedSpeed400Kbps, NeoEsp32RmtChannel7> NeoEsp32Rmt7400KbpsInvertedMethod;
typedef NeoEsp32Rmt7Ws2805InvertedMethod NeoEsp32Rmt7Ws2814InvertedMethod;

#endif // !defined(CONFIG_IDF_TARGET_ESP32S2) && !defined(CONFIG_IDF_TARGET_ESP32S3)
#endif // !defined(CONFIG_IDF_TARGET_ESP32C3)

#if defined(NEOPIXEL_ESP32_RMT_DEFAULT) || defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32S3)

#if defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32S3)

typedef NeoEsp32Rmt1Ws2812xMethod NeoWs2813Method;
typedef NeoEsp32Rmt1Ws2812xMethod NeoWs2812xMethod;
typedef NeoEsp32Rmt1800KbpsMethod NeoWs2812Method;
typedef NeoEsp32Rmt1Ws2812xMethod NeoWs2811Method;
typedef NeoEsp32Rmt1Ws2812xMethod NeoWs2816Method;
typedef NeoEsp32Rmt1Ws2805Method NeoWs2805Method;
typedef NeoEsp32Rmt1Ws2814Method NeoWs2814Method;
typedef NeoEsp32Rmt1Sk6812Method NeoSk6812Method;
typedef NeoEsp32Rmt1Tm1814Method NeoTm1814Method;
typedef NeoEsp32Rmt1Tm1829Method NeoTm1829Method;
typedef NeoEsp32Rmt1Tm1914Method NeoTm1914Method;
typedef NeoEsp32Rmt1Sk6812Method NeoLc8812Method;
typedef NeoEsp32Rmt1Apa106Method NeoApa106Method;
typedef NeoEsp32Rmt1Tx1812Method NeoTx1812Method;
typedef NeoEsp32Rmt1Gs1903Method NeoGs1903Method;

typedef NeoEsp32Rmt1Ws2812xMethod Neo800KbpsMethod;
typedef NeoEsp32Rmt1400KbpsMethod Neo400KbpsMethod;

typedef NeoEsp32Rmt1Ws2812xInvertedMethod NeoWs2813InvertedMethod;
typedef NeoEsp32Rmt1Ws2812xInvertedMethod NeoWs2812xInvertedMethod;
typedef NeoEsp32Rmt1Ws2812xInvertedMethod NeoWs2811InvertedMethod;
typedef NeoEsp32Rmt1800KbpsInvertedMethod NeoWs2812InvertedMethod;
typedef NeoEsp32Rmt1Ws2812xInvertedMethod NeoWs2816InvertedMethod;
typedef NeoEsp32Rmt1Ws2805InvertedMethod NeoWs2805InvertedMethod;
typedef NeoEsp32Rmt1Ws2814InvertedMethod NeoWs2814InvertedMethod;
typedef NeoEsp32Rmt1Sk6812InvertedMethod NeoSk6812InvertedMethod;

#else // defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32S3)

typedef NeoEsp32Rmt6Ws2812xMethod NeoWs2813Method;
typedef NeoEsp32Rmt6Ws2812xMethod NeoWs2812xMethod;
typedef NeoEsp32Rmt6800KbpsMethod NeoWs2812Method;
typedef NeoEsp32Rmt6Ws2812xMethod NeoWs2811Method;
typedef NeoEsp32Rmt6Ws2812xMethod NeoWs2816Method;
typedef NeoEsp32Rmt6Ws2805Method NeoWs2805Method;
typedef NeoEsp32Rmt6Ws2814Method NeoWs2814Method;
typedef NeoEsp32Rmt6Sk6812Method NeoSk6812Method;
typedef NeoEsp32Rmt6Tm1814Method NeoTm1814Method;
typedef NeoEsp32Rmt6Tm1829Method NeoTm1829Method;
typedef NeoEsp32Rmt6Tm1914Method NeoTm1914Method;
typedef NeoEsp32Rmt6Sk6812Method NeoLc8812Method;
typedef NeoEsp32Rmt6Apa106Method NeoApa106Method;
typedef NeoEsp32Rmt6Tx1812Method NeoTx1812Method;
typedef NeoEsp32Rmt6Gs1903Method NeoGs1903Method;

typedef NeoEsp32Rmt6Ws2812xMethod Neo800KbpsMethod;
typedef NeoEsp32Rmt6400KbpsMethod Neo400KbpsMethod;

typedef NeoEsp32Rmt6Ws2812xInvertedMethod NeoWs2813InvertedMethod;
typedef NeoEsp32Rmt6Ws2812xInvertedMethod NeoWs2812xInvertedMethod;
typedef NeoEsp32Rmt6Ws2812xInvertedMethod NeoWs2811InvertedMethod;
typedef NeoEsp32Rmt6800KbpsInvertedMethod NeoWs2812InvertedMethod;
typedef NeoEsp32Rmt6Ws2812xInvertedMethod NeoWs2816InvertedMethod;
typedef NeoEsp32Rmt6Ws2805InvertedMethod NeoWs2805InvertedMethod;
typedef NeoEsp32Rmt6Ws2814InvertedMethod NeoWs2814InvertedMethod;
typedef NeoEsp32Rmt6Sk6812InvertedMethod NeoSk6812InvertedMethod;

#endif // defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32S3)

#endif // defined(NEOPIXEL_ESP32_RMT_DEFAULT) || defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32S3)

#endif // ARDUINO_ARCH_ESP32
