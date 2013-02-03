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
../PropFileReader.cpp \
../Date.cpp \
../DateFormat.cpp \
../Logger.cpp \
../Mutex.cpp \
../PoolThread.cpp \
../StringUtil.cpp \
../Task.cpp \
../TaskPool.cpp \
../Thread.cpp \
../ThreadPool.cpp \
../SelEpolKqEvPrt.cpp

OBJS += \
./BufferedReader.o \
./ClientInterface.o \
./Client.o \
./SSLClient.o \
./ConnectionPool.o \
./GodFather.o \
./IConnectionHandler.o \
./Server.o \
./PropFileReader.o \
./Date.o \
./DateFormat.o \
./Logger.o \
./Mutex.o \
./PoolThread.o \
./StringUtil.o \
./Task.o \
./TaskPool.o \
./Thread.o \
./ThreadPool.o \
./SelEpolKqEvPrt.o

CPP_DEPS += \
./BufferedReader.d \
./ClientInterface.d \
./Client.d \
./SSLClient.d \
./ConnectionPool.d \
./GodFather.d \
./IConnectionHandler.d \
./Server.d \
./PropFileReader.d \
./Date.d \
./DateFormat.d \
./Logger.d \
./Mutex.d \
./PoolThread.d \
./StringUtil.d \
./Task.d \
./TaskPool.d \
./Thread.d \
./ThreadPool.d \
./SelEpolKqEvPrt.d

# Each subdirectory must supply rules for building sources it contributes
%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -Wall -I../include -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '
	