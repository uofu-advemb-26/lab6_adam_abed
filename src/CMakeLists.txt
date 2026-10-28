# This is the main binary. List your C files here.
add_executable(hello_freertos
    hello_freertos.c
    )

pico_set_program_name(hello_freertos "test")
pico_set_program_version(hello_freertos "0.1")

# Modify the below lines to enable/disable output over UART/USB.
# These are mutually exclusive, only set one.
pico_enable_stdio_uart(hello_freertos 1)
pico_enable_stdio_usb(hello_freertos 0)

# Add the libraries to the build
target_link_libraries(hello_freertos PRIVATE
  pico_stdlib
  pico_multicore
  pico_async_context_freertos
  FreeRTOS-Kernel-Heap4
)

# The CYW43 is the Wifi/Bluetooth module. If the board is set to pico_w, this
# variable will be set and the wireless libraries added.
if(PICO_CYW43_SUPPORTED)
    # For led support on pico_w
    target_link_libraries(hello_freertos PRIVATE
        pico_cyw43_arch_none
    )
endif()

# Another pico macro which generates the uf2 file in addition to the elf
pico_add_extra_outputs(hello_freertos)

find_program(OPENOCD openocd)
find_program(PICOTOOL picotool)

add_custom_target(flash ${PICOTOOL} load -f $<TARGET_FILE:hello_freertos> DEPENDS hello_freertos)
add_custom_target(openocd_flash ${OPENOCD} -s ${OPENOCD_PATH} -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c 'adapter speed 5000' -c program "$<TARGET_FILE:hello_freertos>" verify reset exit' DEPENDS hello_freertos)



# Lab 6 - Activity 0: Priority Inversion
add_executable(priority_inversion
    priority_inversion.c
)

pico_set_program_name(priority_inversion "priority_inversion")
pico_set_program_version(priority_inversion "0.1")

pico_enable_stdio_uart(priority_inversion 0)
pico_enable_stdio_usb(priority_inversion 1)

target_link_libraries(priority_inversion PRIVATE
    pico_stdlib
    FreeRTOS-Kernel-Heap4
)

pico_add_extra_outputs(priority_inversion)



# Lab 6 - Activity 2: FreeRTOS Task Scheduling

add_executable(activity2
    activity2.c
)

pico_set_program_name(activity2 "activity2")
pico_set_program_version(activity2 "0.1")

pico_enable_stdio_uart(activity2 0)
pico_enable_stdio_usb(activity2 1)

target_link_libraries(activity2 PRIVATE
    pico_stdlib
    FreeRTOS-Kernel-Heap4
)

pico_add_extra_outputs(activity2)
