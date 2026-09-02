foreach file : [
  # common
    'common/manager.c',
    'common/virtio.c',

  # framebuffer
    'framebuffer/framebuffer.c',

  # irqchip
    'irqchip/arm-gicv2.c',

  # timer
    'timer/arm-timer.c',

  # uart
    'uart/pl011.c',
]
    kernel_c_sources += files(project_root / 'drivers' / file)
endforeach
