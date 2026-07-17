# Embedded Communication Protocols

A hands-on STM32F407 learning repository for embedded communication protocols.  
Each directory is an independent STM32CubeIDE project that moves from peripheral bring-up to practical device integration and industry-relevant communication patterns.

The projects use an STM32F407VGTx target, STM32CubeMX-generated configuration (`.ioc`), STM32 HAL/CMSIS support files, and custom peripheral/device drivers where appropriate. This is a learning and reference repository: projects are intentionally kept separate so that each communication technique can be studied, built, and debugged in isolation.

## What this repository covers

| Protocol | Focus areas | External devices / scenarios |
| --- | --- | --- |
| **USART / UART** | Register-level polling and receive interrupts; circular receive buffers; DMA transmit in direct and FIFO modes; DMA receive; circular DMA; UART IDLE-line detection; command-line parsing | Serial terminal / host interface |
| **SPI** | Polling transfers; chip-select control; device-driver development; status polling; write-enable and busy handling | MAX7219 LED matrix driver, W25Q64FV SPI flash, MCP2515 CAN controller |
| **I²C** | Bus/device initialization and reusable driver development | AT24C256 EEPROM, ADS1115 ADC in one-shot and continuous-conversion use cases |
| **CAN** | Internal bxCAN initialization; loopback and normal mode; receive interrupts; two-node communication | STM32 internal CAN and external MCP2515-based CAN nodes |

## Repository layout

Each top-level project is self-contained and normally contains:

```text
<project>/
├── Core/                 # Application code, interrupt handlers, peripheral setup
├── Drivers/              # STM32 HAL and CMSIS packages
├── <project>.ioc         # STM32CubeMX configuration
├── .project/.cproject    # STM32CubeIDE project metadata
└── Debug/ or Release/    # Local build output (ignored by Git)
```

`Detailed Documents/` contains the companion learning documents for the USART, SPI, I²C, and CAN exercises.

## Learning path and projects

### USART / UART

| Project | Demonstrates |
| --- | --- |
| `UART_Baremetal_Initial_Code` | Initial register-level UART bring-up and polling-based communication |
| `USART_Baremetal_Interrupt_Based` | Receive interrupt handling with `USART2_IRQHandler` |
| `USART_Baremetal_Rx_CircularBuffer` | ISR-to-application producer/consumer flow using a receive ring buffer and overflow handling |
| `USART_TX_DMA_Direct_Mode` | DMA-based USART transmit using direct mode |
| `USART_DMA_TX_FIFO_Mode` | DMA-based USART transmit using FIFO mode |
| `USART_DMA_RX` | DMA-based USART receive |
| `USART_DMA_RX_Circular_Mode` | Continuous receive with circular DMA |
| `USART_DMA_RX_CIRC_Idle_Detection` | Circular DMA receive combined with UART IDLE-line detection for variable-length frames |
| `USART_CLI_Idle_Detection` | DMA/IDLE-driven receive path used by a simple UART command-line interface |

These exercises include patterns commonly needed in firmware that receives asynchronous serial data: non-blocking transfers, interrupt-driven producer/consumer separation, buffer wrap-around, overflow detection, and end-of-frame detection without a fixed message length.

### SPI

| Project | Device / scenario | Demonstrates |
| --- | --- | --- |
| `SPI_Polling_Based_TX` | Generic SPI transfer | Polling-based SPI transmit fundamentals |
| `SPI2_MAX7219` | MAX7219 LED matrix / display driver | SPI device initialization, register writes, display control |
| `SPI2_W25Q_Flash_Init` | W25Q64FV serial flash | Flash identification/status checks and basic initialization |
| `SPI2_W25Q_Flash_Driver_Development` | W25Q64FV serial flash | Custom flash driver work, write-enable sequencing, busy polling, and status-register access |
| `SPI2_MCP2515_Polling_Based` | MCP2515 external CAN controller | SPI initialization, reset, register/status reads, and controller-mode verification |
| `SPI2_MCP2515_LoopBack` | MCP2515 external CAN controller | MCP2515 loopback-mode validation |
| `SPI2_MCP2515_2Nodes_Loopback` | Two MCP2515 nodes | Selecting nodes/SPI buses and two-node loopback testing |
| `SPI_MCP2515_NormalMode_2Nodes` | Two MCP2515 nodes | Normal-mode CAN message exchange over external controllers |
| `SPI_MCP2515_Receive_Interrupt` | Two MCP2515 nodes | Interrupt-driven reception from the MCP2515 |

The MCP2515 projects make the relationship between SPI device control and CAN communication explicit: the microcontroller configures and services the external CAN controller over SPI, while CAN frames are exchanged by the MCP2515 devices.

### I²C

| Project | Device / scenario | Demonstrates |
| --- | --- | --- |
| `I2C_EEPROM_Init` | AT24C256 EEPROM | I²C initialization, GPIO setup, and byte-level EEPROM access |
| `I2C_EEPROM_Driver_Development` | AT24C256 EEPROM | Reusable EEPROM driver development |
| `I2C_ADC_ADS1115_Init` | ADS1115 ADC | ADC bring-up and configuration over I²C |
| `I2C_ADC_ADS1115_driver_development` | ADS1115 ADC | One-shot and continuous-conversion driver development |

### CAN

| Project | Demonstrates |
| --- | --- |
| `CAN_Internal_Initialization` | STM32 internal CAN peripheral initialization |
| `CAN1_Loopback` | Internal CAN loopback for local validation without a physical bus |
| `CAN1_Normal_Mode` | Internal CAN normal-mode communication |
| `CAN1_RX_Interrupt` | CAN receive interrupt handling |

For external CAN work, see the MCP2515 projects in the SPI section above. Together, the projects cover both STM32’s internal CAN peripheral and an SPI-attached external CAN-controller workflow.

## Hardware and software

### Target platform

- STM32F407VGTx microcontroller
- STM32CubeMX configuration files and STM32CubeIDE project files
- STM32 HAL and CMSIS device support included per project
- An ST-LINK-compatible debug/programming connection is expected for hardware testing

### External devices used

- MCP2515 CAN controller module(s)
- W25Q64FV SPI NOR flash
- MAX7219 LED matrix/display driver
- AT24C256 I²C EEPROM
- ADS1115 I²C ADC
- A UART serial terminal/host connection for observing and exercising USART examples

Actual pin assignments, peripheral instances, clock settings, and generated initialization code are available in each project's `.ioc` file and `Core/` sources. Check the specific project before wiring hardware, because configuration can vary between exercises.

## Getting started

1. Install STM32CubeIDE with the STM32F4 device package.
2. Clone the repository.

   ```bash
   git clone <repository-url>
   ```

3. In STM32CubeIDE, use **File → Import → Existing Projects into Workspace** and select the repository directory (or import only the project you want to study).
4. Open that project’s `.ioc` file if you need to review or regenerate CubeMX configuration.
5. Build one project at a time, connect only the hardware required by that example, then flash and debug it.

> Each directory is a standalone project, not a single combined firmware image. Do not attempt to build all folders as one application.

## Where to begin

A useful progression is:

1. **USART:** `UART_Baremetal_Initial_Code` → interrupt-based receive → circular buffer → DMA → circular DMA with IDLE detection.
2. **SPI:** `SPI_Polling_Based_TX` → MAX7219 → W25Q64FV driver → MCP2515 controller and two-node CAN tests.
3. **I²C:** EEPROM initialization → EEPROM driver → ADS1115 initialization → ADC driver development.
4. **CAN:** internal CAN initialization → loopback → normal mode → receive interrupt; then compare that workflow with the MCP2515 external-CAN exercises.

## Notes

- Generated HAL/CMSIS files are included to keep individual projects self-contained.
- `Debug/`, `Release/`, and common compiler artifacts are excluded through `.gitignore`.
- The repository is intended for learning and experimentation. Treat it as a reference and validate timing, electrical interfaces, bus termination, addressing, and error handling for any production design.

## Documentation

Supporting documents are organized under `Detailed Documents/`:

- `Detailed Documents/USART/`
- `Detailed Documents/SPI/`
- `Detailed Documents/I2C/`
- `Detailed Documents/CAN/`

They complement the source projects with protocol foundations, project explanations, and device-specific implementation notes.
