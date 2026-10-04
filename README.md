# Embedded AI

Practical Embedded AI / Edge AI deployment examples for microcontroller-based systems.

This repository currently contains an **LM35 temperature-classification example for the RaayanMini platform**, using a pre-trained TensorFlow Lite model deployed on an STM32 microcontroller with **STM32CubeMX + X-CUBE-AI**.

The repository is intentionally **deployment-focused**:

- pre-trained TensorFlow Lite models are provided,
- STM32/RaayanMini firmware is provided,
- boundary test cases are provided,
- all required deployment steps are documented here.

The model-training notebook and detailed training workflow are outside the scope of this public repository.

---

## Table of Contents

- [1. Project Overview](#1-project-overview)
- [2. Hardware and Software](#2-hardware-and-software)
- [3. Repository Structure](#3-repository-structure)
- [4. Install X-CUBE-AI](#4-install-x-cube-ai)
- [5. Deploy the Model to RaayanMini](#5-deploy-the-model-to-raayanmini)
- [6. Runtime Firmware Flow](#6-runtime-firmware-flow)
- [7. Sensor Preprocessing](#7-sensor-preprocessing)
- [8. Model Preprocessing](#8-model-preprocessing)
- [9. Model Output](#9-model-output)
- [10. Validate the Deployment](#10-validate-the-deployment)
- [11. Debugging](#11-debugging)
- [12. Important Engineering Notes](#12-important-engineering-notes)
- [13. Project Limitations](#13-project-limitations)
- [14. Official References](#14-official-references)
- [15. About](#15-about)

---

## 1. Project Overview

### 1.1 What this project does

The application performs the following runtime sequence:

```text
LM35 temperature sensor
        ↓
STM32 ADC
        ↓
Temperature in °C
        ↓
Normalization
        ↓
INT8 quantization
        ↓
X-CUBE-AI inference
        ↓
COLD / NORMAL / HOT
        ↓
16×2 LCD
```

### 1.2 Why this example uses AI

For fixed temperature thresholds, normal C `if/else` logic would be simpler.

This project intentionally uses a neural network so that the complete **Embedded-AI deployment workflow** can be demonstrated on a problem whose expected result is easy to verify.

### 1.3 Classification rules

| Output index | Class | Temperature range |
|---:|---|---|
| `0` | `COLD` | `< 27.0 °C` |
| `1` | `NORMAL` | `27.0 °C` to `< 35.0 °C` |
| `2` | `HOT` | `>= 35.0 °C` |

Important boundary checks:

```text
26.99 °C → COLD
27.00 °C → NORMAL

34.99 °C → NORMAL
35.00 °C → HOT
```

---

## 2. Hardware and Software

### 2.1 Target hardware

The current reference firmware targets **RaayanMini using STM32F401RBT6**.

Relevant MCU features:

- Arm Cortex-M4
- single-precision FPU
- up to 84 MHz
- 12-bit ADC

The reference CubeMX project uses an 84 MHz system clock.

### 2.2 Hardware connections

#### LM35

```text
LM35 output → PC0
PC0         → ADC1_IN10
```

Reference ADC configuration:

```text
ADC        : ADC1
Channel    : 10
Pin        : PC0
Resolution : 12-bit
Trigger    : software
Mode       : single conversion
```

#### 16×2 LCD

| LCD signal | STM32 pin |
|---|---|
| `D4` | PB0 |
| `D5` | PB1 |
| `D6` | PB2 |
| `D7` | PB3 |
| `RS` | PB4 |
| `RW` | PB5 |
| `EN` | PB8 |

### 2.3 Software requirements

Install:

- STM32CubeMX
- STM32CubeIDE
- X-CUBE-AI software pack

Official tools:

- STM32CubeMX: https://www.st.com/stm32cubemx
- STM32CubeIDE: https://www.st.com/stm32cubeide
- X-CUBE-AI: https://www.st.com/en/embedded-software/x-cube-ai.html

---

## 3. Repository Structure

```text
Embedded-AI/
│
├── README.md
├── .gitignore
├── LICENSE
│
├── Models/
│   ├── temperature_classifier_logits_float32.tflite
│   └── temperature_classifier_logits_int8.tflite
│
├── Test/
│   └── lm35_temperature_dataset_anchored_clean.csv
│
└── STM32/
    └── LM35_AI/
        ├── LM35_AI.ioc
        ├── .project
        ├── .cproject
        ├── Core/
        ├── Drivers/
        └── X-CUBE-AI/
```

### 3.1 Model files

#### Float32 model

```text
Models/temperature_classifier_logits_float32.tflite
```

Provided as a reference model.

#### INT8 model

```text
Models/temperature_classifier_logits_int8.tflite
```

Recommended for the RaayanMini deployment example.

### 3.2 Test cases

```text
Test/LM35_AI_BOUNDARY_TEST_CASES.csv
```

Contains known expected classifications for validating the deployed model.

---

## 4. Install X-CUBE-AI

### 4.1 Install STM32CubeMX

Download and install STM32CubeMX:

https://www.st.com/stm32cubemx

### 4.2 Install the X-CUBE-AI software pack

Open STM32CubeMX and go to:

```text
Help
  ↓
Manage embedded software packages
```

Then:

1. refresh the package list,
2. open the STMicroelectronics package section,
3. locate **X-CUBE-AI**,
4. install the required version,
5. allow CubeMX to download any required supporting components.

The reference project was generated using **X-CUBE-AI 10.2.1**.

If another X-CUBE-AI version is used, regenerate the AI code and runtime together.

---

## 5. Deploy the Model to RaayanMini

### 5.1 Download the repository

From the GitHub repository page:

1. Click **Code**.
2. Select **Download ZIP**.
3. Extract the downloaded ZIP file to a local folder.

For example:

```text
C:\Embedded_AI\
```

After extraction, open the project from the extracted repository folder.

Verify that the following file exists:

```text
Models/temperature_classifier_logits_int8.tflite
```

### 5.2 Open the CubeMX project

Open:

```text
STM32/LM35_AI/LM35_AI.ioc
```

CubeMX should load:

- STM32F401RBT6
- ADC configuration
- GPIO/LCD configuration
- clock configuration
- X-CUBE-AI configuration

### 5.3 Re-select the supplied INT8 model

After downloading and extracting the repository, the `.ioc` may still contain an absolute model path from the original development PC.

In the X-CUBE-AI configuration:

1. select the configured network,
2. browse for the model,
3. choose:

```text
Models/temperature_classifier_logits_int8.tflite
```

### 5.4 Analyze the model

Run **Analyze** in X-CUBE-AI.

For the supplied model, the important checks are approximately:

```text
Input:
  INT8
  1 element

Output:
  INT8
  3 elements

Parameters:
  355

MACC:
  355

Weights:
  ~460 bytes

Activations:
  ~224 bytes
```

The most important requirement is:

```text
1 INT8 input
3 INT8 outputs
```

### 5.5 Generate code

After analysis:

1. open **Project Manager**,
2. verify project output settings,
3. select STM32CubeIDE as the toolchain,
4. click **Generate Code**.

Typical generated X-CUBE-AI files include:

```text
X-CUBE-AI/App/
├── app_x-cube-ai.c
├── app_x-cube-ai.h
├── network.c
├── network.h
├── network_data.c
├── network_data.h
├── network_data_params.c
├── network_data_params.h
├── network_config.h
└── network_generate_report.txt
```

### 5.6 Build in STM32CubeIDE

Open/import the generated STM32CubeIDE project.

Recommended workflow:

```text
Project
  ↓
Clean
  ↓
Build Project
```

### 5.7 Flash RaayanMini

Connect the board through its ST-LINK/debug interface.

Then:

1. select the project,
2. choose **Run** or **Debug**,
3. flash the MCU,
4. reset/run the application.

The LCD should show the measured temperature and predicted class.

---

## 6. Runtime Firmware Flow

### 6.1 High-level flow

The application runtime is divided into three logical stages:

```text
acquire_and_process_data()
        ↓
ai_run()
        ↓
post_process()
```

### 6.2 Input acquisition and preprocessing

```text
ADC
 ↓
raw sample
 ↓
temperature °C
 ↓
normalization
 ↓
INT8 quantization
 ↓
AI input tensor
```

### 6.3 Neural-network inference

The actual network execution occurs inside:

```c
ai_network_run(
    network,
    ai_input,
    ai_output
);
```

### 6.4 Output processing

```text
3 INT8 outputs
      ↓
argmax
      ↓
0 / 1 / 2
      ↓
COLD / NORMAL / HOT
```

---

## 7. Sensor Preprocessing

### 7.1 ADC raw value to voltage

The ADC is 12-bit:

```text
0 … 4095
```

For the reference implementation:

```c
float adc_voltage =
    ((float)adc_raw * 3.3f) / 4095.0f;
```

### 7.2 LM35 voltage to Celsius

LM35 nominal output scale:

```text
10 mV / °C
```

Therefore:

```c
float temperature_c =
    adc_voltage * 100.0f;
```

Combined:

```c
float temperature_c =
    ((float)adc_raw * 3.3f * 100.0f)
    / 4095.0f;
```

### 7.3 Important voltage note

LM35 supply voltage and STM32 ADC reference voltage are different.

The LM35 may be powered from 5 V while the STM32 ADC still uses an approximately 3.3 V analog reference.

Do not apply 5 V directly to STM32 VDDA or an ADC input.

---

## 8. Model Preprocessing

### 8.1 Normalization

The supplied model expects normalized temperature input.

Current constants:

```c
#define TEMP_MEAN  31.02758789f
#define TEMP_STD    4.34976196f
```

Firmware preprocessing:

```c
float normalized =
    (temperature_c - TEMP_MEAN)
    / TEMP_STD;
```

These constants belong to the supplied model. If the model is replaced, verify the preprocessing constants again.

### 8.2 INT8 quantization

The deployment model uses signed INT8 input.

The firmware retrieves the quantization parameters directly from X-CUBE-AI metadata:

```c
float input_scale =
    AI_BUFFER_META_INFO_INTQ_GET_SCALE(
        ai_input[0].meta_info,
        0
    );

int32_t input_zero_point =
    AI_BUFFER_META_INFO_INTQ_GET_ZEROPOINT(
        ai_input[0].meta_info,
        0
    );
```

Conceptually:

```text
q =
round(normalized / scale)
+
zero_point
```

Then clamp:

```text
-128 … 127
```

Example helper:

```c
static ai_i8 quantize_int8(
    float value,
    float scale,
    int32_t zero_point)
{
    float qf =
        (value / scale)
        + (float)zero_point;

    int32_t q;

    if (qf >= 0.0f)
        q = (int32_t)(qf + 0.5f);
    else
        q = (int32_t)(qf - 0.5f);

    if (q > 127)
        q = 127;

    if (q < -128)
        q = -128;

    return (ai_i8)q;
}
```

Feed the model:

```c
data[0][0] =
    quantize_int8(
        normalized,
        input_scale,
        input_zero_point
    );
```

---

## 9. Model Output

### 9.1 Output mapping

```text
output[0] → COLD
output[1] → NORMAL
output[2] → HOT
```

### 9.2 Argmax

The application chooses the largest output:

```c
int max_index = 0;

if (output[1] > output[max_index])
{
    max_index = 1;
}

if (output[2] > output[max_index])
{
    max_index = 2;
}
```

Then:

```text
0 → COLD
1 → NORMAL
2 → HOT
```

Softmax is not required because only the winning class is needed.

---

## 10. Validate the Deployment

### 10.1 Boundary test cases

Use:

```text
Test/LM35_AI_BOUNDARY_TEST_CASES.csv
```

Important expected values:

| Temperature | Expected class |
|---:|---|
| 24.00 °C | COLD |
| 26.99 °C | COLD |
| 27.00 °C | NORMAL |
| 27.01 °C | NORMAL |
| 30.00 °C | NORMAL |
| 34.99 °C | NORMAL |
| 35.00 °C | HOT |
| 35.01 °C | HOT |
| 40.00 °C | HOT |

### 10.2 Recommended verification method

For exact boundary testing, temporarily provide a known software temperature to the preprocessing pipeline.

This is more repeatable than trying to physically hold the LM35 at exactly 27.00 °C or 35.00 °C.

---

## 11. Debugging

### 11.1 Recommended checkpoints

Inspect:

```text
adc_raw
temperature_c
normalized
quantized_input
output[0]
output[1]
output[2]
predicted_class
```

### 11.2 Debugging order

```text
1. ADC raw plausible?
        ↓
2. Temperature correct?
        ↓
3. Normalized value correct?
        ↓
4. INT8 model input correct?
        ↓
5. ai_network_run() successful?
        ↓
6. Outputs valid?
        ↓
7. Class mapping correct?
```

### 11.3 Common issues

#### CubeMX cannot find the TFLite model

Re-select:

```text
Models/temperature_classifier_logits_int8.tflite
```

inside X-CUBE-AI.

#### X-CUBE-AI shows Float32 input

The Float32 model was probably selected accidentally.

Use:

```text
temperature_classifier_logits_int8.tflite
```

#### PC result and STM32 result differ

Check:

```text
same model?
same input temperature?
same normalization?
same INT8 quantization?
same class mapping?
```

#### Temperature itself is wrong

Check the sensor and ADC before debugging the neural network:

- wiring
- ADC channel
- ADC raw value
- VREF/VDDA assumption
- LM35 conversion

#### Prediction flickers near class boundaries

Possible causes:

- ADC noise
- sensor noise
- actual temperature movement
- quantization
- sharp class boundary

Possible later improvements:

- ADC averaging
- filtering
- hysteresis

---

## 12. Important Engineering Notes

### 12.1 STM32 performs inference only

The supplied model is already trained.

The MCU performs:

```text
inference
```

not training.

### 12.2 Preprocessing is part of the model interface

Correct:

```text
temperature
↓
normalization
↓
INT8 conversion
↓
model
```

Incorrect:

```text
temperature
↓
model
```

### 12.3 Model memory is not total firmware memory

X-CUBE-AI network values such as:

```text
weights     ≈ 460 B
activations ≈ 224 B
```

do not represent the complete firmware footprint.

The application also includes:

- HAL
- startup/runtime
- LCD code
- AI runtime
- stack/heap
- application logic

Use the IDE build memory report or linker map for total system usage.

### 12.4 LCD refresh is not inference latency

LCD delays can dominate visible update time.

Measure inference latency around the actual AI execution call if performance measurement is required.

---

## 13. Project Limitations

This example intentionally simplifies a real Embedded-AI application.

The supplied model:

- uses one scalar input,
- uses predefined class boundaries,
- is very small,
- is intended primarily to demonstrate deployment,
- does not replace the need for proper sensor calibration or input validation.

For fixed temperature thresholds, normal C logic remains simpler.

The value of this project is the complete deployment flow:

```text
pre-trained model
↓
TensorFlow Lite
↓
X-CUBE-AI
↓
generated STM32 code
↓
real sensor input
↓
preprocessing
↓
inference
↓
application output
```

---

## 14. Official References

### STM32CubeMX

https://www.st.com/stm32cubemx

### STM32CubeIDE

https://www.st.com/stm32cubeide

### X-CUBE-AI

https://www.st.com/en/embedded-software/x-cube-ai.html

Getting started with X-CUBE-AI:

https://www.st.com/resource/en/user_manual/dm00570145.pdf

Embedded Inference Client API:

https://stm32ai-cs.st.com/assets/embedded-docs/embedded_client_api_legacy.html

### STM32F401

https://www.st.com/en/microcontrollers-microprocessors/stm32f401rb.html

Datasheet:

https://www.st.com/resource/en/datasheet/stm32f401vb.pdf

### LM35

https://www.ti.com/product/LM35

Datasheet:

https://www.ti.com/lit/ds/symlink/lm35.pdf

---

## 15. About

This repository is maintained by **Raayan Systems** as part of ongoing work in:

- Embedded Systems
- Microcontrollers
- Firmware Development
- Embedded AI

The goal is to provide practical and reproducible Embedded-AI deployment examples on real microcontroller hardware.
