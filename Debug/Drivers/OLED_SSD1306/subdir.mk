################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/OLED_SSD1306/ssd1306.c \
../Drivers/OLED_SSD1306/ssd1306_fonts.c 

OBJS += \
./Drivers/OLED_SSD1306/ssd1306.o \
./Drivers/OLED_SSD1306/ssd1306_fonts.o 

C_DEPS += \
./Drivers/OLED_SSD1306/ssd1306.d \
./Drivers/OLED_SSD1306/ssd1306_fonts.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/OLED_SSD1306/%.o Drivers/OLED_SSD1306/%.su Drivers/OLED_SSD1306/%.cyclo: ../Drivers/OLED_SSD1306/%.c Drivers/OLED_SSD1306/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I"D:/workspace/stm32/musical alarm clock/Drivers/OLED_SSD1306" -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-OLED_SSD1306

clean-Drivers-2f-OLED_SSD1306:
	-$(RM) ./Drivers/OLED_SSD1306/ssd1306.cyclo ./Drivers/OLED_SSD1306/ssd1306.d ./Drivers/OLED_SSD1306/ssd1306.o ./Drivers/OLED_SSD1306/ssd1306.su ./Drivers/OLED_SSD1306/ssd1306_fonts.cyclo ./Drivers/OLED_SSD1306/ssd1306_fonts.d ./Drivers/OLED_SSD1306/ssd1306_fonts.o ./Drivers/OLED_SSD1306/ssd1306_fonts.su

.PHONY: clean-Drivers-2f-OLED_SSD1306

