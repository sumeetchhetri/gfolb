################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../BufferedReader.cpp \
../ConnectionPool.cpp \
../GodFather.cpp \
../IConnectionHandler.cpp 

OBJS += \
./BufferedReader.o \
./ConnectionPool.o \
./GodFather.o \
./IConnectionHandler.o 

CPP_DEPS += \
./BufferedReader.d \
./ConnectionPool.d \
./GodFather.d \
./IConnectionHandler.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


