################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../BufferedReader.cpp \
../ClientInterface.cpp \
../Client.cpp \
../SSLClient.cpp \
../ConnectionPool.cpp \
../GodFather.cpp \
../IConnectionHandler.cpp \
../Server.cpp \
../PropFileReader.cpp 

OBJS += \
./BufferedReader.o \
./ClientInterface.o \
./Client.o \
./SSLClient.o \
./ConnectionPool.o \
./GodFather.o \
./IConnectionHandler.o \
./Server.o \
./PropFileReader.o 

CPP_DEPS += \
./BufferedReader.d \
./ClientInterface.d \
./Client.d \
./SSLClient.d \
./ConnectionPool.d \
./GodFather.d \
./IConnectionHandler.d \
./Server.d \
./PropFileReader.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '
